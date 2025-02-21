// networkmanager.h
#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QHostAddress>
#include <QJsonDocument>
#include <QDebug>
#include "message.h"

class NetworkManager : public QObject
{
    Q_OBJECT

public:
    explicit NetworkManager(QObject *parent = nullptr, bool isServer = true);
    ~NetworkManager() override;

    // 서버 관련 함수
    bool startServer(quint16 port = 1234);
    bool stopServer();
    bool isServerRunning() const;

    // 클라이언트 관련 함수
    bool connectToServer(const QString& address, quint16 port = 1234);
    void disconnectFromServer();
    bool isConnectedToServer() const;

    // 공통 함수
    bool sendMessage(const Message& message);

signals:
    void messageReceived(const Message& message);
    void connected();
    void disconnected();
    void errorOccurred(const QString& error);

private slots:
    void handleNewConnection();
    void handleClientConnected();
    void handleDisconnection();
    void handleRead();
    void handleSocketError(QAbstractSocket::SocketError socketError);

private:
    // 설정 및 상태
    bool isServer;
    QString serverAddress;
    quint16 serverPort;
    bool isConnected;

    // 네트워크 객체
    QTcpServer* server;
    QTcpSocket* clientSocket;
    QByteArray buffer;

    // 유틸리티 함수
    void cleanupSocket();
    void cleanupServer();
};

#endif // NETWORKMANAGER_H

