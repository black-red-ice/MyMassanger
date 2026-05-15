#include "ClientDetailDialog.h"
#include "../panels/dealPanel/DealDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QFrame>
#include <QPixmap>
#include <QDialog>
#include <QKeyEvent>
#include <QDebug>
#include <QTimer>

ClientDetailDialog::ClientDetailDialog(const Deal &deal, QWidget *parent)
    : OverlayDialog(parent), m_deal(deal)
{
    setFixedSize(480, 520);
    setStyleSheet("background: transparent;");

    QWidget *container = new QWidget(this);
    container->setObjectName("clientDetailContainer");
    container->setStyleSheet(
        "#clientDetailContainer {"
        "  background: #1e293b;"
        "  border-radius: 18px;"
        "  border: 1px solid #334155;"
        "}"
        );

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(container);

    QVBoxLayout *containerLayout = new QVBoxLayout(container);
    containerLayout->setContentsMargins(0, 0, 0, 0);
    containerLayout->setSpacing(0);

    // Заголовок
    QWidget *header = new QWidget(container);
    header->setStyleSheet(
        "background: #8146f0;"
        "border-top-left-radius: 18px;"
        "border-top-right-radius: 18px;"
        );
    header->setFixedHeight(60);
    QHBoxLayout *headerLayout = new QHBoxLayout(header);
    headerLayout->setContentsMargins(24, 0, 24, 0);

    QLabel *iconLabel = new QLabel();
    QPixmap icon(":/icons/darkTheme/images/darkTheme/user-tie-light.svg");
    iconLabel->setPixmap(icon.scaled(22, 22, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    iconLabel->setStyleSheet("background: transparent;");
    headerLayout->addWidget(iconLabel);

    QLabel *titleLabel = new QLabel("Карточка клиента");
    titleLabel->setStyleSheet("color: white; font-weight: 600; font-size: 18px; background: transparent; margin-left: 8px;");
    headerLayout->addWidget(titleLabel);
    headerLayout->addStretch();

    QPushButton *closeBtn = new QPushButton("✕");
    closeBtn->setFixedSize(36, 36);
    closeBtn->setCursor(Qt::PointingHandCursor);
    closeBtn->setStyleSheet(
        "QPushButton { background: rgba(255,255,255,0.1); border: none; border-radius: 18px; color: white; font-size: 16px; }"
        "QPushButton:hover { background: rgba(255,255,255,0.2); }"
        );
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::reject);

    headerLayout->addWidget(closeBtn);
    containerLayout->addWidget(header);

    // Контент
    QWidget *content = new QWidget(container);
    content->setStyleSheet(
        "background: #1e293b;"
        "border-bottom-left-radius: 18px;"
        "border-bottom-right-radius: 18px;"
        );
    QVBoxLayout *contentLayout = new QVBoxLayout(content);
    contentLayout->setContentsMargins(24, 24, 24, 24);
    contentLayout->setSpacing(16);

    auto addDetailRow = [&](const QString &label, const QString &value) {
        QWidget *row = new QWidget();
        QHBoxLayout *rowLayout = new QHBoxLayout(row);
        rowLayout->setContentsMargins(0, 0, 0, 0);
        rowLayout->setSpacing(12);

        QLabel *labelWidget = new QLabel(label);
        labelWidget->setStyleSheet("color: #94A3B8; font-size: 14px; font-weight: 500; background: transparent;");
        labelWidget->setFixedWidth(120);

        QLabel *valueWidget = new QLabel(value);
        valueWidget->setStyleSheet("color: #f1f5f9; font-size: 14px; font-weight: 600; background: transparent;");
        valueWidget->setWordWrap(true);

        rowLayout->addWidget(labelWidget);
        rowLayout->addWidget(valueWidget, 1);

        contentLayout->addWidget(row);
    };

    addDetailRow("Название:", deal.name);

    QFrame *sep1 = new QFrame();
    sep1->setFrameShape(QFrame::HLine);
    sep1->setStyleSheet("color: #334155;");
    contentLayout->addWidget(sep1);

    addDetailRow("Телефон:", deal.phone);
    addDetailRow("Email:", deal.email.isEmpty() ? "не указан" : deal.email);

    QFrame *sep2 = new QFrame();
    sep2->setFrameShape(QFrame::HLine);
    sep2->setStyleSheet("color: #334155;");
    contentLayout->addWidget(sep2);

    addDetailRow("ИНН:", deal.inn);
    addDetailRow("Отсрочка:", QString::number(deal.delayDays) + " дней");
    addDetailRow("Гараж:", deal.extra.isEmpty() ? "не указан" : deal.extra);

    QFrame *sep3 = new QFrame();
    sep3->setFrameShape(QFrame::HLine);
    sep3->setStyleSheet("color: #334155;");
    contentLayout->addWidget(sep3);

    QString stageText;
    switch (deal.stage) {
    case DealStage::Contact:  stageText = "📞 Связаться"; break;
    case DealStage::Meeting:  stageText = "🤝 Встреча"; break;
    case DealStage::Contract: stageText = "✍️ Подписание"; break;
    case DealStage::Won:      stageText = "💰 Продажа"; break;
    case DealStage::Lost:     stageText = "❌ Отказ"; break;
    }

    QWidget *stageRow = new QWidget();
    QHBoxLayout *stageRowLayout = new QHBoxLayout(stageRow);
    stageRowLayout->setContentsMargins(0, 0, 0, 0);
    stageRowLayout->setSpacing(12);

    QLabel *stageLabel = new QLabel("Стадия сделки:");
    stageLabel->setStyleSheet("color: #94A3B8; font-size: 14px; font-weight: 500; background: transparent;");
    stageLabel->setFixedWidth(120);

    QLabel *stageValue = new QLabel(stageText);
    stageValue->setStyleSheet(
        "color: #94A3B8;"
        "font-size: 14px;"
        "font-weight: 600;"
        "background: transparent;"
        "padding: 4px 0px;"
        );

    stageRowLayout->addWidget(stageLabel);
    stageRowLayout->addWidget(stageValue, 1);

    contentLayout->addWidget(stageRow);
    contentLayout->addSpacing(16);

    // Кнопки
    QWidget *buttonRow = new QWidget();
    QHBoxLayout *buttonLayout = new QHBoxLayout(buttonRow);
    buttonLayout->setContentsMargins(0, 0, 0, 0);
    buttonLayout->setSpacing(12);

    QPushButton *closeButton = new QPushButton("Закрыть");
    closeButton->setCursor(Qt::PointingHandCursor);
    closeButton->setFixedHeight(44);
    closeButton->setStyleSheet(
        "QPushButton { background: #334155; border: none; border-radius: 10px; color: #cbd5e1; font-size: 14px; font-weight: 500; }"
        "QPushButton:hover { background: #475569; }"
        );
    closeButton->setAutoDefault(false);
    closeButton->setDefault(false);
    closeButton->setFocusPolicy(Qt::NoFocus);
    connect(closeButton, &QPushButton::clicked, this, &QDialog::reject);

    QPushButton *editButton = new QPushButton("Редактировать");
    editButton->setCursor(Qt::PointingHandCursor);
    editButton->setFixedHeight(44);
    editButton->setStyleSheet(
        "QPushButton { background: #0EA5E9; border: none; border-radius: 10px; color: white; font-size: 14px; font-weight: 600; }"
        "QPushButton:hover { background: #1d4ed8; }"
        );
    editButton->setAutoDefault(false);
    editButton->setDefault(false);
    editButton->setFocusPolicy(Qt::NoFocus);
    connect(editButton, &QPushButton::clicked, this, [this]() {
        m_editRequested = true;
        reject();
    });

    buttonLayout->addWidget(closeButton, 1);
    buttonLayout->addWidget(editButton, 1);
    contentLayout->addWidget(buttonRow);
    containerLayout->addWidget(content);

    // В самом конце конструктора:
    QTimer::singleShot(0, this, [this]() {
        // Находим все кнопки и отключаем их
        QList<QPushButton*> buttons = findChildren<QPushButton*>();
        for (QPushButton* btn : buttons) {
            btn->setAutoDefault(false);
            btn->setDefault(false);
            btn->setFocusPolicy(Qt::NoFocus);
        }
        // Убираем фокус с диалога
        setFocusPolicy(Qt::NoFocus);
        // Устанавливаем фокус на некликабельный виджет
        if (!children().isEmpty()) {
            QWidget* firstChild = qobject_cast<QWidget*>(children().first());
            if (firstChild) {
                firstChild->setFocus();
            }
        }
    });
}

bool ClientDetailDialog::event(QEvent *e)
{
    if (e->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(e);
        if (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter) {
            return true;  // Блокируем Enter
        }
        if (keyEvent->key() == Qt::Key_Escape) {
            reject();
            return true;
        }
    }
    return QDialog::event(e);
}
