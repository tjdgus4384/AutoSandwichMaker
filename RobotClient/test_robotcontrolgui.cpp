// test_robotcontrolgui.cpp
#include <QtTest/QtTest>
#include "robotcontrolgui.h"
#include "networkmanager.h"
#include "devicemanager.h"

class TestRobotControlGUI : public QObject {
    Q_OBJECT

private slots:
    void testInitializeGUI();
    void testNetworkConnection();
    void testDeviceStatusUpdate();
    void testLogAppending();
};

void TestRobotControlGUI::testInitializeGUI() {
    RobotControlGUI gui;
    QVERIFY(gui.centralWidget != nullptr);
    QVERIFY(gui.mainLayout != nullptr);
    QVERIFY(gui.serverAddressEdit != nullptr);
    QVERIFY(gui.serverPortEdit != nullptr);
    QVERIFY(gui.connectButton != nullptr);
    QVERIFY(gui.networkStatusLabel != nullptr);
    QVERIFY(gui.deviceStatusLabel != nullptr);
    QVERIFY(gui.logTextEdit != nullptr);
}

void TestRobotControlGUI::testNetworkConnection() {
    RobotControlGUI gui;
    QSignalSpy connectSpy(gui.networkManager, &NetworkManager::connected);
    QSignalSpy disconnectSpy(gui.networkManager, &NetworkManager::disconnected);

    gui.serverAddressEdit->setText("localhost");
    gui.serverPortEdit->setText("1234");
    gui.onConnectButtonClicked();

    QCOMPARE(connectSpy.count(), 1);
    QCOMPARE(gui.networkStatusLabel->text(), QString("서버에 연결됨"));

    gui.onConnectButtonClicked();
    QCOMPARE(disconnectSpy.count(), 1);
    QCOMPARE(gui.networkStatusLabel->text(), QString("연결이 해제되었습니다"));
}

void TestRobotControlGUI::testDeviceStatusUpdate() {
    RobotControlGUI gui;
    QSignalSpy statusSpy(gui.deviceManager, &DeviceManager::deviceStatusChanged);

    gui.handleDeviceStatusUpdate("Bread", 1, DeviceStatus::ON, "Baking");
    QCOMPARE(statusSpy.count(), 1);

    QLabel *deviceLabel = gui.deviceLabelMap["Bread"].at(0);
    QCOMPARE(deviceLabel->text(), QString("Bread 1: 작동 중 (Baking)"));
    QCOMPARE(deviceLabel->styleSheet(), QString("QLabel { background-color: green; color: white; padding: 5px; border-radius: 5px; }"));
}

void TestRobotControlGUI::testLogAppending() {
    RobotControlGUI gui;
    QString logMessage = "Test log message";
    gui.appendLog(logMessage);

    QString expectedLog = QString("[%1] %2").arg(QTime::currentTime().toString("hh:mm:ss")).arg(logMessage);
    QCOMPARE(gui.logTextEdit->toPlainText().contains(expectedLog), true);
}

QTEST_MAIN(TestRobotControlGUI)
#include "test_robotcontrolgui.moc"