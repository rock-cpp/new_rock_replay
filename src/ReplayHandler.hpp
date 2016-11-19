#pragma once

#include <iostream>
#include <pocolog_cpp/MultiFileIndex.hpp>
#include <base/Time.hpp>
#include <orocos_cpp/PluginHelper.hpp>
#include <rtt/transports/corba/TaskContextServer.hpp>
#include <boost/thread.hpp>
#include <boost/thread/condition_variable.hpp>

#include "LogTask.hpp"

struct ReplayGraph
{
    std::vector<double> xData;
    std::vector<double> yData;
};

class ReplayHandler
{
    
public:
    ReplayHandler(int argc, char** argv, uint windowSize = 20);
    ~ReplayHandler();
    
    void replaySamples();
    
    void toggle();
    void stop();
    
    void next();
    void previous();
    void setSampleIndex(uint index);
    
    void setReplayFactor(double factor);
       
    const ReplayGraph getGraph() const;
    
    
    inline const std::map<std::string, LogTask*>& getAllLogTasks() { return logTasks; };
    inline const std::string getCurTimeStamp() { return curTimeStamp; };
    inline const std::string getCurSamplePortName() { return curSamplePortName; };
    inline const uint getCurIndex() { return curIndex; };
    inline const size_t getMaxIndex() { return multiIndex->getSize(); };
    inline const double getReplayFactor() { return replayFactor; };
    inline const double getCurrentSpeed() { return currentSpeed; };
    inline const std::vector<std::string> getFileNames() { return filenames; };
    inline const bool isValid() { return valid; };
    inline const bool hasFinished() { return finished; };
    
    
private:  
    bool restartReplay;
    double replayFactor;
    mutable double currentSpeed;
    mutable std::string curTimeStamp;
    mutable std::string curSamplePortName;
    uint curIndex;
    uint maxIndex;
    bool finished;
    bool valid;
    uint windowSize;
    
    bool play;
    boost::thread *replayThread;
    boost::condition_variable cond;
    boost::mutex mut;
    
    std::vector<std::string> filenames;
    std::map<std::string, LogTask *> logTasks;
    std::vector<LogTask *> streamToTask;
    pocolog_cpp::MultiFileIndex *multiIndex;
    
    void replaySample(size_t index) const;
    const base::Time getTimeStamp(size_t globalIndex) const;
    void init();
};