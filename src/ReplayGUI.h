#include "ReplayGUIBase.h"

class ReplayGui : public ReplayGuiBase
{

public:
    ReplayGui();
    ~ReplayGui() = default;
    
    void updateTaskView() override;
    void handleItemChanged(QStandardItem *item) override;
    
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
