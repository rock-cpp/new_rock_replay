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
    std::set<std::string> modelsToLoad;
    multiFileIndex.registerStreamCheck([&](pocolog_cpp::Stream *st){
        std::cout << "Checking " << st->getName() << std::endl;
        pocolog_cpp::InputDataStream *dataStream = dynamic_cast<pocolog_cpp::InputDataStream *>(st);
        
        if(!dataStream)
        {
            return false;
        }
        
        modelsToLoad.emplace(getTaskName(dataStream));
        return true;
    });

    multiFileIndex.createIndex(fileNames);
    loadTypekits(modelsToLoad);
    createLogTasks();
}

void LogTaskManager::deinit()
{
    globalIndex2TaskName.clear();
    name2Task.clear();
}


LogTaskManager::SampleMetadata LogTaskManager::setIndex(size_t index)
{
    try
    {
        pocolog_cpp::InputDataStream *inputStream = dynamic_cast<pocolog_cpp::InputDataStream*>(multiFileIndex.getSampleStream(index));
        size_t globalIndex = multiFileIndex.getGlobalStreamIdx(index);        
        replayCallback = [=](){
            return name2Task.at(globalIndex2TaskName.at(globalIndex)).replaySample(*inputStream, multiFileIndex.getPosInStream(index));
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
    return replayCallback();
}

const std::map<std::string, LogTask> & LogTaskManager::getAllLogTasks()
{
    return name2Task;
}

size_t LogTaskManager::getNumSamples()
{
    return multiFileIndex.getSize();
}


std::string LogTaskManager::getTaskName(pocolog_cpp::InputDataStream* stream)
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

void LogTaskManager::loadTypekits(const std::set<std::string>& modelsToLoad)
{
    for(const auto& modelName : modelsToLoad)
    {
        if(!orocos.loadAllTypekitsForModel(modelName))
        {
            std::cerr << "Could not load typekits for model " << modelName << std::endl;
        }
    }
}

void LogTaskManager::createLogTasks()
{
    for(pocolog_cpp::Stream *st : multiFileIndex.getAllStreams())
    {
        pocolog_cpp::InputDataStream *inputSt = dynamic_cast<pocolog_cpp::InputDataStream*>(st);
        if(inputSt)
        {
            std::string taskName = getTaskName(inputSt);

            auto taskIt = name2Task.find(taskName);
            if(taskIt == name2Task.end())
            {
                taskIt = name2Task.emplace(taskName, taskName).first;
            }

            taskIt->second.addStream(*inputSt);
            globalIndex2TaskName.emplace(multiFileIndex.getGlobalStreamIdx(st), taskName);
        }
    }
}

void LogTaskManager::activateReplayForPort(const std::string& taskName, const std::string& portName, bool on)
{
    const auto taskIt = name2Task.find(taskName);
    if(taskIt != name2Task.end())
    {
        taskIt->second.activateLoggingForPort(portName, on);
    }
}

