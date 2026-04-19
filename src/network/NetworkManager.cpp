#include "NetworkManager.h"
#include <QDebug>

NetworkManager::NetworkManager(QObject *parent)
    : QObject(parent)
{
    socket = new QTcpSocket(this);

    connect(socket, &QTcpSocket::connected, this, &NetworkManager::connected);
    connect(socket, &QTcpSocket::disconnected, this, &NetworkManager::disconnected);
    connect(socket, &QTcpSocket::readyRead, this, &NetworkManager::onReadyRead);
    connect(socket, &QTcpSocket::errorOccurred, this, &NetworkManager::onError);
}

void NetworkManager::connectToServer(const QString &host, quint16 port)
{
    if (socket->state() != QAbstractSocket::UnconnectedState) {
        socket->abort();
    }
    socket->connectToHost(host, port);
    qDebug() << "Connecting to" << host << ":" << port;
}

void NetworkManager::disconnectFromServer()
{
    socket->disconnectFromHost();
}

void NetworkManager::sendMessage(const QString &message)
{
    if (socket->state() == QAbstractSocket::ConnectedState && !message.isEmpty()) {
        socket->write(message.toUtf8());
        socket->flush();
        qDebug() << "Sent:" << message;
    }
}

bool NetworkManager::isConnected() const
{
    return socket->state() == QAbstractSocket::ConnectedState;
}

void NetworkManager::onReadyRead()
{
    QByteArray data = socket->readAll();
    QString allData = QString::fromUtf8(data);

    // Разделяем по \n
    QStringList lines = allData.split('\n', Qt::SkipEmptyParts);

    for (const QString &line : lines) {
        qDebug() << "Received line:" << line;

        if (line.startsWith("CONTACTS:") || line.startsWith("CHATS:") ||
            line.startsWith("LOGIN_OK:") || line.startsWith("REGISTER_OK:") ||
            line.startsWith("ERROR:") || line.startsWith("CONTACTS_ERROR:") ||
            line.startsWith("CHATS_ERROR:") || line.startsWith("LOGIN_ERROR:") ||
            line.startsWith("REGISTER_ERROR:") || line.startsWith("CHAT_READY:") ||
            line.startsWith("MESSAGES:") || line.startsWith("MESSAGES_ERROR:") ||
            line.startsWith("MESSAGE_SENT:") || line.startsWith("MESSAGE_ERROR:") ||
            line.startsWith("NEW_MESSAGE:")) {  // ← ДОБАВИТЬ
            emit commandResponse(line);
        } else {
            emit messageReceived(line);
        }
    }
}

void NetworkManager::onError(QAbstractSocket::SocketError error)
{
    QString errorString = socket->errorString();
    qDebug() << "Socket error:" << error << errorString;
    emit errorOccurred(errorString);
}

void NetworkManager::sendCommand(const QString &command)
{
    if (socket->state() == QAbstractSocket::ConnectedState) {
        socket->write(command.toUtf8());
        socket->flush();
        qDebug() << "Command sent:" << command;
    }
}
