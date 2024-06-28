#include "ArgParser.hpp"
#include "ReplayGui.h"

#include <csignal>

void startHeadless(const ArgParser& argParser)
{
    //There can only be one replayHandler(due to orocos_cpp), and since
    //startGui does use a one on its stack, it would be unusable if this
    //one were static.
    ReplayHandler replayHandler;

    static volatile bool do_quit = false;
    //This must be a function, so it cannot capture anything.
    std::signal(SIGINT, [](int sig) { do_quit = true; });
    replayHandler.init(argParser.fileNames, argParser.prefix, argParser.whiteListTokens, argParser.renamings);
    replayHandler.play();

    while(replayHandler.isPlaying() && !do_quit)
    {
        if(!argParser.quiet){
            std::cout << "replaying [" << replayHandler.getCurIndex() << "/" << replayHandler.getMaxIndex()
                      << "]: " << replayHandler.getCurSamplePortName() << "\r";
        }
        usleep(5000);
    }

    std::cout << std::endl;

    replayHandler.stop();
    std::cout << "replay handler stopped" << std::endl;
    
    while(argParser.no_exit && !do_quit) sleep(1);
}

int startGui(int argc, char* argv[], const ArgParser& argParser)
{
    QApplication a(argc, argv);
    ReplayGui gui;

    gui.initReplayHandler(argParser.fileNames, argParser.prefix, argParser.whiteListTokens, argParser.renamings);
    gui.updateTaskView();

    gui.show();
    return a.exec();
}

int main(int argc, char* argv[])
{
    ArgParser argParser;
    if(argParser.parseArguments(argc, argv))
    {
        if(argParser.headless)
        {
            startHeadless(argParser);
        }
        else
        {
            startGui(argc, argv, argParser);
        }
    }

    return 0;
}
