#pragma once
#include <vector>
#include <string>
#include <regex>
#include <map>

class LogFileHelper 
{
    
public:
    LogFileHelper() = default;
    ~LogFileHelper() = default;
    
    static std::vector<std::string> parseFileNames(const std::vector<std::string> commandLineArgs);
    static std::pair<std::string, std::string> splitStreamName(const std::string& streamName);
    
};
