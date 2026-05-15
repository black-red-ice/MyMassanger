#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H

#include <QObject>
#include <QWebSocket>
#include <QTimer>
#include <QJsonArray>
#include <QJsonObject>

class NetworkManager : public QObject
{
    Q_OBJECT
public:
    explicit NetworkManager(QObject *parent = nullptr);

    void connectToServer(const QString &host, quint16 port);
    void disconnectFromServer();
    void sendMessage(const QString &message);
    void sendCommand(const QString &command);
    void sendJson(const QString& type, const QJsonObject& data);

    bool isConnected() const;
    QString m_buffer;

signals:
    void connected();
    void disconnected();
    void messageReceived(const QString &message);
    void errorOccurred(const QString &error);
    void commandResponse(const QString &response);
    void loginSuccess(int userId);
    void jsonReceived(const QJsonObject& obj);
    void loginResult(bool ok, int userId);
    void contactsReceived(QJsonArray contacts);
    void chatsReceived(QJsonArray chats);
    void messagesReceived(int chatId, QJsonArray messages);
    void newMessage(QJsonObject msg);
    void typing(int chatId, int userId);
    void messageRead(int chatId, int userId);
    void onTextMessageReceived(const QString &message);
    void reconnecting();           // новый сигнал
    void reconnected();            // новый сигнал

private slots:
    void onTextMessage(const QString &message);
    void onConnected();            // новый слот
    void onDisconnected();         // новый слот
    void onError(QAbstractSocket::SocketError error);  // новый слот
    void attemptReconnect();       // новый слот
    void onPong(quint64 elapsedTime, const QByteArray &payload);  // новый слот

private:
    QWebSocket *socket;
    QTimer *m_reconnectTimer;
    QTimer *m_pingTimer;
    QString m_host;
    quint16 m_port;
    int m_reconnectAttempts = 0;
    static const int MAX_RECONNECT_ATTEMPTS = 10;
    static const int RECONNECT_INTERVAL_MS = 3000;
};

#endif // NETWORKMANAGER_H
