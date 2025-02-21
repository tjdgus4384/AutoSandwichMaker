// message.h
#ifndef MESSAGE_H
#define MESSAGE_H

#include <QString>
#include <QJsonObject>
#include <QJsonArray>

// 메시지 타입 정의
enum class MessageType {
    ORDER_NEW,              // 새로운 주문
    ORDER_STATUS_UPDATE,    // 주문 상태 업데이트
    DEVICE_STATUS_UPDATE,   // 장치 상태 업데이트
    ERROR_REPORT           // 에러 보고
};

// 주문 상태 정의
enum class OrderStatus {
    WAITING,        // 대기 중
    PROCESSING,     // 처리 중
    COMPLETED,      // 완료
    ERROR          // 에러
};

// 장치 상태 정의
enum class DeviceStatus {
    OFF,           // 꺼짐
    ON,            // 켜짐
    ERROR         // 에러
};

// 기본 메시지 구조체
struct Message {
    MessageType type;
    QJsonObject data;

    QJsonObject toJson() const {
        QJsonObject json;
        json["type"] = static_cast<int>(type);
        json["data"] = data;
        return json;
    }

    static Message fromJson(const QJsonObject& json) {
        Message msg;
        msg.type = static_cast<MessageType>(json["type"].toInt());
        msg.data = json["data"].toObject();
        return msg;
    }
};

// 주문 메시지 구조체
struct OrderMessage {
    int orderId;
    QString bread;
    QString egg;
    QStringList jams;
    int jamAmount;
    QStringList cheeses;
    OrderStatus status;

    QJsonObject toJson() const {
        QJsonObject json;
        json["orderId"] = orderId;
        json["bread"] = bread;
        json["egg"] = egg;
        json["jams"] = QJsonArray::fromStringList(jams);
        json["jamAmount"] = jamAmount;
        json["cheeses"] = QJsonArray::fromStringList(cheeses);
        json["status"] = static_cast<int>(status);
        return json;
    }

    static OrderMessage fromJson(const QJsonObject& json) {
        OrderMessage order;
        order.orderId = json["orderId"].toInt();
        order.bread = json["bread"].toString();
        order.egg = json["egg"].toString();
        QJsonArray jamsArray = json["jams"].toArray();
        for (const auto& jam : jamsArray) {
            order.jams.append(jam.toString());
        }
        order.jamAmount = json["jamAmount"].toInt();
        QJsonArray cheesesArray = json["cheeses"].toArray();
        for (const auto& cheese : cheesesArray) {
            order.cheeses.append(cheese.toString());
        }
        order.status = static_cast<OrderStatus>(json["status"].toInt());
        return order;
    }
};

// 장치 상태 메시지 구조체
struct DeviceStatusMessage {
    QString moduleType;
    int deviceIndex;
    DeviceStatus status;
    QString currentTask;

    QJsonObject toJson() const {
        QJsonObject json;
        json["moduleType"] = moduleType;
        json["deviceIndex"] = deviceIndex;
        json["status"] = static_cast<int>(status);
        json["currentTask"] = currentTask;
        return json;
    }

    static DeviceStatusMessage fromJson(const QJsonObject& json) {
        DeviceStatusMessage deviceStatus;
        deviceStatus.moduleType = json["moduleType"].toString();
        deviceStatus.deviceIndex = json["deviceIndex"].toInt();
        deviceStatus.status = static_cast<DeviceStatus>(json["status"].toInt());
        deviceStatus.currentTask = json["currentTask"].toString();
        return deviceStatus;
    }
};

#endif // MESSAGE_H
