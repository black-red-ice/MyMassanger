#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QMap>
#include <QPoint>
#include <QStackedWidget>
#include "../network/NetworkManager.h"
#include "Navigation.h"
#include "../panels/dealPanel/DealsPanel.h"
#include "../panels/taskPanel/TasksPanel.h"
#include "panels/documentsPanel/DocumentsPanel.h"
#include "panels/employeesPanel/EmployeesPanel.h"
#include "dialogs/supportdialog.h"
#include <QStandardPaths>
#include <QFile>

class TitleBar;
class LeftNavigation;
class MiddlePanel;
class RightPanel;
class QWidget;
class DealsPanel;
class QNetworkAccessManager;

enum class MessageStatus {
    Pending = 0,
    Sent = 1,
    Delivered = 2,
    Read = 3
};

struct Message {
    int id;
    int chatId;
    int senderId;
    QString text;
    QString time;
    QString clientId;
    qint64 localOrder = 0;
    MessageStatus status = MessageStatus::Pending;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

signals:
    void confirmationSent(bool success);
    void confirmationFailed(const QString &error);
    void registrationCompleted();
    void avatarUploadCompleted(const QString& url);

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    RightPanel* getRightPanel() const { return m_rightPanel; }

    void setCurrentUser(qint64 userId, const QString &username);
    void setNetworkManager(NetworkManager *manager);

    void renderTasks();
    QWidget *m_shadowStrip = nullptr;
    QStackedWidget *m_stack = nullptr;

    void requestProfile();
    void saveProfileToServer();
    qint64 getCurrentUserId() const { return m_currentUserId; }
    void requestClients();
    void saveClientsToServer();
    QJsonArray m_pendingClients;
    void requestCompanies();
    void saveCompaniesToServer();
    QJsonArray m_pendingCompanies;
    void uploadFileHttp(const QString &filePath, DocumentsPanel *panel = nullptr, const QString &docId = QString());
    void downloadFileHttp(const QString &fileId, const QString &savePath);
    void requestDocuments();
    void saveDocumentsToServer();
    QJsonArray m_pendingDocuments;
    void requestUserAvatar(int userId);
    QMap<int, QString> m_userAvatars;
    void uploadAvatarToServer(const QString &localFilePath);
    QString m_avatarUrl;
    void downloadAvatarFromServer(const QString &avatarPath, int userId);
    void updateChatAvatar(int userId, const QString &avatarPath);
    // В mainwindow.h замените getCachedAvatar на:
    QString getCachedAvatar(int userId) const {
        QString cachePath = QStandardPaths::writableLocation(QStandardPaths::CacheLocation)
        + "/avatars/" + QString::number(userId) + ".jpg";
        if (QFile::exists(cachePath)) {
            return cachePath;
        }
        return QString();
    }
    void updateChatAvatars();
    void requestUserProfile(int userId);
    int m_pendingProfileUserId = -1;
    QString m_pendingProfileUsername;
    QString m_pendingProfileAvatar;
    bool m_pendingProfileOnline = false;
    QSet<int> m_pinnedChats;
    void togglePinChat(int chatId);
    void savePinnedChats();
    void loadPinnedChats();
    QNetworkAccessManager* getHttpManager() const;
    QSet<int> m_pendingAvatarRequests;
    void sendEmailConfirmation(const QString &email, const QString &username, const QString &password);
    void confirmEmail(const QString &email, const QString &confirmationCode);
    bool isNetworkConnected() const { return networkManager && networkManager->isConnected(); }
    NetworkManager* getNetworkManager() const { return networkManager; }
    void clearAvatarCache();

protected:
    bool nativeEvent(const QByteArray &eventType, void *message, qintptr *result) override;
    void resizeEvent(QResizeEvent *event) override;
    QWidget* m_dimWidget = nullptr;
    bool eventFilter(QObject *obj, QEvent *event) override;

private slots:
    void onContactSelected(qint64 contactId);
    void onNavigationChanged(NavItem item);
    void onToggleSidePanel(int panelType);
    void onCloseSidePanel();
    void toggleMaximized();

    void onNetworkConnected();
    void onNetworkDisconnected();
    void onMessageReceived(const QString &message);
    void onNetworkError(const QString &error);

    void onSearchUsersByNickname(const QString &nickname);
    void onClearSearch();
    void onSearchContacts(const QString &query);

    void onSendMessage(const QString &text);

    void onTyping();

    void renderChatMessages(int chatId, bool preserveScroll = false);
    bool updateMessageStatusById(int messageId, MessageStatus status);
    void sendDeliveredAck(int chatId, int messageId);
    void sendReadReceipt(int chatId);

    void loadOlderMessages();

    void onOpenChatWithClient(const QString &clientName, const QString &clientPhone);

private:
    void setupUI();
    void updateWindowShadow();

    void displaySearchResults(const QList<QVariantMap> &users);

    void onJson(const QJsonObject& obj);

    void saveLastChat();
    qint64 m_lastChatId = -1;
    qint64 m_lastContactId = -1;
    qint64 m_currentContactId = -1;

    void requestOrCreateChat(qint64 contactId);
    void requestChatHistory(qint64 chatId);
    qint64 m_currentChatId = -1;

    bool m_dragging = false;
    QPoint m_dragPosition;

    TitleBar* m_titleBar = nullptr;
    LeftNavigation* m_leftNav = nullptr;
    MiddlePanel* m_middlePanel = nullptr;
    RightPanel* m_rightPanel = nullptr;
    QStackedWidget* m_centralStack = nullptr;
    QMap<int, QWidget*> m_sidePanels;
    QWidget* m_currentSidePanel = nullptr;
    int m_currentPanelType = -1;

    NetworkManager *networkManager;
    QString serverIp = "87.242.118.96";
    quint16 serverPort = 12345;

    qint64 m_currentUserId = -1;
    QString m_currentUsername;

    void requestContacts();
    void requestChats();

    void onChatsReceived(const QJsonArray &chats);
    void onChatSelected(const QString &chatId);
    void onMessagesReceived(int chatId, const QJsonArray &messages);
    void onNewMessage(const QJsonObject &msg);

    QList<QVariantMap> m_contacts;
    QList<QVariantMap> m_chats;

    QTimer *m_typingTimer = nullptr;
    bool m_isTyping = false;

    // связь: user_id → chat_id
    QMap<int, int> m_userToChat;

    // сообщения по чатам
    QMap<int, QVector<Message>> m_chatMessages;
    QMap<int, int> m_unreadCount;
    QMap<int, int> m_lastReadSent;
    qint64 m_nextLocalOrder = 1;

    // онлайн пользователи
    QSet<int> m_onlineUsers;

    bool m_loadingOlder = false;

    // Убрали QStackedWidget *m_stack;
    QWidget *m_centralWidget;
    QWidget *m_contentWidget;
    QWidget *m_overlay = nullptr;

    QString m_pendingClientName;
    QString m_pendingClientPhone;

    QNetworkAccessManager *m_httpManager;
    void sortChats();
    QString m_pendingEmail;
    QString m_pendingUsername;
    QString m_pendingPassword;
    QString m_confirmationCode;
    SupportDialog *m_supportDialog = nullptr;
    void updateChatLastTime(int chatId, const QString &lastTime);
    void bumpChatToTop(int chatId);
    void updateLastMessageInChat(int chatId, const QString &lastMessage, const QString &lastTime = QString());
};

#endif // MAINWINDOW_H
