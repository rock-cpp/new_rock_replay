#pragma once

#include <rtt/typelib/TypelibMarshallerBase.hpp>
#include <rtt/types/Types.hpp>
#include <rtt/TaskContext.hpp>
#include <rtt/transports/corba/TaskContextServer.hpp>
#include <rtt/transports/corba/CorbaDispatcher.hpp>
#include <rtt/base/OutputPortInterface.hpp>
#include <string>
#include <pocolog_cpp/InputDataStream.hpp>

class LogPort
{
public:
    LogPort(const std::string& name, pocolog_cpp::InputDataStream& inputDataStream);
    ~LogPort() = default;
    
    bool initialize(RTT::TaskContext& parentTask);
    bool replaySample(uint64_t sampleInStream);
    
private:
    std::string name;
    orogen_transports::TypelibMarshallerBase *transport;
    RTT::base::DataSourceBase::shared_ptr sample;
    orogen_transports::TypelibMarshallerBase::Handle *transportHandle;
    RTT::base::OutputPortInterface *port;
    bool loggingActive;
    pocolog_cpp::InputDataStream& inputDataStream;
};
