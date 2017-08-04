#include "ReplayGUI.h"


ReplayGui::ReplayGui()
    : ReplayGuiBase()
{    
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
    
    for(const std::pair<std::string, LogTask*>& cur : replayHandler->getAllLogTasks())
    {        
        TreeViewItem *task = new TreeViewItem(cur.second, cur.first);
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


