// devicemanager.h
#ifndef DEVICEMANAGER_H
#define DEVICEMANAGER_H

#include <QObject>
#include <QMap>
#include <QQueue>
#include <QThread>
#include <QDebug>
#include "device.h"
#include "message.h"

class DeviceManager : public QObject
{
    Q_OBJECT

public:
    explicit DeviceManager(QObject *parent = nullptr);
    ~DeviceManager() override;

    void processNewOrder(const OrderMessage& order);

signals:
    void deviceStatusChanged(const QString& module, int deviceIndex,
                             DeviceStatus status, const QString& currentTask);
    void processingFinished(const QString& module, int deviceIndex, int orderId);
    void logMessage(const QString& message);

private slots:
    void handleDeviceTaskCompleted(Device* device, const QString& module, int orderId);

private:
    struct OrderTask {
        int orderId;
        QString moduleType;
        QString taskDetail;
        bool isProcessing;
        int currentStep;  // 0: Bread, 1: Cheese, 2: Egg, 3: Jam
    };

    // 장치 관리
    QMap<QString, QList<Device*>> devices;
    QMap<Device*, QThread*> deviceThreads;
    QMap<Device*, bool> deviceAvailability;

    // 작업 관리
    QQueue<OrderTask> orderQueue;  // 이름 변경: taskQueue -> orderQueue
    QMap<int, OrderMessage> activeOrders;
    QMap<int, OrderTask> processingOrders;  // 현재 처리 중인 주문 추적

    void initializeDevices();
    void assignNextTask();
    Device* findAvailableDevice(const QString& module);
    QString getNextModule(int currentStep);
    QString createTaskDetail(const QString& module, const OrderMessage& order);
};

#endif //DEVICEMANAGER_H
