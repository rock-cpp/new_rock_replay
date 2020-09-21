#include "LogFileHelper.hpp"

#include <boost/filesystem.hpp>

std::vector<std::string> LogFileHelper::parseFileNames(const std::vector<std::string> commandLineArgs)
{
    using namespace boost::filesystem;

    std::vector<std::string> filenames;

    for(const auto& arg : commandLineArgs)
    {
        if(exists(arg))
        {
            if(is_directory(arg))
            {
                for(const auto& entry : recursive_directory_iterator(arg))
                {
                    if(is_regular_file(entry) && entry.path().extension() == ".log")
                    {
                        filenames.emplace_back(arg + entry.path().filename().string());
                    }
                }
            }
            else
            {
                filenames.emplace_back(arg);
            }
        }
    }

    return filenames;
}

std::pair<std::string, std::string> LogFileHelper::splitStreamName(const std::string& streamName)
{
    std::string taskName = streamName;

    size_t pos = taskName.find('/');
    if(taskName.size() && pos != std::string::npos)
    {
        taskName = taskName.substr(pos + 1, taskName.size());
    }

    size_t portSplitToken = taskName.find_last_of('.');
    return {taskName.substr(0, portSplitToken), taskName.substr(portSplitToken + 1, taskName.size())};
}
