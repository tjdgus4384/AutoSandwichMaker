// networkmanager.cpp
#include "networkmanager.h"
#include <QJsonDocument>
#include <QDebug>

NetworkManager::NetworkManager(QObject *parent, bool isServer)
    : QObject(parent)
    , isServer(isServer)
    , server(nullptr)
    , clientSocket(nullptr)
    , serverAddress("localhost")
    , serverPort(1234)
    , isConnected(false)
{
}

NetworkManager::~NetworkManager()
{
    if (isServer) {
        stopServer();
    } else {
        disconnectFromServer();
    }
}

bool NetworkManager::startServer(quint16 port)
{
    if (!isServer) {
        emit errorOccurred("서버 모드로 설정되지 않았습니다");
        return false;
    }

    cleanupServer();

    server = new QTcpServer(this);
    if (!server) {
        emit errorOccurred("서버 객체 생성 실패");
        return false;
    }

    connect(server, &QTcpServer::newConnection,
            this, &NetworkManager::handleNewConnection);

    if (!server->listen(QHostAddress::Any, port)) {
        QString errorMsg = QString("서버 시작 실패 (포트: %1) - %2")
                               .arg(port)
                               .arg(server->errorString());
        emit errorOccurred(errorMsg);
        cleanupServer();
        return false;
    }

    qDebug() << "서버가 포트" << port << "에서 시작되었습니다";
    return true;
}

bool NetworkManager::connectToServer(const QString& address, quint16 port)
{
    if (isServer) {
        emit errorOccurred("서버 모드에서는 연결할 수 없습니다");
        return false;
    }

    disconnectFromServer();

    serverAddress = address;
    serverPort = port;

    clientSocket = new QTcpSocket(this);

    connect(clientSocket, &QTcpSocket::connected,
            this, &NetworkManager::handleClientConnected);
    connect(clientSocket, &QTcpSocket::disconnected,
            this, &NetworkManager::handleDisconnection);
    connect(clientSocket, &QTcpSocket::readyRead,
            this, &NetworkManager::handleRead);
    connect(clientSocket, &QTcpSocket::errorOccurred,
            this, &NetworkManager::handleSocketError);


    clientSocket->connectToHost(address, port);

    if (!clientSocket->waitForConnected(1234)) {
        emit errorOccurred(QString("서버 연결 실패: %1").arg(clientSocket->errorString()));
        disconnectFromServer();
        return false;
    }

    qDebug() << "서버에 연결됨:" << address << ":" << port;
    return true;
}

bool NetworkManager::stopServer()
{
    if (!isServer || !server) {
        return true;
    }

    cleanupServer();
    emit disconnected();
    return true;
}

void NetworkManager::disconnectFromServer()
{
    if (isServer || !clientSocket) {
        return;
    }

    clientSocket->disconnectFromHost();
    if (clientSocket->state() != QAbstractSocket::UnconnectedState) {
        clientSocket->waitForDisconnected(1000);
    }
    cleanupSocket();
    isConnected = false;
    emit disconnected();
}

bool NetworkManager::isServerRunning() const
{
    return isServer && server && server->isListening();
}

bool NetworkManager::isConnectedToServer() const
{
    return !isServer && clientSocket &&
           clientSocket->state() == QAbstractSocket::ConnectedState;
}

void NetworkManager::cleanupSocket()
{
    if (clientSocket) {
        clientSocket->disconnect();
        delete clientSocket;
        clientSocket = nullptr;
    }
}

void NetworkManager::cleanupServer()
{
    cleanupSocket();
    if (server) {
        server->close();
        delete server;
        server = nullptr;
    }
}

bool NetworkManager::sendMessage(const Message& message)
{
    if (!clientSocket ||
        clientSocket->state() != QAbstractSocket::ConnectedState) {
        return false;
    }

    QJsonDocument doc(message.toJson());
    QByteArray data = doc.toJson(QJsonDocument::Compact);

    // 메시지 크기를 4바이트로 전송
    QByteArray size = QByteArray::number(data.size());
    size.prepend(QByteArray(4 - size.size(), '0'));

    return clientSocket->write(size + data) > 0;
}

void NetworkManager::handleNewConnection()
{
    cleanupSocket();

    clientSocket = server->nextPendingConnection();
    if (!clientSocket) {
        return;
    }

    connect(clientSocket, &QTcpSocket::disconnected,
            this, &NetworkManager::handleDisconnection);
    connect(clientSocket, &QTcpSocket::readyRead,
            this, &NetworkManager::handleRead);
    connect(clientSocket, &QTcpSocket::errorOccurred,
            this, &NetworkManager::handleSocketError);

    isConnected = true;
    emit connected();
}

void NetworkManager::handleClientConnected()
{
    isConnected = true;
    emit connected();
}

void NetworkManager::handleDisconnection()
{
    isConnected = false;
    emit disconnected();

    if (!isServer) {
        cleanupSocket();
    }
}

void NetworkManager::handleRead()
{
    QTcpSocket* socket = qobject_cast<QTcpSocket*>(sender());
    if (!socket) return;

    buffer.append(socket->readAll());

    while (buffer.size() >= 4) {
        int messageSize = buffer.left(4).toInt();
        if (buffer.size() < 4 + messageSize) break;

        QByteArray messageData = buffer.mid(4, messageSize);
        buffer.remove(0, 4 + messageSize);

        QJsonDocument doc = QJsonDocument::fromJson(messageData);
        if (doc.isObject()) {
            emit messageReceived(Message::fromJson(doc.object()));
        }
    }
}

void NetworkManager::handleSocketError(QAbstractSocket::SocketError socketError)
{
    QString errorMsg = isServer ? "서버 오류: " : "클라이언트 오류: ";

    switch (socketError) {
    case QAbstractSocket::ConnectionRefusedError:
        errorMsg += "연결이 거부되었습니다";
        break;
    case QAbstractSocket::RemoteHostClosedError:
        errorMsg += "원격 호스트가 연결을 종료했습니다";
        break;
    case QAbstractSocket::HostNotFoundError:
        errorMsg += "호스트를 찾을 수 없습니다";
        break;
    case QAbstractSocket::SocketAccessError:
        errorMsg += "소켓 접근 권한이 없습니다";
        break;
    case QAbstractSocket::SocketResourceError:
        errorMsg += "시스템의 소켓 자원이 부족합니다";
        break;
    case QAbstractSocket::SocketTimeoutError:
        errorMsg += "작업 시간이 초과되었습니다";
        break;
    case QAbstractSocket::AddressInUseError:
        errorMsg += "주소가 이미 사용 중입니다";
        break;
    default:
        errorMsg += "알 수 없는 오류가 발생했습니다";
    }

    emit errorOccurred(errorMsg);
}
