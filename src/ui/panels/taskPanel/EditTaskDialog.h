#pragma once
#include "overlaydialog.h"
#include "Task.h"

class QLineEdit;
class QTextEdit;
class QDateTimeEdit;
class QComboBox;

class EditTaskDialog : public OverlayDialog
{
    Q_OBJECT

public:
    explicit EditTaskDialog(const Task &task, QWidget *parent = nullptr);
    Task getTask() const;

protected:
    void keyPressEvent(QKeyEvent *event) override;

private:
    QLineEdit *m_title;
    QTextEdit *m_description;
    QDateTimeEdit *m_deadline;
    QComboBox *m_priority;
    QLineEdit *m_tags;
};
