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
    ReplayGui(QMainWindow *parent = 0);
    ~ReplayGui();
    
    void updateTaskView();
    
    
    void initReplayHandler(const QString &title);
    void initReplayHandler(int argc, char* argv[]);    
    
protected:
    Ui::MainWindow ui;
    ReplayHandler replayHandler;
    
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
    void handleItemChanged(QStandardItem *item);
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



class TreeViewItem : public QStandardItem
{
    
private:
    LogTask *logTask;
    
public:
    TreeViewItem(LogTask *logTask, const std::string &taskName)
        : QStandardItem(QString(taskName.c_str()))
        , logTask(logTask)
    {}
    
    ~TreeViewItem()
    {
        while(this->rowCount() > 0)
        {
            QList<QStandardItem*> rows = this->takeRow(0);
            for(QStandardItem *item : rows)
                delete item;
        }
    }
    
    LogTask *getLogTask()
    {
        return logTask;
    }
};
