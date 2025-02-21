// device.cpp

#include "device.h"
#include <QThread>
#include <QDebug>

Device::Device(const QString &module, int index, QObject *parent)
    : QObject(parent),
    moduleType(module),
    deviceIndex(index),
    isProcessing(false)
{
}

void Device::processTask(int orderId, const QString &taskDetail)
{
    if (isProcessing) {
        emit errorOccurred(this, "Device is already processing a task");
        return;
    }

    isProcessing = true;
    emit statusChanged(this, moduleType, deviceIndex, true);

    qDebug() << moduleType << "Device" << deviceIndex
             << "started processing order" << orderId << ":" << taskDetail;

    // 작업 시간 시뮬레이션 (각 모듈별로 다른 처리 시간 설정)
    int processingTime = 5; // 기본값 5초
    if (moduleType == "Bread") {
        processingTime = 10; // 빵은 10초
    } else if (moduleType == "Egg") {
        processingTime = 7;  // 계란은 7초
    } else if (moduleType == "Cheese") {
        processingTime = 3;  // 치즈는 3초
    } else if (moduleType == "Jam") {
        processingTime = 4;  // 잼은 4초
    }

    // 실제 로봇에서는 여기에 하드웨어 제어 코드가 들어갈 것입니다.
    QThread::sleep(processingTime);

    isProcessing = false;
    emit statusChanged(this, moduleType, deviceIndex, false);
    emit taskCompleted(this, moduleType, orderId);

    qDebug() << moduleType << "Device" << deviceIndex
             << "finished processing order" << orderId;
}
