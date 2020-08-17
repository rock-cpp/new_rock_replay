#pragma once

#include <vector>
#include <string>
#include <memory>
#include <map>

#include <pocolog_cpp/MultiFileIndex.hpp>

#include "LogTask.hpp"

class LogTaskManager
{
  
public:
    LogTaskManager() = default;
    ~LogTaskManager() = default;
    
    void init(const std::vector<std::string>& fileNames);
    
private:
    pocolog_cpp::MultiFileIndex multiFileIndex;
    std::map<std::string, std::unique_ptr<LogTask>> name2Task;
};
