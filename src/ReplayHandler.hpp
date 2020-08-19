#pragma once

#include <iostream>
#include <pocolog_cpp/MultiFileIndex.hpp>
#include <base/Time.hpp>
#include <orocos_cpp/PluginHelper.hpp>
#include <rtt/transports/corba/TaskContextServer.hpp>
#include <boost/thread.hpp>
#include <boost/thread/condition_variable.hpp>
#include <regex>
#include <set>
#include <thread>
#include <memory>

#include "LogTask.hpp"
#include "LogTaskManager.hpp"


class ReplayHandler
{
    
public:
    ReplayHandler() = default;
    ~ReplayHandler();
    
    void stop();
    void play();
    void pause();
    
    void next();
    void previous();
    void setSampleIndex(size_t index);
    
    void setReplayFactor(double factor);
    void setMaxSampleIndex(uint index);
    void setSpan(uint minIdx, uint maxIdx);
    
    void loadStreams(int argc, char** argv);
    
    std::vector<std::pair<std::string, std::vector<std::string>>> getTaskNamesWithPorts();
    
    inline const std::string getCurTimeStamp() { return curMetadata.timeStamp.toString(); };
    inline const std::string getCurSamplePortName() { return curMetadata.portName; };
    inline const uint getCurIndex() { return curIndex; };
    inline const size_t getMaxIndex() { return maxIndex; };
    inline const double getReplayFactor() { return replayFactor; };
    inline const double getCurrentSpeed() { return currentSpeed; };
    inline const bool hasFinished() { return finished; };
    inline const bool isPlaying() { return playing; };
    inline void restart() { restartReplay = true; };
    
private:  
    bool restartReplay;
    double replayFactor;
    const double minReplayFactor = 1e-5;
    mutable double currentSpeed;
    LogTaskManager::SampleMetadata curMetadata;
    uint curIndex;
    uint maxIndex;
    bool finished;
    bool running;
    std::thread replayThread;
    
    bool playing;
            
    void init();
    void replaySamples();
    
    LogTaskManager manager;
};
