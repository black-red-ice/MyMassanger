#pragma once
#include "../SidePanel.h"
#include <QCalendarWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QPushButton>
#include <QDate>
#include <QMap>
#include <QDateTime>

struct CalendarEvent {
    QString title;
    QString description;
    QDateTime dateTime;
    QString duration;
    QString eventType;
    QString color;
    QString participants;
};

class CalendarPanel : public SidePanel
{
    Q_OBJECT

public:
    explicit CalendarPanel(QWidget *parent = nullptr);

protected:
    void onAddClicked() override;
    bool eventFilter(QObject *obj, QEvent *event) override;

private slots:
    void onDateSelected(const QDate &date);
    void onNewEvent();
    void onEventClicked(int index, const QDate &date);

private:
    void setupCalendar();
    void updateEventsList(const QDate &date);

    QCalendarWidget *m_calendar;
    QLabel *m_selectedDateLabel;
    QVBoxLayout *m_eventsLayout;
    QWidget *m_eventsContainer;

    QMap<QDate, QList<CalendarEvent>> m_events;
};
