#include "EventItemWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QTextBrowser>
#include <QPushButton>
#include <QPainter>
#include <QPainterPath>
#include <QMouseEvent>
#include <QSizePolicy>
#include <QResizeEvent>
#include <QTimer>
#include <QIcon>

EventItemWidget::EventItemWidget(const CalendarEvent &event, QWidget *parent)
    : QWidget(parent), m_event(event), m_participantsLabel(nullptr)
{
    setObjectName("eventItem");
    setAttribute(Qt::WA_StyledBackground, true);
    setCursor(Qt::PointingHandCursor);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    setStyleSheet(
        "#eventItem { "
        "  background: #1e293b; "
        "  border-radius: 12px; "
        "  border: 1px solid #334155; "
        "}"
        );

    QHBoxLayout *main = new QHBoxLayout(this);
    main->setContentsMargins(16, 8, 16, 8);
    main->setSpacing(12);

    // Контейнер для текста
    QWidget *textContainer = new QWidget();
    textContainer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

    QVBoxLayout *infoLayout = new QVBoxLayout(textContainer);
    infoLayout->setContentsMargins(0, 0, 0, 0);
    infoLayout->setSpacing(2);
    infoLayout->setAlignment(Qt::AlignLeft | Qt::AlignTop);

    // Название события
    m_titleLabel = new QTextBrowser();
    m_titleLabel->setText(m_event.title);
    m_titleLabel->setStyleSheet(
        "QTextBrowser { "
        "  color: #f1f5f9; "
        "  font-size: 14px; "
        "  font-weight: 600; "
        "  background: transparent; "
        "  border: none; "
        "  margin: 0; "
        "  padding: 0; "
        "}"
        );
    m_titleLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    m_titleLabel->setFrameShape(QFrame::NoFrame);
    m_titleLabel->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_titleLabel->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_titleLabel->document()->setDocumentMargin(0);
    m_titleLabel->setMinimumHeight(20);
    infoLayout->addWidget(m_titleLabel);

    // Описание (если есть)
    if (!event.description.isEmpty()) {
        m_descriptionLabel = new QTextBrowser();
        m_descriptionLabel->setText(event.description);
        m_descriptionLabel->setStyleSheet(
            "QTextBrowser { "
            "  color: #94A3B8; "
            "  font-size: 12px; "
            "  background: transparent; "
            "  border: none; "
            "  margin: 0; "
            "  padding: 0; "
            "}"
            );
        m_descriptionLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
        m_descriptionLabel->setFrameShape(QFrame::NoFrame);
        m_descriptionLabel->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        m_descriptionLabel->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        m_descriptionLabel->document()->setDocumentMargin(0);
        m_descriptionLabel->setMinimumHeight(16);
        infoLayout->addWidget(m_descriptionLabel);
    } else {
        m_descriptionLabel = nullptr;
    }

    // Время
    m_timeLabel = new QLabel(event.dateTime.toString("dd.MM.yyyy HH:mm"));
    m_timeLabel->setStyleSheet("color: #64748B; font-size: 11px; background: transparent; margin-top: 2px;");
    m_timeLabel->setAlignment(Qt::AlignLeft);
    m_timeLabel->setWordWrap(true);
    infoLayout->addWidget(m_timeLabel);

    // Участники (только если есть)
    if (!event.participants.isEmpty() && event.participants.trimmed() != "") {
        m_participantsLabel = new QLabel("👥 " + event.participants);
        m_participantsLabel->setStyleSheet("color: #64748B; font-size: 11px; background: transparent;");
        m_participantsLabel->setAlignment(Qt::AlignLeft);
        m_participantsLabel->setWordWrap(true);
        infoLayout->addWidget(m_participantsLabel);
    }

    // Тип события — закруглённый бейдж
    m_typeLabel = new QLabel(event.eventType);
    m_typeLabel->setStyleSheet(
        QString("color: %1; background: rgba(51, 65, 85, 0.8); border-radius: 10px; padding: 2px 8px;")
            .arg(event.color)
        );
    m_typeLabel->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    m_typeLabel->setAlignment(Qt::AlignCenter);
    m_typeLabel->adjustSize(); // подгоняем размер под текст
    infoLayout->addWidget(m_typeLabel, 0, Qt::AlignLeft);

    main->addWidget(textContainer, 1);

    // Вертикальный контейнер для кнопки, чтобы прижать её к низу
    QVBoxLayout *buttonLayout = new QVBoxLayout();
    buttonLayout->setContentsMargins(0, 0, 0, 0);
    buttonLayout->setSpacing(0);
    buttonLayout->addStretch();  // растяжка сверху, чтобы прижать кнопку вниз

    // Кнопка удаления
    m_deleteBtn = new QPushButton();
    m_deleteBtn->setFixedSize(28, 28);
    m_deleteBtn->setCursor(Qt::PointingHandCursor);
    m_deleteBtn->setIcon(QIcon(":/icons/general/images/general/trash-light.svg"));
    m_deleteBtn->setIconSize(QSize(16, 16));
    m_deleteBtn->setStyleSheet(
        "QPushButton {"
        "  background: transparent;"
        "  border: none;"
        "  border-radius: 6px;"
        "  padding: 0px;"
        "}"
        "QPushButton:hover {"
        "  background: rgba(239, 68, 68, 0.2);"
        "}"
        );
    connect(m_deleteBtn, &QPushButton::clicked, this, &EventItemWidget::deleted);
    buttonLayout->addWidget(m_deleteBtn);

    main->addLayout(buttonLayout);

    QTimer::singleShot(0, this, &EventItemWidget::updateHeight);
}

void EventItemWidget::updateHeight()
{
    // Вычисляем высоту только на основе существующих виджетов
    int totalHeight = 16; // отступы карточки

    totalHeight += m_titleLabel->document()->size().height();
    if (m_descriptionLabel) {
        totalHeight += m_descriptionLabel->document()->size().height() + 2;
    }
    totalHeight += m_timeLabel->height() + 2;
    if (m_participantsLabel) {
        totalHeight += m_participantsLabel->height() + 2;
    }
    totalHeight += m_typeLabel->height() + 2;

    // Убеждаемся, что кнопка помещается
    if (totalHeight < 40) totalHeight = 40;

    if (totalHeight > 20 && totalHeight != height()) {
        setFixedHeight(totalHeight);
    }
}

void EventItemWidget::resizeEvent(QResizeEvent *event)
{
    int newWidth = event->size().width() - 70; // учитываем кнопку удаления
    if (newWidth > 50) {
        m_titleLabel->document()->setTextWidth(newWidth);
        if (m_descriptionLabel) {
            m_descriptionLabel->document()->setTextWidth(newWidth);
        }
    }

    QTimer::singleShot(10, this, &EventItemWidget::updateHeight);

    QWidget::resizeEvent(event);
}

void EventItemWidget::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);

    const qreal radius = 12.0;
    const QRectF r = rect().adjusted(0.5, 0.5, -0.5, -0.5);

    QPainterPath cardPath;
    cardPath.addRoundedRect(r, radius, radius);
    p.setClipPath(cardPath);

    const qreal accentWidth = 4.0;
    QRectF accentRect(0.0, 0.0, accentWidth, r.height());

    QPainterPath accent;
    accent.addRoundedRect(accentRect, radius, radius);
    p.fillPath(accent, QColor(m_event.color));
}

void EventItemWidget::mousePressEvent(QMouseEvent *event)
{
    // Если клик не по кнопке удаления
    QPushButton *btn = qobject_cast<QPushButton*>(childAt(event->pos()));
    if (!btn || btn != m_deleteBtn) {
        if (event->button() == Qt::LeftButton) {
            emit clicked();
        }
    }
    QWidget::mousePressEvent(event);
}
