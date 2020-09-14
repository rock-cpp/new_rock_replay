#include "LogTaskManager.hpp"

#include "LogFileHelper.hpp"
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
    streamName2LogTask.clear();
}


LogTaskManager::SampleMetadata LogTaskManager::setIndex(size_t index)
{
    try
    {
        pocolog_cpp::InputDataStream *inputStream = dynamic_cast<pocolog_cpp::InputDataStream*>(multiFileIndex.getSampleStream(index));
        replayCallback = [=](){
            streamName2LogTask.at(inputStream->getName())->activateLoggingForPort("status", false);
            return streamName2LogTask.at(inputStream->getName())->replaySample(inputStream->getIndex(), multiFileIndex.getPosInStream(index));
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
//         return replayCallback(); //TODO: give replay feedback and reset und stop
        replayCallback();
    }
    catch(std::runtime_error& e)
    {
        return false;
    }
    
    return true;
}

std::map<std::string, std::vector<std::pair<std::string, std::string>>> LogTaskManager::getTaskNamesWithPorts()
{
    std::map<std::string, std::vector<std::pair<std::string, std::string>>> taskNames2Ports;
    for(const auto& streamNameIt : streamName2LogTask)
    {
        const std::string& streamName = streamNameIt.first;
        const auto taskAndPortName = LogFileHelper::splitStreamName(streamName);
        
        auto taskNameIt = taskNames2Ports.find(taskAndPortName.first);
        if(taskNameIt == taskNames2Ports.end())
        {
            taskNameIt = taskNames2Ports.emplace(taskAndPortName.first, std::vector<std::pair<std::string, std::string>>()).first;
        }
        
        std::string portType = "";
        for(const auto stream : multiFileIndex.getAllStreams())
        {
            if(stream->getName() == streamName)
            {
                auto inputSt = dynamic_cast<pocolog_cpp::InputDataStream*>(stream);
                portType = inputSt->getCXXType();
            }
        }
        
        taskNameIt->second.push_back({taskAndPortName.second, portType});
    }
    
    return taskNames2Ports;
}

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
    std::shared_ptr<LogTask> logTask;
    const auto taskNameAndPort = LogFileHelper::splitStreamName(streamName);
    
    for(const auto& name2LogTask : streamName2LogTask)
    {
        if(LogFileHelper::splitStreamName(name2LogTask.first).first == taskNameAndPort.first)
        { 
            logTask = name2LogTask.second;
        }
    }
   
    if(!logTask)
    {
        logTask = std::make_shared<LogTask>(taskNameAndPort.first, orocos);
    }
    
    streamName2LogTask.emplace(streamName, logTask);
   
    return *logTask;
}


void LogTaskManager::activateReplayForPort(const std::string& taskName, const std::string& portName, bool on)
{
    LogTask& logTask = findOrCreateLogTask(taskName);
    logTask.activateLoggingForPort(portName, on);
}

