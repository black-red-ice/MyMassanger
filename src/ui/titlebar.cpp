#include "titlebar.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QMouseEvent>
#include <QIcon>
#include <QPixmap>

TitleBar::TitleBar(QWidget *parent) : QWidget(parent)
{
    setFixedHeight(32);
    setStyleSheet("background-color: #1e293b;");

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(12, 0, 8, 0);
    layout->setSpacing(8);

    // Логотип
    QLabel *logoText = new QLabel("Aura");
    logoText->setStyleSheet("color: #f1f5f9; font-size: 14px; font-weight: bold; background-color: transparent;");

    // Кнопка сворачивания
    QPushButton *minimizeBtn = new QPushButton();
    minimizeBtn->setFixedSize(28, 24);
    minimizeBtn->setCursor(Qt::PointingHandCursor);
    minimizeBtn->setIcon(QIcon(":/icons/general/images/general/window-minimize.svg"));
    minimizeBtn->setIconSize(QSize(14, 14));
    minimizeBtn->setStyleSheet(
        "QPushButton { background-color: transparent; border: none; border-radius: 4px; padding-bottom: 5px; }"
        "QPushButton:hover { background-color: #334155; }"
        );
    connect(minimizeBtn, &QPushButton::clicked, this, &TitleBar::minimizeClicked);

    // Кнопка разворачивания
    QPushButton *maximizeBtn = new QPushButton();
    maximizeBtn->setFixedSize(28, 24);
    maximizeBtn->setCursor(Qt::PointingHandCursor);
    maximizeBtn->setIcon(QIcon(":/icons/general/images/general/window-maximize.svg"));
    maximizeBtn->setIconSize(QSize(17, 17));
    maximizeBtn->setStyleSheet(
        "QPushButton { background-color: transparent; border: none; border-radius: 4px; }"
        "QPushButton:hover { background-color: #334155; }"
        );
    connect(maximizeBtn, &QPushButton::clicked, this, &TitleBar::maximizeClicked);

    // Кнопка закрытия
    QPushButton *closeBtn = new QPushButton();
    closeBtn->setFixedSize(28, 24);
    closeBtn->setCursor(Qt::PointingHandCursor);
    closeBtn->setIcon(QIcon(":/icons/general/images/general/xmark.svg"));
    closeBtn->setIconSize(QSize(14, 14));
    closeBtn->setStyleSheet(
        "QPushButton { background-color: transparent; border: none; border-radius: 4px; }"
        "QPushButton:hover { background-color: #ef4444; }"
        );
    connect(closeBtn, &QPushButton::clicked, this, &TitleBar::closeClicked);

    layout->addWidget(logoText);
    layout->addStretch();
    layout->addWidget(minimizeBtn);
    layout->addWidget(maximizeBtn);
    layout->addWidget(closeBtn);
}

void TitleBar::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_dragging = true;
        m_dragPosition = event->globalPosition().toPoint() - parentWidget()->frameGeometry().topLeft();
    }
    QWidget::mousePressEvent(event);
}

void TitleBar::mouseMoveEvent(QMouseEvent *event)
{
    if (m_dragging && (event->buttons() & Qt::LeftButton)) {
        parentWidget()->move(event->globalPosition().toPoint() - m_dragPosition);
    }
    QWidget::mouseMoveEvent(event);
}

void TitleBar::mouseReleaseEvent(QMouseEvent *event)
{
    m_dragging = false;
    QWidget::mouseReleaseEvent(event);
}
