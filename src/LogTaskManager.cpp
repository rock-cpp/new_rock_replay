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
    multiFileIndex = pocolog_cpp::MultiFileIndex();
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
            return name2Task.at(globalIndex2TaskName.at(globalIndex).taskName).replaySample(*inputStream, multiFileIndex.getPosInStream(index));
        };
        
        return {inputStream->getName(), inputStream->getFileIndex().getSampleTime(multiFileIndex.getPosInStream(index)), globalIndex2TaskName.at(globalIndex).canReplay};
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
        try 
        {
            if(orocos.loadAllTypekitsForModel(modelName))
            {
                std::cerr << "No need to load typekits for model " << modelName << std::endl;
            }
        }
        catch(std::runtime_error& e)
        {
            std::cerr << e.what() << std::endl;
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
            bool canReplay = addStreamToTask(taskIt, taskName, *inputSt);
            globalIndex2TaskName.emplace(multiFileIndex.getGlobalStreamIdx(st), GlobalIndexInfo{taskName, canReplay});
        }
    }
}

bool LogTaskManager::addStreamToTask(std::map<std::string, LogTask>::iterator& taskIt, const std::string& taskName, const pocolog_cpp::InputDataStream& inputStream)
{
    try
    {
        if(taskIt == name2Task.end())
        {
            taskIt = name2Task.emplace(taskName, taskName).first;
        }
        taskIt->second.addStream(inputStream);
    }
    catch(...)
    {
        std::cerr << "Replaying for " << inputStream.getName() << " is disabled, as no typekits are found." << std::endl;
        return false;
    }
    
    return true;
}


void LogTaskManager::activateReplayForPort(const std::string& taskName, const std::string& portName, bool on)
{
    const auto taskIt = name2Task.find(taskName);
    if(taskIt != name2Task.end())
    {
        taskIt->second.activateLoggingForPort(portName, on);
    }
}

