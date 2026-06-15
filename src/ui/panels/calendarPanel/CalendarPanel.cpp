#include "CalendarPanel.h"
#include "EventDialog.h"
#include "EventDetailDialog.h"
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

CalendarPanel::CalendarPanel(QWidget *parent)
    : SidePanel(parent,
                "Календарь",
                ":/icons/darkTheme/images/darkTheme/calendar.svg",
                "#10B981",
                "",
                380,
                "#10B981")
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
    m_calendar->setStyleSheet(
        "QCalendarWidget {"
        "  background: #1e293b;"
        "  border: none;"
        "  color: #f1f5f9;"
        "}"
        "QCalendarWidget QToolButton {"
        "  color: #f1f5f9;"
        "  background: transparent;"
        "  border: none;"
        "  border-radius: 8px;"
        "  padding: 6px 12px;"
        "  font-size: 14px;"
        "  font-weight: 600;"
        "}"
        "QCalendarWidget QToolButton:hover {"
        "  background: #334155;"
        "}"
        "QCalendarWidget QToolButton::menu-indicator {"
        "  image: none;"
        "}"
        "QCalendarWidget QTableView {"
        "  background: #1e293b;"
        "  selection-background-color: #10B981;"
        "  selection-color: white;"
        "  border: none;"
        "  outline: none;"
        "}"
        "QCalendarWidget QTableView::item {"
        "  padding: 6px;"
        "  color: #f1f5f9;"
        "  border-radius: 8px;"
        "}"
        "QCalendarWidget QTableView::item:selected {"
        "  background: #10B981;"
        "  color: white;"
        "}"
        "QCalendarWidget QAbstractItemView:enabled {"
        "  color: #f1f5f9;"
        "}"
        "QCalendarWidget QWidget#qt_calendar_navigationbar {"
        "  background: #1e293b;"
        "  border-bottom: 1px solid #334155;"
        "  padding: 8px;"
        "}"
        "QCalendarWidget QSpinBox {"
        "  background: #334155;"
        "  border: none;"
        "  border-radius: 8px;"
        "  color: #f1f5f9;"
        "  padding: 4px 8px;"
        "  font-size: 14px;"
        "}"
        );

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

    // Скроллируемая область для событий
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

        QLabel *emptyIcon = new QLabel("📅");
        emptyIcon->setAlignment(Qt::AlignCenter);
        emptyIcon->setStyleSheet("font-size: 36px; background: transparent;");

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
                // Удаляем событие
                QList<CalendarEvent> &eventsList = m_events[date];
                for (int i = 0; i < eventsList.size(); ++i) {
                    if (eventsList[i].title == event.title &&
                        eventsList[i].dateTime == event.dateTime) {
                        eventsList.removeAt(i);
                        break;
                    }
                }
                // Обновляем список
                updateEventsList(date);
            });
            m_eventsLayout->addWidget(item);
        }
    }

    m_eventsLayout->addStretch();
}

void CalendarPanel::onNewEvent()
{
    QWidget *dimWidget = new QWidget(this->window());
    dimWidget->setObjectName("eventDim");
    dimWidget->setStyleSheet("#eventDim { background-color: rgba(0, 0, 0, 180); }");
    dimWidget->setGeometry(this->window()->rect());
    dimWidget->raise();
    dimWidget->show();

    EventDialog dialog(dimWidget);
    if (dialog.exec() == QDialog::Accepted) {
        QDate selectedDate = m_calendar->selectedDate();

        CalendarEvent event;
        event.title = dialog.getTitle();
        event.description = dialog.getDescription();
        event.dateTime = dialog.getDateTime();
        event.duration = dialog.getDuration();
        event.eventType = dialog.getEventType();
        event.color = dialog.getColor();
        event.participants = dialog.getParticipants();

        if (!event.title.isEmpty()) {
            m_events[selectedDate].append(event);
            updateEventsList(selectedDate);
        }
    }

    dimWidget->deleteLater();
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
        event.color,
        event.participants,
        dimWidget
        );
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
