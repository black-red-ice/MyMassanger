#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QMap>
#include <QPoint>
#include "../network/NetworkManager.h"

class TitleBar;
class LeftNavigation;
class MiddlePanel;
class RightPanel;
class QWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    RightPanel* getRightPanel() const { return m_rightPanel; }

    void setCurrentUser(qint64 userId, const QString &username);
    void setNetworkManager(NetworkManager *manager);

protected:
    //void mousePressEvent(QMouseEvent *event) override;
    //void mouseMoveEvent(QMouseEvent *event) override;
    //void mouseReleaseEvent(QMouseEvent *event) override;
    //bool eventFilter(QObject *obj, QEvent *event) override;
    bool nativeEvent(const QByteArray &eventType, void *message, qintptr *result) override;

private slots:
    void onContactSelected(qint64 contactId);
    void onNavigationChanged(int index);
    void onToggleSidePanel(int panelType);
    void onCloseSidePanel();
    void toggleMaximized();

    void onNetworkConnected();
    void onNetworkDisconnected();
    void onMessageReceived(const QString &message);
    void onNetworkError(const QString &error);
    void onCommandResponse(const QString &response);

private:
    void setupUI();
    void createSidePanels();
    void updateWindowShadow();

    void saveLastChat();
    void restoreLastChat();
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

    NetworkManager *networkManager;
    QString serverIp = "87.242.118.96";
    quint16 serverPort = 12345;

    qint64 m_currentUserId = -1;
    QString m_currentUsername;

    void requestContacts();
    void requestChats();
    QList<QVariantMap> m_contacts;
};

#endif // MAINWINDOW_H
