#include "mainwindow.h"
#include "titlebar.h"
#include "leftnavigation.h"
#include "middlepanel.h"
#include "rightpanel.h"
#include "dialogs/settingsdialog.h"
#include "dialogs/supportdialog.h"
#include "../panels/SidePanel.h"
#include "panels/taskPanel/TasksPanel.h"
#include "panels/companiesPanel/CompaniesPanel.h"
#include "panels/employeesPanel/EmployeesPanel.h"
#include "dialogs/ProfileDialog.h"
#include "../panels/calendarPanel/CalendarPanel.h"
#include "dialogs/UserProfileDialog.h"
#include "PhotoViewer.h"
#include "panels/documentsPanel/DocumentsPanel.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QTimer>
#include <QDebug>
#include <QPainter>
#include <QScreen>
#include <QApplication>
#include <QSettings>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QSet>
#include <QDateTime>
#include <QUuid>
#include <QPropertyAnimation>
#include <algorithm>
#include <QTimer>
#include <QScrollBar>
#include <QMessageBox>
#include <QStackedWidget>
#include <QMouseEvent>
#include <QGraphicsOpacityEffect>
#include <QParallelAnimationGroup>
#include <QGraphicsDropShadowEffect>
#include <QProcess>
#include <QNetworkRequest>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QFile>
#include <QFileInfo>
#include <QNetworkRequest>
#include <QNetworkRequest>
#include <QStandardPaths>
#include <QDir>
#include <QFileInfo>
#include <QByteArray>
#include <QHttpMultiPart>
#include <QFileDialog>
#include <QInputDialog>
#include <QThread>

#ifdef Q_OS_WIN
#include <windows.h>
#include <dwmapi.h>
#include "panels/taskPanel/TasksPanel.h"
#include "panels/dealPanel/DealsPanel.h"
#pragma comment(lib, "dwmapi.lib")

// Определяем недостающие константы для старых SDK
#ifndef DWMWA_USE_IMMERSIVE_DARK_MODE
#define DWMWA_USE_IMMERSIVE_DARK_MODE 20
#endif
#endif

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    networkManager = nullptr;

    qDebug() << "MainWindow constructor started";
    m_currentChatId = 0;
    m_currentContactId = 0;

    m_currentSidePanel = nullptr;
    m_dimWidget = nullptr;
    m_overlay = nullptr;

    setupUI();

    qDebug() << "MainWindow setupUI completed";

#ifdef Q_OS_WIN
    // Применяем настройки окна после инициализации
    QTimer::singleShot(0, this, [this]() {
        HWND hwnd = (HWND)winId();

        // Включаем тёмную тему для заголовка (Windows 10/11)
        BOOL dark = TRUE;
        DwmSetWindowAttribute(hwnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &dark, sizeof(dark));

        // Настраиваем рамку
        LONG style = GetWindowLong(hwnd, GWL_STYLE);
        style &= ~WS_CAPTION;
        SetWindowLong(hwnd, GWL_STYLE, style);

        // Обновляем окно
        SetWindowPos(hwnd, nullptr, 0, 0, 0, 0,
                     SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER);
    });
#endif

    m_httpManager = new QNetworkAccessManager(this);
}

MainWindow::~MainWindow()
{
    // Clean up side panels
    for (auto it = m_sidePanels.begin(); it != m_sidePanels.end(); ++it) {
        if (it.value()) {
            it.value()->deleteLater();
        }
    }
    m_sidePanels.clear();

    // m_overlay будет удален автоматически как child m_centralWidget
}

void MainWindow::setupUI()
{
    // === CENTRAL ===
    m_centralWidget = new QWidget(this);
    setCentralWidget(m_centralWidget);

    m_centralWidget->setStyleSheet("background-color: #0f172a;");

    QVBoxLayout *mainVerticalLayout = new QVBoxLayout(m_centralWidget);
    mainVerticalLayout->setContentsMargins(0, 0, 0, 0);
    mainVerticalLayout->setSpacing(0);

    // === TITLE BAR ===
    m_titleBar = new TitleBar(this);
    m_titleBar->installEventFilter(this);

    connect(m_titleBar, &TitleBar::minimizeClicked, this, &QMainWindow::showMinimized);
    connect(m_titleBar, &TitleBar::maximizeClicked, this, &MainWindow::toggleMaximized);
    connect(m_titleBar, &TitleBar::closeClicked, this, &QMainWindow::close);

    mainVerticalLayout->addWidget(m_titleBar);

    // === CONTENT ===
    m_contentWidget = new QWidget(this);
    m_contentWidget->setStyleSheet("background-color: #1e293b;");

    QHBoxLayout *mainLayout = new QHBoxLayout(m_contentWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    mainVerticalLayout->addWidget(m_contentWidget);

    // === LEFT NAV ===
    m_leftNav = new LeftNavigation(m_contentWidget);

    connect(m_leftNav, &LeftNavigation::currentIndexChanged,
            this, &MainWindow::onNavigationChanged);

    connect(m_leftNav, &LeftNavigation::toggleSidePanel,
            this, &MainWindow::onToggleSidePanel);

    mainLayout->addWidget(m_leftNav);

    // === STACK ===
    m_stack = new QStackedWidget(m_contentWidget);
    m_stack->setStyleSheet("background: transparent;");
    mainLayout->addWidget(m_stack, 1);

    // ===================== CHAT PAGE ==========================
    QWidget *chatPage = new QWidget(m_stack);
    QHBoxLayout *chatLayout = new QHBoxLayout(chatPage);
    chatLayout->setContentsMargins(0, 0, 0, 0);
    chatLayout->setSpacing(0);

    m_middlePanel = new MiddlePanel(chatPage);
    m_rightPanel  = new RightPanel(chatPage);

    chatLayout->addWidget(m_middlePanel, 1);
    chatLayout->addWidget(m_rightPanel, 2);

    m_stack->addWidget(chatPage); // index 0

    // === DEFAULT PAGE ===
    m_stack->setCurrentIndex(0);

    qApp->installEventFilter(this);

    // === CONNECTIONS ===
    connect(m_middlePanel, &MiddlePanel::contactSelected,
            this, &MainWindow::onContactSelected);

    connect(m_middlePanel, &MiddlePanel::searchUsersByNickname,
            this, &MainWindow::onSearchUsersByNickname);

    connect(m_middlePanel, &MiddlePanel::clearSearch,
            this, &MainWindow::onClearSearch);

    connect(m_middlePanel, &MiddlePanel::searchContacts,
            this, &MainWindow::onSearchContacts);

    connect(m_rightPanel, &RightPanel::sendMessageRequested,
            this, &MainWindow::onSendMessage);

    connect(m_rightPanel, &RightPanel::typing,
            this, &MainWindow::onTyping);

    connect(m_rightPanel, &RightPanel::needLoadOlder,
            this, &MainWindow::loadOlderMessages);

    connect(m_middlePanel, &MiddlePanel::chatSelected,
            this, &MainWindow::onChatSelected);

    connect(m_rightPanel, &RightPanel::avatarClicked, this, [this](int userId) {
        QString username;
        for (const QVariantMap &contact : m_contacts) {
            if (contact["id"].toInt() == userId) {
                username = contact["username"].toString();
                break;
            }
        }
        if (username.isEmpty()) username = "Пользователь";

        QString avatarPath = m_userAvatars.value(userId);
        bool online = m_onlineUsers.contains(userId);

        // Запрашиваем профиль и ждем ответ
        requestUserProfile(userId);

        // Сохраняем userId для обработки ответа
        m_pendingProfileUserId = userId;
        m_pendingProfileUsername = username;
        m_pendingProfileAvatar = avatarPath;
        m_pendingProfileOnline = online;
    });

    connect(m_rightPanel, &RightPanel::pinToggled, this, [this](bool pinned) {
        if (m_currentChatId > 0) {
            togglePinChat(m_currentChatId);
        }
    });

    connect(m_rightPanel, &RightPanel::downloadFileRequested, this, [this](const QString &fileUrl, const QString &savePath) {
        // Если fileUrl - это URL с сервера (начинается с /files/)
        if (fileUrl.startsWith("/files/")) {
            downloadFileHttp(fileUrl, savePath);
        } else {
            // Это локальное имя файла - не поддерживается
            qDebug() << "❌ Cannot download local file:" << fileUrl;
        }
    });

    // В setupUI, где подключается fileAttached:
    connect(m_rightPanel, &RightPanel::fileAttached, this, [this](const QString &filePath) {
        QFileInfo fi(filePath);
        QString ext = fi.suffix().toLower();
        bool isImage = (ext == "png" || ext == "jpg" || ext == "jpeg" ||
                        ext == "bmp" || ext == "gif" || ext == "webp");

        qDebug() << "📎 FILE ATTACHED:" << filePath << "isImage:" << isImage;

        if (isImage) {
            m_rightPanel->addImageMessage(filePath, true, 1);

            QFile *file = new QFile(filePath);
            if (file->open(QIODevice::ReadOnly)) {
                QFileInfo fi(filePath);
                QByteArray fileData = file->readAll();
                file->deleteLater();

                qDebug() << "⬆️ UPLOADING IMAGE:" << fi.fileName() << "size:" << fileData.size();

                QString uniqueName = QUuid::createUuid().toString(QUuid::WithoutBraces) + "." + ext;

                QNetworkRequest request(QUrl("http://87.242.118.96:8080/upload"));
                request.setRawHeader("X-File-Name", fi.fileName().toUtf8());
                request.setHeader(QNetworkRequest::ContentTypeHeader, "application/octet-stream");

                QNetworkReply *reply = m_httpManager->post(request, fileData);

                connect(reply, &QNetworkReply::finished, this, [reply, fi, this]() {
                    if (reply->error() == QNetworkReply::NoError) {
                        QByteArray response = reply->readAll();
                        QJsonObject obj = QJsonDocument::fromJson(response).object();
                        QString fileUrl = obj["url"].toString();

                        qDebug() << "✅ UPLOAD SUCCESS:" << fileUrl;

                        if (!fileUrl.isEmpty()) {
                            QString imageMsg = QString("🖼 %1|%2|%3")
                                                   .arg(fi.fileName(), fileUrl, QString::number(fi.size()));

                            QString clientId = QUuid::createUuid().toString(QUuid::WithoutBraces);

                            // 🔥 Добавляем локальное сообщение перед отправкой
                            Message msg;
                            msg.id = -1;
                            msg.chatId = m_currentChatId;
                            msg.senderId = m_currentUserId;
                            msg.text = imageMsg;
                            msg.time = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
                            msg.clientId = clientId;
                            msg.localOrder = m_nextLocalOrder++;
                            msg.status = MessageStatus::Pending;

                            auto &messages = m_chatMessages[m_currentChatId];
                            messages.append(msg);

                            qDebug() << "📤 SENDING IMAGE METADATA:" << imageMsg << "clientId:" << clientId;

                            QJsonObject jsonMsg;
                            jsonMsg["chat_id"] = m_currentChatId;
                            jsonMsg["sender_id"] = m_currentUserId;
                            jsonMsg["content"] = imageMsg;
                            jsonMsg["client_id"] = clientId;
                            networkManager->sendJson("send_message", jsonMsg);
                        }
                    } else {
                        qDebug() << "❌ UPLOAD ERROR:" << reply->errorString();
                    }
                    reply->deleteLater();
                });
            }
        } else {
            // Обычный файл — загружаем через HTTP и отправляем метаданные
            QFile *file = new QFile(filePath);
            if (file->open(QIODevice::ReadOnly)) {
                QFileInfo fi(filePath);
                QByteArray fileData = file->readAll();
                file->deleteLater();

                qDebug() << "⬆️ UPLOADING FILE:" << fi.fileName() << "size:" << fileData.size();

                QNetworkRequest request(QUrl("http://87.242.118.96:8080/upload"));
                request.setRawHeader("X-File-Name", fi.fileName().toUtf8());
                request.setHeader(QNetworkRequest::ContentTypeHeader, "application/octet-stream");

                QNetworkReply *reply = m_httpManager->post(request, fileData);

                connect(reply, &QNetworkReply::finished, this, [reply, fi, this]() {
                    if (reply->error() == QNetworkReply::NoError) {
                        QByteArray response = reply->readAll();
                        QJsonObject obj = QJsonDocument::fromJson(response).object();
                        QString fileUrl = obj["url"].toString();

                        qDebug() << "✅ FILE UPLOAD SUCCESS:" << fileUrl;

                        if (!fileUrl.isEmpty()) {
                            QString fileMsg = QString("📎 %1|%2|%3")
                                                  .arg(fi.fileName(), fileUrl, QString::number(fi.size()));

                            QString clientId = QUuid::createUuid().toString(QUuid::WithoutBraces);

                            // Добавляем локальное сообщение
                            Message msg;
                            msg.id = -1;
                            msg.chatId = m_currentChatId;
                            msg.senderId = m_currentUserId;
                            msg.text = fileMsg;
                            msg.time = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
                            msg.clientId = clientId;
                            msg.localOrder = m_nextLocalOrder++;
                            msg.status = MessageStatus::Pending;

                            auto &messages = m_chatMessages[m_currentChatId];
                            messages.append(msg);

                            // Показываем в UI
                            m_rightPanel->addMessage("📎 " + fi.fileName(), true, static_cast<int>(MessageStatus::Pending));

                            qDebug() << "📤 SENDING FILE METADATA:" << fileMsg << "clientId:" << clientId;

                            QJsonObject jsonMsg;
                            jsonMsg["chat_id"] = m_currentChatId;
                            jsonMsg["sender_id"] = m_currentUserId;
                            jsonMsg["content"] = fileMsg;
                            jsonMsg["client_id"] = clientId;
                            networkManager->sendJson("send_message", jsonMsg);
                        }
                    } else {
                        qDebug() << "❌ FILE UPLOAD ERROR:" << reply->errorString();
                    }
                    reply->deleteLater();
                });
            }
        }
    });

    // === Теневая полоска ===
    m_shadowStrip = new QWidget(m_centralWidget);
    m_shadowStrip->setFixedWidth(20);
    m_shadowStrip->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    m_shadowStrip->setStyleSheet(
        "background: qlineargradient(x1:0, y1:0, x2:1, y2:0, "
        "stop:0 rgba(0, 0, 0, 100), "
        "stop:0.3 rgba(0, 0, 0, 50), "
        "stop:1 rgba(0, 0, 0, 0));"
        );
    m_shadowStrip->show();

    resize(1200, 1200);

    qDebug() << "STACK COUNT:" << m_stack->count();

    m_stack->lower();
    m_leftNav->raise();
}

void MainWindow::onNavigationChanged(NavItem item)
{
    qDebug() << "NAV:" << (int)item;

    switch(item)
    {
    case NavItem::Chat:
        if (m_stack) m_stack->setCurrentIndex(0);
        break;

    case NavItem::Tasks:
        onToggleSidePanel(5);
        break;

    case NavItem::CRM:
        onToggleSidePanel(8);
        break;

    case NavItem::Settings:
        onToggleSidePanel(6);
        break;

    case NavItem::Support:
        onToggleSidePanel(7);
        break;

    case NavItem::Documents:
        onToggleSidePanel(4);
        break;

    case NavItem::Companies:
        onToggleSidePanel(9);
        break;

    case NavItem::Employees:
        onToggleSidePanel(10);
        break;

    default:
        break;
    }
}

void MainWindow::onToggleSidePanel(int panelType)
{
    // 👉 если нажали ту же панель — закрываем
    if (panelType == m_currentPanelType && m_currentSidePanel && m_currentSidePanel->isVisible()) {
        onCloseSidePanel();
        return;
    }

    // 👉 скрываем предыдущую
    if (m_currentSidePanel) {
        m_currentSidePanel->hide();
        m_currentSidePanel = nullptr;
        m_currentPanelType = -1;
    }

    qDebug() << "OPEN PANEL:" << panelType;

    // ================= SETTINGS =================
    if (panelType == 6) {
        QWidget *dim = new QWidget(m_centralWidget);
        dim->setGeometry(m_centralWidget->rect());
        dim->setStyleSheet("background-color: rgba(0,0,0,180);");
        dim->show();
        SettingsDialog dialog(this);
        dialog.exec();
        dim->deleteLater();
        return;
    }

    // ================= SUPPORT =================
    if (panelType == 7) {
        QWidget *dim = new QWidget(m_centralWidget);
        dim->setGeometry(m_centralWidget->rect());
        dim->setStyleSheet("background-color: rgba(0,0,0,150);");
        dim->show();
        SupportDialog dialog(this);
        dialog.exec();
        dim->deleteLater();
        return;
    }

    // ================= PROFILE =================
    if (panelType == 11) {
        QWidget *dim = new QWidget(m_centralWidget);
        dim->setGeometry(m_centralWidget->rect());
        dim->setStyleSheet("background-color: rgba(0,0,0,180);");
        dim->show();

        ProfileDialog dialog(this);
        connect(&dialog, &ProfileDialog::logoutRequested, this, [this]() {
            qApp->quit();
            QProcess::startDetached(qApp->arguments()[0], qApp->arguments().mid(1));
        });
        dialog.exec();

        m_leftNav->updateProfileAvatar();  // ← ВЕРНУТЬ
        dim->deleteLater();
        return;
    }

    // ================= НАСТРОЙКА ОВЕРЛЕЯ =================
    if (!m_overlay) {
        m_overlay = new QWidget(m_centralWidget);
        m_overlay->setObjectName("sidePanelOverlay");
        m_overlay->setStyleSheet("background: transparent;");
    }

    int leftNavRight = m_leftNav->mapTo(m_centralWidget, QPoint(m_leftNav->width(), 0)).x();
    int contentTop = m_contentWidget->mapTo(m_centralWidget, QPoint(0, 0)).y();
    int contentHeight = m_contentWidget->height();
    int overlayWidth = m_centralWidget->width() - leftNavRight;

    m_overlay->setGeometry(leftNavRight, contentTop, overlayWidth, contentHeight);
    m_overlay->show();
    m_overlay->raise();
    m_shadowStrip->raise();

    // ================= TASKS =================
    if (panelType == 5) {
        if (!m_sidePanels.contains(5)) {
            TasksPanel *panel = new TasksPanel(m_overlay);
            panel->setFixedWidth(360);

            // Подключаем сигнал закрытия
            connect(panel, &SidePanel::closeRequested, this, [this]() {
                qDebug() << "TasksPanel closeRequested received";
                onCloseSidePanel();
            });

            m_sidePanels[5] = panel;
        }

        m_currentSidePanel = m_sidePanels[5];
        m_currentPanelType = 5;

        int w = m_currentSidePanel->width();
        int h = m_overlay->height();

        m_currentSidePanel->setGeometry(-w, 0, w, h);
        m_currentSidePanel->setWindowOpacity(0.0);
        m_currentSidePanel->show();
        m_currentSidePanel->raise();

        QPropertyAnimation *posAnim = new QPropertyAnimation(m_currentSidePanel, "geometry");
        posAnim->setDuration(250);
        posAnim->setStartValue(QRect(-w, 0, w, h));
        posAnim->setEndValue(QRect(0, 0, w, h));
        posAnim->setEasingCurve(QEasingCurve::OutCubic);

        QPropertyAnimation *fadeAnim = new QPropertyAnimation(m_currentSidePanel, "windowOpacity");
        fadeAnim->setDuration(200);
        fadeAnim->setStartValue(0.0);
        fadeAnim->setEndValue(1.0);
        fadeAnim->setEasingCurve(QEasingCurve::OutQuad);

        QParallelAnimationGroup *group = new QParallelAnimationGroup;
        group->addAnimation(posAnim);
        group->addAnimation(fadeAnim);
        group->start(QAbstractAnimation::DeleteWhenStopped);

        return;
    }

    // ================= CRM (CLIENTS) =================
    if (panelType == 8) {
        if (!m_sidePanels.contains(8)) {
            DealsPanel *panel = new DealsPanel(m_overlay);
            panel->setFixedWidth(400);

            // Передаем ожидающих клиентов
            if (!m_pendingClients.isEmpty()) {
                panel->setDealsFromJson(m_pendingClients);
                m_pendingClients = QJsonArray();
            }

            // ✅ Подключаем сигналы
            connect(panel, &DealsPanel::openChatWithClient, this, &MainWindow::onOpenChatWithClient);
            connect(panel, &SidePanel::closeRequested, this, [this]() {
                qDebug() << "DealsPanel closeRequested received";
                onCloseSidePanel();
            });

            m_sidePanels[8] = panel;
        }

        m_currentSidePanel = m_sidePanels[8];
        m_currentPanelType = 8;

        int w = m_currentSidePanel->width();
        int h = m_overlay->height();

        m_currentSidePanel->setGeometry(-w, 0, w, h);
        m_currentSidePanel->setWindowOpacity(0.0);
        m_currentSidePanel->show();
        m_currentSidePanel->raise();

        QPropertyAnimation *posAnim = new QPropertyAnimation(m_currentSidePanel, "geometry");
        posAnim->setDuration(250);
        posAnim->setStartValue(QRect(-w, 0, w, h));
        posAnim->setEndValue(QRect(0, 0, w, h));
        posAnim->setEasingCurve(QEasingCurve::OutCubic);

        QPropertyAnimation *fadeAnim = new QPropertyAnimation(m_currentSidePanel, "windowOpacity");
        fadeAnim->setDuration(200);
        fadeAnim->setStartValue(0.0);
        fadeAnim->setEndValue(1.0);
        fadeAnim->setEasingCurve(QEasingCurve::OutQuad);

        QParallelAnimationGroup *group = new QParallelAnimationGroup;
        group->addAnimation(posAnim);
        group->addAnimation(fadeAnim);
        group->start(QAbstractAnimation::DeleteWhenStopped);

        return;
    }

    // ================= COMPANIES =================
    if (panelType == 9) {
        if (!m_sidePanels.contains(9)) {
            CompaniesPanel *panel = new CompaniesPanel(m_overlay);
            if (!m_pendingCompanies.isEmpty()) {
                panel->setCompaniesFromJson(m_pendingCompanies);
                m_pendingCompanies = QJsonArray();
            }
            panel->setFixedWidth(420);

            // Подключаем сигнал закрытия
            connect(panel, &SidePanel::closeRequested, this, [this]() {
                qDebug() << "CompaniesPanel closeRequested received";
                onCloseSidePanel();
            });

            m_sidePanels[9] = panel;
        }

        m_currentSidePanel = m_sidePanels[9];
        m_currentPanelType = 9;

        int w = m_currentSidePanel->width();
        int h = m_overlay->height();

        m_currentSidePanel->setGeometry(-w, 0, w, h);
        m_currentSidePanel->setWindowOpacity(0.0);
        m_currentSidePanel->show();
        m_currentSidePanel->raise();

        QPropertyAnimation *posAnim = new QPropertyAnimation(m_currentSidePanel, "geometry");
        posAnim->setDuration(250);
        posAnim->setStartValue(QRect(-w, 0, w, h));
        posAnim->setEndValue(QRect(0, 0, w, h));
        posAnim->setEasingCurve(QEasingCurve::OutCubic);

        QPropertyAnimation *fadeAnim = new QPropertyAnimation(m_currentSidePanel, "windowOpacity");
        fadeAnim->setDuration(200);
        fadeAnim->setStartValue(0.0);
        fadeAnim->setEndValue(1.0);
        fadeAnim->setEasingCurve(QEasingCurve::OutQuad);

        QParallelAnimationGroup *group = new QParallelAnimationGroup;
        group->addAnimation(posAnim);
        group->addAnimation(fadeAnim);
        group->start(QAbstractAnimation::DeleteWhenStopped);

        return;
    }

    // ================= DOCUMENTS =================
    if (panelType == 4) {
        if (!m_sidePanels.contains(4)) {
            DocumentsPanel *panel = new DocumentsPanel(m_overlay);
            panel->setFixedWidth(400);
            panel->setUsername(m_currentUsername);

            if (!m_pendingDocuments.isEmpty()) {
                panel->setDocumentsFromJson(m_pendingDocuments);
                m_pendingDocuments = QJsonArray();
            }

            // Подключаем скачивание файлов
            connect(panel, &DocumentsPanel::downloadFileRequested, this, [this](const QString &fileUrl, const QString &fileName) {
                QString savePath = QFileDialog::getSaveFileName(nullptr, "Сохранить файл", fileName);
                if (!savePath.isEmpty()) {
                    downloadFileHttp(fileUrl, savePath);
                }
            });

            connect(panel, &SidePanel::closeRequested, this, [this]() {
                qDebug() << "DocumentsPanel closeRequested received";
                onCloseSidePanel();
            });

            m_sidePanels[4] = panel;
        }

        m_currentSidePanel = m_sidePanels[4];
        m_currentPanelType = 4;

        int w = m_currentSidePanel->width();
        int h = m_overlay->height();

        m_currentSidePanel->setGeometry(-w, 0, w, h);
        m_currentSidePanel->setWindowOpacity(0.0);
        m_currentSidePanel->show();
        m_currentSidePanel->raise();

        QPropertyAnimation *posAnim = new QPropertyAnimation(m_currentSidePanel, "geometry");
        posAnim->setDuration(250);
        posAnim->setStartValue(QRect(-w, 0, w, h));
        posAnim->setEndValue(QRect(0, 0, w, h));
        posAnim->setEasingCurve(QEasingCurve::OutCubic);

        QPropertyAnimation *fadeAnim = new QPropertyAnimation(m_currentSidePanel, "windowOpacity");
        fadeAnim->setDuration(200);
        fadeAnim->setStartValue(0.0);
        fadeAnim->setEndValue(1.0);
        fadeAnim->setEasingCurve(QEasingCurve::OutQuad);

        QParallelAnimationGroup *group = new QParallelAnimationGroup;
        group->addAnimation(posAnim);
        group->addAnimation(fadeAnim);
        group->start(QAbstractAnimation::DeleteWhenStopped);

        return;
    }

    // ================= EMPLOYEES =================
    if (panelType == 10) {
        if (!m_sidePanels.contains(10)) {
            EmployeesPanel *panel = new EmployeesPanel(m_overlay);

            // Подключаем сигнал закрытия
            connect(panel, &SidePanel::closeRequested, this, [this]() {
                qDebug() << "EmployeesPanel closeRequested received";
                onCloseSidePanel();
            });

            m_sidePanels[10] = panel;
        }

        m_currentSidePanel = m_sidePanels[10];
        m_currentPanelType = 10;

        int w = m_currentSidePanel->width();
        int h = m_overlay->height();

        m_currentSidePanel->setGeometry(-w, 0, w, h);
        m_currentSidePanel->setWindowOpacity(0.0);
        m_currentSidePanel->show();
        m_currentSidePanel->raise();

        QPropertyAnimation *posAnim = new QPropertyAnimation(m_currentSidePanel, "geometry");
        posAnim->setDuration(250);
        posAnim->setStartValue(QRect(-w, 0, w, h));
        posAnim->setEndValue(QRect(0, 0, w, h));
        posAnim->setEasingCurve(QEasingCurve::OutCubic);

        QPropertyAnimation *fadeAnim = new QPropertyAnimation(m_currentSidePanel, "windowOpacity");
        fadeAnim->setDuration(200);
        fadeAnim->setStartValue(0.0);
        fadeAnim->setEndValue(1.0);
        fadeAnim->setEasingCurve(QEasingCurve::OutQuad);

        QParallelAnimationGroup *group = new QParallelAnimationGroup;
        group->addAnimation(posAnim);
        group->addAnimation(fadeAnim);
        group->start(QAbstractAnimation::DeleteWhenStopped);

        return;
    }
    // ================= CALENDAR =================
    if (panelType == 3) {
        if (!m_sidePanels.contains(3)) {
            CalendarPanel *panel = new CalendarPanel(m_overlay);
            connect(panel, &SidePanel::closeRequested, this, [this]() {
                onCloseSidePanel();
            });
            m_sidePanels[3] = panel;
        }

        m_currentSidePanel = m_sidePanels[3];
        m_currentPanelType = 3;

        int w = m_currentSidePanel->width();
        int h = m_overlay->height();

        m_currentSidePanel->setGeometry(-w, 0, w, h);
        m_currentSidePanel->setWindowOpacity(0.0);
        m_currentSidePanel->show();
        m_currentSidePanel->raise();

        QPropertyAnimation *posAnim = new QPropertyAnimation(m_currentSidePanel, "geometry");
        posAnim->setDuration(250);
        posAnim->setStartValue(QRect(-w, 0, w, h));
        posAnim->setEndValue(QRect(0, 0, w, h));
        posAnim->setEasingCurve(QEasingCurve::OutCubic);

        QPropertyAnimation *fadeAnim = new QPropertyAnimation(m_currentSidePanel, "windowOpacity");
        fadeAnim->setDuration(200);
        fadeAnim->setStartValue(0.0);
        fadeAnim->setEndValue(1.0);
        fadeAnim->setEasingCurve(QEasingCurve::OutQuad);

        QParallelAnimationGroup *group = new QParallelAnimationGroup;
        group->addAnimation(posAnim);
        group->addAnimation(fadeAnim);
        group->start(QAbstractAnimation::DeleteWhenStopped);

        return;
    }

    qDebug() << "WARNING: Unknown panel type:" << panelType;
}

void MainWindow::onCloseSidePanel()
{
    qDebug() << "onCloseSidePanel called";

    if (!m_currentSidePanel || !m_currentSidePanel->isVisible()) {
        qDebug() << "No visible side panel to close";
        if (m_overlay) {
            m_overlay->hide();
        }
        m_shadowStrip->raise();
        m_shadowStrip->show();
        return;
    }

    QWidget *closingPanel = m_currentSidePanel;
    int w = closingPanel->width();
    int h = closingPanel->height();

    qDebug() << "Closing panel of width:" << w << "height:" << h;

    m_currentSidePanel = nullptr;
    m_currentPanelType = -1;

    // Анимация ухода влево
    QPropertyAnimation *posAnim = new QPropertyAnimation(closingPanel, "geometry");
    posAnim->setDuration(180);
    posAnim->setStartValue(QRect(0, 0, w, h));
    posAnim->setEndValue(QRect(-w, 0, w, h));
    posAnim->setEasingCurve(QEasingCurve::InCubic);

    // Анимация затухания
    closingPanel->setWindowOpacity(1.0);
    QPropertyAnimation *fadeAnim = new QPropertyAnimation(closingPanel, "windowOpacity");
    fadeAnim->setDuration(150);
    fadeAnim->setStartValue(1.0);
    fadeAnim->setEndValue(0.0);
    fadeAnim->setEasingCurve(QEasingCurve::InQuad);

    QParallelAnimationGroup *group = new QParallelAnimationGroup;
    group->addAnimation(posAnim);
    group->addAnimation(fadeAnim);

    connect(group, &QParallelAnimationGroup::finished, this, [closingPanel, this]() {
        qDebug() << "Close animation finished";
        closingPanel->hide();
        closingPanel->setGraphicsEffect(nullptr);
        if (m_overlay) {
            m_overlay->hide();
        }
        m_shadowStrip->raise();
        m_shadowStrip->show();
    });

    group->start(QAbstractAnimation::DeleteWhenStopped);
}

void MainWindow::toggleMaximized()
{
    if (isMaximized())
        showNormal();
    else
        showMaximized();
}

#ifdef Q_OS_WIN
bool MainWindow::nativeEvent(const QByteArray &eventType, void *message, qintptr *result)
{
    MSG* msg = static_cast<MSG*>(message);

    if (msg->message == WM_NCHITTEST) {
        QPoint globalPos = QCursor::pos();
        QPoint localPos = mapFromGlobal(globalPos);

        const int borderWidth = 8;

        // Проверяем, что клик не на заголовке (чтобы можно было перетаскивать)
        if (m_titleBar && m_titleBar->geometry().contains(localPos)) {
            QWidget *child = m_titleBar->childAt(m_titleBar->mapFromGlobal(globalPos));
            if (qobject_cast<QPushButton*>(child)) {
                return QMainWindow::nativeEvent(eventType, message, result);
            }
            *result = HTCAPTION;
            return true;
        }

        // Углы (приоритет выше)
        if (localPos.x() <= borderWidth && localPos.y() <= borderWidth) {
            *result = HTTOPLEFT;
            return true;
        }
        if (localPos.x() >= width() - borderWidth && localPos.y() <= borderWidth) {
            *result = HTTOPRIGHT;
            return true;
        }
        if (localPos.x() <= borderWidth && localPos.y() >= height() - borderWidth) {
            *result = HTBOTTOMLEFT;
            return true;
        }
        if (localPos.x() >= width() - borderWidth && localPos.y() >= height() - borderWidth) {
            *result = HTBOTTOMRIGHT;
            return true;
        }

        // Грани
        if (localPos.y() <= borderWidth) {
            *result = HTTOP;
            return true;
        }
        if (localPos.y() >= height() - borderWidth) {
            *result = HTBOTTOM;
            return true;
        }
        if (localPos.x() <= borderWidth) {
            *result = HTLEFT;
            return true;
        }
        if (localPos.x() >= width() - borderWidth) {
            *result = HTRIGHT;
            return true;
        }
    }

    return QMainWindow::nativeEvent(eventType, message, result);
}
#endif

void MainWindow::onNetworkConnected()
{
    qDebug() << "MainWindow: Connected to server!";

    if (m_currentUserId > 0) {
        // 🔥 СНАЧАЛА ЗАГРУЖАЕМ ЧАТЫ
        requestChats();
        requestContacts();
        requestProfile();
        requestClients();
        requestCompanies();
        requestDocuments();

        // Запросить статусы всех онлайн-пользователей
        QJsonObject data;
        networkManager->sendJson("get_online_users", data);

        // 🔥 ДОБАВИТЬ: запросить аватар текущего пользователя
        requestUserAvatar(m_currentUserId);
    }
}

void MainWindow::onNetworkDisconnected()
{
    qDebug() << "Disconnected from server!";

    // Попытка переподключения через 5 секунд
    QTimer::singleShot(5000, this, [this]() {
        if (networkManager && !networkManager->isConnected()) {
            networkManager->connectToServer(serverIp, serverPort);
        }
    });
}

void MainWindow::onNetworkError(const QString &error)
{
    qDebug() << "Network error:" << error;
}

void MainWindow::setCurrentUser(qint64 userId, const QString &username)
{
    qDebug() << "=== setCurrentUser:" << userId << username;

    // 🔥 ОЧИЩАЕМ КЕШ АВАТАРОВ ПРИ СМЕНЕ ПОЛЬЗОВАТЕЛЯ
    clearAvatarCache();

    m_currentUserId = userId;
    m_currentUsername = username;
    setWindowTitle("Aura - " + username);

    if (m_rightPanel) {
        m_rightPanel->setChatTitle("Выберите чат");
    }

    if (networkManager && networkManager->isConnected()) {
        qDebug() << "networkManager connected, calling requests";

        // 🔥 СНАЧАЛА ЧАТЫ
        requestChats();
        requestContacts();
        requestProfile();
        requestClients();
        requestCompanies();
        requestDocuments();
        loadPinnedChats();

        // 🔥 ДОБАВИТЬ: запросить аватар текущего пользователя
        requestUserAvatar(userId);
    } else {
        qDebug() << "networkManager NOT connected!";
    }
}

void MainWindow::requestContacts()
{
    if (!networkManager || !networkManager->isConnected())
        return;

    QJsonObject data;
    data["user_id"] = m_currentUserId;

    networkManager->sendJson("get_contacts", data);

    qDebug() << "Requesting contacts (JSON)";
}

void MainWindow::requestChats()
{
    if (!networkManager || !networkManager->isConnected())
        return;

    QJsonObject data;
    data["user_id"] = m_currentUserId;

    networkManager->sendJson("get_chats", data);

    qDebug() << "Requesting chats (JSON)";
}

void MainWindow::setNetworkManager(NetworkManager *manager)
{
    if (!manager) {
        qFatal("MainWindow: NetworkManager is null!");
        return;
    }

    if (networkManager == manager) {
        qDebug() << "MainWindow: same NetworkManager, skip";
        return;
    }

    if (networkManager) {
        disconnect(networkManager, nullptr, this, nullptr);
    }

    networkManager = manager;

    qDebug() << "MainWindow using NetworkManager:" << networkManager;

    connect(networkManager, &NetworkManager::connected,
            this, &MainWindow::onNetworkConnected);

    connect(networkManager, &NetworkManager::disconnected,
            this, &MainWindow::onNetworkDisconnected);

    connect(networkManager, &NetworkManager::errorOccurred,
            this, &MainWindow::onNetworkError);

    connect(networkManager, &NetworkManager::jsonReceived,
            this, &MainWindow::onJson);

    if (networkManager->isConnected()) {
        qDebug() << "Already connected, calling onNetworkConnected manually";
        onNetworkConnected();
    }

    // Создаем overlay для side panels (если еще не создан)
    if (!m_overlay) {
        m_overlay = new QWidget(m_centralWidget);
        m_overlay->setObjectName("sidePanelOverlay");
        m_overlay->setStyleSheet("background: transparent;");
        m_overlay->hide();
    }
}

void MainWindow::onContactSelected(qint64 contactId)
{
    qDebug() << "Contact selected:" << contactId;

    onClearSearch();

    if (!m_rightPanel)
        return;

    if (contactId == m_currentContactId && m_currentChatId != 0)
        return;

    QString contactName;

    for (const QVariantMap &contact : m_contacts) {
        if (contact["id"].toLongLong() == contactId) {
            contactName = contact["full_name"].toString();
            if (contactName.isEmpty()) {
                contactName = contact["username"].toString();
            }
            break;
        }
    }

    m_currentContactId = contactId;

    m_rightPanel->setChatTitle(contactName);
    m_rightPanel->setContactInfo(contactId, contactName,
                                 m_onlineUsers.contains(contactId), "");

    // Установить аватар из кеша если есть
    if (m_userAvatars.contains(contactId)) {
        m_rightPanel->setChatAvatar(m_userAvatars[contactId]);
    } else {
        // Запросить аватар с сервера
        requestUserAvatar(contactId);
    }

    qDebug() << "SEND user_id =" << contactId;

    if (contactId <= 0) {
        qDebug() << "INVALID contactId";
        return;
    }

    m_rightPanel->clearMessages();

    if (m_userToChat.contains(contactId)) {
        int chatId = m_userToChat[contactId];
        m_currentChatId = chatId;
        requestChatHistory(chatId);
    } else {
        m_currentChatId = 0;
        requestOrCreateChat(contactId);
    }

    saveLastChat();

    if (m_currentSidePanel) {
        onCloseSidePanel();
    }
}

void MainWindow::requestOrCreateChat(qint64 contactId)
{
    if (!networkManager || !networkManager->isConnected()) {
        qDebug() << "Cannot request chat: not connected";
        return;
    }

    QJsonObject data;
    data["user_id"] = contactId;

    networkManager->sendJson("get_or_create_chat", data);

    qDebug() << "Requesting chat with user:" << contactId;
}

void MainWindow::requestChatHistory(qint64 chatId)
{
    if (!networkManager || !networkManager->isConnected()) {
        return;
    }

    QJsonObject data;
    data["chat_id"] = chatId;
    data["limit"] = 50;

    networkManager->sendJson("get_messages", data);

    qDebug() << "Requesting messages for chat:" << chatId;
}

void MainWindow::saveLastChat()
{
    m_lastChatId = m_currentChatId;
    // Можно сохранить в QSettings для восстановления после перезапуска
    QSettings settings("Aura", "Messenger");
    settings.setValue("lastChatId", m_currentChatId);
    settings.setValue("lastContactId", m_currentContactId); // нужно добавить m_currentContactId
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);

    // Обновляем позицию shadow strip
    if (m_shadowStrip && m_leftNav && m_contentWidget) {
        int leftNavRight = m_leftNav->mapTo(m_centralWidget, QPoint(m_leftNav->width(), 0)).x();
        int contentTop = m_contentWidget->mapTo(m_centralWidget, QPoint(0, 0)).y();
        int contentHeight = m_contentWidget->height();
        m_shadowStrip->setGeometry(leftNavRight, contentTop, 20, contentHeight);
        m_shadowStrip->raise();
    }

    // Обновляем геометрию overlay и side panel
    if (m_overlay && m_contentWidget) {
        if (m_overlay->isVisible()) {
            int leftNavRight = m_leftNav->mapTo(m_centralWidget, QPoint(m_leftNav->width(), 0)).x();
            int contentTop = m_contentWidget->mapTo(m_centralWidget, QPoint(0, 0)).y();
            int contentHeight = m_contentWidget->height();
            int overlayWidth = m_centralWidget->width() - leftNavRight;

            m_overlay->setGeometry(leftNavRight, contentTop, overlayWidth, contentHeight);

            if (m_currentSidePanel) {
                int h = m_overlay->height();
                int w = m_currentSidePanel->width();
                m_currentSidePanel->setGeometry(0, 0, w, h);
            }
        }
    }

    // Обновляем dim widget если есть
    if (m_dimWidget) {
        m_dimWidget->setGeometry(this->rect());
    }
}

void MainWindow::onSearchUsersByNickname(const QString &nickname)
{
    QString clean = nickname.trimmed();

    if (clean.startsWith("@")) {
        clean.remove(0, 1);
    }

    if (clean.isEmpty()) {
        qDebug() << "Empty search query";
        return;
    }

    if (!networkManager || !networkManager->isConnected()) {
        qDebug() << "Cannot search: not connected";
        return;
    }

    // Переключаемся на список контактов
    m_middlePanel->showContacts();

    if (m_rightPanel) {
        m_rightPanel->clearMessages();
        m_rightPanel->setChatTitle("Результаты поиска: @" + clean);
    }

    QJsonObject data;
    data["query"] = clean;

    networkManager->sendJson("search_users", data);

    qDebug() << "Searching users:" << clean;
}

void MainWindow::onClearSearch()
{
    qDebug() << "onClearSearch called";

    // Очищаем поле поиска в MiddlePanel
    if (m_middlePanel) {
        // Возвращаемся к списку чатов (а не контактов)
        m_middlePanel->setChats(m_chats);

        // Восстанавливаем заголовок чата
        if (m_currentContactId > 0) {
            QString contactName;
            for (const QVariantMap &contact : m_contacts) {
                if (contact["id"].toLongLong() == m_currentContactId) {
                    contactName = contact["full_name"].toString();
                    if (contactName.isEmpty()) {
                        contactName = contact["username"].toString();
                    }
                    break;
                }
            }
            if (m_rightPanel) {
                m_rightPanel->setChatTitle(contactName);
            }
        } else if (m_rightPanel) {
            m_rightPanel->setChatTitle("Чат");
        }
    }
}

void MainWindow::onSearchContacts(const QString &query)
{
    qDebug() << "onSearchContacts called with query:" << query;

    if (query.isEmpty()) {
        onClearSearch();
        return;
    }

    // Переключаемся на список контактов
    m_middlePanel->showContacts();

    // Фильтрация контактов по имени
    QList<QVariantMap> filtered;
    for (const QVariantMap &contact : m_contacts) {
        QString name = contact["full_name"].toString();
        if (name.isEmpty()) {
            name = contact["username"].toString();
        }
        if (name.contains(query, Qt::CaseInsensitive)) {
            filtered.append(contact);
        }
    }

    qDebug() << "Filtered contacts:" << filtered.size() << "from" << m_contacts.size();
    m_middlePanel->setContacts(filtered);
}

void MainWindow::displaySearchResults(const QList<QVariantMap> &users)
{
    qDebug() << "displaySearchResults called with" << users.size() << "users";

    // Переключаемся на список контактов
    m_middlePanel->showContacts();

    // Отображаем результаты поиска в MiddlePanel
    QList<QVariantMap> contacts;
    for (const QVariantMap &user : users) {
        QVariantMap contact;
        contact["id"] = user["id"];
        contact["username"] = user["username"];
        contact["full_name"] = user["full_name"].toString().isEmpty() ?
                                   user["username"].toString() :
                                   user["full_name"].toString();
        contact["status"] = user["status"];
        contacts.append(contact);
    }

    // Сначала устанавливаем контакты (создаются кнопки)
    m_middlePanel->setContacts(contacts);

    // Затем для каждого пользователя проверяем аватар
    for (const QVariantMap &user : users) {
        int userId = user["id"].toInt();

        // Проверяем кеш
        QString cachePath = QStandardPaths::writableLocation(QStandardPaths::CacheLocation)
                            + "/avatars/" + QString::number(userId) + ".jpg";

        if (QFile::exists(cachePath)) {
            qDebug() << "Avatar in cache for user:" << userId << "- updating UI";
            m_middlePanel->updateUserAvatar(userId, cachePath);
        } else {
            qDebug() << "Avatar not in cache for user:" << userId << "- requesting";
            requestUserAvatar(userId);
        }
    }
}

void MainWindow::onJson(const QJsonObject& obj)
{
    qDebug() << "=== RAW JSON RECEIVED ===";
    qDebug() << QJsonDocument(obj).toJson(QJsonDocument::Indented);
    qDebug() << "==========================";

    QString type = obj["type"].toString();
    QJsonObject data = obj["data"].toObject();

    qDebug() << "JSON TYPE:" << type;

    // Логируем все get_user_avatar сообщения
    if (type == "get_user_avatar") {
        qDebug() << "🔔 get_user_avatar received:" << obj;
    }

    if (type == "contacts") {
        QJsonArray arr = data["contacts"].toArray();
        m_contacts.clear();

        for (const QVariantMap &contact : m_contacts) {
            requestUserAvatar(contact["id"].toInt());
        }

        for (const QJsonValue &v : arr) {
            QJsonObject c = v.toObject();
            QVariantMap contact;
            contact["id"] = c["id"].toInt();
            contact["username"] = c["username"].toString();
            contact["full_name"] = c["full_name"].toString();
            contact["status"] = c["status"].toInt();
            m_contacts.append(contact);

            // 🔥 ЗАПРАШИВАЕМ АВАТАР ДЛЯ КАЖДОГО КОНТАКТА
            requestUserAvatar(contact["id"].toInt());
        }

        // Если ожидаем создание чата с клиентом из CRM
        if (!m_pendingClientName.isEmpty()) {
            for (const QVariantMap &contact : m_contacts) {
                QString name = contact["full_name"].toString();
                if (name.isEmpty()) name = contact["username"].toString();
                if (name == m_pendingClientName) {
                    qint64 contactId = contact["id"].toLongLong();
                    m_rightPanel->setChatTitle(m_pendingClientName);
                    m_currentContactId = contactId;
                    m_currentChatId = 0;
                    requestOrCreateChat(contactId);
                    m_pendingClientName.clear();
                    m_pendingClientPhone.clear();
                    break;
                }
            }
        }

        qDebug() << "Loaded contacts:" << m_contacts.size();
    }
    else if (type == "chats") {
        qDebug() << "=== CHATS RECEIVED ===";
        qDebug() << "Chats count:" << data["chats"].toArray().size();
        m_chats.clear();
        m_unreadCount.clear();

        QJsonArray arr = data["chats"].toArray();

        for (const QJsonValue &v : arr) {
            QJsonObject c = v.toObject();

            QVariantMap chat;
            chat["id"] = c["id"].toInt();

            // Название чата - имя собеседника
            QString chatName = c["name"].toString();
            if (chatName.isEmpty()) chatName = c["title"].toString();

            int otherUserId = c["user_id"].toInt();
            if (chatName.isEmpty() && otherUserId > 0) {
                // Ищем имя в уже загруженных контактах
                for (const QVariantMap &contact : m_contacts) {
                    if (contact["id"].toInt() == otherUserId) {
                        chatName = contact["username"].toString();
                        break;
                    }
                }
            }
            if (chatName.isEmpty()) {
                chatName = "Чат #" + QString::number(c["id"].toInt());
            }

            chat["title"] = chatName;
            chat["name"] = chatName;
            chat["last_message"] = c["last_message"].toString();
            chat["last_time"] = c["last_time"].toString();
            chat["unread"] = c["unread"].toInt();
            chat["user_id"] = otherUserId;

            int chatId = chat["id"].toInt();
            m_unreadCount[chatId] = chat["unread"].toInt();

            if (otherUserId > 0) {
                qDebug() << "Mapping user" << otherUserId << "to chat" << chatId;
                m_userToChat[otherUserId] = chatId;
            }

            sortChats();
            m_middlePanel->setChats(m_chats);

            m_chats.append(chat);

            if (otherUserId > 0) {
                requestUserAvatar(otherUserId);
            }
        }

        m_middlePanel->setChats(m_chats);
        updateChatAvatars();

        // Повторно применяем кешированные аватары
        for (auto it = m_userAvatars.begin(); it != m_userAvatars.end(); ++it) {
            m_middlePanel->updateUserAvatar(it.key(), it.value());
        }
    }
    else if (type == "messages") {
        qint64 chatId = data["chat_id"].toVariant().toLongLong();
        bool hasMore = data["has_more"].toBool();
        int beforeId = data["before_id"].toInt(); // if present, this is pagination load

        if (chatId != m_currentChatId)
            return;

        QJsonArray arr = data["messages"].toArray();

        auto &messages = m_chatMessages[chatId];
        QVector<Message> newMessages;
        newMessages.reserve(arr.size());

        for (const QJsonValue &v : arr) {
            QJsonObject mObj = v.toObject();

            Message msg;
            msg.id = mObj["id"].toInt();
            msg.chatId = chatId;
            msg.senderId = mObj["sender_id"].toInt();
            msg.text = mObj["text"].toString();
            msg.time = mObj["time"].toString();
            msg.clientId = mObj["client_id"].toString();
            MessageStatus serverStatus = static_cast<MessageStatus>(mObj["status"].toInt());
            // Если у нас уже сохранён статус Read — не понижаем его
            if (m_chatMessages.contains(chatId)) {
                for (const Message &existing : m_chatMessages[chatId]) {
                    if (existing.id == msg.id && existing.status == MessageStatus::Read) {
                        msg.status = MessageStatus::Read;
                        goto statusDone;
                    }
                }
            }
            msg.status = serverStatus;
            statusDone:;

            newMessages.append(msg);
        }

        // For pagination (before_id present), prepend messages
        if (beforeId > 0) {
            // Server sends DESC, reverse to ASC for prepend
            std::reverse(newMessages.begin(), newMessages.end());
            // Prepend new messages to existing ones
            QVector<Message> combined;
            combined.reserve(newMessages.size() + messages.size());
            combined.append(newMessages);
            combined.append(messages);
            messages = std::move(combined);
        } else {
            // Initial load: merge with pending
            auto messageOrder = [](const Message &a, const Message &b) {
                auto aKey = a.id > 0 ? a.id : std::numeric_limits<int>::max();
                auto bKey = b.id > 0 ? b.id : std::numeric_limits<int>::max();
                if (aKey != bKey)
                    return aKey < bKey;
                return a.time < b.time;
            };

            std::sort(newMessages.begin(), newMessages.end(), messageOrder);

            newMessages.erase(std::unique(newMessages.begin(), newMessages.end(), [](const Message &a, const Message &b) {
                return a.id > 0 && b.id > 0 && a.id == b.id;
            }), newMessages.end());

            QSet<QString> serverClientIds;
            for (const auto &msg : newMessages) {
                if (!msg.clientId.isEmpty())
                    serverClientIds.insert(msg.clientId);
            }

            QVector<Message> pendingMessages;
            for (const Message &existing : qAsConst(messages)) {
                if (existing.id == -1 && !existing.clientId.isEmpty()) {
                    if (!serverClientIds.contains(existing.clientId)) {
                        pendingMessages.append(existing);
                    }
                }
            }

            messages = newMessages;
            for (const Message &pending : qAsConst(pendingMessages)) {
                messages.append(pending);
            }

            QVector<Message> merged;
            merged.reserve(messages.size());
            QSet<int> seenIds;
            QSet<QString> seenClientIds;

            for (const Message &msg : qAsConst(messages)) {
                if (msg.id > 0) {
                    if (seenIds.contains(msg.id))
                        continue;
                    seenIds.insert(msg.id);
                } else if (!msg.clientId.isEmpty()) {
                    if (seenClientIds.contains(msg.clientId))
                        continue;
                    seenClientIds.insert(msg.clientId);
                }
                merged.append(msg);
            }

            std::sort(merged.begin(), merged.end(), messageOrder);
            messages = std::move(merged);

            const int MAX_MESSAGES = 200;
            if (messages.size() > MAX_MESSAGES) {
                messages.erase(messages.begin(), messages.end() - MAX_MESSAGES);
            }

            int highestRemoteId = 0;
            for (const Message &msg : qAsConst(messages)) {
                if (msg.senderId != m_currentUserId && msg.id > highestRemoteId) {
                    highestRemoteId = msg.id;
                }
            }
            if (highestRemoteId > m_lastReadSent.value(chatId, 0)) {
                m_lastReadSent[chatId] = highestRemoteId;
            }
        }

        // Re-render the chat
        renderChatMessages(chatId, beforeId > 0);

        // Update lazy loading state
        m_rightPanel->setHasMore(hasMore);
        if (beforeId > 0) {
            m_loadingOlder = false;
            m_rightPanel->setLoadingOlder(false);
        }
    }
    else if (type == "new_message") {
        int chatId = data["chat_id"].toInt();
        QString clientId = data["client_id"].toString();

        qDebug() << "📥 NEW MESSAGE received:"
                 << "chatId:" << chatId
                 << "clientId:" << clientId
                 << "senderId:" << data["sender_id"].toInt()
                 << "status:" << data["status"].toInt()
                 << "text:" << data["text"].toString().left(50);

        Message msg;
        msg.id = data["id"].toInt();
        msg.chatId = chatId;
        msg.senderId = data["sender_id"].toInt();
        msg.text = data["text"].toString();
        msg.time = data["time"].toString();
        msg.clientId = clientId;
        msg.status = static_cast<MessageStatus>(data["status"].toInt());

        auto &messages = m_chatMessages[chatId];

        // 🔥 Ищем сообщение с таким же client_id
        bool matchedClient = false;
        if (!clientId.isEmpty()) {
            for (Message &existing : messages) {
                if (!existing.clientId.isEmpty() && existing.clientId == clientId) {
                    qDebug() << "✅ MATCHED by clientId:" << clientId
                             << "old status:" << static_cast<int>(existing.status)
                             << "new status:" << static_cast<int>(msg.status);
                    existing.id = msg.id;
                    existing.time = msg.time;
                    existing.status = msg.status;
                    matchedClient = true;

                    if (chatId == m_currentChatId) {
                        renderChatMessages(chatId);
                    }
                    break;
                }
            }
        }

        if (!matchedClient) {
            qDebug() << "🆕 NEW message, not matched by clientId, adding to list";
            bool alreadyExists = std::any_of(messages.begin(), messages.end(), [&](const Message &existing) {
                if (existing.id > 0 && msg.id > 0)
                    return existing.id == msg.id;
                return false;
            });

            if (!alreadyExists) {
                msg.localOrder = m_nextLocalOrder++;
                messages.append(msg);

                if (chatId == m_currentChatId && msg.senderId != m_currentUserId) {
                    m_rightPanel->addMessage(msg.text, false, static_cast<int>(msg.status));
                }
            }
        }

        if (msg.senderId != m_currentUserId && msg.id > 0 && msg.status < MessageStatus::Delivered) {
            qDebug() << "📩 Sending delivered ACK for msg:" << msg.id;
            sendDeliveredAck(chatId, msg.id);
        }

        if (chatId == m_currentChatId && msg.senderId != m_currentUserId && msg.id > m_lastReadSent.value(chatId, 0)) {
            qDebug() << "📖 Sending read receipt for chat:" << chatId;
            sendReadReceipt(chatId);
            m_lastReadSent[chatId] = qMax(m_lastReadSent.value(chatId, 0), msg.id);
        }
    }
    else if (type == "delivered") {
        int messageId = data["message_id"].toInt();
        if (messageId > 0) {
            updateMessageStatusById(messageId, MessageStatus::Delivered);
        }
    }
    else if (type == "read") {
        int chatId = data["chat_id"].toInt();
        int readByUserId = data["user_id"].toInt();

        if (chatId > 0) {
            // Обновляем статусы сообщений на Read
            bool updated = false;
            for (Message &msg : m_chatMessages[chatId]) {
                if (msg.senderId == m_currentUserId) {
                    MessageStatus newStatus = std::max(msg.status, MessageStatus::Read);
                    if (newStatus != msg.status) {
                        msg.status = newStatus;
                        updated = true;
                    }
                }
            }

            // 🔥 Сбрасываем счётчик непрочитанных, если читающий — это мы
            if (readByUserId == m_currentUserId) {
                m_unreadCount[chatId] = 0;
            }

            if (updated && chatId == m_currentChatId) {
                renderChatMessages(chatId);
            }

            // 🔥 Обновляем список чатов для сброса счётчика
            for (auto &chat : m_chats) {
                if (chat["id"].toInt() == chatId) {
                    chat["unread"] = m_unreadCount.value(chatId, 0);
                    break;
                }
            }
            m_middlePanel->setChats(m_chats);
            updateChatAvatars();
        }
    }
    else if (type == "user_online") {
        int uid = data["user_id"].toInt();
        m_onlineUsers.insert(uid);
        m_middlePanel->setUserOnline(uid, true);

        // Обновить статус в заголовке чата если это текущий собеседник
        if (m_currentContactId == uid) {
            m_rightPanel->setContactInfo(uid, m_rightPanel->getChatTitle(), true, "");
        }
    }
    else if (type == "user_offline") {
        int uid = data["user_id"].toInt();
        m_onlineUsers.remove(uid);
        m_middlePanel->setUserOnline(uid, false);

        if (m_currentContactId == uid) {
            // Обновляем только статус, не трогаем аватар и имя
            m_rightPanel->setContactInfo(uid, m_rightPanel->getChatTitle(), false, "");
        }
    }
    else if (type == "typing") {
        auto d = obj["data"].toObject();

        int chatId = d["chat_id"].toInt();
        QString username = d["username"].toString();

        if (chatId == m_currentChatId) {
            m_rightPanel->showTyping(username);
        }
    }
    else if (type == "chat_ready") {
        int chatId = data["chat_id"].toInt();

        qDebug() << "CHAT READY:" << chatId;

        m_currentChatId = chatId;

        QJsonObject req;
        req["type"] = "get_messages";

        QJsonObject d;
        d["chat_id"] = chatId;

        req["data"] = d;

        networkManager->sendJson("get_messages", d);
    }
    else if (type == "user_online") {
        int uid = data["user_id"].toInt();

        m_middlePanel->setUserOnline(uid, true);
    }
    else if (type == "user_offline") {
        int uid = data["user_id"].toInt();

        m_middlePanel->setUserOnline(uid, false);
    }
    else if (type == "register_ok") {
        qDebug() << "REGISTER SUCCESS";
        emit registrationCompleted();
    }
    else if (type == "search_users") {
        QJsonArray arr = data["users"].toArray();

        if (!arr.isEmpty()) {
            if (!m_pendingClientName.isEmpty()) {
                // Берём первого найденного пользователя
                QJsonObject user = arr.first().toObject();
                qint64 contactId = user["id"].toVariant().toLongLong();
                QString foundName = user["full_name"].toString();
                if (foundName.isEmpty()) {
                    foundName = user["username"].toString();
                }

                m_rightPanel->setChatTitle(foundName);
                m_currentContactId = contactId;
                m_currentChatId = 0;
                requestOrCreateChat(contactId);
                m_pendingClientName.clear();
                m_pendingClientPhone.clear();
            } else {
                // Обычный поиск пользователей
                QList<QVariantMap> users;
                for (const QJsonValue &v : arr) {
                    QVariantMap user = v.toObject().toVariantMap();
                    // 🔥 Убедитесь, что есть full_name
                    if (user["full_name"].toString().isEmpty()) {
                        user["full_name"] = user["username"];
                    }
                    users.append(user);
                }
                displaySearchResults(users);
            }
        } else {
            // Пользователь не найден
            if (!m_pendingClientName.isEmpty()) {
                m_rightPanel->setChatTitle(m_pendingClientName);
                m_rightPanel->addMessage("Клиент \"" + m_pendingClientName + "\" не найден в системе.", false);
                m_pendingClientName.clear();
                m_pendingClientPhone.clear();
            } else {
                // Показываем пустой результат поиска
                m_middlePanel->setContacts(QList<QVariantMap>());
                if (m_rightPanel) {
                    m_rightPanel->setChatTitle("Результаты поиска");
                    m_rightPanel->clearMessages();
                    m_rightPanel->addMessage("Пользователи не найдены.", false);
                }
            }
        }
    }
    else if (type == "get_profile") {
        QJsonObject profile = data["profile"].toObject();
        QSettings settings("Aura", "Messenger");
        settings.setValue("profile/name", profile["name"].toString());
        settings.setValue("profile/position", profile["position"].toString());
        settings.setValue("profile/department", profile["department"].toString());
        settings.setValue("profile/tabNumber", profile["tabNumber"].toString());
        settings.setValue("profile/email", profile["email"].toString());
        settings.setValue("profile/phone", profile["phone"].toString());

        // 🔥 Сохраняем avatarPath в правильный ключ
        QString avatarKey = "userAvatar_" + QString::number(m_currentUserId);
        settings.setValue(avatarKey, profile["avatarPath"].toString());

        m_leftNav->updateProfileAvatar();
    }
    else if (type == "save_profile") {
        qDebug() << "Profile saved to server";
    }
    else if (type == "get_clients") {
        qDebug() << "=== get_clients received";
        QJsonArray clients = data["clients"].toArray();

        DealsPanel *dealsPanel = qobject_cast<DealsPanel*>(m_sidePanels.value(8));
        if (dealsPanel) {
            dealsPanel->setDealsFromJson(clients);
        } else {
            // Сохраняем для передачи при создании панели
            m_pendingClients = clients;
        }
    }
    else if (type == "save_clients") {
        qDebug() << "Clients saved to server";
    }
    else if (type == "get_companies") {
        QJsonArray companies = data["companies"].toArray();
        CompaniesPanel *panel = qobject_cast<CompaniesPanel*>(m_sidePanels.value(9));
        if (panel) panel->setCompaniesFromJson(companies);
        else m_pendingCompanies = companies;
    }
    else if (type == "get_documents") {
        qDebug() << "=== get_documents received";
        QJsonArray documents = data["documents"].toArray();
        DocumentsPanel *panel = qobject_cast<DocumentsPanel*>(m_sidePanels.value(4));
        if (panel) {
            panel->setDocumentsFromJson(documents);
        } else {
            m_pendingDocuments = documents;
        }
    }
    else if (type == "save_documents") {
        qDebug() << "Documents saved to server";
    }
    else if (type == "get_user_avatar") {
        int userId = data["user_id"].toInt();
        QString avatarPath = data["avatar_path"].toString();

        qDebug() << "get_user_avatar - userId:" << userId << "avatarPath:" << avatarPath;

        if (!avatarPath.isEmpty()) {
            // Сохраняем в кеш
            m_userAvatars[userId] = avatarPath;

            // Сохраняем в QSettings для постоянного хранения
            QSettings settings("Aura", "Messenger");
            QString avatarKey = "userAvatar_" + QString::number(userId);
            settings.setValue(avatarKey, avatarPath);

            if (avatarPath.startsWith("/files/")) {
                // Скачиваем с сервера
                downloadAvatarFromServer(avatarPath, userId);
            } else if (QFile::exists(avatarPath)) {
                // Локальный файл - сразу обновляем UI
                m_middlePanel->updateUserAvatar(userId, avatarPath);
                m_middlePanel->updateChatAvatar(userId, avatarPath);

                // Если это текущий пользователь - обновляем левую панель
                if (userId == m_currentUserId && m_leftNav) {
                    m_leftNav->updateProfileAvatar();
                }
            }
        } else {
            // Нет аватара - удаляем из кеша
            qDebug() << "No avatar for user:" << userId;
            m_userAvatars.remove(userId);

            // Очищаем сохраненный путь
            QSettings settings("Aura", "Messenger");
            QString avatarKey = "userAvatar_" + QString::number(userId);
            settings.remove(avatarKey);

            // Обновляем UI с первой буквой
            m_middlePanel->updateUserAvatar(userId, "");
            m_middlePanel->updateChatAvatar(userId, "");
        }
    }
    else if (type == "user_online") {
        int uid = data["user_id"].toInt();
        m_onlineUsers.insert(uid);
        m_middlePanel->setUserOnline(uid, true);

        // Обновить статус в заголовке чата
        if (uid == m_currentContactId) {
            m_rightPanel->setContactInfo(uid, m_rightPanel->getChatTitle(), true, "");
        }
    }
    else if (type == "user_offline") {
        int uid = data["user_id"].toInt();
        m_onlineUsers.remove(uid);
        m_middlePanel->setUserOnline(uid, false);

        if (uid == m_currentContactId) {
            m_rightPanel->setContactInfo(uid, m_rightPanel->getChatTitle(), false, "");
        }
    }
    else if (type == "avatar_updated") {
        int userId = data["user_id"].toInt();
        QString avatarPath = data["avatar_path"].toString();

        qDebug() << "avatar_updated - userId:" << userId << "avatarPath:" << avatarPath;

        // 🔥 СОХРАНЯЕМ В QSETTINGS СРАЗУ
        QSettings settings("Aura", "Messenger");
        QString avatarKey = "userAvatar_" + QString::number(userId);
        settings.setValue(avatarKey, avatarPath);

        m_userAvatars[userId] = avatarPath;

        // Если это локальный путь, копируем в кеш
        if (!avatarPath.startsWith("/files/") && QFile::exists(avatarPath)) {
            QString cacheDir = QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + "/avatars/";
            QDir().mkpath(cacheDir);
            QString cachePath = cacheDir + QString::number(userId) + ".jpg";
            QFile::copy(avatarPath, cachePath);
            m_userAvatars[userId] = cachePath;
            settings.setValue(avatarKey, cachePath);
            avatarPath = cachePath;
        }

        m_middlePanel->updateUserAvatar(userId, avatarPath);
        m_middlePanel->updateChatAvatar(userId, avatarPath);

        // Обновить в заголовке чата
        if (m_currentContactId == userId && m_rightPanel) {
            m_rightPanel->setChatAvatar(avatarPath);
        }

        // Обновить в левой панели (профиль пользователя)
        if (userId == m_currentUserId && m_leftNav) {
            m_leftNav->updateProfileAvatar();
        }
    }
    else if (type == "get_user_profile") {
        if (data["status"].toString() == "ok") {
            QJsonObject profile = data["profile"].toObject();

            // 🔥 Получаем актуальный путь к аватару из кеша
            QString avatarPath = m_pendingProfileAvatar;
            if (!avatarPath.isEmpty() && avatarPath.startsWith("/files/")) {
                // Пробуем получить из кеша
                QString cachePath = QStandardPaths::writableLocation(QStandardPaths::CacheLocation)
                                    + "/avatars/" + QString::number(m_pendingProfileUserId) + ".jpg";
                if (QFile::exists(cachePath)) {
                    avatarPath = cachePath;
                    qDebug() << "Using cached avatar for dialog:" << avatarPath;
                }
            } else if (!avatarPath.isEmpty() && !QFile::exists(avatarPath)) {
                // Если путь не существует, пробуем кеш
                QString cachePath = QStandardPaths::writableLocation(QStandardPaths::CacheLocation)
                                    + "/avatars/" + QString::number(m_pendingProfileUserId) + ".jpg";
                if (QFile::exists(cachePath)) {
                    avatarPath = cachePath;
                }
            }

            QWidget *dim = new QWidget(m_centralWidget);
            dim->setGeometry(m_centralWidget->rect());
            dim->setStyleSheet("background-color: rgba(0,0,0,180);");
            dim->show();

            UserProfileDialog dialog(m_pendingProfileUserId, m_pendingProfileUsername,
                                     avatarPath, m_pendingProfileOnline,
                                     profile, dim);
            dialog.exec();
            dim->deleteLater();
        }
    }
    else if (type == "online_users") {
        QJsonArray users = data["users"].toArray();
        for (const QJsonValue &v : users) {
            int userId = v.toInt();
            m_onlineUsers.insert(userId);
            m_middlePanel->setUserOnline(userId, true);

            // Обновить статус в заголовке чата если это текущий собеседник
            if (m_currentContactId == userId) {
                m_rightPanel->setContactInfo(userId, m_rightPanel->getChatTitle(), true, "");
            }
        }
    }
    else if (type == "confirmation_sent") {
        QString status = data["status"].toString();
        if (status == "ok") {
            emit confirmationSent(true);
        } else {
            QString error = data["error"].toString();
            QMessageBox::warning(this, "Ошибка", "Не удалось отправить код: " + error);
            emit confirmationSent(false);
        }
    }
    else if (type == "register_ok") {
        qDebug() << "REGISTER SUCCESS";
        QMessageBox::information(this, "Успех", "Аккаунт создан. Теперь вы можете войти.");
        emit registrationCompleted();
    }
}

void MainWindow::onChatsReceived(const QJsonArray &chats)
{
    QList<QVariantMap> list;

    for (auto v : chats) {
        QJsonObject o = v.toObject();

        QVariantMap map;
        map["id"] = o["id"].toInt();
        map["title"] = o["title"].toString();

        list.append(map);
    }

    m_middlePanel->setChats(list);
}

void MainWindow::onChatSelected(const QString &chatId)
{
    qDebug() << "=== onChatSelected ===";
    qDebug() << "chatId:" << chatId;

    m_currentChatId = chatId.toInt();
    qDebug() << "m_currentChatId set to:" << m_currentChatId;

    // 🔥 Сбрасываем счётчик непрочитанных
    m_unreadCount[m_currentChatId] = 0;

    for (const QVariantMap &chat : m_chats) {
        if (chat["id"].toInt() == m_currentChatId) {
            int otherUserId = chat["user_id"].toInt();
            QString chatName = chat["title"].toString();

            m_currentContactId = otherUserId;
            m_rightPanel->setChatTitle(chatName);
            m_rightPanel->setContactInfo(otherUserId, chatName,
                                         m_onlineUsers.contains(otherUserId), "");

            if (m_userAvatars.contains(otherUserId)) {
                m_rightPanel->setChatAvatar(m_userAvatars[otherUserId]);
            }
            break;
        }
    }

    m_rightPanel->setPinned(m_pinnedChats.contains(m_currentChatId));

    // Если сообщения уже загружены — просто перерисовать их
    if (m_chatMessages.contains(m_currentChatId) && !m_chatMessages[m_currentChatId].isEmpty()) {
        renderChatMessages(m_currentChatId);
    } else {
        // Иначе запросить с сервера
        m_rightPanel->clearMessages();
        QJsonObject data;
        data["chat_id"] = m_currentChatId;
        data["limit"] = 50;
        networkManager->sendJson("get_messages", data);
    }

    sendReadReceipt(m_currentChatId);
}

void MainWindow::renderChatMessages(int chatId, bool preserveScroll)
{
    if (chatId != m_currentChatId)
        return;

    const auto &messages = m_chatMessages[chatId];

    // Save scroll position if preserving
    int scrollValue = 0;
    if (preserveScroll && m_rightPanel && m_rightPanel->getScrollArea()) {
        scrollValue = m_rightPanel->getScrollArea()->verticalScrollBar()->value();
    }

    m_rightPanel->clearMessages();

    for (const Message &msg : qAsConst(messages)) {
        bool outgoing = (msg.senderId == m_currentUserId);
        m_rightPanel->addMessage(msg.text, outgoing, static_cast<int>(msg.status));
    }

    // Restore scroll position
    if (preserveScroll && m_rightPanel && m_rightPanel->getScrollArea()) {
        m_rightPanel->getScrollArea()->verticalScrollBar()->setValue(scrollValue);
    }
}

bool MainWindow::updateMessageStatusById(int messageId, MessageStatus status)
{
    for (auto it = m_chatMessages.begin(); it != m_chatMessages.end(); ++it) {
        auto &messages = it.value();
        for (Message &msg : messages) {
            if (msg.id > 0 && msg.id == messageId) {
                msg.status = std::max(msg.status, status);
                if (it.key() == m_currentChatId) {
                    renderChatMessages(m_currentChatId);
                }
                return true;
            }
        }
    }
    return false;
}

void MainWindow::sendDeliveredAck(int chatId, int messageId)
{
    if (!networkManager || messageId <= 0)
        return;

    QJsonObject data;
    data["chat_id"] = chatId;
    data["message_id"] = messageId;
    networkManager->sendJson("delivered", data);
}

void MainWindow::sendReadReceipt(int chatId)
{
    if (!networkManager || chatId <= 0)
        return;

    QJsonObject data;
    data["chat_id"] = chatId;
    networkManager->sendJson("read", data);
}

void MainWindow::loadOlderMessages()
{
    if (!networkManager || m_currentChatId <= 0 || m_loadingOlder)
        return;

    auto& messages = m_chatMessages[m_currentChatId];
    if (messages.isEmpty())
        return;

    // Find the smallest id (oldest message)
    int minId = INT_MAX;
    for (const auto& msg : messages) {
        if (msg.id < minId) {
            minId = msg.id;
        }
    }

    if (minId == INT_MAX)
        return;

    m_loadingOlder = true;
    if (m_rightPanel) {
        m_rightPanel->setLoadingOlder(true);
    }

    QJsonObject data;
    data["chat_id"] = m_currentChatId;
    data["before_id"] = minId;
    data["limit"] = 30;

    networkManager->sendJson("get_messages", data);
}

void MainWindow::onMessagesReceived(int chatId, const QJsonArray &messages)
{
    if (chatId != m_currentChatId)
        return;

    for (auto v : messages) {
        QJsonObject msg = v.toObject();

        QString text = msg["text"].toString();
        int sender = msg["sender_id"].toInt();

        bool outgoing = (sender == m_currentUserId);

        m_rightPanel->addMessage(text, outgoing);
    }
}

void MainWindow::onNewMessage(const QJsonObject &msg)
{
    int chatId = msg["chat_id"].toInt();

    if (chatId != m_currentChatId)
        return;

    QString text = msg["text"].toString();

    m_rightPanel->addMessage(text, false);
}

void MainWindow::onSendMessage(const QString &text)
{
    qDebug() << "=== onSendMessage ===";
    qDebug() << "text:" << text;
    qDebug() << "networkManager:" << networkManager;

    if (!networkManager || !networkManager->isConnected()) {
        qDebug() << "❌ Not connected";
        return;
    }

    if (m_currentChatId == 0) {
        qDebug() << "❌ No chat selected, m_currentChatId:" << m_currentChatId;
        return;
    }

    QString clientId = QUuid::createUuid().toString(QUuid::WithoutBraces);

    QJsonObject data;
    data["chat_id"] = m_currentChatId;
    data["sender_id"] = m_currentUserId;
    data["content"] = text;
    data["client_id"] = clientId;

    qDebug() << "Sending message to chat:" << m_currentChatId;
    qDebug() << "Data:" << data;

    networkManager->sendJson("send_message", data);

    Message msg;
    msg.id = -1;
    msg.chatId = m_currentChatId;
    msg.senderId = m_currentUserId;
    msg.text = text;
    msg.time = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    msg.clientId = clientId;
    msg.localOrder = m_nextLocalOrder++;
    msg.status = MessageStatus::Pending;

    auto &messages = m_chatMessages[m_currentChatId];
    bool exists = std::any_of(messages.begin(), messages.end(), [&](const Message &existing) {
        return !existing.clientId.isEmpty() && existing.clientId == clientId;
    });

    if (!exists) {
        messages.append(msg);
    }

    m_rightPanel->addMessage(text, true, static_cast<int>(MessageStatus::Pending));
}

void MainWindow::onTyping()
{
    if (!networkManager || m_currentChatId == 0)
        return;

    if (!m_isTyping) {
        QJsonObject data;
        data["chat_id"] = m_currentChatId;
        networkManager->sendJson("typing", data);
        m_isTyping = true;
    }

    if (!m_typingTimer) {
        m_typingTimer = new QTimer(this);
        m_typingTimer->setSingleShot(true);

        connect(m_typingTimer, &QTimer::timeout, this, [this]() {
            QJsonObject data;
            data["chat_id"] = m_currentChatId;
            networkManager->sendJson("typing_stop", data);
            m_isTyping = false;
        });
    }

    m_typingTimer->start(1500); // 1.5 сек после последнего ввода
}


void MainWindow::onMessageReceived(const QString &text)
{
    qDebug() << "Message received:" << text;

    if (m_rightPanel) {
        m_rightPanel->addMessage(text, false);
    }
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);

        // Проверяем, что нет активного модального окна поверх панели
        QWidget *activeModal = QApplication::activeModalWidget();
        if (m_currentSidePanel && m_currentSidePanel->isVisible() && !activeModal) {
            QWidget *clickedWidget = qobject_cast<QWidget*>(obj);

            if (clickedWidget) {
                QPoint globalPos = mouseEvent->globalPos();

                // Проверяем клик на панели
                QPoint panelLocalPos = m_currentSidePanel->mapFromGlobal(globalPos);
                bool clickedOnPanel = m_currentSidePanel->rect().contains(panelLocalPos);

                // Проверяем клик на leftNav
                QPoint navLocalPos = m_leftNav->mapFromGlobal(globalPos);
                bool clickedOnNav = m_leftNav->rect().contains(navLocalPos);

                // Проверяем клик на полоске тени — игнорируем
                if (m_shadowStrip && m_shadowStrip->isVisible()) {
                    QPoint stripLocalPos = m_shadowStrip->mapFromGlobal(globalPos);
                    if (m_shadowStrip->rect().contains(stripLocalPos)) {
                        return QMainWindow::eventFilter(obj, event);
                    }
                }

                // Проверяем клик на overlay
                if (m_overlay && m_overlay->isVisible()) {
                    QPoint overlayLocalPos = m_overlay->mapFromGlobal(globalPos);
                    bool clickedOnOverlay = m_overlay->rect().contains(overlayLocalPos);

                    // Если клик на overlay но не на панели и не на навигации - закрываем
                    if (clickedOnOverlay && !clickedOnPanel && !clickedOnNav) {
                        qDebug() << "Clicked outside panel on overlay, closing";
                        onCloseSidePanel();
                        return true;
                    }
                }

                // Если клик вне панели и вне навигации — закрываем
                if (!clickedOnPanel && !clickedOnNav) {
                    qDebug() << "Clicked outside panel, closing";
                    onCloseSidePanel();
                    return true;
                }
            }
        }
    }

    return QMainWindow::eventFilter(obj, event);
}

void MainWindow::onOpenChatWithClient(const QString &clientName, const QString &clientPhone)
{
    qDebug() << "=== onOpenChatWithClient: " << clientName << clientPhone;

    // Закрываем CRM панель
    if (m_currentSidePanel) {
        m_currentSidePanel->hide();
        m_currentSidePanel = nullptr;
        m_currentPanelType = -1;
    }
    if (m_overlay) {
        m_overlay->hide();
    }

    // Переключаемся на чат
    if (m_stack) {
        m_stack->setCurrentIndex(0);
    }

    // Ищем клиента в контактах
    qint64 contactId = -1;
    for (const QVariantMap &contact : m_contacts) {
        QString name = contact["full_name"].toString();
        if (name.isEmpty()) {
            name = contact["username"].toString();
        }
        if (name == clientName) {
            contactId = contact["id"].toLongLong();
            break;
        }
    }

    if (contactId > 0) {
        m_rightPanel->setChatTitle(clientName);
        m_rightPanel->clearMessages();
        m_currentContactId = contactId;

        if (m_userToChat.contains(contactId)) {
            m_currentChatId = m_userToChat[contactId];
            requestChatHistory(m_currentChatId);
        } else {
            m_currentChatId = 0;
            requestOrCreateChat(contactId);
        }
    } else {
        m_pendingClientName = clientName;
        m_pendingClientPhone = clientPhone;
        m_rightPanel->setChatTitle(clientName);
        m_rightPanel->clearMessages();
        m_rightPanel->addMessage("Чат с клиентом \"" + clientName + "\" будет доступен после добавления в контакты.", false);
    }
}

void MainWindow::requestProfile()
{
    qDebug() << "=== requestProfile called, connected:" << (networkManager ? networkManager->isConnected() : false);
    if (!networkManager || !networkManager->isConnected()) {
        qDebug() << "requestProfile: NOT CONNECTED, skipping";
        return;
    }
    QJsonObject data;
    networkManager->sendJson("get_profile", data);
    qDebug() << "get_profile sent";
}

void MainWindow::saveProfileToServer()
{
    qDebug() << "=== saveProfileToServer, userId:" << m_currentUserId;
    if (!networkManager || !networkManager->isConnected()) return;

    QSettings settings("Aura", "Messenger");
    QString avatarKey = "userAvatar_" + QString::number(m_currentUserId);
    QString avatarPath = settings.value(avatarKey).toString();

    qDebug() << "avatarPath from settings:" << avatarPath;

    // 🔥 ЕСЛИ ЛОКАЛЬНЫЙ ПУТЬ - НЕ ОТПРАВЛЯЕМ
    if (!avatarPath.isEmpty() && !avatarPath.startsWith("/files/") && !avatarPath.startsWith("http")) {
        qDebug() << "Skipping save_profile - local path not sent to server:" << avatarPath;
        return;
    }

    QJsonObject profile;
    profile["name"] = settings.value("profile/name").toString();
    profile["position"] = settings.value("profile/position").toString();
    profile["department"] = settings.value("profile/department").toString();
    profile["tabNumber"] = settings.value("profile/tabNumber").toString();
    profile["email"] = settings.value("profile/email").toString();
    profile["phone"] = settings.value("profile/phone").toString();
    profile["avatarPath"] = avatarPath;

    QJsonObject data;
    data["profile"] = profile;
    networkManager->sendJson("save_profile", data);
}

void MainWindow::requestClients()
{
    qDebug() << "=== requestClients called";
    if (!networkManager || !networkManager->isConnected()) return;
    QJsonObject data;
    networkManager->sendJson("get_clients", data);
    qDebug() << "get_clients sent";
}

void MainWindow::saveClientsToServer()
{
    qDebug() << "=== saveClientsToServer, userId:" << m_currentUserId;
    if (!networkManager || !networkManager->isConnected()) return;

    DealsPanel *dealsPanel = qobject_cast<DealsPanel*>(m_sidePanels.value(8));
    if (!dealsPanel) {
        qDebug() << "DealsPanel not found, skipping save";
        return;
    }

    QJsonArray clients = dealsPanel->getDealsAsJson();
    QJsonObject data;
    data["clients"] = clients;
    qDebug() << "Sending clients:" << clients;
    networkManager->sendJson("save_clients", data);
}

void MainWindow::requestCompanies()
{
    if (!networkManager || !networkManager->isConnected()) return;
    networkManager->sendJson("get_companies", {});
}

void MainWindow::saveCompaniesToServer()
{
    if (!networkManager || !networkManager->isConnected()) return;
    CompaniesPanel *panel = qobject_cast<CompaniesPanel*>(m_sidePanels.value(9));
    if (!panel) return;
    QJsonObject data;
    data["companies"] = panel->getCompaniesAsJson();
    networkManager->sendJson("save_companies", data);
}

void MainWindow::uploadFileHttp(const QString &filePath, DocumentsPanel *panel, const QString &docId)
{
    QFile *file = new QFile(filePath);
    if (!file->open(QIODevice::ReadOnly)) { delete file; return; }

    QFileInfo fi(filePath);
    QByteArray fileData = file->readAll();
    file->deleteLater();

    QNetworkRequest request(QUrl("http://87.242.118.96:8080/upload"));
    request.setRawHeader("X-File-Name", fi.fileName().toUtf8());
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/octet-stream");

    QNetworkReply *reply = m_httpManager->post(request, fileData);

    connect(reply, &QNetworkReply::finished, this, [reply, fi, panel, docId]() {
        if (reply->error() == QNetworkReply::NoError) {
            QByteArray response = reply->readAll();
            QJsonObject obj = QJsonDocument::fromJson(response).object();
            QString fileUrl = obj["url"].toString();

            qDebug() << "✅ FILE UPLOAD SUCCESS:" << fileUrl;

            // Обновляем URL документа в панели
            if (panel && !docId.isEmpty()) {
                panel->updateDocumentUrl(docId, fileUrl);
            }
        } else {
            qDebug() << "❌ UPLOAD ERROR:" << reply->errorString();
        }
        reply->deleteLater();
    });
}

void MainWindow::downloadFileHttp(const QString &fileUrl, const QString &savePath)
{
    QString fullUrl = fileUrl.startsWith("/files/")
    ? "http://87.242.118.96:8080" + fileUrl
    : fileUrl;

    QNetworkReply *reply = m_httpManager->get(QNetworkRequest(QUrl(fullUrl)));

    connect(reply, &QNetworkReply::finished, this, [reply, savePath]() {
        if (reply->error() == QNetworkReply::NoError) {
            QFile file(savePath);
            if (file.open(QIODevice::WriteOnly)) {
                file.write(reply->readAll());
                file.close();
            }
        }
        reply->deleteLater();
    });
}

void MainWindow::requestDocuments()
{
    if (!networkManager || !networkManager->isConnected()) return;
    networkManager->sendJson("get_documents", {});
}

void MainWindow::saveDocumentsToServer()
{
    if (!networkManager || !networkManager->isConnected()) return;
    DocumentsPanel *panel = qobject_cast<DocumentsPanel*>(m_sidePanels.value(4));
    if (!panel) return;
    QJsonObject data;
    data["documents"] = panel->getDocumentsAsJson();
    networkManager->sendJson("save_documents", data);
}

void MainWindow::requestUserAvatar(int userId)
{
    if (!networkManager || !networkManager->isConnected()) return;

    // 🔥 НЕ ПРОВЕРЯЕМ m_pendingAvatarRequests - всегда запрашиваем заново
    // Это позволит получить актуальный статус аватара

    QJsonObject data;
    data["user_id"] = userId;
    networkManager->sendJson("get_user_avatar", data);

    qDebug() << "Requesting avatar for user:" << userId;
}

void MainWindow::uploadAvatarToServer(const QString &localFilePath)
{
    qDebug() << "=== uploadAvatarToServer:" << localFilePath;

    QFile *file = new QFile(localFilePath);
    if (!file->open(QIODevice::ReadOnly)) {
        qDebug() << "Cannot open avatar file:" << localFilePath;
        delete file;
        emit avatarUploadCompleted("");  // ← сигнал с ошибкой
        return;
    }

    QFileInfo fi(localFilePath);
    QByteArray fileData = file->readAll();
    file->close();
    delete file;

    QNetworkRequest request(QUrl("http://87.242.118.96:8080/upload"));  // ← порт 8080
    request.setRawHeader("X-File-Name", fi.fileName().toUtf8());
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/octet-stream");

    QNetworkReply *reply = m_httpManager->post(request, fileData);

    connect(reply, &QNetworkReply::finished, this, [reply, localFilePath, this]() {
        if (reply->error() == QNetworkReply::NoError) {
            QByteArray response = reply->readAll();
            QJsonObject obj = QJsonDocument::fromJson(response).object();
            QString url = obj["url"].toString();

            qDebug() << "✅ UPLOAD SUCCESS, URL:" << url;

            if (!url.isEmpty()) {
                QSettings settings("Aura", "Messenger");
                QString avatarKey = "userAvatar_" + QString::number(m_currentUserId);

                // Сохраняем URL
                settings.setValue(avatarKey, url);

                // Кешируем локально
                QString cacheDir = QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + "/avatars/";
                QDir().mkpath(cacheDir);

                QString localPath = cacheDir + QString::number(m_currentUserId) + ".jpg";

                QFile::remove(localPath);

                if (QFile::copy(localFilePath, localPath)) {
                    qDebug() << "Avatar cached:" << localPath;
                }

                // Отправляем на сервер URL
                saveProfileToServer();

                if (m_leftNav) {
                    m_leftNav->updateProfileAvatar();
                }

                // 🔥 СИГНАЛ ОБ УСПЕШНОЙ ЗАГРУЗКЕ
                emit avatarUploadCompleted(url);
            } else {
                emit avatarUploadCompleted("");
            }
        } else {
            qDebug() << "❌ UPLOAD ERROR:" << reply->errorString();
            emit avatarUploadCompleted("");
        }
        reply->deleteLater();
    });
}

void MainWindow::downloadAvatarFromServer(const QString &avatarPath, int userId)
{
    QString url = "http://87.242.118.96:8080" + avatarPath;
    qDebug() << "📥 Downloading avatar from:" << url << "for user:" << userId;

    QNetworkRequest request(url);
    QNetworkReply *reply = m_httpManager->get(request);

    connect(reply, &QNetworkReply::finished, this, [reply, userId, this, avatarPath]() {
        if (reply->error() == QNetworkReply::NoError) {
            QByteArray data = reply->readAll();

            if (data.isEmpty()) {
                qDebug() << "❌ Downloaded data is empty for user:" << userId;
                reply->deleteLater();
                return;
            }

            QPixmap pixmap;
            if (!pixmap.loadFromData(data)) {
                qDebug() << "❌ Failed to load pixmap from data for user:" << userId;
                reply->deleteLater();
                return;
            }

            if (!pixmap.isNull()) {
                QString cacheDir = QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + "/avatars/";
                QDir().mkpath(cacheDir);
                QString cachePath = cacheDir + QString::number(userId) + ".jpg";

                if (pixmap.save(cachePath, "JPG")) {
                    qDebug() << "✅ Avatar saved to cache:" << cachePath;

                    // Обновляем m_userAvatars
                    m_userAvatars[userId] = cachePath;

                    // 🔥 СОХРАНЯЕМ В QSETTINGS КЕШИРОВАННЫЙ ПУТЬ
                    QSettings settings("Aura", "Messenger");
                    QString avatarKey = "userAvatar_" + QString::number(userId);
                    settings.setValue(avatarKey, cachePath);

                    // Обновляем UI
                    if (m_middlePanel) {
                        m_middlePanel->updateUserAvatar(userId, cachePath);
                        m_middlePanel->updateChatAvatar(userId, cachePath);
                    }
                    if (m_currentContactId == userId && m_rightPanel) {
                        m_rightPanel->setChatAvatar(cachePath);
                    }
                    if (userId == m_currentUserId && m_leftNav) {
                        m_leftNav->updateProfileAvatar();
                    }
                } else {
                    qDebug() << "❌ Failed to save avatar to cache for user:" << userId;
                }
            }
        } else {
            qDebug() << "❌ Avatar download error for user" << userId << ":" << reply->errorString();
        }
        reply->deleteLater();
    });
}

void MainWindow::updateChatAvatars()
{
    // Обновляем аватары в списке чатов
    for (auto it = m_userAvatars.begin(); it != m_userAvatars.end(); ++it) {
        m_middlePanel->updateChatAvatar(it.key(), it.value());
    }
}

void MainWindow::requestUserProfile(int userId)
{
    if (!networkManager || !networkManager->isConnected()) return;
    QJsonObject data;
    data["user_id"] = userId;
    networkManager->sendJson("get_user_profile", data);
}

void MainWindow::togglePinChat(int chatId)
{
    if (m_pinnedChats.contains(chatId)) {
        m_pinnedChats.remove(chatId);
    } else {
        m_pinnedChats.insert(chatId);
    }
    savePinnedChats();

    // Пересортировать чаты
    sortChats();
    m_middlePanel->setChats(m_chats);
    updateChatAvatars();
}

void MainWindow::savePinnedChats()
{
    QSettings settings("Aura", "Messenger");
    QStringList pinned;
    for (int id : m_pinnedChats) {
        pinned << QString::number(id);
    }
    settings.setValue("pinnedChats", pinned);
}

void MainWindow::loadPinnedChats()
{
    QSettings settings("Aura", "Messenger");
    QStringList pinned = settings.value("pinnedChats").toStringList();
    for (const QString &s : pinned) {
        m_pinnedChats.insert(s.toInt());
    }
}

void MainWindow::sortChats()
{
    // Закрепленные чаты в начало
    std::sort(m_chats.begin(), m_chats.end(), [this](const QVariantMap &a, const QVariantMap &b) {
        int aId = a["id"].toInt();
        int bId = b["id"].toInt();
        bool aPinned = m_pinnedChats.contains(aId);
        bool bPinned = m_pinnedChats.contains(bId);
        if (aPinned && !bPinned) return true;
        if (!aPinned && bPinned) return false;
        return aId > bId; // По ID для стабильности
    });
}

QNetworkAccessManager* MainWindow::getHttpManager() const
{
    return m_httpManager;
}

void MainWindow::sendEmailConfirmation(const QString &email, const QString &username, const QString &password)
{
    qDebug() << "=== sendEmailConfirmation ===";
    qDebug() << "email:" << email;
    qDebug() << "username:" << username;

    if (!networkManager) {
        qDebug() << "NetworkManager is NULL!";
        QMessageBox::warning(nullptr, "Ошибка", "Внутренняя ошибка приложения");
        return;
    }

    // Ждём подключения (до 3 секунд)
    int waitCount = 0;
    while (!networkManager->isConnected() && waitCount < 30) {
        QThread::msleep(100);
        waitCount++;
        qDebug() << "Waiting for connection..." << waitCount;
    }

    if (!networkManager->isConnected()) {
        qDebug() << "Not connected to server!";
        QMessageBox::warning(nullptr, "Ошибка", "Нет подключения к серверу. Пожалуйста, подождите и попробуйте снова.");
        return;
    }

    m_pendingEmail = email;
    m_pendingUsername = username;
    m_pendingPassword = password;

    QJsonObject data;
    data["email"] = email;
    data["username"] = username;
    data["password"] = password;

    qDebug() << "Sending JSON...";
    networkManager->sendJson("send_confirmation", data);
    qDebug() << "JSON sent";
}

void MainWindow::confirmEmail(const QString &email, const QString &confirmationCode)
{
    if (!networkManager || !networkManager->isConnected()) {
        qDebug() << "Not connected to server";
        return;
    }

    QJsonObject data;
    data["email"] = email;
    data["code"] = confirmationCode;

    networkManager->sendJson("confirm_email", data);
    qDebug() << "Sending confirmation for email:" << email;
}

void MainWindow::clearAvatarCache()
{
    // Очищаем map с аватарами
    m_userAvatars.clear();

    // Очищаем папку кеша
    QString cacheDir = QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + "/avatars/";
    QDir dir(cacheDir);
    if (dir.exists()) {
        QFileInfoList files = dir.entryInfoList(QDir::Files);
        for (const QFileInfo &file : files) {
            QFile::remove(file.absoluteFilePath());
        }
        qDebug() << "Avatar cache cleared, removed" << files.size() << "files";
    }
}
