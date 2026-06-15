#include "CalendarPanel.h"
#include "EventDialog.h"
#include "EventDetailDialog.h"
#include "EditEventDialog.h"
#include "EventItemWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QFrame>
#include <QMouseEvent>
#include <QMessageBox>
#include <QLocale>
#include <QCalendarWidget>
#include <QTableView>
#include <QHeaderView>
#include <QUuid>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include "../../mainwindow.h"

CalendarPanel::CalendarPanel(QWidget *parent)
    : SidePanel(parent,
                "Календарь",
                ":/icons/general/images/general/calendar-light.svg",
                "#059669",
                "",
                380,
                "#059669")
{
    QLayoutItem *stretch = getContentLayout()->takeAt(getContentLayout()->count() - 1);
    delete stretch;

    getAddButton()->parentWidget()->hide();
    setSearchVisible(false);
    setupCalendar();

    updateEventsList(QDate::currentDate());
}

void CalendarPanel::setupCalendar()
{
    m_calendar = new QCalendarWidget();
    m_calendar->setGridVisible(false);
    m_calendar->setVerticalHeaderFormat(QCalendarWidget::NoVerticalHeader);
    m_calendar->setSelectionMode(QCalendarWidget::SingleSelection);

    // Отключаем стандартное выделение (синий фон)
    QPalette pal = m_calendar->palette();
    pal.setColor(QPalette::Highlight, Qt::transparent);
    pal.setColor(QPalette::HighlightedText, Qt::white);
    m_calendar->setPalette(pal);

    m_calendar->setStyleSheet(
        "QCalendarWidget { background: #1e293b; border: none; }"
        "QCalendarWidget QToolButton { color: #f1f5f9; background: transparent; border: none; border-radius: 8px; padding: 6px 12px; }"
        "QCalendarWidget QToolButton:hover { background: #334155; }"
        "QCalendarWidget QToolButton::menu-indicator { image: none; }"
        "QCalendarWidget QWidget#qt_calendar_navigationbar { background: #1e293b; border-bottom: 1px solid #334155; padding: 8px; }"
        "QCalendarWidget QSpinBox { background: #334155; border: none; border-radius: 8px; color: #f1f5f9; padding: 4px 8px; }"
        "QCalendarWidget QTableView { background: #1e293b; selection-background-color: transparent; }"
        "QCalendarWidget QTableView::item { background: transparent; border: none; color: #cbd5e1; }"       // цвет всех цифр
        "QCalendarWidget QTableView::item:selected { background: #334155; border-radius: 12px; color: white; }"
        "QCalendarWidget QHeaderView::section { background: #1e293b; border: none; color: #64748B; padding: 8px 4px; }"   // цвет дней недели
        );

    // ========== РЕШЕНИЕ 1: Разделяем цвета цифр и дней недели ==========

    // 1. Дни недели (заголовок) — задаём цвет через стиль с !important
    m_calendar->setStyleSheet(m_calendar->styleSheet() +
                              "QCalendarWidget QHeaderView::section {"
                              "  background: #1e293b;"
                              "  border: none;"
                              "  color: #64748B !important;"   // ← цвет дней недели (более светлый, сероватый)
                              "  padding: 8px 4px;"
                              "  font-weight: 500;"
                              "}"
                              );

    // 2. Цифры месяца (в сетке) — задаём через setHeaderTextFormat и setWeekdayTextFormat
    QTextCharFormat headerFormat = m_calendar->headerTextFormat();
    headerFormat.setForeground(QBrush(QColor("#cbd5e1")));  // ← светло-серый для цифр
    m_calendar->setHeaderTextFormat(headerFormat);

    // Применяем тот же цвет для всех дней недели (для цифр)
    QTextCharFormat weekdayFormat = m_calendar->weekdayTextFormat(Qt::Monday);
    weekdayFormat.setForeground(QBrush(QColor("#cbd5e1")));
    for (int day = Qt::Monday; day <= Qt::Sunday; ++day) {
        m_calendar->setWeekdayTextFormat(static_cast<Qt::DayOfWeek>(day), weekdayFormat);
    }

    // 3. Выбранная дата — текст белый (это уже задано через палитру HighlightedText)
    // Дополнительно можно явно указать цвет для selected через стиль, но палитра уже работает.

    // Настройка таблицы для отключения сетки и фокуса
    QTableView *tableView = m_calendar->findChild<QTableView*>();
    if (tableView) {
        tableView->setShowGrid(false);
        tableView->setFocusPolicy(Qt::NoFocus);
    }

    connect(m_calendar, &QCalendarWidget::clicked, this, &CalendarPanel::onDateSelected);
    getContentLayout()->addWidget(m_calendar);

    // Разделитель
    QFrame *sep = new QFrame();
    sep->setFrameShape(QFrame::HLine);
    sep->setStyleSheet("color: #334155; max-height: 1px;");
    getContentLayout()->addWidget(sep);

    // Заголовок событий
    QWidget *eventsHeader = new QWidget();
    eventsHeader->setStyleSheet("background: transparent;");
    QHBoxLayout *eventsHeaderLayout = new QHBoxLayout(eventsHeader);
    eventsHeaderLayout->setContentsMargins(16, 12, 16, 8);

    QLocale russianLocale(QLocale::Russian);
    m_selectedDateLabel = new QLabel("События на " + russianLocale.toString(QDate::currentDate(), "d MMMM yyyy"));
    m_selectedDateLabel->setStyleSheet("color: #f1f5f9; font-size: 14px; font-weight: 600; background: transparent;");

    QPushButton *addEventBtn = new QPushButton("+ Добавить");
    addEventBtn->setCursor(Qt::PointingHandCursor);
    addEventBtn->setStyleSheet(
        "QPushButton { background: transparent; border: 1px solid #334155; border-radius: 8px; "
        "color: #94A3B8; font-size: 12px; padding: 4px 10px; }"
        "QPushButton:hover { background: #334155; color: #f1f5f9; }"
        );
    connect(addEventBtn, &QPushButton::clicked, this, &CalendarPanel::onNewEvent);

    eventsHeaderLayout->addWidget(m_selectedDateLabel);
    eventsHeaderLayout->addStretch();
    eventsHeaderLayout->addWidget(addEventBtn);

    getContentLayout()->addWidget(eventsHeader);

    // Список событий
    QScrollArea *scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setStyleSheet(
        "QScrollArea { background: #1e293b; border: none; }"
        "QScrollBar:vertical { background: transparent; width: 6px; }"
        "QScrollBar::handle:vertical { background: #334155; border-radius: 3px; }"
        );

    m_eventsContainer = new QWidget();
    m_eventsContainer->setStyleSheet("background: transparent;");
    m_eventsLayout = new QVBoxLayout(m_eventsContainer);
    m_eventsLayout->setContentsMargins(16, 8, 16, 8);
    m_eventsLayout->setSpacing(8);
    m_eventsLayout->addStretch();

    scrollArea->setWidget(m_eventsContainer);
    getContentLayout()->addWidget(scrollArea, 1);
}

void CalendarPanel::onDateSelected(const QDate &date)
{
    QLocale russianLocale(QLocale::Russian);
    m_selectedDateLabel->setText("События на " + russianLocale.toString(date, "d MMMM yyyy"));
    updateEventsList(date);
}

void CalendarPanel::updateEventsList(const QDate &date)
{
    // Очищаем лейаут
    while (m_eventsLayout->count() > 0) {
        QLayoutItem *item = m_eventsLayout->takeAt(0);
        if (item->widget()) delete item->widget();
        delete item;
    }

    QList<CalendarEvent> events = m_events.value(date);

    if (events.isEmpty()) {
        QWidget *emptyWidget = new QWidget();
        QVBoxLayout *emptyLayout = new QVBoxLayout(emptyWidget);
        emptyLayout->setAlignment(Qt::AlignCenter);
        emptyLayout->setContentsMargins(0, 30, 0, 30);

        QLabel *emptyIcon = new QLabel();
        emptyIcon->setAlignment(Qt::AlignCenter);
        emptyIcon->setStyleSheet("background: transparent;");
        QPixmap iconPixmap(":/icons/general/images/general/calendar-light.svg");
        if (!iconPixmap.isNull()) {
            emptyIcon->setPixmap(iconPixmap.scaled(48, 48, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        } else {
            // fallback если иконка не найдена
            emptyIcon->setText("📅");
            emptyIcon->setStyleSheet("font-size: 36px; background: transparent;");
        }

        QLabel *emptyText = new QLabel("На этот день нет запланированных событий");
        emptyText->setAlignment(Qt::AlignCenter);
        emptyText->setStyleSheet("color: #64748B; font-size: 13px; background: transparent;");

        emptyLayout->addWidget(emptyIcon);
        emptyLayout->addSpacing(8);
        emptyLayout->addWidget(emptyText);

        m_eventsLayout->addWidget(emptyWidget);
    } else {
        for (const CalendarEvent &event : events) {
            EventItemWidget *item = new EventItemWidget(event);
            connect(item, &EventItemWidget::clicked, this, [this, event]() {
                onEventClicked(event);
            });
            connect(item, &EventItemWidget::deleted, this, [this, event, date]() {
                QList<CalendarEvent> &eventsList = m_events[date];
                for (int i = 0; i < eventsList.size(); ++i) {
                    if (eventsList[i].title == event.title &&
                        eventsList[i].dateTime == event.dateTime) {
                        eventsList.removeAt(i);
                        break;
                    }
                }
                updateEventsList(date);
            });
            m_eventsLayout->addWidget(item);
        }
    }

    m_eventsLayout->addStretch();
}

void CalendarPanel::onNewEvent()
{
    qDebug() << "=== onNewEvent START ===";

    QWidget *dimWidget = new QWidget(this->window());
    dimWidget->setObjectName("eventDim");
    dimWidget->setStyleSheet("#eventDim { background-color: rgba(0, 0, 0, 180); }");
    dimWidget->setGeometry(this->window()->rect());
    dimWidget->raise();
    dimWidget->show();

    EventDialog dialog(dimWidget);
    if (dialog.exec() == QDialog::Accepted) {
        QDate selectedDate = m_calendar->selectedDate();
        qDebug() << "Selected date:" << selectedDate.toString();

        CalendarEvent event;
        event.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
        event.title = dialog.getTitle();
        event.description = dialog.getDescription();
        event.dateTime = dialog.getDateTime();
        event.duration = dialog.getDuration();
        event.eventType = dialog.getEventType();
        event.color = dialog.getColor();
        event.participants = dialog.getParticipants();

        qDebug() << "New event created - id:" << event.id << "title:" << event.title;

        if (!event.title.isEmpty()) {
            m_events[selectedDate].append(event);
            updateEventsList(selectedDate);
            qDebug() << "Event added to local storage, calling saveEventsToServer()";
            saveEventsToServer();
        } else {
            qDebug() << "Event title is empty, not saving";
        }
    } else {
        qDebug() << "Dialog rejected";
    }

    dimWidget->deleteLater();
    qDebug() << "=== onNewEvent END ===";
}

void CalendarPanel::onEventClicked(const CalendarEvent &event)
{
    QWidget *dimWidget = new QWidget(this->window());
    dimWidget->setObjectName("eventDetailDim");
    dimWidget->setStyleSheet("#eventDetailDim { background-color: rgba(0, 0, 0, 180); }");
    dimWidget->setGeometry(this->window()->rect());
    dimWidget->raise();
    dimWidget->show();

    EventDetailDialog dialog(
        event.title,
        event.description,
        event.dateTime,
        event.duration,
        event.eventType,
        event.participants,
        dimWidget
        );

    // Подключаем сигнал редактирования
    connect(&dialog, &EventDetailDialog::editRequested, this, [this, event, dimWidget]() {
        // Закрываем диалог деталей
        dimWidget->deleteLater();

        // Создаём диалог редактирования
        QWidget *editDimWidget = new QWidget(this->window());
        editDimWidget->setObjectName("editEventDim");
        editDimWidget->setStyleSheet("#editEventDim { background-color: rgba(0, 0, 0, 180); }");
        editDimWidget->setGeometry(this->window()->rect());
        editDimWidget->raise();
        editDimWidget->show();

        EditEventDialog editDialog(event, editDimWidget);

        if (editDialog.exec() == QDialog::Accepted) {
            CalendarEvent updatedEvent = editDialog.getEvent();
            updatedEvent.id = event.id; // Сохраняем ID

            // Находим и обновляем событие
            QDate eventDate = event.dateTime.date();
            QList<CalendarEvent> &events = m_events[eventDate];
            for (int i = 0; i < events.size(); ++i) {
                if (events[i].id == event.id) {
                    // Обновляем все поля
                    events[i].title = updatedEvent.title;
                    events[i].description = updatedEvent.description;
                    events[i].dateTime = updatedEvent.dateTime;
                    events[i].duration = updatedEvent.duration;
                    events[i].eventType = updatedEvent.eventType;
                    events[i].color = updatedEvent.color;
                    events[i].participants = updatedEvent.participants;

                    // Если дата изменилась, перемещаем событие
                    if (updatedEvent.dateTime.date() != eventDate) {
                        m_events[updatedEvent.dateTime.date()].append(events[i]);
                        events.removeAt(i);
                    }
                    break;
                }
            }

            // Обновляем отображение
            updateEventsList(m_calendar->selectedDate());
            m_calendar->update();

            // Сохраняем на сервер
            saveEventsToServer();
        }

        editDimWidget->deleteLater();
    });

    dialog.exec();
    dimWidget->deleteLater();
}

void CalendarPanel::onAddClicked()
{
    onNewEvent();
}

bool CalendarPanel::eventFilter(QObject *obj, QEvent *event)
{
    return SidePanel::eventFilter(obj, event);
}

void CalendarPanel::loadEventsFromServer()
{
    qDebug() << "=== loadEventsFromServer START ===";

    MainWindow *mw = qobject_cast<MainWindow*>(this->window());
    if (!mw) {
        qDebug() << "❌ MainWindow not found!";
        return;
    }

    if (!mw->getNetworkManager() || !mw->getNetworkManager()->isConnected()) {
        qDebug() << "❌ Not connected to server!";
        return;
    }

    QJsonObject data;
    mw->getNetworkManager()->sendJson("get_calendar_events", data);
    qDebug() << "Requested calendar events from server";
}

void CalendarPanel::saveEventsToServer()
{
    qDebug() << "=== saveEventsToServer START ===";

    MainWindow *mw = qobject_cast<MainWindow*>(this->window());
    if (!mw) {
        qDebug() << "❌ MainWindow not found!";
        return;
    }

    if (!mw->getNetworkManager()) {
        qDebug() << "❌ NetworkManager is null!";
        return;
    }

    if (!mw->getNetworkManager()->isConnected()) {
        qDebug() << "❌ Not connected to server!";
        return;
    }

    QJsonArray eventsArray;
    int totalEvents = 0;

    for (auto it = m_events.begin(); it != m_events.end(); ++it) {
        for (const CalendarEvent &event : it.value()) {
            QJsonObject obj;
            obj["id"] = event.id;
            obj["title"] = event.title;
            obj["description"] = event.description;
            obj["dateTime"] = event.dateTime.toString(Qt::ISODate);
            obj["duration"] = event.duration;
            obj["eventType"] = event.eventType;
            obj["color"] = event.color;
            obj["participants"] = event.participants;
            eventsArray.append(obj);
            totalEvents++;

            qDebug() << "  Event:" << event.title << "| date:" << event.dateTime.toString() << "| id:" << event.id;
        }
    }

    qDebug() << "📤 Sending" << totalEvents << "events to server";
    qDebug() << "📤 JSON:" << QJsonDocument(eventsArray).toJson(QJsonDocument::Compact);

    QJsonObject data;
    data["events"] = eventsArray;
    mw->getNetworkManager()->sendJson("save_calendar_events", data);

    qDebug() << "=== saveEventsToServer END ===";
}

void CalendarPanel::setEventsFromJson(const QJsonArray &events)
{
    qDebug() << "=== setEventsFromJson START ===";
    qDebug() << "Received" << events.size() << "events from server";

    m_events.clear();

    for (const QJsonValue &v : events) {
        QJsonObject obj = v.toObject();
        CalendarEvent event;
        event.id = obj["id"].toString();
        event.title = obj["title"].toString();
        event.description = obj["description"].toString();
        event.dateTime = QDateTime::fromString(obj["dateTime"].toString(), Qt::ISODate);
        event.duration = obj["duration"].toString();
        event.eventType = obj["eventType"].toString();
        event.color = obj["color"].toString();
        event.participants = obj["participants"].toString();

        qDebug() << "  Loaded event:" << event.title << "| date:" << event.dateTime.toString() << "| id:" << event.id;

        QDate eventDate = event.dateTime.date();
        m_events[eventDate].append(event);
    }

    updateEventsList(m_calendar->selectedDate());
    m_calendar->update();
    qDebug() << "=== setEventsFromJson END ===";
}
