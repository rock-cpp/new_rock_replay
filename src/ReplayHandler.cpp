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
    setSampleIndex(curIndex);
    replayThread = std::thread(std::bind(&ReplayHandler::replaySamples, this));
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


void ReplayHandler::replaySamples()
{      
    running = true;
    base::Time playingTime;
    
    while(running)
    {
        while(playing && curIndex < maxIndex)
        {
            playingTime = base::Time::now();
            manager.replaySample();
            
            const base::Time previousTime = curMetadata.timeStamp;
            setSampleIndex(curIndex++);
            
            base::Time timeToSleep = curMetadata.timeStamp - previousTime;
            int64_t sleepDuration = timeToSleep.toMilliseconds() * 1 / replayFactor;
            std::this_thread::sleep_for(std::chrono::milliseconds(sleepDuration));
            currentSpeed = 0.5;
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
    curIndex = 0;
    finished = false;
    playing = false;
    restartReplay = true;
    setSampleIndex(curIndex);
}


void ReplayHandler::setReplayFactor(double factor)
{
    this->replayFactor = factor;
    if (this->replayFactor < minReplayFactor)
    {
        this->replayFactor = minReplayFactor;
    }
    restartReplay = true;
}


void ReplayHandler::next()
{
    if(curIndex < maxIndex)
    {
        setSampleIndex(++curIndex);
    }
    
}

void ReplayHandler::previous()
{
    if(curIndex > 0)
    {
        setSampleIndex(--curIndex);;
    }
}


void ReplayHandler::setSampleIndex(size_t index)
{
    curMetadata = manager.setIndex(index);
}

void ReplayHandler::setMaxSampleIndex(uint index)
{
    maxIndex = index;
}

void ReplayHandler::setSpan(uint minIdx, uint maxIdx)
{
    curIndex = minIdx;
    maxIndex = maxIdx;
}


void ReplayHandler::play()
{
    playing = true;
}

void ReplayHandler::pause()
{
    playing = false;
}

