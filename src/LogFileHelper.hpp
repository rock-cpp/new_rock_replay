#pragma once
#include <map>
#include <regex>
#include <string>
#include <vector>

/**
 * @brief Helper class to deal with log file related tasks.
 *
 */
class LogFileHelper
{

public:
    /**
     * @brief Constuctor.
     *
     */
    LogFileHelper() = default;

    /**
     * @brief Destructor.
     *
     */
    ~LogFileHelper() = default;

    /**
     * @brief Parses the command line arguments for log files or folders that contain log files.
     *
     * @param commandLineArgs: List of command line arguments.
     * @return std::vector<std::string> List of filenames with absolute paths.
     */
    static std::vector<std::string> parseFileNames(const std::vector<std::string> commandLineArgs);

    /**
     * @brief Splits a stream name to task and port component, e.g. trajectory_follower.state -> trajectory_follower, state
     *
     * @param streamName: Stream name to split.
     * @return std::pair<std::string, std::string> Pair consisting of task and port name.
     */
    static std::pair<std::string, std::string> splitStreamName(const std::string& streamName);
};
