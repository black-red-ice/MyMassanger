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
                               const QString &participants,
                               QWidget *parent = nullptr);

signals:
    void editRequested();

private slots:
    void onEditClicked();
};
