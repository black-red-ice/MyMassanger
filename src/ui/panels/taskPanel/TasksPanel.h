#pragma once
#include "../SidePanel.h"
#include "Task.h"

class QVBoxLayout;

class TasksPanel : public SidePanel
{
    Q_OBJECT

public:
    explicit TasksPanel(QWidget *parent = nullptr);

protected:
    void onAddClicked() override;

private slots:
    void removeTask(const QString &id);
    void editTask(const QString &id);

private:
    QList<Task> m_tasks;
    QVBoxLayout *m_taskListLayout;

    void renderTasks();
    void clearLayouts();
};
