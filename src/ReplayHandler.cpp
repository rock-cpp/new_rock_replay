#include "ReplayHandler.hpp"
#include "orocos_cpp/TypeRegistry.hpp"


ReplayHandler::ReplayHandler(int argc, char** argv, uint windowSize)
    : windowSize(windowSize)
{
    RTT::corba::TaskContextServer::InitOrb(argc, argv);

    char *installDir = getenv("AUTOPROJ_CURRENT_ROOT");
    
    if(!installDir)
    {
//         std::cout << "Error, could not find AUTOPROJ_CURRENT_ROOT env.sh not sourced ?" << std::endl;
        throw std::runtime_error("Error, could not find AUTOPROJ_CURRENT_ROOT env.sh not sourced ?");
    }
    
//     std::cout << "Loading all typekits " << std::endl;
//     orocos_cpp::PluginHelper::loadAllPluginsInDir(std::string(installDir) + "/install/lib/orocos/gnulinux/types/");
//     orocos_cpp::PluginHelper::loadAllPluginsInDir(std::string(installDir) + "/install/lib/orocos/types/");

    orocos_cpp::TypeRegistry reg;
    
    reg.loadTypelist();
    
    for(int i = 1; i < argc; i++)
    {
        filenames.push_back(argv[i]);
    }
    
    multiIndex = new pocolog_cpp::MultiFileIndex();
    
    RTT::types::TypeInfoRepository::shared_ptr ti = RTT::types::TypeInfoRepository::Instance();

    multiIndex->registerStreamCheck([&](pocolog_cpp::Stream *st){
        std::cout << "Checking " << st->getName() << std::endl;
        pocolog_cpp::InputDataStream *dataStream = dynamic_cast<pocolog_cpp::InputDataStream *>(st);
        if(!dataStream)
        {
            return false;
        }
        
        std::string typestr = dataStream->getType()->getName();
        
        std::string tkName;
        if(!reg.getTypekitDefiningType(typestr, tkName))
        {
            std::cerr << "cannot find " << typestr << " in the type info repository" << std::endl;
            return false;
        }

        if(!orocos_cpp::PluginHelper::loadTypekitAndTransports(tkName))
        {
            return false;
        }
     
        RTT::types::TypeInfo* type = ti->type(dataStream->getCXXType());
        if (! type)
        {
            std::cerr << "2 cannot find " << typestr << " in the type info repository" << std::endl;
            return false;
        }
        
        return true;
        
    }
    );
    
    multiIndex->createIndex(filenames);
    
    streamToTask.resize(multiIndex->getAllStreams().size());

    for(pocolog_cpp::Stream *st : multiIndex->getAllStreams())
    {
        pocolog_cpp::InputDataStream *inputSt = dynamic_cast<pocolog_cpp::InputDataStream *>(st);
        if(!inputSt)
            continue;       

        std::string taskName = st->getName();
        taskName = taskName.substr(0, taskName.find_last_of('.'));
        
        LogTask *task = nullptr;
        auto it = logTasks.find(taskName);
        if(it == logTasks.end())
        {
            task = new LogTask(taskName);
            logTasks.insert(std::make_pair(taskName, task));
        }
        else
        {
            task = it->second;
        }
        task->addStream(*inputSt);

        size_t gIdx = multiIndex->getGlobalStreamIdx(st); 
        if(gIdx > streamToTask.size())
            throw std::runtime_error("Mixup detected");

        streamToTask[gIdx] = task;
    }

    valid = !multiIndex->getAllStreams().empty();
    if(!valid)
    {
        std::cerr << "empty streams loaded" << std::endl;
    }
    else
    {
        init();
    }
}


ReplayHandler::~ReplayHandler()
{
    delete multiIndex;
    
    if(valid)
        delete replayThread;
}

void ReplayHandler::init()
{
    replayFactor = 1.;
    currentSpeed = replayFactor;
    curIndex = 0;
    curTimeStamp = "";
    curSamplePortName = "";
    finished = false;
    play = false;
    maxIndex = multiIndex->getSize() - 1;
    replayThread = new boost::thread(boost::bind(&ReplayHandler::replaySamples, boost::ref(*this)));
}


const ReplayGraph ReplayHandler::getGraph() const
{
    int64_t offset = getTimeStamp(0).microseconds;
    std::vector<int64_t> timestamps;
    std::vector<double> x, y;
    for(size_t i = 0; i < multiIndex->getSize(); i++)
    {
        timestamps.push_back(getTimeStamp(i).microseconds - offset);
    }
    
    double maxStd = 0;
    for(uint i = windowSize; i < timestamps.size(); i++)
    {
        std::vector<int64_t> buf(timestamps.begin() + i - windowSize, timestamps.begin() + i);
        double mean = std::accumulate(buf.begin(), buf.end(), 0.0) / buf.size();
        double variance = 0;
        std::for_each(buf.begin(), buf.end(), [&](int64_t &val){ variance += std::pow(mean - val, 2); });
        variance /= buf.size();
        double stdDeviation = std::sqrt(variance);
        y.push_back(stdDeviation);   
        x.push_back(i);
        
        if(stdDeviation > maxStd)
            maxStd = stdDeviation;
    }
    
    for(int i = 0; i < y.size(); i++)
        y.at(i) = 1 - y.at(i) / maxStd;

    return ReplayGraph {x,y};
}



void ReplayHandler::replaySample(size_t index, bool dryRun) const
{
    try 
    {
        size_t globalStreamIndex = multiIndex->getGlobalStreamIdx(index);
        pocolog_cpp::InputDataStream *inputSt = dynamic_cast<pocolog_cpp::InputDataStream *>(multiIndex->getSampleStream(index));
        curSamplePortName = inputSt->getName();
        if(!dryRun)
            streamToTask[globalStreamIndex]->replaySample(*inputSt, multiIndex->getPosInStream(index)); 
    } 
    catch(...)
    {
        std::cout << "Warning: ignoring corrupt sample: " << index << "/" << maxIndex << std::endl;
    }
}

void ReplayHandler::replaySamples()
{   
    boost::unique_lock<boost::mutex> lock(mut);
    
    std::cout << "Replaying all samples" << std::endl;
    base::Time start(base::Time::now()), lastExecute(base::Time::now());
    
    size_t allSamples = multiIndex->getSize();
    
    restartReplay = true;
    
    base::Time lastStamp;
    base::Time curStamp;
    base::Time toSleep;

    base::Time systemPlayStartTime;
    base::Time logPlayStartTime;
    curStamp = getTimeStamp(curIndex);
    
    while(!finished)
    {
        
        while(!play)
        {
            cond.wait(lock);
            restartReplay = true;
        }

//         std::cout << "Replaying Sample " << curIndex << std::endl;
        
        if(curIndex >= allSamples)
        {
            play = false;
            continue;
        }

        if(restartReplay)
        {
            systemPlayStartTime = base::Time::now();
            
            //expensive call
            curStamp = getTimeStamp(curIndex);
            logPlayStartTime = curStamp;
            
            restartReplay = false;
        }

        //TODO check if chronological ordering is right
        replaySample(curIndex);

        lastStamp = curStamp;

        curIndex++;
        if(curIndex >= allSamples)
        {
            play = false;
            continue;
        }
      
        curStamp = getTimeStamp(curIndex);;
        curTimeStamp = curStamp.toString();
        
        if(lastStamp > curStamp)
        {
            std::cout << "Warning: invalid sample order curStamp " << curStamp <<  " Last Stamp " << lastStamp << std::endl;
        }

        //hm, also expensive, is there a way to reduce this ?
        base::Time curTime = base::Time::now();
        
        //hm, factor... should it not be * replayFactor then ?
        base::Time logTimeSinceStart = (curStamp - logPlayStartTime) / replayFactor;
        base::Time systemTimeSinceStart = (curTime - systemPlayStartTime);
        toSleep = logTimeSinceStart - systemTimeSinceStart;
        
//         std::cout << "Log time since start " << logTimeSinceStart << std::endl;
//         std::cout << "Sys time since start " << systemTimeSinceStart << std::endl;
//         std::cout << "To Sleep " << toSleep.microseconds << std::endl;
        
        if(toSleep.microseconds > 0)
        {
            usleep(toSleep.microseconds);
            currentSpeed = replayFactor;
        }
        else if(toSleep.microseconds == 0)
        {
            currentSpeed = replayFactor;
        }
        else // tosleep < 0
        {
            currentSpeed = logTimeSinceStart.toSeconds() / systemTimeSinceStart.toSeconds();   
        }      
        
        if(curIndex == maxIndex)
            finished = true;
    }

}

const base::Time ReplayHandler::getTimeStamp(size_t globalIndex) const
{    
    pocolog_cpp::Index &idx = multiIndex->getSampleStream(globalIndex)->getFileIndex();
    return idx.getSampleTime(multiIndex->getPosInStream(globalIndex));;
}

void ReplayHandler::stop()
{
    delete replayThread;
    init();
}


void ReplayHandler::setReplayFactor(double factor)
{
    this->replayFactor = factor;
    restartReplay = true;
}


void ReplayHandler::next()
{
    if(curIndex < maxIndex)
    {
        replaySample(++curIndex, true);
        curTimeStamp = getTimeStamp(curIndex).toString();
    }
    
}

void ReplayHandler::previous()
{
    if(curIndex > 0)
    {
        replaySample(--curIndex, true);
        curTimeStamp = getTimeStamp(curIndex).toString();
    }
}


void ReplayHandler::setSampleIndex(uint index)
{
    curIndex = index;
}

void ReplayHandler::toggle(bool restart)
{
    if(!valid)
    {
        std::cerr << "empty streams loaded" << std::endl;
    }
    else
    {
        if(!play)
        {
            play = true;
            restartReplay = true;
            cond.notify_one();
            std::cout << "Starting replay" << std::endl;
            
            if(restart)
            {
                curIndex = 0;
            }
            
        } 
        else
        {
            play = false;
            std::cout << "Stopping replay" << std::endl;
        }
    }
}







