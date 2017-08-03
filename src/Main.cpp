#include "Main.hpp"
#include <typelib/pluginmanager.hh>



SlotWrapper::SlotWrapper(Ui::MainWindow& ui, QStandardItemModel *tasksModel)
    : ui(ui)
    , tasksModel(tasksModel)
{
}

void SlotWrapper::handleItemChanged(QStandardItem *item)
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


void updateTaskView(QStandardItemModel *tasksModel, ReplayHandler *replayHandler)
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


    
int main(int argc, char *argv[])
{
    Typelib::PluginManager::self manager;
    QApplication a(argc, argv);
    ReplayGui gui(&updateTaskView);
    SlotWrapper wrapper(gui.getMainWindow(), gui.getTasksModel());
    QObject::connect(gui.getTasksModel(), SIGNAL(itemChanged(QStandardItem *)), &wrapper, SLOT(handleItemChanged(QStandardItem *)));
    gui.initReplayHandler(argc, argv);
    gui.updateTaskView();
    
    gui.show();    
    return a.exec();
}
