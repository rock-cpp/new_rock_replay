#include "ReplayHandler.hpp"

#include "LogFileHelper.hpp"

#include <boost/algorithm/clamp.hpp>


ReplayHandler::~ReplayHandler()
{       
    deinit();
}


void ReplayHandler::init(const std::vector<std::string>& fileNames)
{
    manager.init(fileNames);
    targetSpeed = 1.;
    currentSpeed = 0;
    curIndex = 0;
    finished = false;
    playing = false;
    running = true;
    replayWasValid = true;
    maxIdx = manager.getNumSamples() - 1;
    minSpan = 0;
    maxSpan = maxIdx;
    setSampleIndex(curIndex);
    replayThread = std::thread(std::bind(&ReplayHandler::replaySamples, this));
}

void ReplayHandler::deinit()
{
    {
        std::lock_guard<std::mutex> lock(playMutex);
        playing = false;
        running = false;
    }
    playCondition.notify_one();
    replayThread.join();
    manager.deinit();
}


std::map<std::string, std::vector<std::pair<std::string, std::string>>> ReplayHandler::getTaskNamesWithPorts()
{
    return manager.getTaskNamesWithPorts();
}


void ReplayHandler::replaySamples()
{          
    while(running)
    {
        timeBeforeSleep = base::Time::now();
        timeToSleep = 0;
        
        {
            std::unique_lock<std::mutex> lock(playMutex);
            playCondition.wait(lock, [this]{return playing || !running;});
        }
        
        while(playing)
        {
            replayWasValid = manager.replaySample();
            if(curIndex < maxSpan)
            {
                calculateRelativeSpeed();
                next();
                calculateTimeToSleep();

                std::unique_lock<std::mutex> lock(playMutex);
                playCondition.wait_for(lock, std::chrono::milliseconds(timeToSleep), [this]{return !playing || !running;});
            }
            else
            {
                finished = true;
                playing = false;
            }
        }
    }
}

void ReplayHandler::calculateTimeToSleep()
{
    timeToSleep = (curMetadata.timeStamp - previousSampleTime).toMilliseconds() / targetSpeed;
    timeBeforeSleep = base::Time::now();
}

void ReplayHandler::calculateRelativeSpeed()
{
    int64_t diff = (base::Time::now() - timeBeforeSleep).toMilliseconds();
    if(diff && timeToSleep)
    {
        currentSpeed = static_cast<double>(timeToSleep) / diff;
    }
    else
    {
        currentSpeed = 1;
    }
}


void ReplayHandler::stop()
{
    curIndex = minSpan;
    finished = false;
    
    std::lock_guard<std::mutex> lock(playMutex);
    playing = false;
    setSampleIndex(curIndex);
    playCondition.notify_one();
}


void ReplayHandler::setReplaySpeed(float speed)
{
    constexpr float minimumSpeed = 0.01;
    targetSpeed = std::max(speed, minimumSpeed);
}


void ReplayHandler::next()
{
    if(curIndex < maxSpan)
    {
        previousSampleTime = curMetadata.timeStamp;
        setSampleIndex(++curIndex);
    }
}

void ReplayHandler::previous()
{
    if(curIndex)
    {
        setSampleIndex(--curIndex);
    }
}


void ReplayHandler::setSampleIndex(uint64_t index)
{
    curIndex = boost::algorithm::clamp(index, minSpan, maxSpan);
    curMetadata = manager.setIndex(index);
}

void ReplayHandler::setMinSpan(uint64_t minIdx)
{
    minSpan = minIdx;
}

void ReplayHandler::setMaxSpan(uint64_t maxIdx)
{
    maxSpan = maxIdx;
}

void ReplayHandler::play()
{
    {
        std::lock_guard<std::mutex> lock(playMutex);
        playing = true;
    }
    playCondition.notify_one();
}

void ReplayHandler::pause()
{
    {
        std::lock_guard<std::mutex> lock(playMutex);
        playing = false;
    }
    currentSpeed = 0;
}

void ReplayHandler::activateReplayForPort(const std::string& taskName, const std::string& portName, bool on)
{
    manager.activateReplayForPort(taskName, portName, on);
}
