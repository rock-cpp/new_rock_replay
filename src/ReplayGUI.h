#ifndef REPLAY_GUI_H
#define REPLAY_GUI_H

#include "ReplayHandler.hpp"

#include <QMainWindow>
#include <QStandardItemModel>
#include <QTimer>

#include "ui_main.h"

class TreeViewItem : public QStandardItem
{
private:
    LogTask *logTask;
    std::string portName;
    
public:
    TreeViewItem(LogTask *logTask, const std::string &portName);
    
    LogTask *getLogTask()
    {
        return logTask;
    }
    
    const std::string getPortName()
    {
        return portName;
    }
    
    ~TreeViewItem()
    {
    }
};

class ReplayGui : public QMainWindow
{
    Q_OBJECT

public:
    ReplayGui(QMainWindow *parent = 0);
    ~ReplayGui();
    
    void initReplayHandler(int argc, char* argv[]);
    void updateTaskView();

private:
    Ui::MainWindow ui;
    ReplayHandler *replayHandler;
    
    // models
    QStandardItemModel *tasksModel;
    
    // icons
    QIcon playIcon, pauseIcon;
    
    // timers
    QTimer *statusUpdateTimer;
    QTimer *checkFinishedTimer;
    
    
    double sliderToBox(int val);
    int boxToSlider(double val);

    
public slots:
    void togglePlay();
    void stopPlay();
    void statusUpdate();
    void setSpeedBox();
    void setSpeedSlider();
    void handleRestart();
    void forward();
    void backward();
    void progressSliderUpdate();
    void handleCheckedChanged(QStandardItem *item);
};

#endif // REPLAY_GUI_H
