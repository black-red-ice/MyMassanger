#pragma once
#include "overlaydialog.h"
#include <QDateTime>

class EventDetailDialog : public OverlayDialog
{
    Q_OBJECT

public:
    explicit EventDetailDialog(const QString &title,
                               const QString &description,
                               const QDateTime &dateTime,
                               const QString &duration,
                               const QString &eventType,
                               const QString &color,
                               const QString &participants,
                               QWidget *parent = nullptr);

signals:
    void editRequested();  // Сигнал для редактирования

private slots:
    void onEditClicked();  // Слот для кнопки редактирования

private:
    QString m_title;
    QString m_description;
    QDateTime m_dateTime;
    QString m_duration;
    QString m_eventType;
    QString m_color;
    QString m_participants;
};
