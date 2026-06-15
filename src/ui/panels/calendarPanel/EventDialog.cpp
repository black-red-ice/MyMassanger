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
#include <QCalendarWidget>
#include <QTableView>
#include <QHeaderView>

EventDialog::EventDialog(QWidget *parent)
    : OverlayDialog(parent), m_selectedColor("#059669")
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
        "background: #059669; border-top-left-radius: 18px; border-top-right-radius: 18px;"
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
        "  border-color: #059669; outline: none;"
        "}"
        // Убираем пунктирную рамку
        "QComboBox:focus, QDateTimeEdit:focus {"
        "  outline: none;"
        "}"
        "QComboBox::item:focus, QDateTimeEdit::item:focus {"
        "  outline: none;"
        "}"
        "QComboBox QAbstractItemView::item:focus {"
        "  outline: none;"
        "  border: none;"
        "}"
        "QComboBox::drop-down:focus {"
        "  outline: none;"
        "}"
        // QComboBox стили
        "QComboBox::drop-down {"
        "  border: none;"
        "  width: 30px;"
        "  border-top-right-radius: 10px;"
        "  border-bottom-right-radius: 10px;"
        "}"
        "QComboBox::down-arrow {"
        "  image: none;"
        "  border-left: 5px solid transparent;"
        "  border-right: 5px solid transparent;"
        "  border-top: 6px solid #f1f5f9;"
        "  margin-right: 10px;"
        "}"
        "QComboBox QAbstractItemView {"
        "  background: #0f172a;"
        "  color: #f1f5f9;"
        "  border: 1px solid #334155;"
        "  border-radius: 10px;"
        "  selection-background-color: #334155;"
        "  outline: none;"
        "}"
        "QComboBox QAbstractItemView::item {"
        "  padding: 8px 12px;"
        "  color: #f1f5f9;"
        "}"
        "QComboBox QAbstractItemView::item:selected {"
        "  background: #334155;"
        "  outline: none;"
        "}"
        // QDateTimeEdit стили
        "QDateTimeEdit::drop-down {"
        "  border: none;"
        "  width: 30px;"
        "  border-top-right-radius: 10px;"
        "  border-bottom-right-radius: 10px;"
        "}"
        "QDateTimeEdit::down-arrow {"
        "  image: none;"
        "  border-left: 5px solid transparent;"
        "  border-right: 5px solid transparent;"
        "  border-top: 6px solid #f1f5f9;"
        "  margin-right: 10px;"
        "}"
        // Календарь для QDateTimeEdit
        "QDateTimeEdit QCalendarWidget {"
        "  background: #1e293b;"
        "  color: #f1f5f9;"
        "  border: none;"
        "}"
        "QDateTimeEdit QCalendarWidget QWidget#qt_calendar_navigationbar {"
        "  background: #1e293b;"
        "  border-bottom: 1px solid #334155;"
        "  padding: 8px;"
        "}"
        "QDateTimeEdit QCalendarWidget QToolButton {"
        "  color: #f1f5f9;"
        "  background: transparent;"
        "  border: none;"
        "  border-radius: 8px;"
        "  padding: 6px 12px;"
        "  font-size: 14px;"
        "  font-weight: 600;"
        "}"
        "QDateTimeEdit QCalendarWidget QToolButton:hover {"
        "  background: #334155;"
        "}"
        "QDateTimeEdit QCalendarWidget QToolButton::menu-indicator {"
        "  image: none;"
        "}"
        "QDateTimeEdit QCalendarWidget QSpinBox {"
        "  background: #334155;"
        "  border: none;"
        "  border-radius: 8px;"
        "  color: #f1f5f9;"
        "  padding: 4px 8px;"
        "  font-size: 14px;"
        "}"
        "QDateTimeEdit QCalendarWidget QTableView {"
        "  background: #1e293b;"
        "  selection-background-color: transparent;"
        "  border: none;"
        "  gridline-color: transparent;"
        "}"
        "QDateTimeEdit QCalendarWidget QTableView::item {"
        "  background: transparent;"
        "  border: none;"
        "  color: #cbd5e1;"
        "  padding: 8px 4px;"
        "  border-radius: 0px;"
        "  min-width: 32px;"
        "}"
        "QDateTimeEdit QCalendarWidget QTableView::item:selected {"
        "  background: #334155;"
        "  border-radius: 12px;"
        "  color: white;"
        "}"
        "QDateTimeEdit QCalendarWidget QTableView::item:hover {"
        "  background: #2d3a4e;"
        "  border-radius: 12px;"
        "}"
        // Убираем пунктирную рамку
        "QDateTimeEdit QCalendarWidget QTableView::item:focus {"
        "  outline: none;"
        "  border: none;"
        "}"
        "QDateTimeEdit QCalendarWidget QTableView::item:selected:focus {"
        "  outline: none;"
        "  border: none;"
        "}"
        "QDateTimeEdit QCalendarWidget:focus {"
        "  outline: none;"
        "}"
        // Дни недели
        "QDateTimeEdit QCalendarWidget QHeaderView::section {"
        "  background: #1e293b;"
        "  border: none;"
        "  color: #64748B;"
        "  padding: 8px 2px;"
        "  font-weight: 500;"
        "  min-width: 40px;"
        "  width: 40px;"
        "  text-align: center;"
        "}"
        // Скрываем вертикальный заголовок
        "QDateTimeEdit QCalendarWidget QTableView QHeaderView::section:vertical {"
        "  width: 20px;"
        "  min-width: 20px;"
        "  background: #1e293b;"
        "  color: #64748B;"
        "  border: none;"
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
    setupDateTimeEdit();  // Настройка календаря
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
            // Обновляем все кнопки
            QList<QPushButton*> btns = findChildren<QPushButton*>();
            for (QPushButton* b : btns) {
                if (b->width() == 30) {
                    QString c = b->styleSheet().contains("#1d4ed8") ? "#1d4ed8" :
                                    b->styleSheet().contains("#10B981") ? "#10B981" :
                                    b->styleSheet().contains("#8B5CF6") ? "#8B5CF6" :
                                    b->styleSheet().contains("#0EA5E9") ? "#0EA5E9" :
                                    b->styleSheet().contains("#F59E0B") ? "#F59E0B" :
                                    b->styleSheet().contains("#EF4444") ? "#EF4444" : "#EC4899";
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
        "QPushButton { background: #059669; border: none; border-radius: 10px; color: white; font-size: 14px; font-weight: 600; }"
        "QPushButton:hover { background: #047857; }"
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

void EventDialog::setupDateTimeEdit()
{
    QCalendarWidget *calendar = m_dateTime->calendarWidget();
    if (!calendar) return;

    // Устанавливаем фиксированный меньший размер
    calendar->setFixedSize(320, 260);

    calendar->setGridVisible(false);
    calendar->setVerticalHeaderFormat(QCalendarWidget::NoVerticalHeader);
    calendar->setHorizontalHeaderFormat(QCalendarWidget::ShortDayNames);  // Короткие названия дней (Пн, Вт, Ср)

    QPalette pal = calendar->palette();
    pal.setColor(QPalette::Highlight, Qt::transparent);
    pal.setColor(QPalette::HighlightedText, Qt::white);
    calendar->setPalette(pal);

    calendar->setStyleSheet(
        "QCalendarWidget {"
        "  background: #1e293b;"
        "  border: none;"
        "}"
        "QCalendarWidget QToolButton {"
        "  color: #f1f5f9;"
        "  background: transparent;"
        "  border: none;"
        "  border-radius: 8px;"
        "  padding: 4px 8px;"
        "  font-size: 12px;"
        "  font-weight: 600;"
        "}"
        "QCalendarWidget QToolButton:hover {"
        "  background: #334155;"
        "}"
        "QCalendarWidget QToolButton::menu-indicator {"
        "  image: none;"
        "}"
        "QCalendarWidget QWidget#qt_calendar_navigationbar {"
        "  background: #1e293b;"
        "  border-bottom: 1px solid #334155;"
        "  padding: 4px;"
        "}"
        "QCalendarWidget QSpinBox {"
        "  background: #334155;"
        "  border: none;"
        "  border-radius: 6px;"
        "  color: #f1f5f9;"
        "  padding: 2px 6px;"
        "  font-size: 12px;"
        "}"
        "QCalendarWidget QTableView {"
        "  background: #1e293b;"
        "  border: none;"
        "}"
        "QCalendarWidget QTableView::item {"
        "  background: transparent;"
        "  border: none;"
        "  color: #cbd5e1;"
        "  padding: 6px 4px;"
        "  font-size: 12px;"
        "}"
        "QCalendarWidget QTableView::item:selected {"
        "  background: #334155;"
        "  border-radius: 10px;"
        "  color: white;"
        "}"
        "QCalendarWidget QTableView::item:selected:focus {"
        "  background: #334155;"
        "  border-radius: 10px;"
        "  color: white;"
        "  outline: none;"
        "}"
        "QCalendarWidget QTableView::item:focus {"
        "  outline: none;"
        "}"
        "QCalendarWidget QHeaderView::section {"
        "  background: #1e293b;"
        "  border: none;"
        "  color: #64748B;"
        "  padding: 4px 2px;"
        "  font-weight: 500;"
        "  font-size: 11px;"
        "  min-width: 28px;"
        "}"
        );

    QTableView *tableView = calendar->findChild<QTableView*>();
    if (tableView) {
        tableView->setFocusPolicy(Qt::NoFocus);
        tableView->setSelectionMode(QAbstractItemView::SingleSelection);
        tableView->setSelectionBehavior(QAbstractItemView::SelectItems);
        tableView->setStyleSheet(
            "QTableView {"
            "  outline: none;"
            "  border: none;"
            "}"
            "QTableView::item:focus {"
            "  outline: none;"
            "  border: none;"
            "}"
            );
    }

    calendar->setFocusPolicy(Qt::ClickFocus);
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
