#include "ReplayGui.h"
    
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    ReplayGui gui;
    
    gui.initReplayHandler(argc, argv);
    gui.updateTaskView();
    
    gui.show();    
    return a.exec();
}
