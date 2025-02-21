// device.h
#ifndef DEVICE_H
#define DEVICE_H

#include <QObject>
#include <QString>

class Device : public QObject
{
    Q_OBJECT

public:
    explicit Device(const QString &module, int index, QObject *parent = nullptr);

    QString getModuleType() const { return moduleType; }
    int getDeviceIndex() const { return deviceIndex; }

signals:
    void taskCompleted(Device* device, const QString &module, int orderId);
    void statusChanged(Device* device, const QString &module, int deviceIndex, bool isOn);
    void errorOccurred(Device* device, const QString &errorMessage);

public slots:
    void processTask(int orderId, const QString &taskDetail);

private:
    QString moduleType;
    int deviceIndex;
    bool isProcessing;
};

#endif // DEVICE_H
