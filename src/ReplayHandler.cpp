#include "ReplayHandler.hpp"
#include <boost/algorithm/string.hpp>
#include <orocos_cpp/TypeRegistry.hpp>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <rtt/transports/corba/CorbaDispatcher.hpp>
#include <rtt/typelib/TypelibMarshallerBase.hpp>
#include <typelib/typedisplay.hh>
#include <orocos_cpp/orocos_cpp.hpp>
#include "FileLoader.hpp"


void ReplayHandler::loadStreams(int argc, char** argv)
{
    auto f = std::vector<std::regex>();
    auto g = std::map<std::string, std::string>();
    manager.init(FileLoader::parseFileNames(argc, argv, f, g));
    init();
}


ReplayHandler::~ReplayHandler()
{       
    playing = false;
    running = false;
//     cond.notify_all();
    replayThread.join();
}


void ReplayHandler::init()
{
    replayFactor = 1.;
    currentSpeed = replayFactor;
    curIndex = 0;
    finished = false;
    playing = false;
    maxIndex = manager.getNumSamples() - 1;
    manager.setIndex(curIndex);       
    replayThread = std::thread(std::bind(&ReplayHandler::replaySamples, this));
}


bool ReplayHandler::replaySample(size_t index, bool dryRun)
{
   return manager.setIndex(index);
}

std::vector<std::pair<std::string, std::vector<std::string>>> ReplayHandler::getTaskNamesWithPorts()
{
    std::vector<std::pair<std::string, std::vector<std::string>>> taskNamesWithPorts;
    for(const auto& name2Task : manager.getAllLogTasks())
    {
        auto foo = name2Task.second.getTaskContext()->ports()->getPortNames();
        taskNamesWithPorts.emplace_back(name2Task.first, foo);
    }
        
    return taskNamesWithPorts;
}

bool ReplayHandler::checkSampleIdx()
{
    if(curIndex > maxIndex)
    {
        finished = true;
        playing = false;
        varMut.unlock();
        return true;
    }
    return false;
}


void ReplayHandler::replaySamples()
{   
    boost::unique_lock<boost::mutex> lock(mut);
    running = true;
    restartReplay = true;
    
    base::Time curStamp;
    base::Time toSleep;

    base::Time systemPlayStartTime;
    base::Time logPlayStartTime;
//     curStamp = getTimeStamp(curIndex);
    
    while(running)
    {
        while(playing && curIndex < maxIndex)
        {
                manager.replaySample();
                manager.setIndex(curIndex++);
                const auto& metadata = manager.getMetaData();
                curSamplePortName = metadata.portName;
                curTimeStamp = metadata.timeStamp;
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                std::cout << "replaying sample " << curIndex << std::endl;
        }
    }
    
//     while(running)
//     {
//         while(!playing)
//         {
//             cond.wait(lock);
//             restartReplay = true;
//         }
//         
//         varMut.lock();
//         
//         if(checkSampleIdx())
//             continue;
//         
//         
//         if(restartReplay)
//         {
//             systemPlayStartTime = base::Time::now();
//             
//             //expensive call
// //             curStamp = getTimeStamp(curIndex);
//             logPlayStartTime = curStamp;
//             restartReplay = false;
//         }
//         
//         //TODO check if chronological ordering is right
//         //TODO if logging for port is not selected, skip cur index
//         // (allow higher replayFactor)
//         if (!replaySample(curIndex++))
//         {
//             varMut.unlock();
//             continue;
//         }
//         
//         if(checkSampleIdx())
//             continue;
//         
//         manager.setIndex(curIndex);
// //         curStamp = getTimeStamp(curIndex);    
// 
//         //hm, also expensive, is there a way to reduce this ?
//         base::Time curTime = base::Time::now();
//         
//         //hm, factor... should it not be * replayFactor then ?
//         int64_t tSinceStart = curStamp.microseconds - logPlayStartTime.microseconds;
//         tSinceStart = std::max<int64_t>(0, tSinceStart);
//         base::Time logTimeSinceStart = base::Time::fromMicroseconds(tSinceStart / replayFactor);
//         base::Time systemTimeSinceStart = (curTime - systemPlayStartTime);
//         toSleep = logTimeSinceStart - systemTimeSinceStart;
// 
//         varMut.unlock();
//         
//         if(!playing)
//             continue;
//         
//         if(toSleep.microseconds > 0)
//         {
//             factorChangeCond.timed_wait(lock, boost::posix_time::microseconds(toSleep.microseconds));
//             currentSpeed = replayFactor;
//         }
//         else if(toSleep.microseconds == 0)
//         {
//             currentSpeed = replayFactor;
//         }
//         else // tosleep < 0
//         {
//             currentSpeed = logTimeSinceStart.toSeconds() / systemTimeSinceStart.toSeconds();   
//         }      
//     }

}

void ReplayHandler::stop()
{
    varMut.lock();
    curIndex = 0;
    finished = false;
    playing = false;
    restartReplay = true;
    replaySample(curIndex, true);
    varMut.unlock();
}


void ReplayHandler::setReplayFactor(double factor)
{
    varMut.lock();
    this->replayFactor = factor;
    if (this->replayFactor < minReplayFactor)
    {
        this->replayFactor = minReplayFactor;
    }
    restartReplay = true;
    factorChangeCond.notify_one();
    varMut.unlock();
}


void ReplayHandler::next()
{
    varMut.lock();
    if(curIndex < maxIndex)
    {
        replaySample(++curIndex);
    }
    varMut.unlock();
    
}

void ReplayHandler::previous()
{
    varMut.lock();
    if(curIndex > 0)
    {
        replaySample(--curIndex);
    }
    varMut.unlock();
}


void ReplayHandler::setSampleIndex(uint index)
{
    varMut.lock();
    curIndex = index;
    replaySample(curIndex, true); // do a dry run for metadata update
    varMut.unlock();
}

void ReplayHandler::setMaxSampleIndex(uint index)
{
    varMut.lock();
    maxIndex = index;
    varMut.unlock();
}

void ReplayHandler::setSpan(uint minIdx, uint maxIdx)
{
    varMut.lock();
    curIndex = minIdx;
    maxIndex = maxIdx;
    varMut.unlock();
}


void ReplayHandler::play()
{
    varMut.lock();
    if(valid)
    {
        playing = true;
        restartReplay = true;
        cond.notify_one();
    }
    varMut.unlock();
}

void ReplayHandler::pause()
{
    varMut.lock();
    playing = false;
    varMut.unlock();
}
