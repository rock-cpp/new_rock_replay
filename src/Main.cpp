#include "LogFileHelper.hpp"
#include "ReplayGui.h"

#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/trivial.hpp>
#include <boost/program_options.hpp>

std::string prefix;
std::vector<std::string> fileArgs;
std::vector<std::string> fileNames;
bool verbose = false;

/**
 * @brief Parses the command line arguments.
 *
 * @param argc: Argument counter.
 * @param argv: Argument values.
 * @return bool True if all positional argument requirements are met, false otherwise.
 */
bool parseArguments(int argc, char* argv[])
{
    using namespace boost::program_options;

    options_description desc("Usage: rock-replay2 {logfile|*}.log or folder\nAvailable options");
    desc.add_options()
        ("help", "show this message")
        ("prefix", value<std::string>(&prefix), "add prefix to all tasks")
        ("verbose", bool_switch(&verbose), "show additional output")
        ("log-files", value<std::vector<std::string>>(&fileArgs), "log files");

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

    return true;
}

int main(int argc, char* argv[])
{
    if(parseArguments(argc, argv))
    {
        QApplication a(argc, argv);
        ReplayGui gui;

        boost::log::core::get()->set_logging_enabled(verbose);

        gui.initReplayHandler(fileNames, prefix);
        gui.updateTaskView();

        gui.show();
        return a.exec();
    }

    return 0;
}
