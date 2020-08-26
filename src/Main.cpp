#include "ReplayGui.h"
    
int main(int argc, char *argv[])
{
    if(argc < 2)
    {
        std::cout << "Usage: rock-replay2 {logfile|*}.log" << std::endl;
        return 0;
    }
        
    QApplication a(argc, argv);
    ReplayGui gui;
    
    gui.initReplayHandler(argc, argv);
    gui.updateTaskView();
    
    gui.show();    
    return a.exec();
}
