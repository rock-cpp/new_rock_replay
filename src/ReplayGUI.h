#ifndef REPLAY_GUI_H
#define REPLAY_GUI_H

#include "ReplayHandler.hpp"

#include <QMainWindow>
#include <QStandardItemModel>
#include <QTimer>
#include <qwt_data.h>

#include "ui_main.h"

class GraphWrapper : public QwtData
{
public:
    GraphWrapper(const std::shared_ptr<ReplayGraph> graph)
        : xData(graph->xData)
        , yData(graph->yData)
    {}
    
    GraphWrapper(std::vector<double> x, std::vector<double> y)
        : xData(x)
        , yData(y)
    {}

    virtual QwtData* copy() const
    {
        return new GraphWrapper(xData, yData);
    }
    
    virtual size_t size() const
    {
        return xData.size();
    }
    
    virtual double x(size_t i) const
    {
        return xData.at(i) + 500;
    }
    
    virtual double y(size_t i) const
    {
        return yData.at(i);
    }
    
private:
    std::vector<double> xData, yData;
    
};

class TreeViewRootItem : public QStandardItem
{
private:
    LogTask *logTask;
    
public:
    TreeViewRootItem(LogTask *logTask, const std::string &taskName);
    
    LogTask *getLogTask()
    {
        return logTask;
    }
    
    ~TreeViewRootItem()
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
    std::shared_ptr<GraphWrapper> graphWrapper;
    
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
    int oldSpanSliderLower, oldSpanSliderUpper;

    
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
    void handleProgressSliderPressed();
    void handleCheckedChanged(QStandardItem *item);
    void handleSpanSlider();
    void showInfoAbout();
};

#endif // REPLAY_GUI_H
