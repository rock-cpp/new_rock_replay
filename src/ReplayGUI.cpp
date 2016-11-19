#include "ReplayGUI.h"
#include <qwt_abstract_scale_draw.h>
#include <qwt_plot_curve.h>

ReplayGui::ReplayGui(QMainWindow *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
    
    
    // initing models
    tasksModel = new QStandardItemModel();
    ui.taskNameList->setModel(tasksModel);    
    tasksModel->setHorizontalHeaderItem(0, new QStandardItem(QString("Tasknames")));
    
    // timer
    statusUpdateTimer = new QTimer();
    statusUpdateTimer->setInterval(10);
    checkFinishedTimer = new QTimer();
    checkFinishedTimer->setInterval(10);
    
    // speed bar
    ui.speedBar->setMinimum(0);
    ui.speedBar->setFormat("paused");
    ui.speedBar->setValue(0);
    
    // plot
    ui.qwtPlot->enableAxis(QwtPlot::yLeft, false);
    ui.qwtPlot->enableAxis(QwtPlot::xBottom, false);
    ui.qwtPlot->setFixedHeight(30);
    
    
    // icons
    playIcon.addFile(QString::fromUtf8(":/icons/icons/Icons-master/picol_latest_prerelease_svg/controls_play.svg"), QSize(), QIcon::Normal, QIcon::On);
    pauseIcon.addFile(QString::fromUtf8(":/icons/icons/Icons-master/picol_latest_prerelease_svg/controls_pause.svg"), QSize(), QIcon::Normal, QIcon::On);
    
    // slot connections
    QObject::connect(ui.playButton, SIGNAL(clicked()), this, SLOT(togglePlay()));
    QObject::connect(ui.stopButton, SIGNAL(clicked()), this, SLOT(stopPlay()));
    QObject::connect(statusUpdateTimer, SIGNAL(timeout()), this, SLOT(statusUpdate()));
    QObject::connect(ui.speedBox, SIGNAL(valueChanged(double)), this, SLOT(setSpeedBox()));
    QObject::connect(ui.speedSlider, SIGNAL(sliderReleased()), this, SLOT(setSpeedSlider()));
    QObject::connect(checkFinishedTimer, SIGNAL(timeout()), this, SLOT(handleRestart()));
    
}

ReplayGui::~ReplayGui()
{
    delete replayHandler;
}


void ReplayGui::initReplayHandler(int argc, char* argv[])
{
    replayHandler = new ReplayHandler(argc, argv);
    
    // speed bar
    ui.speedBar->setMaximum(100);
    
    // progress bar
    ui.progressSlider->setMaximum(replayHandler->getMaxIndex());
    
    // labels
    ui.numSamplesLabel->setText(QString(("/ " + std::to_string(replayHandler->getMaxIndex())).c_str()));
    
    // plot
    QwtPlotCurve *curve = new QwtPlotCurve("Data");
    ReplayGraph graph = replayHandler->getGraph();
    curve->setData(QVector<double>::fromStdVector(graph.xData), QVector<double>::fromStdVector(graph.yData));
    curve->attach(ui.qwtPlot);
    
    // window title
    if(argc == 2) 
        this->setWindowTitle(QString(argv[1]));
    else if(argc > 2)
        this->setWindowTitle(QString("Multi Logfile Replay"));
}


void ReplayGui::updateTaskView()
{
    for(const std::pair<std::string, LogTask*>& cur : replayHandler->getAllLogTasks())
    {        
        QStandardItem *task = new QStandardItem(QString(cur.first.c_str()));    
        
        for(const std::string& portName : cur.second->getTaskContext()->ports()->getPortNames())
        {
            task->appendRow(new QStandardItem(QString(portName.c_str())));
            //portTypes.append(new QStandardItem(QString(cur.second->getTaskContext()->getPort(portName)->getTypeInfo()->getTypeName().c_str())));
        }
     
        tasksModel->appendRow(task);
    }
}



int ReplayGui::boxToSlider(double val)
{
    if(val <= 1)
    {
        return val * 50;
    }
    else if(val <= 1000)
    {
        return (val / 40.0) + 50;
    }
    else
    {
        return (val / 400.0) + 75;
    }
}

double ReplayGui::sliderToBox(int val)
{
    if(val <= 50)
    {
        return val / 50.0;
    }
    else if(val <= 75)
    {
        return (val - 50) * 40.0;
    }
    else
    {
        return (val - 75) * 400.0;
    }
}



// #######################################
// ######### SLOT IMPLEMENTATION #########
// #######################################

void ReplayGui::togglePlay()
{
    replayHandler->toggle();
    if(ui.playButton->isChecked())
    {
        ui.playButton->setIcon(pauseIcon);
        ui.speedBar->setFormat("%p%");
        statusUpdateTimer->start();
        checkFinishedTimer->start();
    }
    else
    {
        ui.playButton->setIcon(playIcon);
        ui.playButton->setChecked(false);
        statusUpdateTimer->stop();
        checkFinishedTimer->stop();
        ui.speedBar->setValue(0);
        ui.speedBar->setFormat("paused");
    }
}


void ReplayGui::handleRestart()
{
    if(replayHandler->hasFinished())
    {
        checkFinishedTimer->stop();
        stopPlay();
        replayHandler->setReplayFactor(ui.speedBox->value());
        statusUpdate();
        
        if(ui.repeatButton->isChecked())
        {
            std::cout << "checked repeat" << std::endl;
        }    
    }
}



void ReplayGui::statusUpdate()
{
    ui.speedBar->setValue(replayHandler->getCurrentSpeed() * ui.speedBar->maximum());
    ui.curSampleNum->setText(QString::number(replayHandler->getCurIndex()));
    ui.curTimestamp->setText(replayHandler->getCurTimeStamp().c_str());
    ui.curPortName->setText(replayHandler->getCurSamplePortName().c_str());
    ui.progressSlider->setSliderPosition(replayHandler->getCurIndex());        
}

void ReplayGui::stopPlay()
{
    replayHandler->stop();
    ui.playButton->setIcon(playIcon);
    ui.playButton->setChecked(false);
    statusUpdateTimer->stop();
    ui.speedBar->setValue(0);
    ui.speedBar->setFormat("paused");
}


void ReplayGui::setSpeedBox()
{
    double speed = ui.speedBox->value();
    replayHandler->setReplayFactor(speed);
    ui.speedSlider->setValue(boxToSlider(speed));
    
}

void ReplayGui::setSpeedSlider()
{
    double speed = sliderToBox(ui.speedSlider->value());
    replayHandler->setReplayFactor(speed);
    ui.speedBox->setValue(speed);
}


