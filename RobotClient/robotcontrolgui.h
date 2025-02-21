// robotcontrolgui.h
#ifndef ROBOTCONTROLGUI_H
#define ROBOTCONTROLGUI_H

#include <QMainWindow>
#include <QLabel>
#include <QTextEdit>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include "networkmanager.h"
#include "devicemanager.h"

class RobotControlGUI : public QMainWindow
{
    Q_OBJECT

public:
    explicit RobotControlGUI(QWidget *parent = nullptr);
    ~RobotControlGUI() override;

private slots:
    void onConnectButtonClicked();
    void handleNetworkMessage(const Message& message);
    void handleNetworkError(const QString& error);
    void handleNetworkConnection();
    void handleNetworkDisconnection();
    void handleDeviceStatusUpdate(const QString& module, int deviceIndex,
                                  DeviceStatus status, const QString& currentTask);
    void handleProcessingFinished(const QString& module, int deviceIndex, int orderId);
    void appendLog(const QString& message);

private:
    // GUI 요소
    QWidget *centralWidget;
    QVBoxLayout *mainLayout;

    // 네트워크 연결 제어
    QLineEdit *serverAddressEdit;
    QLineEdit *serverPortEdit;
    QPushButton *connectButton;
    QLabel *networkStatusLabel;

    // 장치 상태 표시
    QLabel *deviceStatusLabel;
    QGridLayout *deviceGridLayout;
    QMap<QString, QList<QLabel*>> deviceLabelMap;

    // 로그 표시
    QLabel *logLabel;
    QTextEdit *logTextEdit;

    // 매니저 객체
    NetworkManager *networkManager;
    DeviceManager *deviceManager;

    // GUI 초기화 함수
    void initializeGUI();
    void setupNetworkControl();
    void setupDeviceStatus();
    void setupLogArea();
    void updateDeviceStatusDisplay(const QString& module, int deviceIndex,
                                   DeviceStatus status, const QString& currentTask = "");
    void updateNetworkStatus(const QString& status, const QString& color);
};

#endif // ROBOTCONTROLGUI_H
