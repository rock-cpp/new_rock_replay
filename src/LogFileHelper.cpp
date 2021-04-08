#include "LogFileHelper.hpp"

#include <boost/filesystem.hpp>
#include <regex>

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

    // FIXME: Why to remove /?
    //     size_t pos = taskName.find('/');
    //     if(taskName.size() && pos != std::string::npos)
    //     {
    //         taskName = taskName.substr(pos + 1, taskName.size());
    //     }

    size_t portSplitToken = taskName.find_last_of('.');
    return {taskName.substr(0, portSplitToken), taskName.substr(portSplitToken + 1, taskName.size())};
}

bool LogFileHelper::isWhiteListed(const std::string& streamName, const std::vector<std::string>& whiteListRegEx)
{
    if(whiteListRegEx.empty())
    {
        return true;
    }

    for(const auto& entry : whiteListRegEx)
    {
        if(std::regex_match(streamName, std::regex(entry)))
        {
            return true;
        }
    }

    return false;
}

std::map<std::string, std::string> LogFileHelper::parseRenamings(const std::vector<std::string>& renamings) 
{
    std::map<std::string, std::string> renamingMap;

    for(const auto& renaming : renamings)
    {
        auto delimPos = renaming.find(":");
        if(delimPos != std::string::npos)
        {
            renamingMap.emplace(renaming.substr(0, delimPos), renaming.substr(delimPos + 1, renaming.size()));
        }
    }

    return renamingMap;
}
