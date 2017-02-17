#include "ReplayGUI.h"
#include "qxtspanslider.h"
#include <QtGui/QMessageBox>
#include <QFileDialog>


TreeViewRootItem::TreeViewRootItem(LogTask *logTask, const std::string &taskName)
    : QStandardItem(QString(taskName.c_str())),
      logTask(logTask)
{
    
}

TreeViewRootItem::~TreeViewRootItem()
{
    while(this->rowCount() > 0)
    {
        QList<QStandardItem*> rows = this->takeRow(0);
        for(QStandardItem *item : rows)
            delete item;
    }
}


void ReplayGui::handleCheckedChanged(QStandardItem *item)
{
    TreeViewRootItem *taskItem = nullptr;
    if (!item->parent() || !(taskItem = dynamic_cast<TreeViewRootItem*>(item->parent())))
    {
        return;
    }
    
    const QModelIndex index = tasksModel->indexFromItem(item);
    QItemSelectionModel *selModel = ui.taskNameList->selectionModel();
    const std::string portName = item->text().toStdString();
    taskItem->getLogTask()->activateLoggingForPort(portName, item->checkState() == Qt::Checked);
    selModel->select(QItemSelection(index, index), item->checkState() == Qt::Checked ? QItemSelectionModel::Select : QItemSelectionModel::Deselect);
}

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
    ui.speedBar->setMaximum(100);
    
    
    // make port and timestamp line edits grey
    QPalette replayInfoPalette;
    replayInfoPalette.setColor(QPalette::Base,Qt::lightGray);
    ui.curPortName->setPalette(replayInfoPalette);
    ui.curPortName->setReadOnly(true);
    ui.curTimestamp->setPalette(replayInfoPalette);
    ui.curTimestamp->setReadOnly(true);
    ui.curSampleNum->setPalette(replayInfoPalette);
    ui.curSampleNum->setReadOnly(true);
    
    
    // icons
    playIcon.addFile(QString::fromUtf8(":/icons/icons/Icons-master/picol_latest_prerelease_svg/controls_play.svg"), QSize(), QIcon::Normal, QIcon::On);
    pauseIcon.addFile(QString::fromUtf8(":/icons/icons/Icons-master/picol_latest_prerelease_svg/controls_pause.svg"), QSize(), QIcon::Normal, QIcon::On);
    
    replayHandler = nullptr;
    stoppedBySlider = false;
    
    // slot connections
    QObject::connect(ui.playButton, SIGNAL(clicked()), this, SLOT(togglePlay()));
    QObject::connect(ui.stopButton, SIGNAL(clicked()), this, SLOT(stopPlay()));
    QObject::connect(ui.forwardButton, SIGNAL(clicked()), this, SLOT(forward()));
    QObject::connect(ui.backwardButton, SIGNAL(clicked()), this, SLOT(backward()));
    QObject::connect(statusUpdateTimer, SIGNAL(timeout()), this, SLOT(statusUpdate()));
    QObject::connect(ui.speedBox, SIGNAL(valueChanged(double)), this, SLOT(setSpeedBox()));
    QObject::connect(ui.speedSlider, SIGNAL(sliderReleased()), this, SLOT(setSpeedSlider()));
    QObject::connect(ui.progressSlider, SIGNAL(sliderReleased()), this, SLOT(progressSliderUpdate()));
    QObject::connect(ui.progressSlider, SIGNAL(sliderPressed()), this, SLOT(handleProgressSliderPressed()));
    QObject::connect(checkFinishedTimer, SIGNAL(timeout()), this, SLOT(handleRestart()));
    QObject::connect(ui.intervalSlider, SIGNAL(sliderReleased()), this, SLOT(handleSpanSlider()));
    QObject::connect(ui.infoAbout, SIGNAL(triggered()), this, SLOT(showInfoAbout()));
    QObject::connect(ui.actionOpenLogfile, SIGNAL(triggered()), this, SLOT(showOpenFile()));
    QObject::connect(tasksModel, SIGNAL(itemChanged(QStandardItem *)), this, SLOT(handleCheckedChanged(QStandardItem *)));
}

ReplayGui::~ReplayGui()
{
    replayHandler->stop();
    delete replayHandler;
}


void ReplayGui::initReplayHandler(int argc, char* argv[])
{
    if(replayHandler)
    {
        delete replayHandler;
    }
    replayHandler = new ReplayHandler(argc, argv);    
    replayHandler->setReplayFactor(ui.speedBox->value());
    
    // progress bar
    ui.progressSlider->setMaximum(replayHandler->getMaxIndex());
    
    // labels
    ui.numSamplesLabel->setText(QString(("/ " + std::to_string(replayHandler->getMaxIndex() + 1)).c_str()));
    
    // span slider
    ui.intervalSlider->setHandleMovementMode(QxtSpanSlider::NoOverlapping);
    ui.intervalSlider->setMaximum(replayHandler->getMaxIndex());
    ui.intervalSlider->setSpan(0, replayHandler->getMaxIndex());
    ui.intervalSlider->setLowerPosition(0);
    ui.intervalSlider->setUpperPosition(replayHandler->getMaxIndex());
    
    
    // window title
    switch(argc)
    {
        case 1:
            this->setWindowTitle("Rock-Replay");
            break;
        case 2:
            this->setWindowTitle(QString(argv[1]));
            break;
        default:
            this->setWindowTitle(QString("Multi Logfile Replay"));
            break;
    }        
}


void ReplayGui::updateTaskView()
{   
    while(tasksModel->rowCount() > 0)
    {
        QList<QStandardItem*> rows = tasksModel->takeRow(0);
        for(QStandardItem *item : rows)
            delete item;
    }
    
    for(const std::pair<std::string, LogTask*>& cur : replayHandler->getAllLogTasks())
    {        
        TreeViewRootItem *task = new TreeViewRootItem(cur.second, cur.first);
        task->setCheckable(false);
        
        for(const std::string& portName : cur.second->getTaskContext()->ports()->getPortNames())
        {
            QStandardItem *port = new QStandardItem(portName.c_str());
            port->setCheckable(true);
            port->setData(Qt::Checked, Qt::CheckStateRole);
            task->appendRow(port);
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
    if(!replayHandler->isPlaying())
    {
        replayHandler->play();
        ui.playButton->setChecked(true);
        ui.playButton->setIcon(pauseIcon);
        ui.speedBar->setFormat("%p%");
        statusUpdateTimer->start();
        checkFinishedTimer->start();
    }
    else
    {
        replayHandler->pause();
        ui.playButton->setIcon(playIcon);
        ui.playButton->setChecked(false);
        statusUpdateTimer->stop();
        checkFinishedTimer->stop();
        ui.speedBar->setValue(0);
        ui.speedBar->setFormat("paused");
        stoppedBySlider = false;
    }
}


void ReplayGui::handleRestart()
{
    if(replayHandler->hasFinished())
    {
        checkFinishedTimer->stop();
        stopPlay();
        statusUpdate();
        stoppedBySlider = false;
        if(ui.repeatButton->isChecked())
        {
            togglePlay();
        }    
    }
}



void ReplayGui::statusUpdate()
{
    ui.speedBar->setValue(round(replayHandler->getCurrentSpeed() * ui.speedBar->maximum()));
    ui.curSampleNum->setText(QString::number(replayHandler->getCurIndex()));
    ui.curTimestamp->setText(replayHandler->getCurTimeStamp().c_str());
    ui.curPortName->setText(replayHandler->getCurSamplePortName().c_str());
    ui.progressSlider->setSliderPosition(replayHandler->getCurIndex());        
}

void ReplayGui::stopPlay()
{
    if(replayHandler->isValid())
    {
        replayHandler->stop();
        replayHandler->setReplayFactor(ui.speedBox->value()); // ensure that old replay speed is kept
        replayHandler->setSampleIndex(ui.intervalSlider->lowerPosition()); // ensure that old span is kept
        replayHandler->setMaxSampleIndex(ui.intervalSlider->upperPosition());
    }
    ui.playButton->setIcon(playIcon);
    ui.playButton->setChecked(false);
    statusUpdateTimer->stop();
    ui.speedBar->setValue(0);
    ui.speedBar->setFormat("paused");
    statusUpdate();
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

void ReplayGui::backward()
{
    replayHandler->previous();
    statusUpdate();
}

void ReplayGui::forward()
{
    replayHandler->next();
    statusUpdate();
}

void ReplayGui::progressSliderUpdate()
{ 
    replayHandler->setSampleIndex(ui.progressSlider->value());
    statusUpdate();
    if(stoppedBySlider)
    {
        togglePlay();
        statusUpdateTimer->start();
    }
}

void ReplayGui::handleProgressSliderPressed()
{
    stoppedBySlider = replayHandler->isPlaying();
    stopPlay();
}

void ReplayGui::handleSpanSlider()
{
    stopPlay();
    replayHandler->setSampleIndex(ui.intervalSlider->lowerPosition());
    replayHandler->setMaxSampleIndex(ui.intervalSlider->upperPosition());
}


void ReplayGui::showOpenFile()
{
    QStringList fileNames = QFileDialog::getOpenFileNames(this, "Select logfile(s)", "", "Logfiles: *.log");
    QStringList fileNamesCopy = fileNames;  // doc says so

    std::vector<std::string> stdStrings{"rock-replay2"};
    std::vector<char*> cStrings;
    for(QList<QString>::iterator it = fileNamesCopy.begin(); it != fileNamesCopy.end(); it++)
    {
        stdStrings.push_back(it->toStdString());
    }
    
    for(std::string &s : stdStrings)
    {
        cStrings.push_back(&s.front());
    }
    
    stopPlay();
    initReplayHandler(cStrings.size(), cStrings.data());
    updateTaskView();
    statusUpdate();
}



void ReplayGui::showInfoAbout()
{
    QMessageBox::information(this, "Credits",
                             QString("PICOL iconset: http://www.picol.org\n").append(
                             "SpanSlider: https://bitbucket.org/libqxt/libqxt/wiki/Home"), QMessageBox::Ok, 0);
}

