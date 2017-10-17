#ifndef REPLAY_GUI_H
#define REPLAY_GUI_H

#include "ReplayHandler.hpp"

#include <QMainWindow>
#include <QStandardItemModel>
#include <QTimer>

#include "ui_main.h"


enum GUI_MODES
{
    PAUSED = 0,
    PLAYING = 1
};

class ReplayGuiBase : public QMainWindow
{
    Q_OBJECT

public:
    ReplayGuiBase(QMainWindow *parent = 0);
    ~ReplayGuiBase();
    
    void initReplayHandler(ReplayHandler *replayHandler, const QString &title);
    void initReplayHandler(int argc, char* argv[]);    
    virtual void updateTaskView() = 0;
    
protected:
    Ui::MainWindow ui;
    ReplayHandler *replayHandler;
    
    // models
    QStandardItemModel *tasksModel;
    
    void shiftAToB();
    

private:    
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
    virtual void handleItemChanged(QStandardItem *item) = 0;
    void togglePlay();
    void stopPlay();
    void statusUpdate();
    void setSpeedBox();
    void setSpeedSlider();
    void forward();
    void backward();
    void progressSliderUpdate();
    void handleProgressSliderPressed();
    void handleSpanSlider();
    void showInfoAbout();
    void showOpenFile();
    void handleRestart();    
    void setIntervalA();
    void setIntervalB();
};

#endif // REPLAY_GUI_H
