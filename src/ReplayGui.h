#pragma once

#include "ReplayHandler.hpp"

#include <QMainWindow>
#include <QStandardItemModel>
#include <QTimer>

#include "ui_main.h"


class ReplayGui : public QMainWindow
{
    Q_OBJECT

public:
    ReplayGui(QMainWindow *parent = 0);
    ~ReplayGui();
    
    void updateTaskView();
        
    void initReplayHandler(int argc, char* argv[]);    
    
protected:
    Ui::MainWindow ui;
    ReplayHandler replayHandler;
    
    // models
    QStandardItemModel *tasksModel;
        

private:    
    // icons
    QIcon playIcon, pauseIcon;
    
    // timers
    QTimer *statusUpdateTimer;
    QTimer *checkFinishedTimer;
    
    void setGuiPaused();
    void setGuiPlaying();

    
public slots:
    void handleItemChanged(QStandardItem *item);
    void togglePlay();
    void stopPlay();
    void statusUpdate();
    void setSpeedBox();
    void forward();
    void backward();
    void progressSliderUpdate();
    void showInfoAbout();
    void showOpenFile();
    void handleRestart();    
    void setIntervalA();
    void setIntervalB();
    
};



class TreeViewItem : public QStandardItem
{
    
public:
    TreeViewItem(const std::string &taskName)
        : QStandardItem(QString(taskName.c_str()))
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
};
