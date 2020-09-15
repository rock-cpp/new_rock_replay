#pragma once

#include <vector>
#include <string>
#include <memory>
#include <map>

#include <orocos_cpp/orocos_cpp.hpp>
#include <pocolog_cpp/MultiFileIndex.hpp>

#include "LogTask.hpp"

class LogTaskManager
{  
public:
    struct SampleMetadata 
    {
        std::string portName;
        base::Time timeStamp;
        bool valid;
    };
    
    using PortInfo = std::pair<std::string, std::string>;
    using PortCollection = std::vector<PortInfo>;
    using TaskCollection = std::map<std::string, LogTask::PortCollection>;
    
    LogTaskManager();
    ~LogTaskManager() = default;
    
    void init(const std::vector<std::string>& fileNames, const std::string& prefix);
    SampleMetadata setIndex(size_t index);
    bool replaySample();
    void activateReplayForPort(const std::string& taskName, const std::string& portName, bool on);
    TaskCollection getTaskCollection();
    size_t getNumSamples();
    
private:
    void createLogTasks();
    LogTask& findOrCreateLogTask(const std::string& streamName);
    
    std::string prefix;
    pocolog_cpp::MultiFileIndex multiFileIndex;
    orocos_cpp::OrocosCpp orocos;
    std::function<bool()> replayCallback;
    std::map<std::string, std::shared_ptr<LogTask>> streamName2LogTask;
};
