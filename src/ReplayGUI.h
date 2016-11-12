#ifndef REPLAY_GUI_H
#define REPLAY_GUI_H

#include "ReplayHandler.hpp"
#include "Vizkit3dPluginRepository.hpp"

#include <QMainWindow>
#include <QStringListModel>
#include <QTimer>

#include "ui_main.h"

class ReplayGui : public QMainWindow
{
    Q_OBJECT

public:
    ReplayGui(QMainWindow *parent = 0);
    ~ReplayGui();
    
    void initReplayHandler(int argc, char* argv[]);
    void updateTaskNames();

private:
    Ui::MainWindow ui;
    ReplayHandler *replayHandler;
    
    // models
    QStringListModel *taskNameListModel;
    
    // lists
    QStringList *taskNameList;
    
    // icons
    QIcon playIcon, pauseIcon;
    
    // timers
    QTimer *statusUpdateTimer;
    
    // labels
    QLabel *label_sample_count;
    
    double sliderToBox(int val);
    int boxToSlider(double val);
    
    Vizkit3dPluginRepository *pluginRepo;
    
    
public slots:
    void togglePlay();
    void stopPlay();
    void statusUpdate();
    void setSpeedBox();
    void setSpeedSlider();
    
};

#endif // REPLAY_GUI_H
