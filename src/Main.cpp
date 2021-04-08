#include "ArgParser.hpp"
#include "ReplayGui.h"

#include <csignal>

ReplayHandler replayHandler;

int main(int argc, char* argv[])
{
    ArgParser argParser;
    if(argParser.parseArguments(argc, argv))
    {
        if(argParser.headless)
        {
            std::signal(SIGINT, [](int sig) { replayHandler.stop(); });
            replayHandler.init(argParser.fileNames, argParser.prefix, argParser.whiteListTokens);
            replayHandler.play();

            while(replayHandler.isPlaying())
            {
                std::cout << "replaying [" << replayHandler.getCurIndex() << "/" << replayHandler.getMaxIndex()
                          << "]: " << replayHandler.getCurSamplePortName() << "\r";
            }

            std::cout << std::endl;

            replayHandler.stop();
            std::cout << "replay handler stopped" << std::endl;
        }
        else
        {
            QApplication a(argc, argv);
            ReplayGui gui;

            gui.initReplayHandler(argParser.fileNames, argParser.prefix, argParser.whiteListTokens);
            gui.updateTaskView();

            gui.show();
            return a.exec();
        }
    }

    return 0;
}
