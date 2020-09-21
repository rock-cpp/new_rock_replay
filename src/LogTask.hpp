#pragma once
#include <orocos_cpp/orocos_cpp.hpp>
#include <pocolog_cpp/InputDataStream.hpp>
#include <rtt/TaskContext.hpp>
#include <rtt/base/OutputPortInterface.hpp>
#include <rtt/transports/corba/CorbaDispatcher.hpp>
#include <rtt/transports/corba/TaskContextServer.hpp>
#include <rtt/typelib/TypelibMarshallerBase.hpp>
#include <rtt/types/Types.hpp>
#include <string>

class LogTask
{
    struct PortHandle
    {
        PortHandle(const std::string& name, orogen_transports::TypelibMarshallerBase* transport, RTT::base::DataSourceBase::shared_ptr sample, orogen_transports::TypelibMarshallerBase::Handle* transportHandle, RTT::base::OutputPortInterface* port, bool active, pocolog_cpp::InputDataStream& inputDataStream)
            : name(name), transport(transport), sample(sample), transportHandle(transportHandle), port(port), active(active), inputDataStream(inputDataStream)
        {
        }
        std::string name;
        orogen_transports::TypelibMarshallerBase* transport;
        RTT::base::DataSourceBase::shared_ptr sample;
        orogen_transports::TypelibMarshallerBase::Handle* transportHandle;
        RTT::base::OutputPortInterface* port;
        bool active;
        pocolog_cpp::InputDataStream& inputDataStream;
    };

public:
    using PortInfo = std::pair<std::string, std::string>;
    using PortCollection = std::vector<PortInfo>;

    LogTask(const std::string& taskName, const std::string& prefix, orocos_cpp::OrocosCpp& orocos);
    ~LogTask();

    void addStream(pocolog_cpp::InputDataStream& stream);
    bool replaySample(uint64_t streamIndex, uint64_t indexInStream);
    void activateLoggingForPort(const std::string& portName, bool activate = true);
    PortCollection getPortCollection();
    std::string getName();

private:
    std::unique_ptr<PortHandle> createPortHandle(const std::string& portName, pocolog_cpp::InputDataStream& inputStream);
    bool canPortBeSkipped(bool& result, std::unique_ptr<PortHandle>& portHandle);
    bool unmarshalSample(std::unique_ptr<PortHandle>& portHandle, uint64_t indexInStream);
    void checkTaskStateChange(std::unique_ptr<PortHandle>& portHandle);

    std::string prefixedName;
    std::unique_ptr<RTT::TaskContext> task;
    std::map<uint64_t, std::unique_ptr<PortHandle>> streamIdx2Port;
};
