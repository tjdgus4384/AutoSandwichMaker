// ordermanager.cpp

#include "ordermanager.h"
#include <QDebug>

OrderManager::OrderManager(QObject *parent)
    : QObject(parent), nextOrderId(1)
{
}

void OrderManager::submitOrder(const QString& bread, const QString& egg,
                               const QStringList& jams, int jamAmount,
                               const QStringList& cheeses)
{
    OrderMessage order;
    order.orderId = nextOrderId++;
    order.bread = bread;
    order.egg = egg;
    order.jams = jams;
    order.jamAmount = jamAmount;
    order.cheeses = cheeses;
    order.status = OrderStatus::WAITING;

    activeOrders[order.orderId] = order;
    emit newOrderCreated(order);
    emit logMessage(QString("새로운 주문이 생성되었습니다. (주문 ID: %1)").arg(order.orderId));
}

QList<OrderMessage> OrderManager::getActiveOrders() const
{
    return activeOrders.values();
}

void OrderManager::handleDeviceStatusUpdate(const DeviceStatusMessage& status)
{
    emit logMessage(QString("%1 장치 %2: %3 - %4")
                        .arg(status.moduleType)
                        .arg(status.deviceIndex)
                        .arg(status.status == DeviceStatus::ON ? "작동 중" : "대기 중")
                        .arg(status.currentTask));
}

void OrderManager::handleOrderStatusUpdate(int orderId, const QString& module, OrderStatus status)
{
    if (!activeOrders.contains(orderId)) {
        qDebug() << "Unknown order ID:" << orderId;
        return;
    }

    updateOrderStatus(orderId, module, status);
}

void OrderManager::updateOrderStatus(int orderId, const QString& module, OrderStatus status)
{
    if (!activeOrders.contains(orderId)) return;

    OrderMessage& order = activeOrders[orderId];
    order.status = status;

    QString statusStr;
    switch(status) {
    case OrderStatus::WAITING:
        statusStr = "대기 중";
        break;
    case OrderStatus::PROCESSING:
        statusStr = module + " 처리 중";
        break;
    case OrderStatus::COMPLETED:
        statusStr = "완료됨";
        break;
    case OrderStatus::ERROR:
        statusStr = "오류 발생";
        break;
    }

    emit orderStatusChanged(orderId, statusStr);
    emit logMessage(QString("주문 %1: %2").arg(orderId).arg(statusStr));

    if (status == OrderStatus::COMPLETED && isOrderComplete(order)) {
        emit orderCompleted(orderId);
        activeOrders.remove(orderId);
    }
}

bool OrderManager::isOrderComplete(const OrderMessage& order) const
{
    // 모든 필수 작업이 완료되었는지 확인
    bool breadDone = true; // 빵은 필수
    bool eggDone = !order.egg.isEmpty();
    bool jamDone = order.jams.isEmpty() || order.jamAmount > 0;
    bool cheeseDone = order.cheeses.isEmpty();

    return breadDone && eggDone && jamDone && cheeseDone;
}
