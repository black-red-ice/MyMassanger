#include "SupportCard.h"

#include <QVBoxLayout>
#include <QPainter>
#include <QPixmap>
#include <QMouseEvent>

SupportCard::SupportCard(const QString &iconPath,
                         const QString &title,
                         const QString &desc,
                         const QString &color,
                         const QString &categoryKey,
                         QWidget *parent)
    : QWidget(parent), m_categoryKey(categoryKey)
{
    setCursor(Qt::PointingHandCursor);

    setStyleSheet(QString(
                      "QWidget {"
                      " background-color: #1e293b;"
                      " border-radius: 12px;"
                      " border: 1px solid rgba(29,78,216,0.2);"
                      "}"
                      "QWidget:hover { border: 1px solid %1; }"
                      ).arg(color));

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(20,20,20,20);
    layout->setSpacing(8);

    QLabel *iconLabel = new QLabel();
    QPixmap pix(iconPath);
    iconLabel->setPixmap(pix.scaled(32,32, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    iconLabel->setAlignment(Qt::AlignCenter);

    QLabel *titleLabel = new QLabel(title);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("color:white;font-weight:600;");

    QLabel *descLabel = new QLabel(desc);
    descLabel->setAlignment(Qt::AlignCenter);
    descLabel->setWordWrap(true);
    descLabel->setStyleSheet("color:#94a3b8;font-size:12px;");

    layout->addWidget(iconLabel);
    layout->addWidget(titleLabel);
    layout->addWidget(descLabel);
}

void SupportCard::mousePressEvent(QMouseEvent *event)
{
    emit clicked(m_categoryKey);
    QWidget::mousePressEvent(event);
}
