#pragma once
#include <map>
#include <regex>
#include <string>
#include <vector>

class LogFileHelper
{

public:
    LogFileHelper() = default;
    ~LogFileHelper() = default;

    static std::vector<std::string> parseFileNames(const std::vector<std::string> commandLineArgs);
    static std::pair<std::string, std::string> splitStreamName(const std::string& streamName);
};
