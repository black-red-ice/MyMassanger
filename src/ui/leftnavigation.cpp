#include "leftnavigation.h"
#include "mainwindow.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QIcon>
#include <QSettings>
#include <QFile>
#include <QPainter>
#include <QPainterPath>
#include <QStandardPaths>

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
    commLabel->setStyleSheet("color: #64748b; font-size: 11px; font-weight: bold; padding: 0px;");
    commLabel->setAlignment(Qt::AlignCenter);

    // Кнопки
    m_btnChat = new QPushButton();
    m_btnEmployees = new QPushButton();
    m_btnProjects = new QPushButton();

    // Секция "РАБОТА"
    QLabel *workLabel = new QLabel("РАБОТА");
    workLabel->setStyleSheet("color: #64748b; font-size: 11px; font-weight: bold; padding: 0px;");
    workLabel->setAlignment(Qt::AlignCenter);

    m_btnTasks = new QPushButton();
    m_btnCalendar = new QPushButton();
    m_btnDocs = new QPushButton();

    // Секция "CRM"
    QLabel *crmLabel = new QLabel("CRM");
    crmLabel->setStyleSheet("color: #64748b; font-size: 11px; font-weight: bold; padding: 0px;");
    crmLabel->setAlignment(Qt::AlignCenter);

    m_btnCrmClients = new QPushButton();
    m_btnCrmCompanies = new QPushButton();

    // Секция "СИСТЕМА"
    QLabel *systemLabel = new QLabel("СИСТЕМА");
    systemLabel->setStyleSheet("color: #64748b; font-size: 11px; font-weight: bold; padding: 0px;");
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
    addNavButton(m_btnCrmClients, ":/icons/darkTheme/images/darkTheme/user-tie.svg");
    addNavButton(m_btnCrmCompanies, ":/icons/darkTheme/images/darkTheme/building.svg");
    addNavButton(m_btnSettings, ":/icons/darkTheme/images/darkTheme/gear.svg");
    addNavButton(m_btnSupport, ":/icons/darkTheme/images/darkTheme/headset.svg");

    // Соединения
    connect(m_btnChat, &QPushButton::clicked, this, &LeftNavigation::onChatClicked);
    connect(m_btnEmployees, &QPushButton::clicked, this, &LeftNavigation::onEmployeesClicked);
    connect(m_btnProjects, &QPushButton::clicked, this, &LeftNavigation::onProjectsClicked);
    connect(m_btnTasks, &QPushButton::clicked, this, &LeftNavigation::onTasksClicked);
    connect(m_btnCalendar, &QPushButton::clicked, this, &LeftNavigation::onCalendarClicked);
    connect(m_btnDocs, &QPushButton::clicked, this, &LeftNavigation::onDocsClicked);
    connect(m_btnCrmClients, &QPushButton::clicked, this, &LeftNavigation::onCrmClientsClicked);
    connect(m_btnCrmCompanies, &QPushButton::clicked, this, &LeftNavigation::onCrmCompaniesClicked);
    connect(m_btnSettings, &QPushButton::clicked, this, &LeftNavigation::onSettingsClicked);
    connect(m_btnSupport, &QPushButton::clicked, this, &LeftNavigation::onSupportClicked);

    // Компоновка
    m_mainLayout->addSpacing(20);  // Верхний отступ

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
    m_mainLayout->addWidget(crmLabel, 0, Qt::AlignCenter);
    m_mainLayout->addSpacing(10);
    m_mainLayout->addWidget(m_btnCrmClients, 0, Qt::AlignCenter);
    m_mainLayout->addWidget(m_btnCrmCompanies, 0, Qt::AlignCenter);

    m_mainLayout->addSpacing(20);
    m_mainLayout->addWidget(systemLabel, 0, Qt::AlignCenter);
    m_mainLayout->addSpacing(10);
    m_mainLayout->addWidget(m_btnSettings, 0, Qt::AlignCenter);
    m_mainLayout->addWidget(m_btnSupport, 0, Qt::AlignCenter);

    // Профиль внизу
    m_btnProfile = new QPushButton();
    m_btnProfile->setFixedSize(48, 48);
    m_btnProfile->setIcon(QIcon(":/icons/darkTheme/images/darkTheme/user.svg"));
    m_btnProfile->setIconSize(QSize(24, 24));
    m_btnProfile->setCursor(Qt::PointingHandCursor);
    m_btnProfile->setStyleSheet(
        "QPushButton { background-color: #1d4ed8; border: none; border-radius: 24px; }"
        "QPushButton:hover { background-color: #2563eb; }"
        );
    connect(m_btnProfile, &QPushButton::clicked, this, &LeftNavigation::onProfileClicked);

    m_mainLayout->addStretch();
    m_mainLayout->addWidget(m_btnProfile, 0, Qt::AlignCenter);
    m_mainLayout->addSpacing(10);

    // Вызываем обновление аватара в конце
    updateProfileAvatar();

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
        "QPushButton { background-color: transparent; border: none; border-radius: 12px; outline: none; }"
        "QPushButton:hover { background-color: #334155; }"
        "QPushButton:focus { outline: none; }"
        );
}

void LeftNavigation::onChatClicked()
{
    m_btnChat->setStyleSheet(
        "QPushButton { background-color: #1d4ed8; border: none; border-radius: 12px; outline: none; }"
        "QPushButton:hover { background-color: #2563eb; }"
        );
    resetOtherButtons(m_btnChat);
    emit currentIndexChanged(NavItem::Chat);
}

void LeftNavigation::onEmployeesClicked()
{
    m_btnEmployees->setStyleSheet(
        "QPushButton { background-color: #1d4ed8; border: none; border-radius: 12px; outline: none; }"
        "QPushButton:hover { background-color: #2563eb; }"
        );
    resetOtherButtons(m_btnEmployees);
    emit currentIndexChanged(NavItem::Employees);
}

void LeftNavigation::onProjectsClicked()
{
    m_btnProjects->setStyleSheet(
        "QPushButton { background-color: #1d4ed8; border: none; border-radius: 12px; outline: none; }"
        "QPushButton:hover { background-color: #2563eb; }"
        );
    resetOtherButtons(m_btnProjects);
    // emit toggleSidePanel(2);
}

void LeftNavigation::onTasksClicked()
{
    m_btnTasks->setStyleSheet(
        "QPushButton { background-color: #1d4ed8; border: none; border-radius: 12px; outline: none; }"
        "QPushButton:hover { background-color: #2563eb; }"
        );
    resetOtherButtons(m_btnTasks);
    emit currentIndexChanged(NavItem::Tasks);
}

void LeftNavigation::onCalendarClicked()
{
    m_btnCalendar->setStyleSheet(
        "QPushButton { background-color: #1d4ed8; border: none; border-radius: 12px; outline: none; }"
        "QPushButton:hover { background-color: #2563eb; }"
        );
    resetOtherButtons(m_btnCalendar);
    emit toggleSidePanel(3);
}

void LeftNavigation::onDocsClicked()
{
    m_btnDocs->setStyleSheet(
        "QPushButton { background-color: #1d4ed8; border: none; border-radius: 12px; outline: none; }"
        "QPushButton:hover { background-color: #2563eb; }"
        );
    resetOtherButtons(m_btnDocs);
    emit currentIndexChanged(NavItem::Documents);
}

void LeftNavigation::onSettingsClicked()
{
    m_btnSettings->setStyleSheet(
        "QPushButton { background-color: #1d4ed8; border: none; border-radius: 12px; outline: none; }"
        "QPushButton:hover { background-color: #2563eb; }"
        );
    resetOtherButtons(m_btnSettings);
    emit currentIndexChanged(NavItem::Settings);
}

void LeftNavigation::onSupportClicked()
{
    m_btnSupport->setStyleSheet(
        "QPushButton { background-color: #1d4ed8; border: none; border-radius: 12px; outline: none; }"
        "QPushButton:hover { background-color: #2563eb; }"
        );
    resetOtherButtons(m_btnSupport);
    emit currentIndexChanged(NavItem::Support);
}

void LeftNavigation::onCrmClientsClicked()
{
    m_btnCrmClients->setStyleSheet(
        "QPushButton { background-color: #1d4ed8; border: none; border-radius: 12px; outline: none; }"
        "QPushButton:hover { background-color: #2563eb; }"
        );
    resetOtherButtons(m_btnCrmClients);
    emit currentIndexChanged(NavItem::CRM);
}

void LeftNavigation::onCrmCompaniesClicked()
{
    m_btnCrmCompanies->setStyleSheet(
        "QPushButton { background-color: #1d4ed8; border: none; border-radius: 12px; outline: none; }"
        "QPushButton:hover { background-color: #2563eb; }"
        );
    resetOtherButtons(m_btnCrmCompanies);
    emit currentIndexChanged(NavItem::Companies);
}

void LeftNavigation::onProfileClicked()
{
    emit toggleSidePanel(11);
}

void LeftNavigation::resetOtherButtons(QPushButton *activeBtn)
{
    QList<QPushButton*> all = {m_btnChat, m_btnEmployees, m_btnProjects, m_btnTasks,
                                m_btnCalendar, m_btnDocs, m_btnCrmClients,
                                m_btnCrmCompanies, m_btnSettings, m_btnSupport, m_btnProfile};
    for (auto btn : all) {
        if (btn != activeBtn) {
            btn->setStyleSheet(
                "QPushButton { background-color: transparent; border: none; border-radius: 12px; outline: none; }"
                "QPushButton:hover { background-color: #334155; }"
                );
        }
    }

    // У профиля border-radius 24px (круглый)
    if (activeBtn != m_btnProfile) {
        m_btnProfile->setStyleSheet(
            "QPushButton { background-color: #1d4ed8; border: none; border-radius: 24px; outline: none; }"
            "QPushButton:hover { background-color: #2563eb; }"
            );
    }
}

void LeftNavigation::updateProfileAvatar()
{
    QSettings settings("Aura", "Messenger");

    MainWindow *mw = qobject_cast<MainWindow*>(this->window());
    qint64 userId = mw ? mw->getCurrentUserId() : 0;

    if (userId <= 0) {
        setDefaultAvatar();
        return;
    }

    QString avatarKey = "userAvatar_" + QString::number(userId);
    QString avatarPath = settings.value(avatarKey).toString();

    qDebug() << "updateProfileAvatar - userId:" << userId << "avatarPath:" << avatarPath;

    if (avatarPath.isEmpty()) {
        setDefaultAvatar();
        // Запрашиваем с сервера
        if (mw) {
            mw->requestUserAvatar(userId);
        }
        return;
    }

    // Проверяем кеш
    QString finalPath = avatarPath;
    if (avatarPath.startsWith("/files/")) {
        QString cachePath = QStandardPaths::writableLocation(QStandardPaths::CacheLocation)
        + "/avatars/" + QString::number(userId) + ".jpg";
        if (QFile::exists(cachePath)) {
            finalPath = cachePath;
        } else {
            // Если нет в кеше, запрашиваем с сервера
            if (mw) {
                mw->requestUserAvatar(userId);
            }
            setDefaultAvatar();
            return;
        }
    }

    if (QFile::exists(finalPath)) {
        QPixmap pixmap(finalPath);
        if (!pixmap.isNull()) {
            QPixmap rounded(48, 48);
            rounded.fill(Qt::transparent);
            QPainter painter(&rounded);
            painter.setRenderHint(QPainter::Antialiasing, true);
            QPainterPath path;
            path.addRoundedRect(0, 0, 48, 48, 12, 12);
            painter.setClipPath(path);
            painter.drawPixmap(0, 0, pixmap.scaled(48, 48, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
            painter.end();

            m_btnProfile->setIcon(QIcon(rounded));
            m_btnProfile->setIconSize(QSize(48, 48));
            m_btnProfile->setText("");
            m_btnProfile->setStyleSheet(
                "QPushButton { background: transparent; border: none; border-radius: 12px; }"
                "QPushButton:hover { background: transparent; }"
                );
            return;
        }
    }

    setDefaultAvatar();
}

void LeftNavigation::setDefaultAvatar()
{
    m_btnProfile->setIcon(
        QIcon(":/icons/darkTheme/images/darkTheme/user.svg"));

    m_btnProfile->setIconSize(QSize(24, 24));

    m_btnProfile->setStyleSheet(
        "QPushButton { background-color: #1d4ed8; border: none; border-radius: 12px; }"
        "QPushButton:hover { background-color: #2563eb; }"
        );
}
