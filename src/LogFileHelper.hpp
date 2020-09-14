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
    
    static std::vector<std::string> parseFileNames(int argc, char* argv[], std::vector<std::regex>& regExps, std::map<std::string, std::string>& logfiles2Prefix);
    static std::pair<std::string, std::string> splitStreamName(const std::string& streamName);
    
};
