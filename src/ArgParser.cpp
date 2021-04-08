#include "ArgParser.hpp"

#include "LogFileHelper.hpp"

#include <iostream>

bool ArgParser::parseArguments(int argc, char* argv[])
{
    using namespace boost::program_options;

    options_description desc(
        "Usage: rock-replay2 {logfile|*}.log or folder.\nLogging can be controlled via base-logging variables.\nAvailable options");
    desc.add_options()("help", "show this message")("prefix", value<std::string>(&prefix), "add prefix to all tasks")(
        "whitelist", value<std::string>(&whiteListInput), "comma-separated list of regular expressions to filter streams")(
        "headless", bool_switch(&headless), "only use the cli")("log-files", value<std::vector<std::string>>(&fileArgs), "log files");

    positional_options_description p;
    p.add("log-files", -1);

    variables_map vm;
    store(command_line_parser(argc, argv).options(desc).positional(p).run(), vm);
    notify(vm);

    fileNames = LogFileHelper::parseFileNames(fileArgs);
    if(vm.count("help") || !vm.count("log-files") || fileNames.empty())
    {
        std::cout << desc << std::endl;
        return false;
    }

    if(vm.count("whitelist"))
    {
        boost::tokenizer<boost::char_separator<char>> tokens(whiteListInput, boost::char_separator<char>(","));
        whiteListTokens.assign(tokens.begin(), tokens.end());
    }

    return true;
}