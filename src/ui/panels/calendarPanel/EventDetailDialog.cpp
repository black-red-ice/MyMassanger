#include "EventDetailDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>

EventDetailDialog::EventDetailDialog(const QString &title,
                                     const QString &description,
                                     const QDateTime &dateTime,
                                     const QString &duration,
                                     const QString &eventType,
                                     const QString &participants,
                                     QWidget *parent)
    : OverlayDialog(parent)
{
    setFixedSize(540, 560);
    setStyleSheet("background: transparent;");

    QWidget *container = new QWidget(this);
    container->setObjectName("eventDetailContainer");
    container->setStyleSheet(
        "#eventDetailContainer { background: #1e293b; border-radius: 18px; border: 1px solid #334155; }"
        );

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(container);

    QVBoxLayout *containerLayout = new QVBoxLayout(container);
    containerLayout->setContentsMargins(0, 0, 0, 0);
    containerLayout->setSpacing(0);

    // Заголовок (всегда бирюзовый)
    QWidget *header = new QWidget();
    header->setStyleSheet(
        "background: #059669; border-top-left-radius: 18px; border-top-right-radius: 18px;"
        );
    header->setFixedHeight(60);
    QHBoxLayout *headerLayout = new QHBoxLayout(header);
    headerLayout->setContentsMargins(24, 0, 24, 0);

    QLabel *titleLabel = new QLabel("Детали события");
    titleLabel->setStyleSheet("color: white; font-size: 18px; font-weight: 600; background: transparent;");

    QPushButton *closeBtn = new QPushButton("✕");
    closeBtn->setFixedSize(36, 36);
    closeBtn->setCursor(Qt::PointingHandCursor);
    closeBtn->setStyleSheet(
        "QPushButton { background: rgba(255,255,255,0.1); border: none; border-radius: 18px; color: white; font-size: 16px; }"
        "QPushButton:hover { background: rgba(255,255,255,0.2); }"
        );
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::reject);

    headerLayout->addWidget(titleLabel);
    headerLayout->addStretch();
    headerLayout->addWidget(closeBtn);
    containerLayout->addWidget(header);

    // Контент
    QWidget *content = new QWidget();
    content->setStyleSheet("background: #1e293b; border-bottom-left-radius: 18px; border-bottom-right-radius: 18px;");
    QVBoxLayout *contentLayout = new QVBoxLayout(content);
    contentLayout->setContentsMargins(24, 24, 24, 24);
    contentLayout->setSpacing(16);

    auto addField = [&](const QString &label, const QString &value) {
        QLabel *lbl = new QLabel(label);
        lbl->setStyleSheet("color: #94A3B8; font-size: 14px; font-weight: 500; background: transparent;");
        contentLayout->addWidget(lbl);

        QLabel *val = new QLabel(value.isEmpty() ? "—" : value);
        val->setStyleSheet("color: #f1f5f9; font-size: 14px; font-weight: 600; background: transparent; padding: 8px 0;");
        val->setWordWrap(true);
        contentLayout->addWidget(val);
    };

    addField("Название события", title);
    addField("Дата и время", dateTime.toString("dd.MM.yyyy hh:mm"));
    addField("Продолжительность", duration);
    addField("Тип события", eventType);
    addField("Описание", description);
    addField("Участники", participants);

    contentLayout->addStretch();

    // Кнопки в горизонтальном ряду
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(12);

    // Кнопка "Закрыть"
    QPushButton *closeButton = new QPushButton("Закрыть");
    closeButton->setFixedHeight(44);
    closeButton->setCursor(Qt::PointingHandCursor);
    closeButton->setStyleSheet(
        "QPushButton { background: #334155; border: none; border-radius: 10px; color: #cbd5e1; font-size: 14px; font-weight: 500; }"
        "QPushButton:hover { background: #475569; }"
        );
    connect(closeButton, &QPushButton::clicked, this, &QDialog::reject);

    // Кнопка "Редактировать"
    QPushButton *editBtn = new QPushButton("Редактировать");
    editBtn->setFixedHeight(44);
    editBtn->setCursor(Qt::PointingHandCursor);
    editBtn->setStyleSheet(
        "QPushButton { background: #059669; border: none; border-radius: 10px; color: white; font-size: 14px; font-weight: 600; }"
        "QPushButton:hover { background: #047857; }"
        );
    connect(editBtn, &QPushButton::clicked, this, &EventDetailDialog::onEditClicked);

    buttonLayout->addWidget(closeButton);
    buttonLayout->addWidget(editBtn);
    contentLayout->addLayout(buttonLayout);

    containerLayout->addWidget(content);
}

void EventDetailDialog::onEditClicked()
{
    emit editRequested();
    accept();
}
