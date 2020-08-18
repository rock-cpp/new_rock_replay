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
    struct SampleMetadata 
    {
        std::string portName;
        std::string timeStamp;
        std::function<bool()> replayCallback;
    };
  
public:
    LogTaskManager();
    ~LogTaskManager() = default;
    
    void init(const std::vector<std::string>& fileNames);
    bool setIndex(size_t index);
    bool replaySample();
    const std::map<std::string, LogTask>& getAllLogTasks();
    size_t getNumSamples();
    const SampleMetadata& getMetaData() const;
    
private:
    std::string getTaskName(pocolog_cpp::InputDataStream* stream);
    void loadTypekits(const std::set<std::string>& modelsToLoad);
    void createLogTasks();
    
    pocolog_cpp::MultiFileIndex multiFileIndex;
    std::map<std::string, LogTask> name2Task;
    std::map<size_t, std::string> globalIndex2TaskName;
    orocos_cpp::OrocosCpp orocos;
    SampleMetadata sampleMetadata;
};
