#include "mainwindow.h"
#include "titlebar.h"
#include "leftnavigation.h"
#include "middlepanel.h"
#include "rightpanel.h"
#include "dialogs/settingsdialog.h"
#include "dialogs/supportdialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QTimer>
#include <QDebug>
#include <QPainter>
#include <QScreen>
#include <QApplication>
#include <QSettings>

#ifdef Q_OS_WIN
#include <windows.h>
#include <dwmapi.h>
#pragma comment(lib, "dwmapi.lib")

// Определяем недостающие константы для старых SDK
#ifndef DWMWA_USE_IMMERSIVE_DARK_MODE
#define DWMWA_USE_IMMERSIVE_DARK_MODE 20
#endif
#endif

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setupUI();
    createSidePanels();

    // Создаем менеджер сети
    networkManager = new NetworkManager(this);

    // Подключаем сигналы NetworkManager
    connect(networkManager, &NetworkManager::connected, this, &MainWindow::onNetworkConnected);
    connect(networkManager, &NetworkManager::disconnected, this, &MainWindow::onNetworkDisconnected);
    connect(networkManager, &NetworkManager::messageReceived, this, &MainWindow::onMessageReceived);
    connect(networkManager, &NetworkManager::errorOccurred, this, &MainWindow::onNetworkError);
    connect(networkManager, &NetworkManager::commandResponse, this, &MainWindow::onCommandResponse);
    connect(m_middlePanel, &MiddlePanel::contactSelected, this, &MainWindow::onContactSelected);

    // // Подключаем отправку сообщений из RightPanel
    // connect(m_rightPanel, &RightPanel::sendMessageRequested,
    //         this, [this](const QString &message) {
    //             if (networkManager && networkManager->isConnected() && m_currentChatId > 0) {
    //                 QString command = QString("SEND_MESSAGE:%1:%2:%3")
    //                 .arg(m_currentChatId)
    //                     .arg(m_currentUserId)
    //                     .arg(message);
    //                 networkManager->sendCommand(command);
    //                 qDebug() << "Sending message to chat:" << m_currentChatId;
    //             } else {
    //                 qDebug() << "Cannot send message: not connected or no chat selected";
    //                 m_rightPanel->addMessage("⚠️ Не выбран чат или нет подключения", false);
    //             }
    //         });

    // Автоматическое подключение при запуске
    networkManager->connectToServer(serverIp, serverPort);

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

}

MainWindow::~MainWindow() = default;

void MainWindow::setupUI()
{
    QWidget *centralWidget = new QWidget(this);
    centralWidget->setStyleSheet("background-color: #0f172a;");

    QVBoxLayout *mainVerticalLayout = new QVBoxLayout(centralWidget);
    mainVerticalLayout->setContentsMargins(0, 0, 0, 0);
    mainVerticalLayout->setSpacing(0);

    m_titleBar = new TitleBar(this);
    m_titleBar->installEventFilter(this);
    connect(m_titleBar, &TitleBar::minimizeClicked, this, &QMainWindow::showMinimized);
    connect(m_titleBar, &TitleBar::maximizeClicked, this, &MainWindow::toggleMaximized);
    connect(m_titleBar, &TitleBar::closeClicked, this, &QMainWindow::close);

    QWidget *contentWidget = new QWidget();
    contentWidget->setStyleSheet("background-color: #1e293b;");
    QHBoxLayout *mainLayout = new QHBoxLayout(contentWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    m_leftNav = new LeftNavigation(this);
    connect(m_leftNav, &LeftNavigation::currentIndexChanged, this, &MainWindow::onNavigationChanged);
    connect(m_leftNav, &LeftNavigation::toggleSidePanel, this, &MainWindow::onToggleSidePanel);

    m_middlePanel = new MiddlePanel(this);
    m_rightPanel = new RightPanel(this);

    mainLayout->addWidget(m_leftNav);
    mainLayout->addWidget(m_middlePanel);
    mainLayout->addWidget(m_rightPanel);

    mainVerticalLayout->addWidget(m_titleBar);
    mainVerticalLayout->addWidget(contentWidget);

    setCentralWidget(centralWidget);
    resize(1200, 800);
}

void MainWindow::createSidePanels()
{
    // Пока заглушки
}

void MainWindow::onNavigationChanged(int index)
{
    Q_UNUSED(index)
}

void MainWindow::onToggleSidePanel(int panelType)
{
    if (m_currentSidePanel) {
        m_currentSidePanel->hide();
        m_currentSidePanel = nullptr;
    }

    if (panelType == 6) {
        SettingsDialog dialog(this);
        dialog.exec();
        return;
    }
    if (panelType == 7) {
        SupportDialog dialog(this);
        dialog.exec();
        return;
    }

    if (m_sidePanels.contains(panelType)) {
        m_currentSidePanel = m_sidePanels.value(panelType);
        m_currentSidePanel->show();
    }
}

void MainWindow::onCloseSidePanel()
{
    if (m_currentSidePanel) {
        m_currentSidePanel->hide();
        m_currentSidePanel = nullptr;
    }
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
        requestContacts();
        requestChats();
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

void MainWindow::onMessageReceived(const QString &message)
{
    qDebug() << "New message:" << message;
    m_rightPanel->addMessage(message, false);
}

void MainWindow::onNetworkError(const QString &error)
{
    qDebug() << "Network error:" << error;
}

void MainWindow::setCurrentUser(qint64 userId, const QString &username)
{
    m_currentUserId = userId;
    m_currentUsername = username;
    setWindowTitle("Aura - " + username);

    if (m_rightPanel) {
        m_rightPanel->setChatTitle("Добро пожаловать, " + username + "!");
    }

    // Загружаем контакты и чаты (ОДИН РАЗ!)
    if (networkManager && networkManager->isConnected()) {
        requestContacts();
        requestChats();
    }

    // Восстанавливаем последний чат с задержкой (после загрузки контактов)
    QTimer::singleShot(500, this, [this]() {
        restoreLastChat();
    });
}

void MainWindow::requestContacts()
{
    if (networkManager && networkManager->isConnected()) {
        QString command = QString("GET_CONTACTS:%1").arg(m_currentUserId);
        networkManager->sendCommand(command);
    }
}

void MainWindow::requestChats()
{
    if (networkManager && networkManager->isConnected()) {
        QString command = QString("GET_CHATS:%1").arg(m_currentUserId);
        networkManager->sendCommand(command);
    }
}

void MainWindow::onCommandResponse(const QString &response)
{
    QStringList parts = response.split(':');
    QString command = parts[0];

    qDebug() << "onCommandResponse:" << command;

    if (command == "CONTACTS" && parts.size() >= 2) {
        QStringList contactsData = parts[1].split(',');
        m_contacts.clear();

        for (const QString &data : contactsData) {
            if (data.isEmpty()) continue;
            QStringList fields = data.split('|');
            if (fields.size() >= 4) {
                QVariantMap contact;
                contact["id"] = fields[0].toLongLong();
                contact["username"] = fields[1];
                contact["full_name"] = fields[2];
                contact["status"] = fields[3].toInt();
                m_contacts.append(contact);
            }
        }

        qDebug() << "Loaded" << m_contacts.size() << "contacts";

        if (m_middlePanel) {
            m_middlePanel->setContacts(m_contacts);
        }
    }
    else if (command == "CHATS" && parts.size() >= 2) {
        QStringList chatsData = parts[1].split(',');
        qDebug() << "Loaded" << chatsData.size() << "chats";
        // TODO: m_middlePanel->setChats(chats);
    }
    else if (command == "CHAT_READY" && parts.size() >= 2) {
        qint64 chatId = parts[1].toLongLong();
        qDebug() << "Chat ready, ID:" << chatId;
        m_currentChatId = chatId;
        requestChatHistory(chatId);
    }
    else if (command == "NEW_MESSAGE" && parts.size() >= 6) {
        qint64 chatId = parts[1].toLongLong();
        qint64 senderId = parts[3].toLongLong();
        QString senderName = parts[4];
        QString content = parts[5];

        for (int i = 6; i < parts.size(); ++i) {
            content += ":" + parts[i];
        }

        // НЕ добавляем свои сообщения повторно
        if (senderId == m_currentUserId) {
            qDebug() << "Skipping own message echo";
            return;
        }

        if (chatId == m_currentChatId) {
            m_rightPanel->clearMessages();
            QString displayText = senderName + ": " + content;
            m_rightPanel->addMessage(displayText, false);
            qDebug() << "Added message to chat:" << displayText;
        }
    }
    else if (command == "MESSAGES") {
        // Формат: MESSAGES:chatId:данные
        // Находим первое двоеточие после "MESSAGES"
        int firstColon = response.indexOf(':');
        int secondColon = response.indexOf(':', firstColon + 1);

        if (secondColon == -1) {
            qDebug() << "Invalid MESSAGES format";
            return;
        }

        qint64 chatId = response.mid(firstColon + 1, secondColon - firstColon - 1).toLongLong();
        QString messagesStr = response.mid(secondColon + 1);

        qDebug() << "Loading messages for chat:" << chatId << "(current:" << m_currentChatId << ")";

        if (chatId != m_currentChatId) {
            qDebug() << "Messages for another chat, ignoring";
            return;
        }

        m_rightPanel->clearMessages();

        if (messagesStr.isEmpty()) {
            qDebug() << "No messages for chat" << chatId;
            return;
        }

        QStringList messagesData = messagesStr.split(',', Qt::SkipEmptyParts);
        qDebug() << "Parsing" << messagesData.size() << "messages";

        for (const QString &data : messagesData) {
            QStringList fields = data.split('|');
            if (fields.size() >= 6) {
                qint64 messageId = fields[0].toLongLong();
                qint64 senderId = fields[1].toLongLong();
                QString senderUsername = fields[2];
                QString senderFullName = fields[3];
                QString content = fields[4];
                QString createdAt = fields[5];

                QString senderName = senderFullName.isEmpty() ? senderUsername : senderFullName;
                bool isOutgoing = (senderId == m_currentUserId);

                m_rightPanel->addMessage(senderName + ": " + content, isOutgoing);
            }
        }

        qDebug() << "Loaded" << messagesData.size() << "messages for chat" << chatId;
    }
}

void MainWindow::setNetworkManager(NetworkManager *manager)
{
    if (networkManager) {
        // Отключаем все сигналы старого менеджера
        disconnect(networkManager, nullptr, this, nullptr);
        networkManager->deleteLater();
    }

    networkManager = manager;
    networkManager->setParent(this);

    // Подключаем сигналы заново
    connect(networkManager, &NetworkManager::connected, this, &MainWindow::onNetworkConnected);
    connect(networkManager, &NetworkManager::disconnected, this, &MainWindow::onNetworkDisconnected);
    connect(networkManager, &NetworkManager::messageReceived, this, &MainWindow::onMessageReceived);
    connect(networkManager, &NetworkManager::errorOccurred, this, &MainWindow::onNetworkError);
    connect(networkManager, &NetworkManager::commandResponse, this, &MainWindow::onCommandResponse);

    // Подключаем отправку сообщений из RightPanel
    connect(m_rightPanel, &RightPanel::sendMessageRequested,
            this, [this](const QString &message) {
                if (!networkManager || !networkManager->isConnected()) {
                    qDebug() << "Cannot send message: not connected";
                    return;
                }

                if (m_currentChatId <= 0) {
                    qDebug() << "Cannot send message: no chat selected";
                    return;
                }

                // Оптимистичное отображение
                m_rightPanel->addMessage(m_currentUsername + ": " + message, true);

                QString command = QString("SEND_MESSAGE:%1:%2:%3")
                                      .arg(m_currentChatId)
                                      .arg(m_currentUserId)
                                      .arg(message);
                networkManager->sendCommand(command);
                qDebug() << "Sending message to chat:" << m_currentChatId;
            });

    // Если уже подключены, загружаем контакты
    if (networkManager->isConnected()) {
        qDebug() << "Already connected, requesting contacts...";
        requestContacts();
        requestChats();
    }

}

void MainWindow::onContactSelected(qint64 contactId)
{
    qDebug() << "Contact selected:" << contactId;

    if (!m_rightPanel) {
        qDebug() << "ERROR: m_rightPanel is null!";
        return;
    }

    // Находим контакт в списке
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

    qDebug() << "Contact name:" << contactName;

    if (!m_rightPanel) {
        qDebug() << "ERROR: m_rightPanel is null!";
        return;
    }

    // Обновляем заголовок
    m_rightPanel->setChatTitle(contactName);

    // Очищаем сообщения
    m_rightPanel->clearMessages();

    // Запрашиваем или создаём чат
    requestOrCreateChat(contactId);

    m_currentContactId = contactId;
    saveLastChat();
}

void MainWindow::requestOrCreateChat(qint64 contactId)
{
    if (!networkManager || !networkManager->isConnected()) {
        qDebug() << "Cannot request chat: not connected";
        return;
    }

    // Сначала проверяем, есть ли уже чат с этим пользователем
    QString command = QString("GET_OR_CREATE_CHAT:%1:%2").arg(m_currentUserId).arg(contactId);
    networkManager->sendCommand(command);
    qDebug() << "Requesting chat with user:" << contactId;
}

void MainWindow::requestChatHistory(qint64 chatId)
{
    if (!networkManager || !networkManager->isConnected()) {
        return;
    }

    QString command = QString("GET_MESSAGES:%1:%2").arg(chatId).arg(50); // 50 сообщений
    networkManager->sendCommand(command);
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

void MainWindow::restoreLastChat()
{
    QSettings settings("Aura", "Messenger");
    qint64 lastChatId = settings.value("lastChatId", -1).toLongLong();
    qint64 lastContactId = settings.value("lastContactId", -1).toLongLong();

    if (lastChatId > 0 && lastContactId > 0) {
        m_currentChatId = lastChatId;
        m_currentContactId = lastContactId;

        // Находим имя контакта
        QString contactName;
        for (const QVariantMap &contact : m_contacts) {
            if (contact["id"].toLongLong() == lastContactId) {
                contactName = contact["full_name"].toString();
                if (contactName.isEmpty()) {
                    contactName = contact["username"].toString();
                }
                break;
            }
        }

        m_rightPanel->setChatTitle(contactName);
        requestChatHistory(lastChatId);
    }
}
