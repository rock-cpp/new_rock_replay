#ifndef REPLAY_GUI_H
#define REPLAY_GUI_H

#include "ReplayHandler.hpp"

#include <QMainWindow>
#include <QStandardItemModel>
#include <QTimer>

#include "ui_main.h"


class TreeViewRootItem : public QStandardItem
{
private:
    LogTask *logTask;
    
public:
    TreeViewRootItem(LogTask *logTask, const std::string &taskName);
    virtual ~TreeViewRootItem();
    
    LogTask *getLogTask()
    {
        return logTask;
    }
    

};

enum GUI_MODES
{
    PAUSED = 0,
    PLAYING = 1
};

class ReplayGui : public QMainWindow
{
    Q_OBJECT

public:
    ReplayGui(QMainWindow *parent = 0);
    ~ReplayGui();
    
    void initReplayHandler(ReplayHandler *replayHandler, const QString &title);
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
    bool stoppedBySlider;
    void changeGUIMode(GUI_MODES mode);
 

    
public slots:
    void togglePlay();
    void stopPlay();
    void statusUpdate();
    void setSpeedBox();
    void setSpeedSlider();
    void forward();
    void backward();
    void progressSliderUpdate();
    void handleProgressSliderPressed();
    void handleCheckedChanged(QStandardItem *item);
    void handleSpanSlider();
    void showInfoAbout();
    void showOpenFile();
    void handleRestart();    
};

#endif // REPLAY_GUI_H
