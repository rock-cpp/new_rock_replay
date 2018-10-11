#include "ReplayHandler.hpp"
#include <boost/algorithm/string.hpp>
#include <orocos_cpp/TypeRegistry.hpp>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <rtt/transports/corba/CorbaDispatcher.hpp>
#include <rtt/typelib/TypelibMarshallerBase.hpp>
#include <typelib/typedisplay.hh>


void ReplayHandler::loadStreams(int argc, char** argv, MATCH_MODE mode)
{
    RTT::corba::TaskContextServer::InitOrb(argc, argv);    
    
    // load basic typekits
    orocos_cpp::PluginHelper::loadTypekitAndTransports("orocos");
    
    multiIndex = new pocolog_cpp::MultiFileIndex();    
    
    orocos_cpp::TypeRegistry reg;
    reg.loadTypeRegistries();
    RTT::types::TypeInfoRepository::shared_ptr ti = RTT::types::TypeInfoRepository::Instance();

    std::vector<std::regex> regExps;
    std::map<std::string, std::string> logfiles2Prefix;
    std::vector<std::string> fileNames = parseFilenames(argc, argv, regExps, logfiles2Prefix);

    multiIndex->registerStreamCheck([&](pocolog_cpp::Stream *st){
        std::cout << "Checking " << st->getName() << std::endl;
        pocolog_cpp::InputDataStream *dataStream = dynamic_cast<pocolog_cpp::InputDataStream *>(st);
        if(!dataStream)
        {
            return false;
        }
        
        bool matches;
        switch(mode)
        {
            case REGEX:
                matches = regExps.empty();
                for(std::regex exp : regExps)
                {
                    if(std::regex_search(dataStream->getName(), exp))
                        matches = true;
                }
                break;
            case WHITELIST:
                matches = whiteList.find(dataStream->getName()) != whiteList.end();
                break;
        }
        
        if(!matches)
        {
            std::cout << "skipping non-whitelisted stream " << dataStream->getName() << std::endl;
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

    multiIndex->createIndex(fileNames);
    streamToTask.resize(multiIndex->getAllStreams().size());

    for(pocolog_cpp::Stream *st : multiIndex->getAllStreams())
    {
        pocolog_cpp::InputDataStream *inputSt = dynamic_cast<pocolog_cpp::InputDataStream *>(st);
        if(!inputSt)
            continue;       

        std::string taskName = st->getName();
        
        // filter all '/'
        size_t pos = taskName.find('/');
        if(taskName.size() && pos != std::string::npos)
        {
            taskName = taskName.substr(pos + 1, taskName.size());
        }
        
        // remove port name
        taskName = taskName.substr(0, taskName.find_last_of('.'));
        
        //add prefix if active
        if(logfiles2Prefix.find(inputSt->getFileStream().getFileName()) != logfiles2Prefix.end())
        {
            taskName = logfiles2Prefix.at(inputSt->getFileStream().getFileName()) + taskName;
        }
        
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

    valid = !multiIndex->getAllStreams().empty() && !fileNames.empty() && multiIndex->getSize() > 0;
    if(!valid)
    {
        std::cerr << "empty streams loaded" << std::endl;
        exit(0);
    }
    else
    {
        init();
    }

}


std::vector<std::string> ReplayHandler::parseFilenames(int argc, char* argv[], std::vector<std::regex>& regExps, std::map<std::string, std::string>& logfiles2Prefix)
{
    std::vector<std::string> filenames;
    
    for(int i = 1; i < argc; i++)
    {
        std::string argv2String(argv[i]);
        std::string whiteList = "--white-list=";
        std::size_t pos = argv2String.find(whiteList);
        
        if(pos != std::string::npos) 
        {
            std::string params = argv2String.substr(whiteList.length(), argv2String.length());
            boost::split(regExps, params, boost::is_any_of(","));
            continue;
        }
    
        std::string prefix = "--prefix=";
        pos = argv2String.find(prefix);
    
        if(pos != std::string::npos) 
        {
            std::string params = argv2String.substr(prefix.length(), argv2String.length());
            std::vector<std::string> prefixTuples;
            boost::split(prefixTuples, params, boost::is_any_of(","));
            
            for(const std::string pair : prefixTuples)
            {
                std::size_t sep = pair.find(":");
                if(sep != std::string::npos)
                {
                    std::string logfile = pair.substr(0, sep);
                    std::string prefix = pair.substr(sep + 1, pair.length());
                    
                    std::cout << "prefixing: " << logfile << " -> " << prefix << std::endl;
                    logfiles2Prefix.insert(std::make_pair(logfile, prefix));
                }
            }
            
            continue;
        }
        
        struct stat file_stat;
        if(stat(argv[i], &file_stat) == -1)
        {
            std::cerr << "stat error: couldn't open folder" << std::endl;
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
            closedir(dir);
        }
        else if(S_ISREG(file_stat.st_mode))
        {
            filenames.push_back(argv[i]);
        }
    }
    
    return filenames;
}



ReplayHandler::~ReplayHandler()
{       
    RTT::corba::CorbaDispatcher::ReleaseAll();
    RTT::corba::TaskContextServer::CleanupServers();
       
    for(std::map<std::string, LogTask*>::iterator it = logTasks.begin(); it != logTasks.end(); it++)
        delete it->second;
        
    delete multiIndex;    
}


void ReplayHandler::init()
{
    replayFactor = 1.;
    currentSpeed = replayFactor;
    curIndex = 0;
    finished = false;
    playing = false;
    maxIndex = multiIndex->getSize() - 1;
    replaySample(curIndex, true);       
    boost::thread(boost::bind(&ReplayHandler::replaySamples, boost::ref(*this)));
}

const base::Time ReplayHandler::getMinIdxTimeStamp()
{
    return getTimeStamp(curIndex);
}

const base::Time ReplayHandler::getMaxIdxTimeStamp()
{
    return getTimeStamp(maxIndex);
}



bool ReplayHandler::replaySample(size_t index, bool dryRun)
{
    try 
    {
        size_t globalStreamIndex = multiIndex->getGlobalStreamIdx(index);
        pocolog_cpp::InputDataStream *inputSt = dynamic_cast<pocolog_cpp::InputDataStream *>(multiIndex->getSampleStream(index));
        if (dryRun || (!dryRun && streamToTask[globalStreamIndex]->replaySample(*inputSt, multiIndex->getPosInStream(index))))
        {
            curSamplePortName = inputSt->getName();
            curTimeStamp = getTimeStamp(index).toString();
        }
        
        return true;
    }
    catch(...)
    {
        std::cout << "Warning: ignoring corrupt sample: " << index << "/" << maxIndex << std::endl;     
    }
    
    return false;
}

bool ReplayHandler::checkSampleIdx()
{
    if(curIndex > maxIndex)
    {
        finished = true;
        playing = false;
        varMut.unlock();
        return true;
    }
    return false;
}


void ReplayHandler::replaySamples()
{   
    boost::unique_lock<boost::mutex> lock(mut);
    
    restartReplay = true;
    
    base::Time curStamp;
    base::Time toSleep;

    base::Time systemPlayStartTime;
    base::Time logPlayStartTime;
    curStamp = getTimeStamp(curIndex);
    
    while(1)
    {
        while(!playing)
        {
            cond.wait(lock);
            restartReplay = true;
        }
        
        varMut.lock();
        
        if(checkSampleIdx())
            continue;
        
        
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
        if (!replaySample(curIndex++))
        {
            varMut.unlock();
            continue;
        }
        
        if(checkSampleIdx())
            continue;
        
        replaySample(curIndex, true);
        curStamp = getTimeStamp(curIndex);    

        //hm, also expensive, is there a way to reduce this ?
        base::Time curTime = base::Time::now();
        
        //hm, factor... should it not be * replayFactor then ?
        int64_t tSinceStart = curStamp.microseconds - logPlayStartTime.microseconds;
        tSinceStart = std::max<int64_t>(0, tSinceStart);
        base::Time logTimeSinceStart = base::Time::fromMicroseconds(tSinceStart / replayFactor);
        base::Time systemTimeSinceStart = (curTime - systemPlayStartTime);
        toSleep = logTimeSinceStart - systemTimeSinceStart;

        varMut.unlock();
        
        if(!playing)
            continue;
        
        if(toSleep.microseconds > 0)
        {
            factorChangeCond.timed_wait(lock, boost::posix_time::microseconds(toSleep.microseconds));
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
    }

}

const base::Time ReplayHandler::getTimeStamp(size_t globalIndex)
{    
    pocolog_cpp::Index &idx = multiIndex->getSampleStream(globalIndex)->getFileIndex();
    return idx.getSampleTime(multiIndex->getPosInStream(globalIndex));
}

void ReplayHandler::stop()
{
    varMut.lock();
    curIndex = 0;
    finished = false;
    playing = false;
    restartReplay = true;
    replaySample(curIndex, true);
    varMut.unlock();
}


void ReplayHandler::setReplayFactor(double factor)
{
    varMut.lock();
    this->replayFactor = factor;
    if (this->replayFactor < minReplayFactor)
    {
        this->replayFactor = minReplayFactor;
    }
    restartReplay = true;
    factorChangeCond.notify_one();
    varMut.unlock();
}


void ReplayHandler::next()
{
    varMut.lock();
    if(curIndex < maxIndex)
    {
        replaySample(++curIndex);
    }
    varMut.unlock();
    
}

void ReplayHandler::previous()
{
    varMut.lock();
    if(curIndex > 0)
    {
        replaySample(--curIndex);
    }
    varMut.unlock();
}


void ReplayHandler::setSampleIndex(uint index)
{
    varMut.lock();
    curIndex = index;
    replaySample(curIndex, true); // do a dry run for metadata update
    varMut.unlock();
}

void ReplayHandler::setMaxSampleIndex(uint index)
{
    varMut.lock();
    maxIndex = index;
    varMut.unlock();
}

void ReplayHandler::setSpan(uint minIdx, uint maxIdx)
{
    varMut.lock();
    curIndex = minIdx;
    maxIndex = maxIdx;
    varMut.unlock();
}


void ReplayHandler::play()
{
    varMut.lock();
    if(valid)
    {
        playing = true;
        restartReplay = true;
        cond.notify_one();
    }
    varMut.unlock();
}

void ReplayHandler::pause()
{
    varMut.lock();
    playing = false;
    varMut.unlock();
}
