#include "LogTask.hpp"

#include "LogFileHelper.hpp"
#include <rtt/typelib/TypelibMarshallerBase.hpp>
#include <rtt/types/Types.hpp>
#include <rtt/TaskContext.hpp>
#include <rtt/transports/corba/TaskContextServer.hpp>
#include <rtt/transports/corba/CorbaDispatcher.hpp>
#include <rtt/base/OutputPortInterface.hpp>
#include <string>


LogTask::LogTask(const std::string& taskName, orocos_cpp::OrocosCpp& orocos)
{    
    try
    {
        orocos.loadAllTypekitsForModel(taskName);
        
        task = std::unique_ptr<RTT::TaskContext>(new RTT::TaskContext(taskName));
        RTT::corba::TaskContextServer::Create(task.get());
        RTT::corba::CorbaDispatcher* dispatcher = RTT::corba::CorbaDispatcher::Instance(task->ports());
        dispatcher->setScheduler(ORO_SCHED_OTHER);
        dispatcher->setPriority(RTT::os::LowestPriority);
        
        std::cout << "created task " << taskName << std::endl;
    }
    catch(std::runtime_error& e)
    {
        std::cerr << "could not create task " << taskName << " due to missing typekit" << std::endl;
    }
}

LogTask::~LogTask()
{
    RTT::corba::TaskContextServer::CleanupServer(task.get());
}


void LogTask::activateLoggingForPort(const std::string& portName, bool activate)
{
    for(const auto& idx2Port : streamIdx2Port)
    {
        if(idx2Port.second->getName() == portName)
        {
            idx2Port.second->activateReplay(activate);
        }
    }
}

void LogTask::addStream(pocolog_cpp::InputDataStream& stream)
{
    const auto taskAndPortName = LogFileHelper::splitStreamName(stream.getName());
    
    auto logPort = std::unique_ptr<LogPort>(new LogPort(taskAndPortName.second, stream));
  
    if(task)
    {
        logPort->initialize(*task);
    }    

    streamIdx2Port.emplace(stream.getIndex(), std::move(logPort));
}

bool LogTask::replaySample(uint64_t streamIndex, uint64_t indexInStream)
{        
    return streamIdx2Port.at(streamIndex)->replaySample(indexInStream);
}
