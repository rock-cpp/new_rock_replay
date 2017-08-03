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

class ReplayGui : public QMainWindow
{
    Q_OBJECT

public:
    ReplayGui(std::function<void(QStandardItemModel*, ReplayHandler*)> updateTaskView, QMainWindow *parent = 0);
    ~ReplayGui();
    
    void initReplayHandler(ReplayHandler *replayHandler, const QString &title);
    void initReplayHandler(int argc, char* argv[]);
    inline QStandardItemModel* getTasksModel() { return tasksModel; };
    inline Ui::MainWindow& getMainWindow() { return ui; };
    inline ReplayHandler* getReplayHandler() { return replayHandler; };
    inline void updateTaskView() { updateTaskViewCallback(getTasksModel(), getReplayHandler()); };
    
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

    std::function<void(QStandardItemModel*, ReplayHandler*)> updateTaskViewCallback;

    
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
    void handleSpanSlider();
    void showInfoAbout();
    void showOpenFile();
    void handleRestart();    
};

#endif // REPLAY_GUI_H
