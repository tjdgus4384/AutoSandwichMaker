// robotcontrolgui.cpp
#include "robotcontrolgui.h"
#include <QDebug>
#include <QIntValidator>

RobotControlGUI::RobotControlGUI(QWidget *parent)
    : QMainWindow(parent)
{
    // GUI 초기화
    centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    mainLayout = new QVBoxLayout(centralWidget);

    // 매니저 객체 초기화
    networkManager = new NetworkManager(this, false);  // 클라이언트 모드
    deviceManager = new DeviceManager(this);

    // GUI 설정
    initializeGUI();

    // 네트워크 이벤트 연결
    connect(networkManager, &NetworkManager::messageReceived,
            this, &RobotControlGUI::handleNetworkMessage);
    connect(networkManager, &NetworkManager::errorOccurred,
            this, &RobotControlGUI::handleNetworkError);
    connect(networkManager, &NetworkManager::connected,
            this, &RobotControlGUI::handleNetworkConnection);
    connect(networkManager, &NetworkManager::disconnected,
            this, &RobotControlGUI::handleNetworkDisconnection);

    // 장치 매니저 이벤트 연결
    connect(deviceManager, &DeviceManager::deviceStatusChanged,
            this, &RobotControlGUI::handleDeviceStatusUpdate);
    connect(deviceManager, &DeviceManager::processingFinished,
            this, &RobotControlGUI::handleProcessingFinished);
    connect(deviceManager, &DeviceManager::logMessage,
            this, &RobotControlGUI::appendLog);

    setWindowTitle("샌드위치 제조 로봇 제어 시스템");
}

RobotControlGUI::~RobotControlGUI()
{
    delete networkManager;
    delete deviceManager;
}

void RobotControlGUI::initializeGUI()
{
    setupNetworkControl();
    setupDeviceStatus();
    setupLogArea();
}

void RobotControlGUI::setupNetworkControl()
{
    QHBoxLayout *networkLayout = new QHBoxLayout;

    QLabel *addressLabel = new QLabel("서버 주소:", this);
    serverAddressEdit = new QLineEdit(this);
    serverAddressEdit->setText("localhost");

    QLabel *portLabel = new QLabel("포트:", this);
    serverPortEdit = new QLineEdit(this);
    serverPortEdit->setText("1234");
    serverPortEdit->setValidator(new QIntValidator(1024, 65535, this));

    connectButton = new QPushButton("연결", this);
    networkStatusLabel = new QLabel("연결 대기 중...", this);
    networkStatusLabel->setStyleSheet("QLabel { color: orange; }");

    networkLayout->addWidget(addressLabel);
    networkLayout->addWidget(serverAddressEdit);
    networkLayout->addWidget(portLabel);
    networkLayout->addWidget(serverPortEdit);
    networkLayout->addWidget(connectButton);
    networkLayout->addWidget(networkStatusLabel);
    networkLayout->addStretch();

    mainLayout->addLayout(networkLayout);

    connect(connectButton, &QPushButton::clicked,
            this, &RobotControlGUI::onConnectButtonClicked);
}

void RobotControlGUI::setupDeviceStatus()
{
    deviceStatusLabel = new QLabel("장치 상태:", this);
    mainLayout->addWidget(deviceStatusLabel);

    deviceGridLayout = new QGridLayout;
    QStringList modules = {"Bread", "Cheese", "Egg", "Jam"};
    QStringList translations = {"Bread", "Cheese", "Egg", "Jam"};

    for (int i = 0; i < modules.size(); ++i) {
        QLabel *moduleLabel = new QLabel(translations[i], this);
        deviceGridLayout->addWidget(moduleLabel, 0, i);

        QList<QLabel*> deviceLabels;
        for (int j = 0; j < 2; ++j) {
            QLabel *deviceLabel = new QLabel(QString("%1 %2: 대기 중")
                                                 .arg(translations[i])
                                                 .arg(j + 1), this);
            deviceLabel->setStyleSheet(
                "QLabel { background-color: gray; color: white; "
                "padding: 5px; border-radius: 5px; }");
            deviceGridLayout->addWidget(deviceLabel, j + 1, i);
            deviceLabels.append(deviceLabel);
        }
        deviceLabelMap[modules[i]] = deviceLabels;
    }

    mainLayout->addLayout(deviceGridLayout);
}

void RobotControlGUI::setupLogArea()
{
    logLabel = new QLabel("작업 로그:", this);
    mainLayout->addWidget(logLabel);

    logTextEdit = new QTextEdit(this);
    logTextEdit->setReadOnly(true);
    logTextEdit->setMinimumHeight(200);
    mainLayout->addWidget(logTextEdit);
}

void RobotControlGUI::onConnectButtonClicked()
{
    if (networkManager->isServerRunning()) {
        // 연결 해제
        networkManager->stopServer();  // stopServer가 연결 해제도 처리
        connectButton->setText("연결");
        serverAddressEdit->setEnabled(true);
        serverPortEdit->setEnabled(true);
        updateNetworkStatus("연결이 해제되었습니다", "orange");
    } else {
        // 연결 시도
        serverAddressEdit->setEnabled(false);
        serverPortEdit->setEnabled(false);
        connectButton->setEnabled(false);
        updateNetworkStatus("연결 시도 중...", "orange");

        bool ok;
        int port = serverPortEdit->text().toInt(&ok);
        if (!ok || port < 1024 || port > 65535) {
            updateNetworkStatus("잘못된 포트 번호입니다", "red");
            serverAddressEdit->setEnabled(true);
            serverPortEdit->setEnabled(true);
            connectButton->setEnabled(true);
            return;
        }

        if (networkManager->connectToServer(serverAddressEdit->text(), port)) {
            connectButton->setText("연결 해제");
            updateNetworkStatus("서버에 연결됨", "green");
        }
        connectButton->setEnabled(true);
    }
}

void RobotControlGUI::handleNetworkMessage(const Message& message)
{
    switch (message.type) {
    case MessageType::ORDER_NEW: {
        OrderMessage order = OrderMessage::fromJson(message.data);
        appendLog(QString("새 주문 수신 (ID: %1)").arg(order.orderId));
        deviceManager->processNewOrder(order);
        break;
    }
    default:
        qDebug() << "Unknown message type received";
        break;
    }
}

void RobotControlGUI::handleNetworkError(const QString& error)
{
    updateNetworkStatus("오류: " + error, "red");
    connectButton->setText("연결");
    connectButton->setEnabled(true);
    serverAddressEdit->setEnabled(true);
    serverPortEdit->setEnabled(true);
}

void RobotControlGUI::handleNetworkConnection()
{
    updateNetworkStatus("서버에 연결됨", "green");
}

void RobotControlGUI::handleNetworkDisconnection()
{
    updateNetworkStatus("서버와 연결이 끊어짐", "orange");
    connectButton->setText("연결");
    serverAddressEdit->setEnabled(true);
    serverPortEdit->setEnabled(true);
}

void RobotControlGUI::updateNetworkStatus(const QString& status, const QString& color)
{
    networkStatusLabel->setText(status);
    networkStatusLabel->setStyleSheet(QString("QLabel { color: %1; }").arg(color));
    appendLog(status);
}

void RobotControlGUI::handleDeviceStatusUpdate(const QString& module, int deviceIndex,
                                               DeviceStatus status, const QString& currentTask)
{
    updateDeviceStatusDisplay(module, deviceIndex, status, currentTask);

    // 상태 업데이트를 서버에 전송
    DeviceStatusMessage statusMsg;
    statusMsg.moduleType = module;
    statusMsg.deviceIndex = deviceIndex;
    statusMsg.status = status;
    statusMsg.currentTask = currentTask;

    Message message;
    message.type = MessageType::DEVICE_STATUS_UPDATE;
    message.data = statusMsg.toJson();
    networkManager->sendMessage(message);
}

void RobotControlGUI::updateDeviceStatusDisplay(const QString& module, int deviceIndex,
                                                DeviceStatus status, const QString& currentTask)
{
    if (deviceLabelMap.contains(module) && (deviceIndex - 1) < deviceLabelMap[module].size()) {
        QLabel *deviceLabel = deviceLabelMap[module][deviceIndex - 1];
        QString statusText = QString("%1 %2: %3")
                                 .arg(module)
                                 .arg(deviceIndex)
                                 .arg(status == DeviceStatus::ON ? "작동 중" : "대기 중");

        if (!currentTask.isEmpty()) {
            statusText += QString(" (%1)").arg(currentTask);
        }

        deviceLabel->setText(statusText);
        QString color = status == DeviceStatus::ON ? "green" :
                            status == DeviceStatus::ERROR ? "red" : "gray";
        deviceLabel->setStyleSheet(QString("QLabel { background-color: %1; "
                                           "color: white; padding: 5px; "
                                           "border-radius: 5px; }").arg(color));
    }
}

void RobotControlGUI::handleProcessingFinished(const QString& module, int deviceIndex, int orderId)
{
    appendLog(QString("작업 완료 - 모듈: %1, 장치: %2, 주문 ID: %3")
                  .arg(module).arg(deviceIndex).arg(orderId));

    // 작업 완료 메시지를 서버에 전송
    Message message;
    message.type = MessageType::ORDER_STATUS_UPDATE;

    QJsonObject data;
    data["orderId"] = orderId;
    data["module"] = module;
    data["status"] = static_cast<int>(OrderStatus::COMPLETED);

    message.data = data;
    networkManager->sendMessage(message);
}

void RobotControlGUI::appendLog(const QString& message)
{
    QString timestamp = QTime::currentTime().toString("hh:mm:ss");
    logTextEdit->append(QString("[%1] %2").arg(timestamp).arg(message));
}

