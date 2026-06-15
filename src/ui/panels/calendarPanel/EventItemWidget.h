#pragma once
#include <QWidget>
#include "CalendarEvent.h"

class QLabel;
class QTextBrowser;
class QPushButton;

class EventItemWidget : public QWidget
{
    Q_OBJECT

public:
    explicit EventItemWidget(const CalendarEvent &event, QWidget *parent = nullptr);
    CalendarEvent getEvent() const { return m_event; }

signals:
    void clicked();
    void deleted();

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void updateHeight();

private:
    CalendarEvent m_event;
    QTextBrowser *m_titleLabel;
    QTextBrowser *m_descriptionLabel;
    QLabel *m_timeLabel;
    QLabel *m_participantsLabel;
    QLabel *m_typeLabel;
    QPushButton *m_deleteBtn;
};
