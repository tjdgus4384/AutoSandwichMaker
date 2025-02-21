// devicemanager.cpp
#include "devicemanager.h"

DeviceManager::DeviceManager(QObject *parent)
    : QObject(parent)
{
    initializeDevices();
}

DeviceManager::~DeviceManager()
{
    for (auto thread : deviceThreads.values()) {
        thread->quit();
        thread->wait();
        delete thread;
    }
}

void DeviceManager::initializeDevices()
{
    QStringList modules = { "Bread", "Cheese", "Egg", "Jam" };

    for (const QString &module : modules) {
        for (int i = 0; i < 2; ++i) {
            Device* device = new Device(module, i + 1);
            QThread* thread = new QThread(this);

            device->moveToThread(thread);
            deviceThreads[device] = thread;
            devices[module].append(device);
            deviceAvailability[device] = true;

            connect(thread, &QThread::started, device, [](){ /* Device idle */ });
            connect(device, &Device::taskCompleted,
                    this, &DeviceManager::handleDeviceTaskCompleted);
            connect(thread, &QThread::finished, device, &QObject::deleteLater);

            thread->start();

            emit deviceStatusChanged(module, i + 1, DeviceStatus::OFF, "");
            emit logMessage(QString("%1 장치 %2 초기화 완료").arg(module).arg(i + 1));
        }
    }
}

void DeviceManager::processNewOrder(const OrderMessage& order)
{
    // 새로운 OrderTask 생성
    OrderTask newTask;
    newTask.orderId = order.orderId;
    newTask.currentStep = 0;
    newTask.isProcessing = false;
    newTask.moduleType = "Bread";  // 초기 모듈은 항상 Bread
    newTask.taskDetail = createTaskDetail("Bread", order);

    // 주문 큐에 추가
    orderQueue.enqueue(newTask);
    activeOrders[order.orderId] = order;

    emit logMessage(QString("새 주문 수신 (ID: %1)").arg(order.orderId));

    // 작업 할당 시도
    assignNextTask();
}

void DeviceManager::assignNextTask()
{
    // 현재 처리 중인 주문들 확인
    for (auto it = processingOrders.begin(); it != processingOrders.end(); ++it) {
        if (it.value().isProcessing) {
            continue;  // 이미 처리 중인 주문은 건너뛰기
        }

        // 다음 단계로 진행
        OrderTask& task = it.value();
        QString nextModule = getNextModule(task.currentStep);
        if (!nextModule.isEmpty()) {
            Device* device = findAvailableDevice(nextModule);
            if (device) {
                // 장치 할당 및 작업 시작
                deviceAvailability[device] = false;
                task.isProcessing = true;
                task.moduleType = nextModule;
                task.taskDetail = createTaskDetail(nextModule, activeOrders[task.orderId]);

                emit deviceStatusChanged(nextModule, device->getDeviceIndex(), DeviceStatus::ON, task.taskDetail);
                emit logMessage(QString("주문 %1: %2 시작").arg(task.orderId).arg(task.taskDetail));

                QMetaObject::invokeMethod(device, "processTask", Qt::QueuedConnection,
                                          Q_ARG(int, task.orderId),
                                          Q_ARG(QString, task.taskDetail));
            }
        }
    }

    // 대기 중인 새 주문 처리
    while (!orderQueue.isEmpty()) {
        OrderTask task = orderQueue.dequeue();

        // 이미 처리 중인 주문인지 확인
        if (processingOrders.contains(task.orderId)) {
            continue;
        }

        Device* device = findAvailableDevice("Bread");
        if (device) {
            // 장치 할당 및 작업 시작
            deviceAvailability[device] = false;
            task.isProcessing = true;
            processingOrders[task.orderId] = task;

            emit deviceStatusChanged("Bread", device->getDeviceIndex(), DeviceStatus::ON, task.taskDetail);
            emit logMessage(QString("주문 %1: %2 시작").arg(task.orderId).arg(task.taskDetail));

            QMetaObject::invokeMethod(device, "processTask", Qt::QueuedConnection,
                                      Q_ARG(int, task.orderId),
                                      Q_ARG(QString, task.taskDetail));
        } else {
            // 사용 가능한 장치가 없으면 다시 큐에 추가
            orderQueue.enqueue(task);
            break;
        }
    }
}

void DeviceManager::handleDeviceTaskCompleted(Device* device, const QString& module, int orderId)
{
    // 장치 상태 업데이트
    deviceAvailability[device] = true;
    emit deviceStatusChanged(module, device->getDeviceIndex(), DeviceStatus::OFF, "");

    if (!processingOrders.contains(orderId)) {
        return;
    }

    OrderTask& task = processingOrders[orderId];
    task.isProcessing = false;
    task.currentStep++;

    if (task.currentStep >= 4) {
        // 모든 단계 완료
        emit logMessage(QString("주문 %1 완료").arg(orderId));
        activeOrders.remove(orderId);
        processingOrders.remove(orderId);
    }

    // 다음 작업 할당 시도
    assignNextTask();
}

Device* DeviceManager::findAvailableDevice(const QString& module)
{
    if (!devices.contains(module)) return nullptr;

    for (Device* device : devices[module]) {
        if (deviceAvailability.value(device, true)) {
            return device;
        }
    }
    return nullptr;
}

QString DeviceManager::getNextModule(int currentStep)
{
    switch (currentStep) {
    case 0: return "Bread";
    case 1: return "Cheese";
    case 2: return "Egg";
    case 3: return "Jam";
    default: return "";
    }
}

QString DeviceManager::createTaskDetail(const QString& module, const OrderMessage& order)
{
    if (module == "Bread")
        return QString("빵 굽기: %1").arg(order.bread);
    else if (module == "Cheese")
        return QString("치즈 추가: %1").arg(order.cheeses.join(", "));
    else if (module == "Egg")
        return QString("계란후라이: %1").arg(order.egg);
    else if (module == "Jam")
        return QString("잼 바르기: %1 (%2%)").arg(order.jams.join(", ")).arg(order.jamAmount);
    return "";
}
