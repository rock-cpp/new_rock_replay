#pragma once
#include <vector>
#include <string>
#include <regex>
#include <map>

class FileLoader 
{
    
public:
    FileLoader() = default;
    ~FileLoader() = default;
    
    static std::vector<std::string> parseFileNames(int argc, char* argv[], std::vector<std::regex>& regExps, std::map<std::string, std::string>& logfiles2Prefix);
    
    
};
