// test_device.cpp
#include <QtTest/QtTest>
#include "device.h"

class TestDevice : public QObject {
    Q_OBJECT

private slots:
    void testInitialization();
    void testProcessTask();
    void testProcessTaskWhileBusy();
    void testSignalEmissions();
};

void TestDevice::testInitialization() {
    Device device("Bread", 1, nullptr);
    QCOMPARE(device.moduleType, QString("Bread"));
    QCOMPARE(device.deviceIndex, 1);
    QCOMPARE(device.isProcessing, false);
}

void TestDevice::testProcessTask() {
    Device device("Cheese", 2, nullptr);
    QSignalSpy statusSpy(&device, &Device::statusChanged);
    QSignalSpy taskSpy(&device, &Device::taskCompleted);

    device.processTask(101, "Make Cheese Sandwich");

    QVERIFY(statusSpy.wait());
    QCOMPARE(statusSpy.count(), 2);
    QCOMPARE(taskSpy.count(), 1);
}

void TestDevice::testProcessTaskWhileBusy() {
    Device device("Egg", 3, nullptr);
    QSignalSpy errorSpy(&device, &Device::errorOccurred);

    device.processTask(102, "Cook Eggs");
    device.processTask(103, "Cook More Eggs");

    QCOMPARE(errorSpy.count(), 1);
}

void TestDevice::testSignalEmissions() {
    Device device("Jam", 4, nullptr);
    QSignalSpy statusSpy(&device, &Device::statusChanged);
    QSignalSpy taskSpy(&device, &Device::taskCompleted);

    device.processTask(104, "Spread Jam");

    QVERIFY(statusSpy.wait());
    QCOMPARE(statusSpy.count(), 2);
    QCOMPARE(taskSpy.count(), 1);

    QList<QVariant> firstStatusArgs = statusSpy.takeFirst();
    QCOMPARE(firstStatusArgs.at(1).toString(), QString("Jam"));
    QCOMPARE(firstStatusArgs.at(2).toInt(), 4);
    QCOMPARE(firstStatusArgs.at(3).toBool(), true);

    QList<QVariant> secondStatusArgs = statusSpy.takeFirst();
    QCOMPARE(secondStatusArgs.at(1).toString(), QString("Jam"));
    QCOMPARE(secondStatusArgs.at(2).toInt(), 4);
    QCOMPARE(secondStatusArgs.at(3).toBool(), false);

    QList<QVariant> taskArgs = taskSpy.takeFirst();
    QCOMPARE(taskArgs.at(1).toString(), QString("Jam"));
    QCOMPARE(taskArgs.at(2).toInt(), 104);
}

QTEST_MAIN(TestDevice)
#include "test_device.moc"