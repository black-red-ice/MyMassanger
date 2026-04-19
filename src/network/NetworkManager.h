#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H

#include <QObject>
#include <QTcpSocket>

class NetworkManager : public QObject
{
    Q_OBJECT
public:
    explicit NetworkManager(QObject *parent = nullptr);

    void connectToServer(const QString &host, quint16 port);
    void disconnectFromServer();
    void sendMessage(const QString &message);
    void sendCommand(const QString &command);

    bool isConnected() const;

signals:
    void connected();
    void disconnected();
    void messageReceived(const QString &message);
    void errorOccurred(const QString &error);
    void commandResponse(const QString &response);

private slots:
    void onReadyRead();
    void onError(QAbstractSocket::SocketError error);

private:
    QTcpSocket *socket;
};

#endif // NETWORKMANAGER_H
