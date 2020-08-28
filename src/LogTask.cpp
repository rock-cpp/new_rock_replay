#include "LogTask.hpp"
#include <rtt/typelib/TypelibMarshallerBase.hpp>
#include <rtt/types/Types.hpp>
#include <rtt/TaskContext.hpp>
#include <rtt/transports/corba/TaskContextServer.hpp>
#include <rtt/transports/corba/CorbaDispatcher.hpp>
#include <rtt/base/OutputPortInterface.hpp>
#include <string>


LogTask::LogTask(const std::string& taskName, orocos_cpp::OrocosCpp& orocos)
{    
    orocos.loadAllTypekitsForModel(taskName);
    
    task = std::unique_ptr<RTT::TaskContext>(new RTT::TaskContext(taskName));
    RTT::corba::TaskContextServer::Create(task.get());
    RTT::corba::CorbaDispatcher* dispatcher = RTT::corba::CorbaDispatcher::Instance(task->ports());
    dispatcher->setScheduler(ORO_SCHED_OTHER);
    dispatcher->setPriority(RTT::os::LowestPriority);
    
    std::cout << "created task " << taskName << std::endl;
}

LogTask::~LogTask()
{
   
}


void LogTask::activateLoggingForPort(const std::string& portName, bool activate)
{
   
}

bool LogTask::addStream(pocolog_cpp::InputDataStream& stream)
{
    size_t nameStart = stream.getName().find_last_of('.') + 1;
    std::string portName = stream.getName().substr(nameStart, stream.getName().size());
    
    auto logPort = std::unique_ptr<LogPort>(new LogPort(portName, stream));
  
    if(logPort->initialize(*task))
    {
        streamIdx2Port.emplace(stream.getIndex(), std::move(logPort));
        return true;
    }
    
    return false;
}

bool LogTask::replaySample(uint64_t streamIndex, uint64_t indexInStream)
{        
    return streamIdx2Port.at(streamIndex)->replaySample(indexInStream);
}
