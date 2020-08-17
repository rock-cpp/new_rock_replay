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

#include "LogTask.hpp"


class ReplayHandler
{
    
public:
    ReplayHandler() = default;
    ~ReplayHandler();
   
    enum MATCH_MODE
    {
        REGEX = 0,
        WHITELIST
    };
    
    void stop();
    void play();
    void pause();
    
    void next();
    void previous();
    void setSampleIndex(uint index);
    
    void setReplayFactor(double factor);
    void setMaxSampleIndex(uint index);
    void setSpan(uint minIdx, uint maxIdx);
    
    void loadStreams(int argc, char** argv, MATCH_MODE mode);
    inline void pushStream(const std::string streamName) { whiteList.insert(streamName); };
    
    inline const std::map<std::string, LogTask*>& getAllLogTasks() { return logTasks; };
    inline const std::string getCurTimeStamp() { return curTimeStamp; };
    inline const std::string getCurSamplePortName() { return curSamplePortName; };
    inline const uint getCurIndex() { return curIndex; };
    inline const size_t getMaxIndex() { return maxIndex; };
    inline const double getReplayFactor() { return replayFactor; };
    inline const double getCurrentSpeed() { return playing ? currentSpeed : 0; };
    inline const bool isValid() { return valid; };
    inline const bool hasFinished() { return finished; };
    inline const bool isPlaying() { return playing; };
    inline void restart() { restartReplay = true; };
    
    const base::Time getMinIdxTimeStamp();
    const base::Time getMaxIdxTimeStamp();
    
private:  
    bool restartReplay;
    double replayFactor;
    const double minReplayFactor = 1e-5;
    mutable double currentSpeed;
    std::string curTimeStamp;
    std::string curSamplePortName;
    uint curIndex;
    uint maxIndex;
    bool finished;
    bool valid;
    bool running;
    std::thread replayThread;
    
    bool playing;
    boost::condition_variable cond, factorChangeCond;
    boost::mutex mut;
    boost::mutex varMut;
    
    std::map<std::string, LogTask *> logTasks;
    std::vector<LogTask *> streamToTask;
    pocolog_cpp::MultiFileIndex multiIndex;
    std::set<std::string> whiteList;
        
    const base::Time getTimeStamp(size_t globalIndex);
    bool replaySample(size_t index, bool dryRun = false);
    void init();
    void replaySamples();
    bool checkSampleIdx();
    
    std::string getTaskName(pocolog_cpp::InputDataStream* stream);
    
};
