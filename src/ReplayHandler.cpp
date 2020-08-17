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
    orocos_cpp::OrocosCppConfig config;
    orocos_cpp::OrocosCpp orocos;
    orocos.initialize(config);

    std::vector<std::regex> regExps;
    std::map<std::string, std::string> logfiles2Prefix;
    std::vector<std::string> fileNames = FileLoader::parseFileNames(argc, argv, regExps, logfiles2Prefix);
    std::set<std::string> modelsToLoad;
    
    multiIndex.registerStreamCheck([&](pocolog_cpp::Stream *st){
        
        std::cout << "Checking " << st->getName() << std::endl;
        pocolog_cpp::InputDataStream *dataStream = dynamic_cast<pocolog_cpp::InputDataStream *>(st);
        if(!dataStream)
        {
            return false;
        }
        
        modelsToLoad.emplace(getTaskName(dataStream));
        
        return true;
        
    });

    multiIndex.createIndex(fileNames);
    streamToTask.resize(multiIndex.getAllStreams().size());
    for(const auto& modelName : modelsToLoad)
    {
        if(!orocos.loadAllTypekitsForModel(modelName))
        {
            std::cerr << "Could not load typekits for model " << modelName << std::endl;
        }
    }
    
    for(pocolog_cpp::Stream *st : multiIndex.getAllStreams())
    {
        pocolog_cpp::InputDataStream *inputSt = dynamic_cast<pocolog_cpp::InputDataStream *>(st);
        if(!inputSt)
            continue;       
        
        std::string taskName = getTaskName(inputSt);

        auto taskIt = logTasks.find(taskName);
        if(taskIt == logTasks.end())
        {
            taskIt = logTasks.emplace(taskName, new LogTask(taskName)).first;
        }

        taskIt->second->addStream(*inputSt);

        size_t gIdx = multiIndex.getGlobalStreamIdx(st); 
        if(gIdx > streamToTask.size())
            throw std::runtime_error("Mixup detected");

//         streamToTask[gIdx] = task;
    }

    valid = !multiIndex.getAllStreams().empty() && !fileNames.empty() && multiIndex.getSize() > 0;
    if(!valid)
    {
        std::cerr << "empty streams loaded" << std::endl;
        exit(0);
    }
    else
    {
        init();
    }

}

std::string ReplayHandler::getTaskName(pocolog_cpp::InputDataStream* stream)
{
    std::string taskName = stream->getName();
        
    // filter all '/'
    size_t pos = taskName.find('/');
    if(taskName.size() && pos != std::string::npos)
    {
        taskName = taskName.substr(pos + 1, taskName.size());
    }
    
    // remove port name
    taskName = taskName.substr(0, taskName.find_last_of('.'));
    
    return taskName;
}

ReplayHandler::~ReplayHandler()
{       
    RTT::corba::CorbaDispatcher::ReleaseAll();
    RTT::corba::TaskContextServer::CleanupServers();
//     playing = false;
//     running = false;
//     cond.notify_all();
    replayThread.join();
    
    //delete multiIndex;    
}


void ReplayHandler::init()
{
    replayFactor = 1.;
    currentSpeed = replayFactor;
    curIndex = 0;
    finished = false;
    playing = false;
    maxIndex = multiIndex.getSize() - 1;
    replaySample(curIndex, true);       
    replayThread = std::thread(std::bind(&ReplayHandler::replaySamples, this));
}

const base::Time ReplayHandler::getMinIdxTimeStamp()
{
    return getTimeStamp(curIndex);
}

const base::Time ReplayHandler::getMaxIdxTimeStamp()
{
    return getTimeStamp(maxIndex);
}



bool ReplayHandler::replaySample(size_t index, bool dryRun)
{
    try 
    {
        size_t globalStreamIndex = multiIndex.getGlobalStreamIdx(index);
        pocolog_cpp::InputDataStream *inputSt = dynamic_cast<pocolog_cpp::InputDataStream *>(multiIndex.getSampleStream(index));
        if (dryRun || (!dryRun && streamToTask[globalStreamIndex]->replaySample(*inputSt, multiIndex.getPosInStream(index))))
        {
            curSamplePortName = inputSt->getName();
            curTimeStamp = getTimeStamp(index).toString();
        }
        
        return true;
    }
    catch(...)
    {
        std::cout << "Warning: ignoring corrupt sample: " << index << "/" << maxIndex << std::endl;     
    }
    
    return false;
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
//     boost::unique_lock<boost::mutex> lock(mut);
//     running = true;
//     restartReplay = true;
//     
//     base::Time curStamp;
//     base::Time toSleep;
// 
//     base::Time systemPlayStartTime;
//     base::Time logPlayStartTime;
//     curStamp = getTimeStamp(curIndex);
    
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
//             curStamp = getTimeStamp(curIndex);
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
//         replaySample(curIndex, true);
//         curStamp = getTimeStamp(curIndex);    
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

const base::Time ReplayHandler::getTimeStamp(size_t globalIndex)
{    
    pocolog_cpp::Index &idx = multiIndex.getSampleStream(globalIndex)->getFileIndex();
    return idx.getSampleTime(multiIndex.getPosInStream(globalIndex));
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
