#include "leftnavigation.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QIcon>

LeftNavigation::LeftNavigation(QWidget *parent) : QWidget(parent)
{
    setupUI();
}

void LeftNavigation::setupUI()
{
    setFixedWidth(80);
    setStyleSheet("background-color: #1e293b;");

    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->setSpacing(5);

    // Секция "ОБЩЕНИЕ"
    QLabel *commLabel = new QLabel("ОБЩЕНИЕ");
    commLabel->setStyleSheet("color: #64748b; font-size: 11px; font-weight: bold;");
    commLabel->setAlignment(Qt::AlignCenter);

    // Кнопки
    m_btnChat = new QPushButton();
    m_btnEmployees = new QPushButton();
    m_btnProjects = new QPushButton();

    // Секция "РАБОТА"
    QLabel *workLabel = new QLabel("РАБОТА");
    workLabel->setStyleSheet("color: #64748b; font-size: 11px; font-weight: bold;");
    workLabel->setAlignment(Qt::AlignCenter);

    m_btnTasks = new QPushButton();
    m_btnCalendar = new QPushButton();
    m_btnDocs = new QPushButton();

    // Секция "СИСТЕМА"
    QLabel *systemLabel = new QLabel("СИСТЕМА");
    systemLabel->setStyleSheet("color: #64748b; font-size: 11px; font-weight: bold;");
    systemLabel->setAlignment(Qt::AlignCenter);

    m_btnSettings = new QPushButton();
    m_btnSupport = new QPushButton();

    // Иконки
    addNavButton(m_btnChat, ":/icons/darkTheme/images/darkTheme/comments-solid-full.svg");
    addNavButton(m_btnEmployees, ":/icons/darkTheme/images/darkTheme/users.svg");
    addNavButton(m_btnProjects, ":/icons/darkTheme/images/darkTheme/diagram-project.svg");
    addNavButton(m_btnTasks, ":/icons/darkTheme/images/darkTheme/list-check.svg");
    addNavButton(m_btnCalendar, ":/icons/darkTheme/images/darkTheme/calendar.svg");
    addNavButton(m_btnDocs, ":/icons/darkTheme/images/darkTheme/folder.svg");
    addNavButton(m_btnSettings, ":/icons/darkTheme/images/darkTheme/gear.svg");
    addNavButton(m_btnSupport, ":/icons/darkTheme/images/darkTheme/headset.svg");

    // Соединения
    connect(m_btnChat, &QPushButton::clicked, this, &LeftNavigation::onChatClicked);
    connect(m_btnEmployees, &QPushButton::clicked, this, &LeftNavigation::onEmployeesClicked);
    connect(m_btnProjects, &QPushButton::clicked, this, &LeftNavigation::onProjectsClicked);
    connect(m_btnTasks, &QPushButton::clicked, this, &LeftNavigation::onTasksClicked);
    connect(m_btnCalendar, &QPushButton::clicked, this, &LeftNavigation::onCalendarClicked);
    connect(m_btnDocs, &QPushButton::clicked, this, &LeftNavigation::onDocsClicked);
    connect(m_btnSettings, &QPushButton::clicked, this, &LeftNavigation::onSettingsClicked);
    connect(m_btnSupport, &QPushButton::clicked, this, &LeftNavigation::onSupportClicked);

    // Компоновка
    m_mainLayout->addSpacing(20);
    m_mainLayout->addWidget(commLabel, 0, Qt::AlignCenter);
    m_mainLayout->addSpacing(10);
    m_mainLayout->addWidget(m_btnChat, 0, Qt::AlignCenter);
    m_mainLayout->addWidget(m_btnEmployees, 0, Qt::AlignCenter);
    m_mainLayout->addWidget(m_btnProjects, 0, Qt::AlignCenter);
    m_mainLayout->addSpacing(20);
    m_mainLayout->addWidget(workLabel, 0, Qt::AlignCenter);
    m_mainLayout->addSpacing(10);
    m_mainLayout->addWidget(m_btnTasks, 0, Qt::AlignCenter);
    m_mainLayout->addWidget(m_btnCalendar, 0, Qt::AlignCenter);
    m_mainLayout->addWidget(m_btnDocs, 0, Qt::AlignCenter);
    m_mainLayout->addSpacing(20);
    m_mainLayout->addWidget(systemLabel, 0, Qt::AlignCenter);
    m_mainLayout->addSpacing(10);
    m_mainLayout->addWidget(m_btnSettings, 0, Qt::AlignCenter);
    m_mainLayout->addWidget(m_btnSupport, 0, Qt::AlignCenter);
    m_mainLayout->addStretch();

    // Активная кнопка по умолчанию (чат)
    m_btnChat->setStyleSheet(
        "QPushButton { background-color: #1d4ed8; border: none; border-radius: 12px; }"
        "QPushButton:hover { background-color: #2563eb; }"
        );
}

void LeftNavigation::addNavButton(QPushButton *btn, const QString &iconPath)
{
    btn->setFixedSize(48, 48);
    btn->setIcon(QIcon(iconPath));
    btn->setIconSize(QSize(24, 24));
    btn->setCursor(Qt::PointingHandCursor);
    btn->setStyleSheet(
        "QPushButton { background-color: transparent; border: none; border-radius: 12px; }"
        "QPushButton:hover { background-color: #334155; }"
        );
}

void LeftNavigation::onChatClicked()
{
    m_currentIndex = 0;
    emit currentIndexChanged(0);
    // Сброс стилей
    m_btnChat->setStyleSheet(
        "QPushButton { background-color: #1d4ed8; border: none; border-radius: 12px; }"
        "QPushButton:hover { background-color: #2563eb; }"
        );
    QList<QPushButton*> others = {m_btnEmployees, m_btnProjects, m_btnTasks, m_btnCalendar, m_btnDocs, m_btnSettings, m_btnSupport};
    for (auto btn : others) {
        btn->setStyleSheet(
            "QPushButton { background-color: transparent; border: none; border-radius: 12px; }"
            "QPushButton:hover { background-color: #334155; }"
            );
    }
}

void LeftNavigation::onEmployeesClicked()
{
    emit toggleSidePanel(1);
    onChatClicked(); // временно, чтобы сбросить стиль
}

void LeftNavigation::onProjectsClicked()
{
    emit toggleSidePanel(2);
    onChatClicked();
}

void LeftNavigation::onTasksClicked()
{
    emit toggleSidePanel(3);
    onChatClicked();
}

void LeftNavigation::onCalendarClicked()
{
    emit toggleSidePanel(4);
    onChatClicked();
}

void LeftNavigation::onDocsClicked()
{
    emit toggleSidePanel(5);
    onChatClicked();
}

void LeftNavigation::onSettingsClicked()
{
    emit toggleSidePanel(6);
    onChatClicked();
}

void LeftNavigation::onSupportClicked()
{
    emit toggleSidePanel(7);
    onChatClicked();
}
