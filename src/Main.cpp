#include "ReplayGui.h"

#include "LogFileHelper.hpp"
#include <boost/program_options.hpp>

std::string prefix;
std::vector<std::string> fileArgs;
std::vector<std::string> fileNames;

bool parseArguments(int argc, char* argv[])
{
    using namespace boost::program_options;
    
    options_description desc("Usage: rock-replay2 {logfile|*}.log or folder\nAvailable options");
    desc.add_options()
        ("help", "show this message")
        ("prefix", value<std::string>(), "add prefix to all tasks")
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
    
    if(vm.count("prefix"))
    {
        prefix = vm["prefix"].as<std::string>();
    }
    
    return true;
}
    
int main(int argc, char *argv[])
{
    if(parseArguments(argc, argv))
    {
        QApplication a(argc, argv);
        ReplayGui gui;
        
        gui.initReplayHandler(fileNames, prefix);
        gui.updateTaskView();
        
        gui.show();    
        return a.exec();
    }
    
    return 0;
}
