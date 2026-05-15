#pragma once
#include <QWidget>
#include "Task.h"

class QLabel;
class QPushButton;

class TaskItemWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TaskItemWidget(const Task &task, QWidget *parent = nullptr);
    Task getTask() const { return m_task; }  // Добавить

signals:
    void toggled();
    void deleted();
    void editRequested();  // Добавить

protected:
    void paintEvent(QPaintEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;  // Добавить

private:
    Task m_task;
};
