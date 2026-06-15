#pragma once
#include "../SidePanel.h"
#include "CalendarEvent.h"
#include <QCalendarWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QPushButton>
#include <QDate>
#include <QMap>

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

private:
    void setupCalendar();
    void updateEventsList(const QDate &date);
    void onEventClicked(const CalendarEvent &event);

    QCalendarWidget *m_calendar;
    QLabel *m_selectedDateLabel;
    QVBoxLayout *m_eventsLayout;
    QWidget *m_eventsContainer;

    QMap<QDate, QList<CalendarEvent>> m_events;
};
