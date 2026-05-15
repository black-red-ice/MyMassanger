#include "CalendarPanel.h"
#include "EventDialog.h"
#include "EventDetailDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QFrame>
#include <QMouseEvent>
#include <QMessageBox>

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

    m_selectedDateLabel = new QLabel("События на " + QDate::currentDate().toString("d MMMM yyyy"));
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
    m_selectedDateLabel->setText("События на " + date.toString("d MMMM yyyy"));
    updateEventsList(date);
}

void CalendarPanel::updateEventsList(const QDate &date)
{
    while (m_eventsLayout->count() > 1) {
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

        m_eventsLayout->insertWidget(0, emptyWidget);
    } else {
        for (int i = 0; i < events.size(); ++i) {
            const CalendarEvent &event = events[i];

            QWidget *eventCard = new QWidget();
            eventCard->setStyleSheet(
                QString("background: #1e293b; border-radius: 10px; border: 1px solid #334155; "
                        "border-left: 4px solid %1; padding: 10px 14px;")
                    .arg(event.color)
                );
            eventCard->setCursor(Qt::PointingHandCursor);

            QHBoxLayout *cardLayout = new QHBoxLayout(eventCard);
            cardLayout->setContentsMargins(0, 0, 0, 0);
            cardLayout->setSpacing(10);

            QLabel *dot = new QLabel("●");
            dot->setStyleSheet(QString("color: %1; font-size: 10px; background: transparent;").arg(event.color));

            QVBoxLayout *textLayout = new QVBoxLayout();
            textLayout->setSpacing(2);

            QLabel *eventLabel = new QLabel(event.title);
            eventLabel->setStyleSheet("color: #f1f5f9; font-size: 13px; font-weight: 500; background: transparent;");
            eventLabel->setWordWrap(true);

            QLabel *timeLabel = new QLabel(event.dateTime.toString("hh:mm"));
            timeLabel->setStyleSheet("color: #64748B; font-size: 11px; background: transparent;");

            textLayout->addWidget(eventLabel);
            textLayout->addWidget(timeLabel);

            cardLayout->addWidget(dot);
            cardLayout->addLayout(textLayout, 1);

            // Данные для клика
            eventCard->setProperty("eventIndex", i);
            eventCard->setProperty("eventDate", date);
            eventCard->installEventFilter(this);

            m_eventsLayout->insertWidget(m_eventsLayout->count() - 1, eventCard);
        }
    }
}

void CalendarPanel::onAddClicked()
{
    onNewEvent();
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

void CalendarPanel::onEventClicked(int index, const QDate &date)
{
    QList<CalendarEvent> events = m_events.value(date);
    if (index < 0 || index >= events.size()) return;

    const CalendarEvent &event = events[index];

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

bool CalendarPanel::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress) {
        QWidget *widget = qobject_cast<QWidget*>(obj);
        if (widget && widget->property("eventIndex").isValid()) {
            int index = widget->property("eventIndex").toInt();
            QDate date = widget->property("eventDate").toDate();
            onEventClicked(index, date);
            return true;
        }
    }
    return SidePanel::eventFilter(obj, event);
}
