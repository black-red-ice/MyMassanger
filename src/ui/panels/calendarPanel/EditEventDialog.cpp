#include "EditEventDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QTextEdit>
#include <QDateTimeEdit>
#include <QComboBox>
#include <QPushButton>
#include <QKeyEvent>
#include <QCalendarWidget>
#include <QTableView>
#include <QHeaderView>

EditEventDialog::EditEventDialog(const CalendarEvent &event, QWidget *parent)
    : OverlayDialog(parent), m_originalEvent(event), m_selectedColor(event.color)
{
    setFixedSize(540, 760);
    setStyleSheet("background: transparent;");
    setupUI();
}

void EditEventDialog::setupUI()
{
    QWidget *container = new QWidget(this);
    container->setObjectName("editEventDialogContainer");
    container->setStyleSheet(
        "#editEventDialogContainer { background: #1e293b; border-radius: 18px; border: 1px solid #334155; }"
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
        "background: #059669; border-top-left-radius: 18px; border-top-right-radius: 18px;"
        );
    header->setFixedHeight(60);
    QHBoxLayout *headerLayout = new QHBoxLayout(header);
    headerLayout->setContentsMargins(24, 0, 24, 0);

    QLabel *title = new QLabel("Редактировать событие");
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
        "  border-color: #059669; outline: none;"
        "}"
        "QComboBox:focus, QDateTimeEdit:focus { outline: none; }"
        "QComboBox::item:focus, QDateTimeEdit::item:focus { outline: none; }"
        "QComboBox QAbstractItemView::item:focus { outline: none; border: none; }"
        "QComboBox::drop-down:focus { outline: none; }"
        "QComboBox::drop-down { border: none; width: 30px; border-top-right-radius: 10px; border-bottom-right-radius: 10px; }"
        "QComboBox::down-arrow { image: none; border-left: 5px solid transparent; border-right: 5px solid transparent; border-top: 6px solid #f1f5f9; margin-right: 10px; }"
        "QComboBox QAbstractItemView { background: #0f172a; color: #f1f5f9; border: 1px solid #334155; border-radius: 10px; selection-background-color: #334155; outline: none; }"
        "QComboBox QAbstractItemView::item { padding: 8px 12px; color: #f1f5f9; }"
        "QComboBox QAbstractItemView::item:selected { background: #334155; outline: none; }"
        "QDateTimeEdit::drop-down { border: none; width: 30px; border-top-right-radius: 10px; border-bottom-right-radius: 10px; }"
        "QDateTimeEdit::down-arrow { image: none; border-left: 5px solid transparent; border-right: 5px solid transparent; border-top: 6px solid #f1f5f9; margin-right: 10px; }";

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

    // Добавляем отслеживание изменения текста для убирания красной рамки
    connect(m_title, &QLineEdit::textChanged, this, [this]() {
        if (!m_title->text().trimmed().isEmpty()) {
            m_title->setStyleSheet(
                "QLineEdit { background: #0f172a; border: 1px solid #334155; border-radius: 10px; color: #f1f5f9; padding: 12px 16px; font-size: 14px; }"
                "QLineEdit:focus { border-color: #059669; }"
                );
        }
    });

    // Дата + Время
    QHBoxLayout *dateRow = new QHBoxLayout();
    dateRow->setSpacing(12);

    QVBoxLayout *dateCol = new QVBoxLayout();
    QLabel *dateLabel = new QLabel("Дата");
    dateLabel->setStyleSheet("color: #f8fafc; font-size: 14px; font-weight: 600; background: transparent;");
    dateCol->addWidget(dateLabel);
    m_dateTime = new QDateTimeEdit(m_originalEvent.dateTime);
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
    int durationIndex = m_duration->findText(m_originalEvent.duration);
    m_duration->setCurrentIndex(durationIndex != -1 ? durationIndex : 1);
    m_duration->setStyleSheet(inputStyle);
    m_duration->setFixedHeight(44);
    timeCol->addWidget(m_duration);
    dateRow->addLayout(timeCol, 1);

    contentLayout->addLayout(dateRow);

    // Тип события
    addLabel("Тип события");
    m_eventType = new QComboBox();
    m_eventType->addItems({"Встреча", "Проект", "Задача", "Напоминание", "Личное"});
    int typeIndex = m_eventType->findText(m_originalEvent.eventType);
    m_eventType->setCurrentIndex(typeIndex != -1 ? typeIndex : 0);
    m_eventType->setStyleSheet(inputStyle);
    m_eventType->setFixedHeight(44);
    contentLayout->addWidget(m_eventType);

    // Цвет события
    addLabel("Цвет события");
    QHBoxLayout *colorRow = new QHBoxLayout();
    colorRow->setSpacing(8);

    QStringList colors = {"#1d4ed8", "#10B981", "#8B5CF6", "#0EA5E9", "#F59E0B", "#EF4444", "#EC4899"};
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
            updateColorButtons();
        });
        colorRow->addWidget(colorBtn);
        m_colorButtons.append(colorBtn);
    }
    colorRow->addStretch();
    contentLayout->addLayout(colorRow);

    // Описание
    addLabel("Описание");
    m_description = new QTextEdit();
    m_description->setText(m_originalEvent.description);
    m_description->setPlaceholderText("Добавьте описание события...");
    m_description->setStyleSheet(inputStyle);
    m_description->setFixedHeight(80);
    contentLayout->addWidget(m_description);

    // Участники
    addLabel("Участники (через запятую)");
    m_participants = new QLineEdit();
    m_participants->setText(m_originalEvent.participants);
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

    QPushButton *saveBtn = new QPushButton("Сохранить изменения");
    saveBtn->setFixedHeight(44);
    saveBtn->setCursor(Qt::PointingHandCursor);
    saveBtn->setStyleSheet(
        "QPushButton { background: #059669; border: none; border-radius: 10px; color: white; font-size: 14px; font-weight: 600; }"
        "QPushButton:hover { background: #047857; }"
        );
    connect(saveBtn, &QPushButton::clicked, this, [this]() {
        if (m_title->text().trimmed().isEmpty()) {
            m_title->setStyleSheet(
                "QLineEdit { background: #0f172a; border: 1px solid #EF4444; border-radius: 10px; color: #f1f5f9; padding: 12px 16px; font-size: 14px; }"
                "QLineEdit:focus { border-color: #EF4444; }"
                );
            m_title->setFocus();
            return;
        }
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

void EditEventDialog::updateColorButtons()
{
    for (QPushButton *btn : m_colorButtons) {
        QString color = btn->styleSheet();
        color.remove("QPushButton { background: ");
        color.remove("; border-radius: 15px; border: 2px solid ");
        QString c = color.split(";").first();
        btn->setStyleSheet(
            QString("QPushButton { background: %1; border-radius: 15px; border: 2px solid %2; }")
                .arg(c, c == m_selectedColor ? "white" : "transparent")
            );
    }
}

CalendarEvent EditEventDialog::getEvent() const
{
    CalendarEvent event;
    event.title = m_title->text().trimmed();
    event.description = m_description->toPlainText().trimmed();
    event.dateTime = m_dateTime->dateTime();
    event.duration = m_duration->currentText();
    event.eventType = m_eventType->currentText();
    event.color = m_selectedColor;
    event.participants = m_participants->text().trimmed();
    return event;
}

void EditEventDialog::keyPressEvent(QKeyEvent *event)
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
