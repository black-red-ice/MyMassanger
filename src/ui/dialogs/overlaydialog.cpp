#include "overlaydialog.h"
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QShowEvent>
#include <QCloseEvent>
#include <QScreen>
#include <QApplication>

OverlayDialog::OverlayDialog(QWidget *parent) : QDialog(parent)
{
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setModal(true);

    m_effect = new QGraphicsOpacityEffect(this);
    m_effect->setOpacity(0.0);
    setGraphicsEffect(m_effect);

    m_animation = new QPropertyAnimation(m_effect, "opacity", this);
    m_animation->setDuration(200);
    m_animation->setEasingCurve(QEasingCurve::InOutQuad);
}

OverlayDialog::~OverlayDialog() = default;

void OverlayDialog::setOpacity(double opacity)
{
    m_opacity = opacity;
    if (m_effect) {
        m_effect->setOpacity(opacity);
    }
}

void OverlayDialog::showEvent(QShowEvent *event)
{
    QDialog::showEvent(event);

    // Центрируем окно при показе
    if (parentWidget()) {
        QRect parentGeometry = parentWidget()->geometry();
        int x = parentGeometry.x() + (parentGeometry.width() - width()) / 2;
        int y = parentGeometry.y() + (parentGeometry.height() - height()) / 2;
        move(x, y);
    } else {
        QRect screenGeometry = QApplication::primaryScreen()->geometry();
        int x = (screenGeometry.width() - width()) / 2;
        int y = (screenGeometry.height() - height()) / 2;
        move(x, y);
    }

    startFadeIn();
}

void OverlayDialog::closeEvent(QCloseEvent *event)
{
    startFadeOut();
    event->accept();
}

void OverlayDialog::startFadeIn()
{
    if (m_animation) {
        m_animation->stop();
        m_animation->setStartValue(0.0);
        m_animation->setEndValue(1.0);
        m_animation->start();
    }
}

void OverlayDialog::startFadeOut()
{
    if (m_animation) {
        m_animation->stop();
        m_animation->setStartValue(1.0);
        m_animation->setEndValue(0.0);
        m_animation->start();
        connect(m_animation, &QPropertyAnimation::finished, this, &QDialog::close);
    } else {
        QDialog::close();
    }
}
