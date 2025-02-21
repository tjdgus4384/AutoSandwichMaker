// ordermanager.h
#ifndef ORDERMANAGER_H
#define ORDERMANAGER_H

#include <QObject>
#include <QMap>
#include <QQueue>
#include "message.h"

class OrderManager : public QObject
{
    Q_OBJECT

public:
    explicit OrderManager(QObject *parent = nullptr);

    void submitOrder(const QString& bread, const QString& egg,
                     const QStringList& jams, int jamAmount,
                     const QStringList& cheeses);
    QList<OrderMessage> getActiveOrders() const;

signals:
    void orderStatusChanged(int orderId, const QString& status);
    void orderCompleted(int orderId);
    void newOrderCreated(const OrderMessage& order);
    void logMessage(const QString& message);

public slots:
    void handleDeviceStatusUpdate(const DeviceStatusMessage& status);
    void handleOrderStatusUpdate(int orderId, const QString& module, OrderStatus status);

private:
    int nextOrderId;
    QMap<int, OrderMessage> activeOrders;
    QQueue<OrderMessage> pendingOrders;

    void updateOrderStatus(int orderId, const QString& module, OrderStatus status);
    bool isOrderComplete(const OrderMessage& order) const;
};

#endif // ORDERMANAGER_H
