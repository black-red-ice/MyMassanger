#include "TasksPanel.h"
#include "TaskDialog.h"
#include "TaskItemWidget.h"
#include "TaskStorage.h"
#include "EditTaskDialog.h"
#include <QVBoxLayout>
#include <QUuid>
#include <QTimer>

TasksPanel::TasksPanel(QWidget *parent)
    : SidePanel(parent,
                "Мои задачи",
                ":/icons/general/images/general/list-check-light.svg",
                "#0EA5E9",
                "Создать новую задачу",
                360,
                "#0EA5E9",
                ":/icons/general/images/general/circle-plus.svg")  // ← иконка кнопки
{
    // Убираем стандартный stretch
    QLayoutItem *stretch = getContentLayout()->takeAt(getContentLayout()->count() - 1);
    delete stretch;

    m_taskListLayout = new QVBoxLayout();
    m_taskListLayout->setSpacing(12);
    getContentLayout()->addLayout(m_taskListLayout);
    getContentLayout()->addStretch();

    m_tasks = TaskStorage::load();
    setSearchVisible(false);
    renderTasks();
}

void TasksPanel::onAddClicked()
{
    qDebug() << "=== onAddClicked START ===";

    TaskDialog dialog(this->window());
    dialog.setWindowModality(Qt::ApplicationModal);

    if (dialog.exec() == QDialog::Accepted) {
        Task t = dialog.getTask();
        t.id = QUuid::createUuid().toString();
        t.status = TaskStatus::Todo;
        m_tasks.append(t);
        renderTasks();
        TaskStorage::save(m_tasks);
    }

    qDebug() << "=== onAddClicked END ===";
}

void TasksPanel::renderTasks()
{
    qDebug() << "renderTasks START, count:" << m_tasks.size();
    clearLayouts();
    qDebug() << "clearLayouts done";

    for (const Task &t : qAsConst(m_tasks)) {
        qDebug() << "Creating TaskItemWidget for:" << t.title;
        TaskItemWidget *item = new TaskItemWidget(t);
        qDebug() << "TaskItemWidget created";

        connect(item, &TaskItemWidget::deleted, this, [=]() { removeTask(t.id); });
        connect(item, &TaskItemWidget::toggled, this, [=]() {
            for (Task &stored : m_tasks) {
                if (stored.id == t.id) {
                    stored.status = stored.status == TaskStatus::Done ? TaskStatus::Todo : TaskStatus::Done;
                    TaskStorage::save(m_tasks);
                    break;
                }
            }
        });
        connect(item, &TaskItemWidget::editRequested, this, [=]() { editTask(t.id); });

        qDebug() << "Adding to layout";
        m_taskListLayout->addWidget(item);
        qDebug() << "Added to layout";
    }
    qDebug() << "renderTasks END";
}

void TasksPanel::clearLayouts()
{
    while (QLayoutItem *item = m_taskListLayout->takeAt(0)) {
        if (item->widget()) delete item->widget();
        delete item;
    }
}

void TasksPanel::removeTask(const QString &id)
{
    for (int i = 0; i < m_tasks.size(); ++i) {
        if (m_tasks[i].id == id) {
            m_tasks.removeAt(i);
            break;
        }
    }
    renderTasks();
    TaskStorage::save(m_tasks);
}

void TasksPanel::editTask(const QString &id)
{
    qDebug() << "=== editTask START, id:" << id;

    Task *taskToEdit = nullptr;
    for (Task &t : m_tasks) {
        if (t.id == id) {
            taskToEdit = &t;
            break;
        }
    }

    if (!taskToEdit) {
        qDebug() << "Task not found!";
        return;
    }

    qDebug() << "Task found:" << taskToEdit->title;

    // Используем стековый объект вместо new
    EditTaskDialog dialog(*taskToEdit, this->window());
    dialog.setWindowModality(Qt::ApplicationModal);

    if (dialog.exec() == QDialog::Accepted) {
        Task updated = dialog.getTask();
        taskToEdit->title = updated.title;
        taskToEdit->description = updated.description;
        taskToEdit->tags = updated.tags;
        taskToEdit->deadline = updated.deadline;
        taskToEdit->priority = updated.priority;

        TaskStorage::save(m_tasks);

        // Отложенный renderTasks
        QTimer::singleShot(50, this, [this]() {
            renderTasks();
        });
    }


    qDebug() << "=== editTask END ===";
}
