#include "LogTaskManager.hpp"

#include <orocos_cpp/orocos_cpp.hpp>

LogTaskManager::LogTaskManager()
{
    orocos_cpp::OrocosCppConfig config;
    orocos_cpp::OrocosCpp orocos;
    orocos.initialize(config);
}


void LogTaskManager::init(const std::vector<std::string>& fileNames)
{
    multiFileIndex.registerStreamCheck([&](pocolog_cpp::Stream *st){
        std::cout << "Checking " << st->getName() << std::endl;
        return dynamic_cast<pocolog_cpp::InputDataStream *>(st);
    });

    multiFileIndex.createIndex(fileNames);
    createLogTasks();
}

void LogTaskManager::deinit()
{
    multiFileIndex = pocolog_cpp::MultiFileIndex();
}


LogTaskManager::SampleMetadata LogTaskManager::setIndex(size_t index)
{
    try
    {
        pocolog_cpp::InputDataStream *inputStream = dynamic_cast<pocolog_cpp::InputDataStream*>(multiFileIndex.getSampleStream(index));
        replayCallback = [=](){
            return streamName2LogTask.at(inputStream->getName())->replaySample(multiFileIndex.getGlobalStreamIdx(inputStream), multiFileIndex.getPosInStream(index));
        };
        
        return {inputStream->getName(), inputStream->getFileIndex().getSampleTime(multiFileIndex.getPosInStream(index)), true};
    }
    catch(...)
    {
        
    }
    
    return {"", base::Time::fromString(""), false};
}


bool LogTaskManager::replaySample()
{
    try 
    {
        replayCallback();
    }
    catch(std::runtime_error& e)
    {
        return false;
    }
    
    return true;
}

// const std::map<std::string, LogTask> & LogTaskManager::getAllLogTasks()
// {
//     return {};
// //     return name2Task;
// }

size_t LogTaskManager::getNumSamples()
{
    return multiFileIndex.getSize();
}

void LogTaskManager::createLogTasks()
{    
    for(pocolog_cpp::Stream *st : multiFileIndex.getAllStreams())
    {
        pocolog_cpp::InputDataStream* inputSt = dynamic_cast<pocolog_cpp::InputDataStream*>(st);
        if(inputSt)
        {
            LogTask& logTask = findOrCreateLogTask(inputSt->getName());          
            logTask.addStream(*inputSt);
        }
    }
}

LogTask& LogTaskManager::findOrCreateLogTask(const std::string& streamName)
{
    const auto taskNameFunc = [](const std::string& name) {
        std::string taskName = name;        
        
        // filter all '/'
        size_t pos = taskName.find('/');
        if(taskName.size() && pos != std::string::npos)
        {
            taskName = taskName.substr(pos + 1, taskName.size());
        }
        
        // remove port name
        return taskName.substr(0, taskName.find_last_of('.'));
    };
    
    std::shared_ptr<LogTask> logTask;
    for(const auto& name2LogTask : streamName2LogTask)
    {
        if(taskNameFunc(name2LogTask.first) == taskNameFunc(streamName))
        { 
            logTask = name2LogTask.second;
        }
    }
   
    if(!logTask)
    {
        logTask = std::make_shared<LogTask>(taskNameFunc(streamName), orocos);
    }
    
    streamName2LogTask.emplace(streamName, logTask);
   
    return *logTask;
}


void LogTaskManager::activateReplayForPort(const std::string& taskName, const std::string& portName, bool on)
{
//     const auto taskIt = name2Task.find(taskName);
//     if(taskIt != name2Task.end())
//     {
//         taskIt->second.activateLoggingForPort(portName, on);
//     }
}

