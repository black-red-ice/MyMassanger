#pragma once
#include "overlaydialog.h"
#include "Task.h"

class QLineEdit;
class QTextEdit;
class QDateTimeEdit;
class QComboBox;
class QPushButton;

class TaskDialog : public OverlayDialog
{
    Q_OBJECT

public:
    explicit TaskDialog(QWidget *parent = nullptr);

    Task getTask() const;
    void setTask(const Task &task);  // Добавить

protected:
    void keyPressEvent(QKeyEvent *event) override;

private:
    QLineEdit *m_title;
    QTextEdit *m_description;
    QDateTimeEdit *m_deadline;
    QComboBox *m_priority;
    QLineEdit *m_tags;
    QPushButton *m_saveBtn;

    bool m_editMode = false;  // Добавить

signals:
    void taskCreated(const Task& task);
};
