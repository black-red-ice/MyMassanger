#pragma once
#include "overlaydialog.h"

class QLineEdit;
class QTextEdit;
class QDateTimeEdit;
class QComboBox;

class EventDialog : public OverlayDialog
{
    Q_OBJECT

protected:
    void keyPressEvent(QKeyEvent *event) override;

public:
    explicit EventDialog(QWidget *parent = nullptr);

    QString getTitle() const;
    QString getDescription() const;
    QDateTime getDateTime() const;
    QString getDuration() const;
    QString getEventType() const;
    QString getColor() const;
    QString getParticipants() const;

private:
    QLineEdit *m_title;
    QTextEdit *m_description;
    QDateTimeEdit *m_dateTime;
    QComboBox *m_duration;
    QComboBox *m_eventType;
    QLineEdit *m_participants;
    QString m_selectedColor;
    void setupDateTimeEdit();
};
