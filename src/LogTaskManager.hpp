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
    
    LogTaskManager();
    ~LogTaskManager() = default;
    
    void init(const std::vector<std::string>& fileNames);
    void deinit();
    SampleMetadata setIndex(size_t index);
    bool replaySample();
    void activateReplayForPort(const std::string& taskName, const std::string& portName, bool on);
    std::map<std::string, std::vector<std::pair<std::string, std::string>>> getTaskNamesWithPorts();
    size_t getNumSamples();
    
private:
    void createLogTasks();
    LogTask& findOrCreateLogTask(const std::string& streamName);
    
    
    pocolog_cpp::MultiFileIndex multiFileIndex;
    orocos_cpp::OrocosCpp orocos;
    std::function<bool()> replayCallback;
    std::map<std::string, std::shared_ptr<LogTask>> streamName2LogTask;
};
