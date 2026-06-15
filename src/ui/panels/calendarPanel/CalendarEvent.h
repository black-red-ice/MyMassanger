#pragma once
#include <QString>
#include <QDateTime>

struct CalendarEvent {
    QString id;
    QString title;
    QString description;
    QDateTime dateTime;
    QString duration;
    QString eventType;
    QString color;
    QString participants;
};
