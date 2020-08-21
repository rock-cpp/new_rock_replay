#pragma once

#include <base/Time.hpp>
#include <thread>
#include <memory>
#include <future>

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
    void setSampleIndex(uint64_t index);
    
    void setReplaySpeed(float speed);
    void setMinSpan(uint64_t minIdx);
    void setMaxSpan(uint64_t maxIdx);
    
    void activateReplayForPort(const std::string& taskName, const std::string& portName, bool on);
        
    void init(const std::vector<std::string>& fileNames);
    void deinit();
    std::vector<std::pair<std::string, std::vector<std::string>>> getTaskNamesWithPorts();
    
    const std::string getCurTimeStamp() { return curMetadata.timeStamp.toString(); };
    const std::string getCurSamplePortName() { return curMetadata.portName; };
    const uint getCurIndex() { return curIndex; };
    const size_t getMaxIndex() { return maxIdx; };
    const uint64_t getMinSpan() {return minSpan;};
    const uint64_t getMaxSpan() {return maxSpan;};
    const double getReplayFactor() { return targetSpeed; };
    const double getCurrentSpeed() { return currentSpeed; };
    const bool hasFinished() { return finished; };
    const bool isPlaying() { return playing; };
    const size_t getNumSamples() { return manager.getNumSamples(); };
    
private:
    void replaySamples();
    void calculateTimeToSleep();
    void calculateRelativeSpeed();
    
    float targetSpeed;
    float currentSpeed;
    LogTaskManager::SampleMetadata curMetadata;
    uint64_t curIndex;
    uint64_t minSpan;
    uint64_t maxSpan;
    uint64_t maxIdx;
    bool finished;
    bool running;
    std::mutex playMutex;
    std::condition_variable playCondition;
    std::thread replayThread;
    
    base::Time previousSampleTime;
    base::Time timeBeforeSleep;
    int64_t timeToSleep;
    
    bool playing;
        

    LogTaskManager manager;
};
