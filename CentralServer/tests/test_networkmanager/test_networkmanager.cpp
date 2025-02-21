#include <QtTest/QtTest>
#include "networkmanager.h"
#include <QSignalSpy>
#include <QTimer>

class TestNetworkManager : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void testServerStartStop();
    void testClientConnectDisconnect();
    void testMessageSendReceive();

private:
    NetworkManager *serverManager;
    NetworkManager *clientManager;
    quint16 testPort;
};

void TestNetworkManager::initTestCase()
{
    serverManager = new NetworkManager(nullptr, true);   // 서버 모드
    clientManager = new NetworkManager(nullptr, false);  // 클라이언트 모드
    testPort = 12345; // 테스트용 포트
}

void TestNetworkManager::cleanupTestCase()
{
    delete serverManager;
    delete clientManager;
}

void TestNetworkManager::testServerStartStop()
{
    QSignalSpy errorSpy(serverManager, &NetworkManager::errorOccurred);

    // 서버 시작
    QVERIFY(serverManager->startServer(testPort));
    QVERIFY(serverManager->isServerRunning());

    // 이미 실행중인 서버 다시 start 시도 -> 실패 혹은 기존 유지
    // 여기서는 별도 에러 처리는 없으나 다시 시작하더라도 문제가 없도록 설계되었는지 확인
    // 일단 동일 포트에서 재시작 시도는 생략

    // 서버 정지
    QVERIFY(serverManager->stopServer());
    QVERIFY(!serverManager->isServerRunning());

    QCOMPARE(errorSpy.count(), 0);
}

void TestNetworkManager::testClientConnectDisconnect()
{
    // 서버 다시 시작
    QVERIFY(serverManager->startServer(testPort));

    QSignalSpy connectedSpy(clientManager, &NetworkManager::connected);
    QSignalSpy disconnectedSpy(clientManager, &NetworkManager::disconnected);
    QSignalSpy errorSpy(clientManager, &NetworkManager::errorOccurred);

    // 정상 연결
    QVERIFY(clientManager->connectToServer("localhost", testPort));
    QTRY_COMPARE(connectedSpy.count(), 1); // 연결 신호 수신 대기

    // 연결 상태 확인
    QVERIFY(clientManager->isConnectedToServer());

    // 서버 정지 -> 클라이언트 disconnect 시그널 발생
    QVERIFY(serverManager->stopServer());
    QTRY_COMPARE(disconnectedSpy.count(), 1);

    QVERIFY(!clientManager->isConnectedToServer());

    // 에러는 발생하지 않았는지 확인
    QCOMPARE(errorSpy.count(), 0);
}

void TestNetworkManager::testMessageSendReceive()
{
    // 서버 시작
    QVERIFY(serverManager->startServer(testPort));

    QSignalSpy serverConnectedSpy(serverManager, &NetworkManager::connected);
    QSignalSpy serverDisconnectedSpy(serverManager, &NetworkManager::disconnected);
    QSignalSpy serverMessageSpy(serverManager, &NetworkManager::messageReceived);
    QSignalSpy serverErrorSpy(serverManager, &NetworkManager::errorOccurred);

    QSignalSpy clientConnectedSpy(clientManager, &NetworkManager::connected);
    QSignalSpy clientDisconnectedSpy(clientManager, &NetworkManager::disconnected);
    QSignalSpy clientMessageSpy(clientManager, &NetworkManager::messageReceived);
    QSignalSpy clientErrorSpy(clientManager, &NetworkManager::errorOccurred);

    // 클라이언트 연결
    QVERIFY(clientManager->connectToServer("localhost", testPort));
    QTRY_COMPARE(clientConnectedSpy.count(), 1);
    QTRY_COMPARE(serverConnectedSpy.count(), 1);

    // 서버->클라이언트 메시지 전송 테스트
    {
        Message serverMsg;
        serverMsg.type = MessageType::ORDER_NEW;
        QJsonObject dataObj;
        dataObj["testKey"] = "testValue";
        serverMsg.data = dataObj;

        QVERIFY(serverManager->sendMessage(serverMsg));

        // 클라이언트에서 메시지 수신 대기
        QTRY_COMPARE(clientMessageSpy.count(), 1);
        QList<QVariant> args = clientMessageSpy.takeFirst();
        Message receivedMsg = qvariant_cast<Message>(args.at(0));
        QCOMPARE(receivedMsg.type, MessageType::ORDER_NEW);
        QVERIFY(receivedMsg.data.contains("testKey"));
        QCOMPARE(receivedMsg.data["testKey"].toString(), QString("testValue"));
    }

    // 클라이언트->서버 메시지 전송 테스트
    {
        Message clientMsg;
        clientMsg.type = MessageType::DEVICE_STATUS_UPDATE;
        QJsonObject dataObj;
        dataObj["deviceStatus"] = "ON";
        clientMsg.data = dataObj;

        QVERIFY(clientManager->sendMessage(clientMsg));

        // 서버에서 메시지 수신 대기
        QTRY_COMPARE(serverMessageSpy.count(), 1);
        QList<QVariant> args = serverMessageSpy.takeFirst();
        Message receivedMsg = qvariant_cast<Message>(args.at(0));
        QCOMPARE(receivedMsg.type, MessageType::DEVICE_STATUS_UPDATE);
        QVERIFY(receivedMsg.data.contains("deviceStatus"));
        QCOMPARE(receivedMsg.data["deviceStatus"].toString(), QString("ON"));
    }

    // 클라이언트 연결 해제
    clientManager->disconnectFromServer();
    QTRY_COMPARE(clientDisconnectedSpy.count(), 1);
    QTRY_COMPARE(serverDisconnectedSpy.count(), 1);

    // 에러 발생 없음 확인
    QCOMPARE(serverErrorSpy.count(), 0);
    QCOMPARE(clientErrorSpy.count(), 0);
}

QTEST_MAIN(TestNetworkManager)
#include "test_networkmanager.moc"