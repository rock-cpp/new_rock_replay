#include "ReplayGui.h"

#include "LogFileHelper.hpp"
#include <boost/program_options.hpp>

std::string prefix;
std::vector<std::string> fileArgs;

void parseArguments(int argc, char* argv[])
{
    using namespace boost::program_options;
    
    options_description desc("Usage: rock-replay2 {logfile|*}.log\nAvailable options");
    desc.add_options()
        ("help", "show this message")
        ("prefix", value<std::string>(), "add prefix to all tasks")
        ("log-files", value<std::vector<std::string>>(), "log files");

    positional_options_description p;
    p.add("log-files", -1);
        
    variables_map vm;
    store(command_line_parser(argc, argv).options(desc).positional(p).run(), vm);
    notify(vm);    

    if(vm.count("help"))
    {
        std::cout << desc << std::endl;
    }
    
    if(vm.count("prefix"))
    {
        prefix = vm["prefix"].as<std::string>();
    }
    
    if(vm.count("log-files"))
    {
        fileArgs = vm["log-files"].as<std::vector<std::string>>();
    }
}
    
int main(int argc, char *argv[])
{
    parseArguments(argc, argv);
    
    QApplication a(argc, argv);
    ReplayGui gui;
    
    gui.initReplayHandler(LogFileHelper::parseFileNames(fileArgs), prefix);
    gui.updateTaskView();
    
    gui.show();    
    return a.exec();
}
