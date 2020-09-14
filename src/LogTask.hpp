#pragma once
#include <string>
#include <pocolog_cpp/InputDataStream.hpp>
#include <orocos_cpp/orocos_cpp.hpp>
#include "LogPort.hpp"


class LogTask
{
    
public:
    LogTask(const std::string &taskName, orocos_cpp::OrocosCpp& orocos);
    ~LogTask();
    
    void addStream(pocolog_cpp::InputDataStream& stream);
    bool replaySample(uint64_t streamIndex, uint64_t indexInStream);    
    void activateLoggingForPort(const std::string& portName, bool activate = true);

private:
    std::unique_ptr<RTT::TaskContext> task;
    std::map<uint64_t, std::unique_ptr<LogPort>> streamIdx2Port;
};
