#pragma once
#include <string>

static std::string getLogFilePath()
{
    std::string fileLocation(__FILE__);
    return fileLocation.substr(0, fileLocation.find_last_of('/')) + "/../logs/";
}