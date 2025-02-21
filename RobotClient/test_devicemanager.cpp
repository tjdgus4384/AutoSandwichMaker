// test_devicemanager.cpp
#include <QtTest/QtTest>
#include "devicemanager.h"
#include "device.h"

class TestDeviceManager : public QObject {
    Q_OBJECT

private slots:
    void testHandleDeviceTaskCompleted();
    void testHandleDeviceTaskCompletedWithNonExistentOrder();
    void testHandleDeviceTaskCompletedWithAllStepsCompleted();
};

void TestDeviceManager::testHandleDeviceTaskCompleted() {
    DeviceManager manager;
    OrderMessage order;
    order.orderId = 1;
    order.bread = "Whole Wheat";
    order.cheeses = {"Cheddar"};
    order.egg = "Sunny Side Up";
    order.jams = {"Strawberry"};
    order.jamAmount = 50;

    manager.processNewOrder(order);

    Device* device = manager.findAvailableDevice("Bread");
    QVERIFY(device != nullptr);

    QSignalSpy statusSpy(&manager, &DeviceManager::deviceStatusChanged);
    QSignalSpy logSpy(&manager, &DeviceManager::logMessage);

    manager.handleDeviceTaskCompleted(device, "Bread", 1);

    QCOMPARE(statusSpy.count(), 1);
    QCOMPARE(logSpy.count(), 1);

    QList<QVariant> statusArgs = statusSpy.takeFirst();
    QCOMPARE(statusArgs.at(0).toString(), QString("Bread"));
    QCOMPARE(statusArgs.at(1).toInt(), device->getDeviceIndex());
    QCOMPARE(statusArgs.at(2).toInt(), DeviceStatus::OFF);

    QList<QVariant> logArgs = logSpy.takeFirst();
    QCOMPARE(logArgs.at(0).toString(), QString("주문 1: 치즈 추가: Cheddar 시작"));
}

void TestDeviceManager::testHandleDeviceTaskCompletedWithNonExistentOrder() {
    DeviceManager manager;
    Device* device = new Device("Bread", 1, nullptr);

    QSignalSpy statusSpy(&manager, &DeviceManager::deviceStatusChanged);

    manager.handleDeviceTaskCompleted(device, "Bread", 999);

    QCOMPARE(statusSpy.count(), 1);

    QList<QVariant> statusArgs = statusSpy.takeFirst();
    QCOMPARE(statusArgs.at(0).toString(), QString("Bread"));
    QCOMPARE(statusArgs.at(1).toInt(), device->getDeviceIndex());
    QCOMPARE(statusArgs.at(2).toInt(), DeviceStatus::OFF);
}

void TestDeviceManager::testHandleDeviceTaskCompletedWithAllStepsCompleted() {
    DeviceManager manager;
    OrderMessage order;
    order.orderId = 2;
    order.bread = "Whole Wheat";
    order.cheeses = {"Cheddar"};
    order.egg = "Sunny Side Up";
    order.jams = {"Strawberry"};
    order.jamAmount = 50;

    manager.processNewOrder(order);

    Device* device = manager.findAvailableDevice("Bread");
    QVERIFY(device != nullptr);

    QSignalSpy logSpy(&manager, &DeviceManager::logMessage);

    for (int i = 0; i < 4; ++i) {
        manager.handleDeviceTaskCompleted(device, manager.getNextModule(i), 2);
    }

    QCOMPARE(logSpy.count(), 5); // 4 steps + 1 completion log

    QList<QVariant> logArgs = logSpy.takeLast();
    QCOMPARE(logArgs.at(0).toString(), QString("주문 2 완료"));
}

QTEST_MAIN(TestDeviceManager)
#include "test_devicemanager.moc"