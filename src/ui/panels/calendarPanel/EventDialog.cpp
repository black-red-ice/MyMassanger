#include "EventDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QTextEdit>
#include <QDateTimeEdit>
#include <QComboBox>
#include <QPushButton>
#include <QKeyEvent>

EventDialog::EventDialog(QWidget *parent)
    : OverlayDialog(parent), m_selectedColor("#1d4ed8")
{
    setFixedSize(540, 760);
    setStyleSheet("background: transparent;");

    QWidget *container = new QWidget(this);
    container->setObjectName("eventDialogContainer");
    container->setStyleSheet(
        "#eventDialogContainer { background: #1e293b; border-radius: 18px; border: 1px solid #334155; }"
        );

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(container);

    QVBoxLayout *containerLayout = new QVBoxLayout(container);
    containerLayout->setContentsMargins(0, 0, 0, 0);
    containerLayout->setSpacing(0);

    // Заголовок
    QWidget *header = new QWidget();
    header->setStyleSheet(
        "background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #1d4ed8, stop:1 #2563eb);"
        "border-top-left-radius: 18px; border-top-right-radius: 18px;"
        );
    header->setFixedHeight(60);
    QHBoxLayout *headerLayout = new QHBoxLayout(header);
    headerLayout->setContentsMargins(24, 0, 24, 0);

    QLabel *title = new QLabel("Новое событие");
    title->setStyleSheet("color: white; font-size: 18px; font-weight: 600; background: transparent;");

    QPushButton *closeBtn = new QPushButton("✕");
    closeBtn->setFixedSize(36, 36);
    closeBtn->setCursor(Qt::PointingHandCursor);
    closeBtn->setStyleSheet(
        "QPushButton { background: rgba(255,255,255,0.1); border: none; border-radius: 18px; color: white; font-size: 16px; }"
        "QPushButton:hover { background: rgba(255,255,255,0.2); }"
        );
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::reject);

    headerLayout->addWidget(title);
    headerLayout->addStretch();
    headerLayout->addWidget(closeBtn);
    containerLayout->addWidget(header);

    // Контент
    QWidget *content = new QWidget();
    content->setStyleSheet("background: #1e293b; border-bottom-left-radius: 18px; border-bottom-right-radius: 18px;");
    QVBoxLayout *contentLayout = new QVBoxLayout(content);
    contentLayout->setContentsMargins(24, 24, 24, 24);
    contentLayout->setSpacing(16);

    QString inputStyle =
        "QLineEdit, QTextEdit, QDateTimeEdit, QComboBox {"
        "  background: #0f172a; border: 1px solid #334155; border-radius: 10px;"
        "  color: #f1f5f9; padding: 12px 16px; font-size: 14px;"
        "}"
        "QLineEdit:focus, QTextEdit:focus, QDateTimeEdit:focus, QComboBox:focus {"
        "  border-color: #1d4ed8;"
        "}";

    auto addLabel = [&](const QString &text) {
        QLabel *label = new QLabel(text);
        label->setStyleSheet("color: #f8fafc; font-size: 14px; font-weight: 600; background: transparent;");
        contentLayout->addWidget(label);
    };

    // Название события
    addLabel("Название события");
    m_title = new QLineEdit();
    m_title->setPlaceholderText("Введите название события...");
    m_title->setStyleSheet(inputStyle);
    m_title->setFixedHeight(44);
    contentLayout->addWidget(m_title);

    // Дата + Время
    QHBoxLayout *dateRow = new QHBoxLayout();
    dateRow->setSpacing(12);

    QVBoxLayout *dateCol = new QVBoxLayout();
    QLabel *dateLabel = new QLabel("Дата");
    dateLabel->setStyleSheet("color: #f8fafc; font-size: 14px; font-weight: 600; background: transparent;");
    dateCol->addWidget(dateLabel);
    m_dateTime = new QDateTimeEdit(QDateTime::currentDateTime());
    m_dateTime->setCalendarPopup(true);
    m_dateTime->setStyleSheet(inputStyle);
    m_dateTime->setFixedHeight(44);
    dateCol->addWidget(m_dateTime);
    dateRow->addLayout(dateCol, 1);

    QVBoxLayout *timeCol = new QVBoxLayout();
    QLabel *timeLabel = new QLabel("Продолжительность");
    timeLabel->setStyleSheet("color: #f8fafc; font-size: 14px; font-weight: 600; background: transparent;");
    timeCol->addWidget(timeLabel);
    m_duration = new QComboBox();
    m_duration->addItems({"30 минут", "1 час", "1.5 часа", "2 часа", "3 часа", "4 часа", "Весь день"});
    m_duration->setCurrentIndex(1);
    m_duration->setStyleSheet(inputStyle);
    m_duration->setFixedHeight(44);
    timeCol->addWidget(m_duration);
    dateRow->addLayout(timeCol, 1);

    contentLayout->addLayout(dateRow);

    // Тип события
    addLabel("Тип события");
    m_eventType = new QComboBox();
    m_eventType->addItems({"Встреча", "Проект", "Задача", "Напоминание", "Личное"});
    m_eventType->setStyleSheet(inputStyle);
    m_eventType->setFixedHeight(44);
    contentLayout->addWidget(m_eventType);

    // Цвет события
    addLabel("Цвет события");
    QHBoxLayout *colorRow = new QHBoxLayout();
    colorRow->setSpacing(8);

    QStringList colors = {"#1d4ed8", "#10B981", "#8B5CF6", "#0EA5E9", "#F59E0B"};
    for (const QString &color : colors) {
        QPushButton *colorBtn = new QPushButton();
        colorBtn->setFixedSize(30, 30);
        colorBtn->setCursor(Qt::PointingHandCursor);
        colorBtn->setStyleSheet(
            QString("QPushButton { background: %1; border-radius: 15px; border: 2px solid %2; }")
                .arg(color, m_selectedColor == color ? "white" : "transparent")
            );
        connect(colorBtn, &QPushButton::clicked, this, [this, color]() {
            m_selectedColor = color;
            // Обновляем все кнопки
            QList<QPushButton*> btns = findChildren<QPushButton*>();
            for (QPushButton* b : btns) {
                if (b->width() == 30) {
                    QString c = b->styleSheet().contains("#1d4ed8") ? "#1d4ed8" :
                                    b->styleSheet().contains("#10B981") ? "#10B981" :
                                    b->styleSheet().contains("#8B5CF6") ? "#8B5CF6" :
                                    b->styleSheet().contains("#0EA5E9") ? "#0EA5E9" : "#F59E0B";
                    b->setStyleSheet(
                        QString("QPushButton { background: %1; border-radius: 15px; border: 2px solid %2; }")
                            .arg(c, c == color ? "white" : "transparent")
                        );
                }
            }
        });
        colorRow->addWidget(colorBtn);
    }
    colorRow->addStretch();
    contentLayout->addLayout(colorRow);

    // Описание
    addLabel("Описание");
    m_description = new QTextEdit();
    m_description->setPlaceholderText("Добавьте описание события...");
    m_description->setStyleSheet(inputStyle);
    m_description->setFixedHeight(80);
    contentLayout->addWidget(m_description);

    // Участники
    addLabel("Участники (через запятую)");
    m_participants = new QLineEdit();
    m_participants->setPlaceholderText("Иван Петров, Анна Сидорова...");
    m_participants->setStyleSheet(inputStyle);
    m_participants->setFixedHeight(44);
    contentLayout->addWidget(m_participants);

    contentLayout->addSpacing(8);

    // Кнопки
    QHBoxLayout *buttonRow = new QHBoxLayout();
    buttonRow->setSpacing(12);

    QPushButton *cancelBtn = new QPushButton("Отмена");
    cancelBtn->setFixedHeight(44);
    cancelBtn->setCursor(Qt::PointingHandCursor);
    cancelBtn->setStyleSheet(
        "QPushButton { background: #334155; border: none; border-radius: 10px; color: #cbd5e1; font-size: 14px; font-weight: 500; }"
        "QPushButton:hover { background: #475569; }"
        );
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);

    QPushButton *saveBtn = new QPushButton("Сохранить событие");
    saveBtn->setFixedHeight(44);
    saveBtn->setCursor(Qt::PointingHandCursor);
    saveBtn->setStyleSheet(
        "QPushButton { background: #1d4ed8; border: none; border-radius: 10px; color: white; font-size: 14px; font-weight: 600; }"
        "QPushButton:hover { background: #2563eb; }"
        );
    connect(saveBtn, &QPushButton::clicked, this, [this]() {
        accept();
    });

    buttonRow->addWidget(cancelBtn, 1);
    buttonRow->addWidget(saveBtn, 1);
    contentLayout->addLayout(buttonRow);

    saveBtn->setDefault(true);
    saveBtn->setAutoDefault(true);
    cancelBtn->setAutoDefault(false);

    containerLayout->addWidget(content);
}

void EventDialog::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
        if (m_title->text().trimmed().isEmpty()) {
            m_title->setStyleSheet(
                "QLineEdit { background: #0f172a; border: 1px solid #EF4444; border-radius: 10px; color: #f1f5f9; padding: 12px 16px; font-size: 14px; }"
                );
            m_title->setFocus();
            return;
        }
        accept();
        return;
    }
    if (event->key() == Qt::Key_Escape) {
        reject();
        return;
    }
    QDialog::keyPressEvent(event);
}

QString EventDialog::getTitle() const { return m_title->text(); }
QString EventDialog::getDescription() const { return m_description->toPlainText(); }
QDateTime EventDialog::getDateTime() const { return m_dateTime->dateTime(); }
QString EventDialog::getDuration() const { return m_duration->currentText(); }
QString EventDialog::getEventType() const { return m_eventType->currentText(); }
QString EventDialog::getColor() const { return m_selectedColor; }
QString EventDialog::getParticipants() const { return m_participants->text(); }
