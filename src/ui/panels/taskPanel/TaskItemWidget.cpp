#include "TaskItemWidget.h"
#include "../widgets/FlowLayout.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QIcon>
#include <QSize>
#include <QPainter>
#include <QPainterPath>
#include <QSizePolicy>
#include <QStyle>
#include <QMouseEvent>

TaskItemWidget::TaskItemWidget(const Task &task, QWidget *parent)
    : QWidget(parent), m_task(task)
{
    auto softWrapLongTokens = [](const QString &s, int chunk = 24) -> QString {
        // QLabel word-wrap doesn't break long tokens without whitespace.
        // Insert zero-width spaces into long tokens so the layout can wrap instead of overflowing.
        QString out;
        out.reserve(s.size() + s.size() / chunk);

        int run = 0;
        for (const QChar &ch : s) {
            const bool isBreak = ch.isSpace() || ch == '-' || ch == '/' || ch == '_' || ch == '.';
            if (isBreak) {
                run = 0;
                out.append(ch);
                continue;
            }

            out.append(ch);
            run++;
            if (run >= chunk) {
                out.append(QChar(0x200B)); // zero-width space
                run = 0;
            }
        }
        return out;
    };

    // Scope styling to this widget only (avoid cascading to child widgets)
    setObjectName("taskItem");
    setAttribute(Qt::WA_StyledBackground, true);
    setStyleSheet(
        "#taskItem { "
        "  background: #1e293b; "
        "  border-radius: 12px; "
        "  border: 1px solid rgba(14, 165, 233, 0.35); "
        "}"
    );

    QHBoxLayout *main = new QHBoxLayout(this);
    // Match template: checkbox should be placed inside padding area (avoid overlapping border-left).
    main->setSpacing(10);
    // Leave extra left inset for the tapered accent stripe.
    main->setContentsMargins(16, 10, 16, 10);

    QPushButton *checkboxBtn = new QPushButton(this);
    checkboxBtn->setCheckable(true);
    checkboxBtn->setFixedSize(22, 22);
    checkboxBtn->setCursor(Qt::PointingHandCursor);
    checkboxBtn->setStyleSheet(
        "QPushButton {"
        "  background: transparent;"
        "  border: none;"
        "  padding: 0px;"
        "}"
        );

    // Устанавливаем иконку в зависимости от статуса
    if (task.status == TaskStatus::Done) {
        checkboxBtn->setIcon(QIcon(":/icons/general/images/general/square-check.svg"));
    } else {
        checkboxBtn->setIcon(QIcon(":/icons/general/images/general/square.svg"));
    }
    checkboxBtn->setIconSize(QSize(20, 20));
    checkboxBtn->setChecked(task.status == TaskStatus::Done);

    QVBoxLayout *text = new QVBoxLayout();
    text->setContentsMargins(0, 0, 0, 0);
    // Reduce vertical gaps between title/description/tags/deadline.
    text->setSpacing(2);

    QLabel *title = new QLabel(softWrapLongTokens(task.title));
    title->setWordWrap(true);
    title->setStyleSheet("color: white; font-weight: bold; font-size: 14px; margin-bottom: 3px;");
    title->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    title->setMinimumWidth(0);

    QLabel *desc = new QLabel(softWrapLongTokens(task.description));
    desc->setWordWrap(true);
    desc->setStyleSheet("color: #94A3B8; font-size: 12px; margin-bottom: 6px;");
    desc->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    desc->setMinimumWidth(0);

    text->addWidget(title);
    text->addWidget(desc);

    // Tags
    if (!task.tags.isEmpty() || task.priority == TaskPriority::High) {
        QWidget *tagsWrap = new QWidget(this);
        tagsWrap->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

        FlowLayout *tagsLayout = new FlowLayout(tagsWrap, 0, 6, 6);
        QStringList tags = task.tags.split(",", Qt::SkipEmptyParts);

        for (const QString &tag : tags) {
            QLabel *tagLabel = new QLabel(softWrapLongTokens(tag.trimmed(), 14));
            tagLabel->setStyleSheet(
                "background: rgba(14, 165, 233, 0.1); "
                "color: #0EA5E9; "
                "padding: 2px 10px; "
                "border-radius: 10px; "
                "font-size: 11px;"
                "min-height: 20px;"
                "max-height: 20px;"
                );
            tagLabel->setMinimumWidth(0);
            tagsLayout->addWidget(tagLabel);
        }

        // Если приоритет высокий — красный тег
        if (task.priority == TaskPriority::High) {
            QLabel *priorityTag = new QLabel("Высокий приоритет");
            priorityTag->setStyleSheet(
                "background: rgba(239, 68, 68, 0.15); "
                "color: #EF4444; "
                "padding: 2px 10px; "
                "border-radius: 10px; "
                "font-size: 11px;"
                "font-weight: 500;"
                "min-height: 20px;"
                "max-height: 20px;"
                );
            priorityTag->setMinimumWidth(0);
            tagsLayout->addWidget(priorityTag);
        }

        tagsWrap->setLayout(tagsLayout);
        text->addWidget(tagsWrap);
    }

    // Дата снизу
    QLabel *deadline = new QLabel(task.deadline.toString("dd.MM hh:mm"));
    deadline->setStyleSheet("color: #94A3B8; font-size: 11px; padding: 2px 0px;");
    deadline->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    text->addWidget(deadline);

    QPushButton *delBtn = new QPushButton();
    delBtn->setFixedSize(24, 24);
    delBtn->setCursor(Qt::PointingHandCursor);
    delBtn->setIcon(QIcon(":/icons/general/images/general/trash.svg"));
    delBtn->setIconSize(QSize(16, 16));
    delBtn->setStyleSheet(
        "QPushButton {"
        "  background: transparent;"
        "  border: none;"
        "  padding: 0px;"
        "}"
        );

    connect(checkboxBtn, &QPushButton::clicked, this, [this, checkboxBtn]() {
        m_task.status = checkboxBtn->isChecked() ? TaskStatus::Done : TaskStatus::Todo;
        if (m_task.status == TaskStatus::Done) {
            checkboxBtn->setIcon(QIcon(":/icons/general/images/general/square-check.svg"));
        } else {
            checkboxBtn->setIcon(QIcon(":/icons/general/images/general/square.svg"));
        }
        checkboxBtn->setIconSize(QSize(20, 20));
        emit toggled();
    });
    connect(delBtn, &QPushButton::clicked, this, &TaskItemWidget::deleted);

    QVBoxLayout *leftCol = new QVBoxLayout();
    leftCol->setContentsMargins(0, 0, 0, 0);
    leftCol->setSpacing(0);
    leftCol->addWidget(checkboxBtn, 0, Qt::AlignTop);
    leftCol->addSpacing(8);
    leftCol->addWidget(delBtn, 0, Qt::AlignBottom);

    main->addLayout(leftCol);
    main->addLayout(text);
    main->setStretchFactor(text, 1);

    // Keep natural height, but don't expand vertically to fill freed space.
    // This still allows the widget to grow when text wraps.
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
}

void TaskItemWidget::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);

    const qreal radius = 12.0;
    const QRectF r = rect().adjusted(0.5, 0.5, -0.5, -0.5);

    // Обрезаем по форме карточки
    QPainterPath cardPath;
    cardPath.addRoundedRect(r, radius, radius);
    p.setClipPath(cardPath);

    // Accent: простая закруглённая полоса
    const qreal accentWidth = 4.0;
    const qreal accentRadius = 12.0;

    QRectF accentRect(0.0, 0.0, accentWidth, r.height());

    QPainterPath accent;
    accent.addRoundedRect(accentRect, accentRadius, accentRadius);

    p.fillPath(accent, QColor("#0EA5E9"));
}

void TaskItemWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    QWidget::mouseDoubleClickEvent(event);
    emit editRequested();
}
