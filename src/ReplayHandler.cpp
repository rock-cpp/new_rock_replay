#include "ReplayHandler.hpp"
#include "orocos_cpp/TypeRegistry.hpp"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>


ReplayHandler::ReplayHandler(int argc, char** argv, uint windowSize)
    : windowSize(windowSize)
{
    RTT::corba::TaskContextServer::InitOrb(argc, argv);

    char *installDir = getenv("AUTOPROJ_CURRENT_ROOT");
    
    if(!installDir)
    {
        throw std::runtime_error("Error, could not find AUTOPROJ_CURRENT_ROOT env.sh not sourced ?");
    }
    
    // load all typekits
    orocos_cpp::PluginHelper::loadAllPluginsInDir(std::string(installDir) + "/install/lib/orocos/gnulinux/types/");
    orocos_cpp::PluginHelper::loadAllPluginsInDir(std::string(installDir) + "/install/lib/orocos/types/");

    orocos_cpp::TypeRegistry reg;
    
    reg.loadTypelist();
    
    parseFilenames(argc, argv);
    
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


void ReplayHandler::parseFilenames(int argc, char* argv[])
{
    for(int i = 1; i < argc; i++)
    {
        struct stat file_stat;
        if(stat(argv[i], &file_stat) == -1)
        {
            std::cerr << "stat error" << std::endl;
            exit(EXIT_FAILURE);
        }
        
        if(S_ISDIR(file_stat.st_mode))
        {
            DIR *dir = opendir(argv[i]);
            if(!dir) 
            {
                std::cerr << "directory opening error" << std::endl;
                exit(EXIT_FAILURE);   
            }
            else
            {
                dirent *entry;
                std::string dir_path = std::string(argv[i]);
                if(dir_path.back() != '/')
                    dir_path.append("/");
                
                while((entry = readdir(dir)) != 0)
                {
                    std::string filename = entry->d_name;
                    if(filename.substr(filename.find_last_of(".") + 1) == "log")
                    {
                        if(filename == "orocos.log")
                            continue;
                        
                        filenames.push_back(std::string(dir_path).append(filename));
                    }
                }
            }
        }
        else if(S_ISREG(file_stat.st_mode))
        {
            filenames.push_back(argv[i]);
        }
    }
}



ReplayHandler::~ReplayHandler()
{       
    RTT::corba::TaskContextServer::DestroyOrb();
    
    delete multiIndex;
    
    for(std::map<std::string, LogTask*>::iterator it = logTasks.begin(); it != logTasks.end(); it++)
        delete it->second;
    
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

base::Time ReplayHandler::extractTimeFromStream(size_t index)
{
    pocolog_cpp::Index &idx = multiIndex->getSampleStream(index)->getFileIndex();
    return idx.getSampleTime(multiIndex->getPosInStream(index));
}

void ReplayHandler::enableGraph()
{
    if(!graph.get())
        buildGraph();
}



void ReplayHandler::buildGraph()
{
    int64_t offset = extractTimeFromStream(0).toMicroseconds();
    std::vector<double> x, y;
    std::vector<int64_t> unbiasedTimestamps; // buffer for stddev calculation
    for(size_t i = 0; i < multiIndex->getSize(); i++)
    {
        base::Time curTime = extractTimeFromStream(i);
        unbiasedTimestamps.push_back(curTime.microseconds - offset);
    }
    
    double maxStd = 0;
    for(uint i = windowSize; i < unbiasedTimestamps.size(); i++)
    {
        std::vector<int64_t> buf(unbiasedTimestamps.begin() + i - windowSize, unbiasedTimestamps.begin() + i);
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

    graph = std::make_shared<ReplayGraph>();
    graph->xData = x;
    graph->yData = y;
}



bool ReplayHandler::replaySample(size_t index, bool dryRun) const
{
    try 
    {
        size_t globalStreamIndex = multiIndex->getGlobalStreamIdx(index);
        pocolog_cpp::InputDataStream *inputSt = dynamic_cast<pocolog_cpp::InputDataStream *>(multiIndex->getSampleStream(index));
        if (dryRun || (!dryRun && streamToTask[globalStreamIndex]->replaySample(*inputSt, multiIndex->getPosInStream(index))))
        {
            curSamplePortName = inputSt->getName();
        }
        
        return true;
    }
    catch(...)
    {
        std::cout << "Warning: ignoring corrupt sample: " << index << "/" << maxIndex << std::endl;     
    }
    
    return false;
}

void ReplayHandler::replaySamples()
{   
    boost::unique_lock<boost::mutex> lock(mut);
    
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
        //TODO if logging for port is not selected, skip cur index
        // (allow higher replayFactor)
        if (!replaySample(curIndex))
        {
            curIndex++;
            continue;
        }

        lastStamp = curStamp;

        curIndex++;
        if(curIndex >= allSamples)
        {
            play = false;
            continue;
        }
      
        curStamp = getTimeStamp(curIndex);
        curTimeStamp = curStamp.toString();
        
        if(lastStamp > curStamp)
        {
            std::cout << "Warning: invalid sample order curStamp " << curStamp <<  " Last Stamp " << lastStamp << std::endl;
        }

        //hm, also expensive, is there a way to reduce this ?
        base::Time curTime = base::Time::now();
        
        //hm, factor... should it not be * replayFactor then ?
        base::Time logTimeSinceStart = base::Time::fromMicroseconds((curStamp.microseconds - logPlayStartTime.microseconds) / replayFactor);
        base::Time systemTimeSinceStart = (curTime - systemPlayStartTime);
        toSleep = logTimeSinceStart - systemTimeSinceStart;

        // std::cout << "Log time since start " << logTimeSinceStart << std::endl;
        // std::cout << "Sys time since start " << systemTimeSinceStart << std::endl;
        // std::cout << "To Sleep " << toSleep.microseconds << std::endl;
        
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

const base::Time ReplayHandler::getTimeStamp(size_t globalIndex)
{    
    return extractTimeFromStream(globalIndex);
}

void ReplayHandler::stop()
{
    delete replayThread;
    init();
}


void ReplayHandler::setReplayFactor(double factor)
{
    this->replayFactor = factor;
    if (this->replayFactor < minReplayFactor)
    {
        this->replayFactor = minReplayFactor;
    }
    
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
    replaySample(curIndex, true); // do a dry run for metadata update
    curTimeStamp = getTimeStamp(curIndex).toString();
}

void ReplayHandler::setMaxSampleIndex(uint index)
{
    maxIndex = index;
}


void ReplayHandler::toggle()
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
        } 
        else
        {
            play = false;
        }
    }
}