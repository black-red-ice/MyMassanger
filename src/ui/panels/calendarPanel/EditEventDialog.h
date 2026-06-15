#pragma once
#include "overlaydialog.h"
#include "CalendarEvent.h"
#include <QDateTime>

class QLineEdit;
class QTextEdit;
class QDateTimeEdit;
class QComboBox;

class EditEventDialog : public OverlayDialog
{
    Q_OBJECT

public:
    explicit EditEventDialog(const CalendarEvent &event, QWidget *parent = nullptr);

    CalendarEvent getEvent() const;

protected:
    void keyPressEvent(QKeyEvent *event) override;

private:
    void setupUI();
    void updateColorButtons();

    QLineEdit *m_title;
    QTextEdit *m_description;
    QDateTimeEdit *m_dateTime;
    QComboBox *m_duration;
    QComboBox *m_eventType;
    QLineEdit *m_participants;
    QString m_selectedColor;
    QList<QPushButton*> m_colorButtons;

    CalendarEvent m_originalEvent;
};
