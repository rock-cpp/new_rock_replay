#include "ReplayGUI.h"

#include <QFileDialog>
#include <QMessageBox>


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
    
    stoppedBySlider = false;
    
    // slot connections
    QObject::connect(ui.playButton, SIGNAL(clicked()), this, SLOT(togglePlay()));
    QObject::connect(ui.stopButton, SIGNAL(clicked()), this, SLOT(stopPlay()));
    QObject::connect(ui.forwardButton, SIGNAL(clicked()), this, SLOT(forward()));
    QObject::connect(ui.backwardButton, SIGNAL(clicked()), this, SLOT(backward()));
    QObject::connect(ui.intervalAButton, SIGNAL(clicked()), this, SLOT(setIntervalA()));
    QObject::connect(ui.intervalBButton, SIGNAL(clicked()), this, SLOT(setIntervalB()));
    QObject::connect(statusUpdateTimer, SIGNAL(timeout()), this, SLOT(statusUpdate()));
    QObject::connect(ui.speedBox, SIGNAL(valueChanged(double)), this, SLOT(setSpeedBox()));
    QObject::connect(ui.speedSlider, SIGNAL(sliderReleased()), this, SLOT(setSpeedSlider()));
    QObject::connect(ui.progressSlider, SIGNAL(sliderReleased()), this, SLOT(progressSliderUpdate()));
    QObject::connect(ui.progressSlider, SIGNAL(sliderPressed()), this, SLOT(handleProgressSliderPressed()));
    QObject::connect(checkFinishedTimer, SIGNAL(timeout()), this, SLOT(handleRestart()));
    QObject::connect(ui.intervalSlider, SIGNAL(sliderReleased()), this, SLOT(handleSpanSlider()));
    QObject::connect(ui.infoAbout, SIGNAL(triggered()), this, SLOT(showInfoAbout()));
    QObject::connect(ui.actionOpenLogfile, SIGNAL(triggered()), this, SLOT(showOpenFile()));
    QObject::connect(tasksModel, SIGNAL(itemChanged(QStandardItem *)), this, SLOT(handleItemChanged(QStandardItem *)));
}



ReplayGui::~ReplayGui()
{
    replayHandler.stop();
}

void ReplayGui::initReplayHandler(const QString &title)
{
    replayHandler.setReplayFactor(ui.speedBox->value());
    
    // progress bar
    ui.progressSlider->setMaximum(replayHandler.getMaxIndex());
    
    // labels
    ui.numSamplesLabel->setText(QString(("/ " + std::to_string(replayHandler.getMaxIndex())).c_str()));
    
    // span slider
    ui.intervalSlider->setHandleMovementMode(QxtSpanSlider::NoOverlapping);
    ui.intervalSlider->setMaximum(replayHandler.getMaxIndex());
    ui.intervalSlider->setSpan(0, replayHandler.getMaxIndex());
    ui.intervalSlider->setLowerPosition(0);
    ui.intervalSlider->setUpperPosition(replayHandler.getMaxIndex());
    
    
    // window title
    this->setWindowTitle(title);
    statusUpdate();
}


void ReplayGui::initReplayHandler(int argc, char* argv[])
{
    replayHandler.loadStreams(argc, argv);
    
    QString title;
    // window title
    switch(argc)
    {
        case 1:
            title = "Rock-Replay";
            break;
        case 2:
            title = QString(argv[1]);
            break;
        default:
            title = QString("Multi Logfile Replay");
            break;
    }        
    
    initReplayHandler(title);
}


int ReplayGui::boxToSlider(double val)
{
    if(val <= 1)
    {
        return val * 50;
    }
    else
    {
        return val / 2.0 + 50;
    }
}

double ReplayGui::sliderToBox(int val)
{
    if(val <= 50)
    {
        return val / 50.0;
    }
    else
    {
        return (val - 50) * 2.0;
    }
}

void ReplayGui::shiftAToB()
{
    int upper = ui.intervalSlider->upperPosition();
    ui.intervalSlider->setLowerPosition(upper + 1);
    ui.intervalSlider->setUpperPosition(ui.intervalSlider->maximum());
    replayHandler.setSpan(ui.intervalSlider->lowerPosition(), ui.intervalSlider->upperPosition());
    replayHandler.next();
    statusUpdate();
}



// #######################################
// ######### SLOT IMPLEMENTATION #########
// #######################################


void ReplayGui::setIntervalA()
{
    if(!replayHandler.isPlaying())
    {
        ui.intervalSlider->setLowerPosition(ui.progressSlider->sliderPosition());
        replayHandler.setSpan(ui.intervalSlider->lowerPosition(), ui.intervalSlider->upperPosition());
    }
}

void ReplayGui::setIntervalB()
{
    if(!replayHandler.isPlaying())
    {
        ui.intervalSlider->setUpperPosition(ui.progressSlider->sliderPosition());
        replayHandler.setSpan(ui.intervalSlider->lowerPosition(), ui.intervalSlider->upperPosition());
    }
}


void ReplayGui::togglePlay()
{
    if(!replayHandler.isPlaying())
    {
        checkFinishedTimer->start();
        replayHandler.play();
        changeGUIMode(PAUSED);
    }
    else
    {
        checkFinishedTimer->stop();
        replayHandler.pause();
        changeGUIMode(PLAYING);
    }
}


void ReplayGui::handleRestart()
{
    if(replayHandler.hasFinished())
    {
        stopPlay();
        statusUpdate();
        stoppedBySlider = false;
        checkFinishedTimer->stop();
        if(ui.repeatButton->isChecked())
        {
            checkFinishedTimer->start();
            togglePlay();
        }
    }
}



void ReplayGui::changeGUIMode(GUI_MODES mode)
{
    switch(mode)
    {
        case PLAYING:
            ui.playButton->setIcon(playIcon);
            ui.playButton->setChecked(false);
            statusUpdateTimer->stop();
            checkFinishedTimer->stop();
            ui.speedBar->setValue(0);
            ui.speedBar->setFormat("paused");
            stoppedBySlider = false;
            break;
        case PAUSED:
            ui.playButton->setChecked(true);
            ui.playButton->setIcon(pauseIcon);
            ui.speedBar->setFormat("%p%");
            statusUpdateTimer->start();
            checkFinishedTimer->start();
            break;
    }
}



void ReplayGui::statusUpdate()
{
    ui.speedBar->setValue(round(replayHandler.getCurrentSpeed() * ui.speedBar->maximum()));
    ui.curSampleNum->setText(QString::number(replayHandler.getCurIndex()));
    ui.curTimestamp->setText(replayHandler.getCurTimeStamp().c_str());
    ui.curPortName->setText(replayHandler.getCurSamplePortName().c_str());
    ui.progressSlider->setSliderPosition(replayHandler.getCurIndex());        
}

void ReplayGui::stopPlay()
{
    if(replayHandler.isValid())
    {
        replayHandler.stop();
        replayHandler.setReplayFactor(ui.speedBox->value()); // ensure that old replay speed is kept
        replayHandler.setSpan(ui.intervalSlider->lowerPosition(), ui.intervalSlider->upperPosition());
        changeGUIMode(PLAYING);
        statusUpdate();
    }
}


void ReplayGui::setSpeedBox()
{
    double speed = ui.speedBox->value();
    replayHandler.setReplayFactor(speed);
    ui.speedSlider->setValue(boxToSlider(speed));
}

void ReplayGui::setSpeedSlider()
{
    double speed = sliderToBox(ui.speedSlider->value());
    replayHandler.setReplayFactor(speed);
    ui.speedBox->setValue(speed);
}

void ReplayGui::backward()
{
    replayHandler.previous();
    statusUpdate();
}

void ReplayGui::forward()
{
    replayHandler.next();
    statusUpdate();
}

void ReplayGui::progressSliderUpdate()
{ 
    replayHandler.setSampleIndex(ui.progressSlider->value());
    statusUpdate();
    if(stoppedBySlider)
    {
        togglePlay();
        statusUpdateTimer->start();
    }
}

void ReplayGui::handleProgressSliderPressed()
{
    stoppedBySlider = replayHandler.isPlaying();
    stopPlay();
}

void ReplayGui::handleSpanSlider()
{
    stopPlay();
    replayHandler.setSpan(ui.intervalSlider->lowerPosition(), ui.intervalSlider->upperPosition());
   
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
















void ReplayGui::handleItemChanged(QStandardItem* item)
{
    TreeViewItem *taskItem = nullptr;
    if (!item->parent() || !(taskItem = dynamic_cast<TreeViewItem*>(item->parent())))
    {
        return;
    }
    
    const QModelIndex index = tasksModel->indexFromItem(item);
    QItemSelectionModel *selModel = ui.taskNameList->selectionModel();
    const std::string portName = item->text().toStdString();
    taskItem->getLogTask()->activateLoggingForPort(portName, item->checkState() == Qt::Checked);
    selModel->select(QItemSelection(index, index), item->checkState() == Qt::Checked ? QItemSelectionModel::Select : QItemSelectionModel::Deselect);
}



void ReplayGui::updateTaskView()
{
    while(tasksModel->rowCount() > 0)
    {
        QList<QStandardItem*> rows = tasksModel->takeRow(0);
        for(QStandardItem *item : rows)
            delete item;
    }
    
    for(const auto& nameToTask : replayHandler.getAllLogTasks())
    {        
//         TreeViewItem *task = new TreeViewItem(nameToTask.second, nameToTask.first);
//         task->setCheckable(false);
//         
//         for(const std::string& portName : nameToTask.second->getTaskContext()->ports()->getPortNames())
//         {
//             QStandardItem *port = new QStandardItem(portName.c_str());
//             port->setCheckable(true);
//             port->setData(Qt::Checked, Qt::CheckStateRole);
//             task->appendRow(port);
//             //portTypes.append(new QStandardItem(QString(cur.second->getTaskContext()->getPort(portName)->getTypeInfo()->getTypeName().c_str())));
//         }
//      
//         tasksModel->appendRow(task);
    }
}


