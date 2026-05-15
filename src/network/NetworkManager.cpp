#include "NetworkManager.h"
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QString>

NetworkManager::NetworkManager(QObject *parent)
    : QObject(parent)
{
    socket = new QWebSocket();

    connect(socket, &QWebSocket::connected, this, &NetworkManager::onConnected);
    connect(socket, &QWebSocket::disconnected, this, &NetworkManager::onDisconnected);
    connect(socket, &QWebSocket::textMessageReceived, this, &NetworkManager::onTextMessage);
    connect(socket, &QWebSocket::pong, this, &NetworkManager::onPong);

    // Таймер реконнекта
    m_reconnectTimer = new QTimer(this);
    m_reconnectTimer->setInterval(RECONNECT_INTERVAL_MS);
    connect(m_reconnectTimer, &QTimer::timeout, this, &NetworkManager::attemptReconnect);

    // Пинг-таймер для проверки живости соединения
    m_pingTimer = new QTimer(this);
    m_pingTimer->setInterval(15000); // каждые 15 секунд
    connect(m_pingTimer, &QTimer::timeout, this, [this]() {
        if (socket->state() == QAbstractSocket::ConnectedState) {
            socket->ping();
        }
    });

    m_host = "87.242.118.96";
    m_port = 12345;
    socket->open(QUrl(QString("ws://%1:%2").arg(m_host).arg(m_port)));
}

void NetworkManager::onConnected()
{
    qDebug() << "Connected to server";
    m_reconnectAttempts = 0;
    m_reconnectTimer->stop();
    m_pingTimer->start();
    emit connected();
    emit reconnected();
}

void NetworkManager::onDisconnected()
{
    qDebug() << "Disconnected from server";
    m_pingTimer->stop();
    emit disconnected();

    // Запускаем реконнект
    if (m_reconnectAttempts < MAX_RECONNECT_ATTEMPTS) {
        m_reconnectTimer->start();
        emit reconnecting();
    }
}

void NetworkManager::onError(QAbstractSocket::SocketError error)
{
    Q_UNUSED(error)
    qDebug() << "Socket error:" << socket->errorString();
}

void NetworkManager::onPong(quint64 elapsedTime, const QByteArray &payload)
{
    Q_UNUSED(payload)
    qDebug() << "Pong received, latency:" << elapsedTime << "ms";
}

void NetworkManager::attemptReconnect()
{
    m_reconnectAttempts++;
    qDebug() << "Reconnect attempt" << m_reconnectAttempts << "of" << MAX_RECONNECT_ATTEMPTS;

    if (socket->state() != QAbstractSocket::UnconnectedState) {
        socket->abort();
    }

    socket->open(QUrl(QString("ws://%1:%2").arg(m_host).arg(m_port)));
}

void NetworkManager::connectToServer(const QString &host, quint16 port)
{
    m_host = host;
    m_port = port;

    if (socket->state() != QAbstractSocket::UnconnectedState) {
        socket->abort();
    }
    socket->open(QUrl(QString("ws://%1:%2").arg(host).arg(port)));
    qDebug() << "Connecting to" << host << ":" << port;
}

void NetworkManager::disconnectFromServer()
{
    m_reconnectTimer->stop();
    m_pingTimer->stop();
    m_reconnectAttempts = MAX_RECONNECT_ATTEMPTS; // запрещаем реконнект
    socket->close();
}

bool NetworkManager::isConnected() const
{
    return socket->state() == QAbstractSocket::ConnectedState;
}

void NetworkManager::sendJson(const QString& type, const QJsonObject& data)
{
    if (!socket || socket->state() != QAbstractSocket::ConnectedState) {
        qDebug() << "Socket not connected!";
        // Авто-реконнект при попытке отправки
        if (m_reconnectAttempts < MAX_RECONNECT_ATTEMPTS && !m_reconnectTimer->isActive()) {
            qDebug() << "Triggering reconnect from sendJson...";
            m_reconnectTimer->start();
        }
        return;
    }

    QJsonObject obj;
    obj["type"] = type;
    obj["data"] = data;

    qDebug() << "FINAL JSON:" << QJsonDocument(obj).toJson();

    QJsonDocument doc(obj);
    QString message = doc.toJson(QJsonDocument::Compact);

    socket->sendTextMessage(message);

    qDebug() << "SENT JSON:" << message;
}

void NetworkManager::onTextMessage(const QString &message)
{
    qDebug() << "RECEIVED:" << message;

    QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8());
    if (!doc.isObject()) return;

    emit jsonReceived(doc.object());
}

