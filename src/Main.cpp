#include "ReplayGUI.h"
#include <typelib/pluginmanager.hh>


    
int main(int argc, char *argv[])
{
    Typelib::PluginManager::self manager;
    QApplication a(argc, argv);
    ReplayGui gui;
    
    gui.initReplayHandler(argc, argv);
    gui.updateTaskView();
    
    gui.show();    
    return a.exec();
}
