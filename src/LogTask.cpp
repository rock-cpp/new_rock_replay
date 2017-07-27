#include "LogTask.hpp"
#include <rtt/typelib/TypelibMarshallerBase.hpp>
#include <rtt/types/Types.hpp>
#include <rtt/TaskContext.hpp>
#include <rtt/transports/corba/TaskContextServer.hpp>
#include <rtt/transports/corba/CorbaDispatcher.hpp>
#include <rtt/base/OutputPortInterface.hpp>
#include <string>

class PortHandle
{
public:
    std::string name;
    orogen_transports::TypelibMarshallerBase *transport;
    RTT::base::DataSourceBase::shared_ptr sample;
    orogen_transports::TypelibMarshallerBase::Handle *transportHandle;
    RTT::base::OutputPortInterface *port;
    bool loggingActive;
};

LogTask::LogTask(const std::string& name)
{
    std::string correctedName(name);
    
    if(correctedName.size() && correctedName[0] == '/')
    {
        correctedName = correctedName.substr(1, correctedName.size());
    }
    
    task = new RTT::TaskContext(correctedName);

    RTT::corba::TaskContextServer::Create( task );
    RTT::corba::CorbaDispatcher::Instance( task->ports(), ORO_SCHED_OTHER, RTT::os::LowestPriority );
}

LogTask::~LogTask()
{
    delete task;
    for(std::map<std::string, PortHandle*>::iterator it = name2handle.begin(); it != name2handle.end(); it++)
        delete it->second;
}


void LogTask::activateLoggingForPort(const std::string& portName, bool activate)
{
    if (name2handle.find(portName) != name2handle.end())
    {
        PortHandle *handle = name2handle[portName];
        handle->loggingActive = activate;
    }
}

bool LogTask::createReplayPort(const std::string& portname, const std::string& typestr, PortHandle &handle)
{
    RTT::types::TypeInfoRepository::shared_ptr ti = RTT::types::TypeInfoRepository::Instance();
    RTT::types::TypeInfo* type = ti->type(typestr);
    if (! type)
    {
        std::cerr << "cannot find " << typestr << " in the type info repository" << std::endl;
        return false;
    }
    
    RTT::base::PortInterface *testIfExists = task->ports()->getPort(portname);
    if(testIfExists) {
        std::cerr << "port with name " << portname << " already exists" << std::endl;
        return false;
    }

    RTT::base::OutputPortInterface *writer= type->outputPort(portname);
    orogen_transports::TypelibMarshallerBase* transport =
        dynamic_cast<orogen_transports::TypelibMarshallerBase*>(type->getProtocol(orogen_transports::TYPELIB_MARSHALLER_ID));
    if (! transport)
    {
        log(RTT::Error) << "cannot report ports of type " << type->getTypeName() << " as no typekit generated by orogen defines it" << RTT::endlog();
        return false;
    }

    //TODO check if local type is same as logfile type
    
//     m_registry->merge(transport->getRegistry());
//     if (! m_registry->has(transport->getMarshallingType()))
//     {
//         log(RTT::Error) << "cannot report ports of type " << type->getTypeName() << " as I can't find a typekit Typelib registry that defines it" << RTT::endlog();
//         return false;
//     }

    task->ports()->addPort(writer->getName(), *writer);

    try {
        handle.name = writer->getName();
        handle.port = writer;
        handle.transportHandle = transport->createSample();
        handle.transport = transport;
        handle.sample = transport->getDataSource(handle.transportHandle);
        handle.loggingActive = true;
    } catch ( RTT::internal::bad_assignment& ba ) {
        return false;
    }
    return true;
}

bool LogTask::addStream(const pocolog_cpp::InputDataStream& stream)
{
    size_t idx = stream.getIndex();
    
    if(idx >= handles.size())
    {
        handles.resize(idx+1, nullptr);
    }

    PortHandle *handle = new PortHandle;
    
    size_t nameStart = stream.getName().find_last_of('.') + 1;
    std::string portName = stream.getName().substr(nameStart, stream.getName().size());
    
    if(!createReplayPort(portName, stream.getCXXType(), *handle))
    {
        delete handle;
        throw std::runtime_error("Error, could not create replay port");
        return false;
    }

    std::cout << "Created port " << idx << " name " << portName << std::endl;
    
    handles[idx] = handle;
    name2handle[portName] = handle;
    
    return true;
}

bool LogTask::replaySample(pocolog_cpp::InputDataStream& stream, size_t sampleNr)
{        
    size_t idx = stream.getIndex();
//     std::cout << "TaskName is " << task->getName() << " streamName " << stream.getName() << " idx " << idx << std::endl; 
    
    if(idx >= handles.size() || !handles[idx])
    {
        throw std::runtime_error("LogTask::replaySample Error, got stream with unregistered index");
    }

    PortHandle &handle(*handles[idx]);
    if (!handle.loggingActive)
    {
        return false;
    }
    
    //optimization, do nothing if nobody is listening to this port
    if(!handle.port->connected())
    {
        return true;
    }
    
    std::vector<uint8_t> data;
    if(!stream.getSampleData(data, sampleNr))
    {
        std::cout << "Warning, could not replay sample: " << stream.getName() << " " << sampleNr <<  std::endl;
        return false;
    }
    
    try {
        handle.transport->unmarshal(data, handle.transportHandle);
    } catch (...) {
        std::cout << "caught marshall error.." << std::endl;
        return false;
    }
    
    // currently we're only supporting default states
    if(handle.port->getName() == "state")
    {
        switch(std::stoi(handle.sample.get()->toString()))
        {
            case 0:  // INIT
                task->configure();
                break;
            case 1:  // PRE_OPERATIONAL
                break;
            case 2: // FATAL_ERROR
                break;
            case 3: // EXCEPTION
                break;
            case 4: // STOPPED
                task->stop();
                break;
            case 5: // RUNNING
                task->start();
                break;
            case 6:  // RUNTIME_ERROR
                break;
            default:
                task->start();
        }
    }
    
    handle.port->write(handle.sample);
    return true;
}