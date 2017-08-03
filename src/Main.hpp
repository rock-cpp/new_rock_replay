#pragma once

#include "ReplayGUI.h"
#include <QStandardItemModel>
#include <QApplication>

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



class SlotWrapper : public QObject
{
    Q_OBJECT
    
private:
    Ui::MainWindow &ui;
    QStandardItemModel *tasksModel;
    
public:
    SlotWrapper(Ui::MainWindow& ui, QStandardItemModel *tasksModel);
    ~SlotWrapper() = default;
    
public slots:
    void handleItemChanged(QStandardItem *item);
};
    
