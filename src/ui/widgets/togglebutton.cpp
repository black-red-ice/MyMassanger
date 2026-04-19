#include "togglebutton.h"
#include <QHBoxLayout>
#include <QMouseEvent>

ToggleButton::ToggleButton(bool checked, QWidget *parent)
    : QWidget(parent), m_checked(checked)
{
    setFixedSize(44, 24);
    setCursor(Qt::PointingHandCursor);

    m_track = new QWidget(this);
    m_track->setFixedSize(44, 24);
    m_track->setStyleSheet(m_checked ? "background-color: #1d4ed8; border-radius: 12px;" : "background-color: #475569; border-radius: 12px;");

    m_knob = new QWidget(this);
    m_knob->setFixedSize(20, 20);
    m_knob->setStyleSheet("background-color: white; border-radius: 10px;");
    m_knob->move(m_checked ? 22 : 2, 2);

    m_animation = new QPropertyAnimation(m_knob, "pos", this);
    m_animation->setDuration(150);
    m_animation->setEasingCurve(QEasingCurve::InOutQuad);
}

void ToggleButton::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_checked = !m_checked;
        updateVisuals();
        emit toggled(m_checked);
    }
    QWidget::mousePressEvent(event);
}

void ToggleButton::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event)
    updateVisuals();
}

void ToggleButton::updateVisuals()
{
    m_track->setStyleSheet(m_checked ? "background-color: #1d4ed8; border-radius: 12px;" : "background-color: #475569; border-radius: 12px;");
    m_animation->stop();
    m_animation->setEndValue(m_checked ? QPoint(22, 2) : QPoint(2, 2));
    m_animation->start();
}
