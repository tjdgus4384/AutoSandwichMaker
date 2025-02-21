// ordermanagergui.cpp
#include "ordermanagergui.h"

OrderManagerGUI::OrderManagerGUI(QWidget *parent)
    : QMainWindow(parent)
    , nextOrderId(1)
{
    centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    mainLayout = new QVBoxLayout(centralWidget);

    initializeGUI();
    setupNetworkConnections();

    setWindowTitle("샌드위치 주문 관리 시스템");
    setMinimumSize(400, 600);
}

OrderManagerGUI::~OrderManagerGUI()
{
    if (networkManager) {
        networkManager->stopServer();
        delete networkManager;
    }
}

void OrderManagerGUI::initializeGUI()
{
    setupServerControl();
    setupOrderInputs();
    setupOrderStatusTable();
}

void OrderManagerGUI::setupServerControl()
{
    QHBoxLayout *serverLayout = new QHBoxLayout;

    QLabel *portLabel = new QLabel("포트:", this);
    portSpinBox = new QSpinBox(this);
    portSpinBox->setRange(1024, 65535);
    portSpinBox->setValue(1234);

    startServerButton = new QPushButton("서버 시작", this);
    networkStatusLabel = new QLabel("서버 시작 대기 중...", this);
    networkStatusLabel->setStyleSheet("QLabel { color: orange; }");

    serverLayout->addWidget(portLabel);
    serverLayout->addWidget(portSpinBox);
    serverLayout->addWidget(startServerButton);
    serverLayout->addWidget(networkStatusLabel);
    serverLayout->addStretch();

    mainLayout->addLayout(serverLayout);
}

void OrderManagerGUI::setupOrderInputs()
{
    // 빵 선택
    breadLabel = new QLabel("빵 종류:", this);
    ryeBreadRadioButton = new QRadioButton("호밀빵", this);
    whiteBreadRadioButton = new QRadioButton("흰빵", this);
    breadButtonGroup = new QButtonGroup(this);
    breadButtonGroup->addButton(ryeBreadRadioButton);
    breadButtonGroup->addButton(whiteBreadRadioButton);

    QHBoxLayout *breadLayout = new QHBoxLayout;
    breadLayout->addWidget(breadLabel);
    breadLayout->addWidget(ryeBreadRadioButton);
    breadLayout->addWidget(whiteBreadRadioButton);
    breadLayout->addStretch();
    mainLayout->addLayout(breadLayout);

    // 계란 선택
    eggLabel = new QLabel("계란후라이:", this);
    hardBoiledEggRadioButton = new QRadioButton("완숙", this);
    softBoiledEggRadioButton = new QRadioButton("반숙", this);
    eggButtonGroup = new QButtonGroup(this);
    eggButtonGroup->addButton(hardBoiledEggRadioButton);
    eggButtonGroup->addButton(softBoiledEggRadioButton);

    QHBoxLayout *eggLayout = new QHBoxLayout;
    eggLayout->addWidget(eggLabel);
    eggLayout->addWidget(hardBoiledEggRadioButton);
    eggLayout->addWidget(softBoiledEggRadioButton);
    eggLayout->addStretch();
    mainLayout->addLayout(eggLayout);

    // 잼 선택
    jamLabel = new QLabel("잼 종류:", this);
    strawberryJamCheckBox = new QCheckBox("딸기잼", this);
    appleJamCheckBox = new QCheckBox("사과잼", this);

    QHBoxLayout *jamLayout = new QHBoxLayout;
    jamLayout->addWidget(jamLabel);
    jamLayout->addWidget(strawberryJamCheckBox);
    jamLayout->addWidget(appleJamCheckBox);
    jamLayout->addStretch();
    mainLayout->addLayout(jamLayout);

    // 잼 양 설정
    jamAmountLabel = new QLabel("잼의 양:", this);
    jamAmountSlider = new QSlider(Qt::Horizontal, this);
    jamAmountSlider->setRange(0, 100);
    jamAmountSlider->setTickInterval(50);
    jamAmountSlider->setTickPosition(QSlider::TicksBelow);
    jamAmountSlider->setValue(50);
    mainLayout->addWidget(jamAmountLabel);
    mainLayout->addWidget(jamAmountSlider);

    // 치즈 선택
    cheeseLabel = new QLabel("치즈:", this);
    mozzarellaCheckBox = new QCheckBox("모짜렐라", this);
    cheddarCheckBox = new QCheckBox("체다", this);

    QHBoxLayout *cheeseLayout = new QHBoxLayout;
    cheeseLayout->addWidget(cheeseLabel);
    cheeseLayout->addWidget(mozzarellaCheckBox);
    cheeseLayout->addWidget(cheddarCheckBox);
    cheeseLayout->addStretch();
    mainLayout->addLayout(cheeseLayout);

    // 주문 제출 버튼
    submitButton = new QPushButton("주문 제출", this);
    mainLayout->addWidget(submitButton);

    connect(submitButton, &QPushButton::clicked, this, &OrderManagerGUI::onOrderSubmit);
}

void OrderManagerGUI::setupOrderStatusTable()
{
    statusLabel = new QLabel("주문 상태:", this);
    mainLayout->addWidget(statusLabel);

    orderStatusTable = new QTableWidget(this);
    orderStatusTable->setColumnCount(3);
    orderStatusTable->setHorizontalHeaderLabels(QStringList() << "주문 ID" << "세부 사항" << "상태");
    orderStatusTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    orderStatusTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    mainLayout->addWidget(orderStatusTable);

    logLabel = new QLabel("작업 로그:", this);
    mainLayout->addWidget(logLabel);

    logTextEdit = new QTextEdit(this);
    logTextEdit->setReadOnly(true);
    logTextEdit->setMinimumHeight(150);
    mainLayout->addWidget(logTextEdit);
}

void OrderManagerGUI::setupNetworkConnections()
{
    networkManager = new NetworkManager(this, true);  // 서버 모드

    connect(networkManager, &NetworkManager::messageReceived,
            this, &OrderManagerGUI::handleNetworkMessage);
    connect(networkManager, &NetworkManager::errorOccurred,
            this, &OrderManagerGUI::handleNetworkError);
    connect(networkManager, &NetworkManager::connected,
            this, &OrderManagerGUI::handleNetworkConnection);
    connect(networkManager, &NetworkManager::disconnected,
            this, &OrderManagerGUI::handleNetworkDisconnection);
    connect(startServerButton, &QPushButton::clicked,
            this, &OrderManagerGUI::onStartServerClicked);
}

QString OrderManagerGUI::validateOrder()
{
    QStringList missingItems;

    if (!breadButtonGroup->checkedButton()) {
        missingItems << "빵 종류";
    }

    if (!eggButtonGroup->checkedButton()) {
        missingItems << "계란 종류";
    }

    if (missingItems.isEmpty()) {
        return QString();
    } else {
        return QString("다음 항목을 선택해주세요: %1").arg(missingItems.join(", "));
    }
}

void OrderManagerGUI::onStartServerClicked()
{
    if (networkManager->isServerRunning()) {
        networkManager->stopServer();
        startServerButton->setText("서버 시작");
        portSpinBox->setEnabled(true);
        updateNetworkStatus("서버가 중지되었습니다", "orange");
    } else {
        startServerButton->setEnabled(false);
        updateNetworkStatus("서버 시작 중...", "orange");

        if (networkManager->startServer(portSpinBox->value())) {
            startServerButton->setText("서버 중지");
            portSpinBox->setEnabled(false);
            updateNetworkStatus(QString("서버 실행 중 (포트: %1)").arg(portSpinBox->value()), "green");
        }
        startServerButton->setEnabled(true);
    }
}

void OrderManagerGUI::onOrderSubmit()
{
    QString validationError = validateOrder();
    if (!validationError.isEmpty()) {
        appendLog("주문 오류: " + validationError);
        return;
    }

    if (!networkManager || !networkManager->isServerRunning()) {
        appendLog("오류: 서버가 실행되고 있지 않습니다. 서버를 먼저 시작해주세요.");
        return;
    }

    try {
        OrderMessage order;
        order.orderId = nextOrderId;
        order.bread = breadButtonGroup->checkedButton()->text();
        order.egg = eggButtonGroup->checkedButton()->text();

        if (strawberryJamCheckBox->isChecked())
            order.jams << "딸기잼";
        if (appleJamCheckBox->isChecked())
            order.jams << "사과잼";
        order.jamAmount = jamAmountSlider->value();

        if (mozzarellaCheckBox->isChecked())
            order.cheeses << "모짜렐라";
        if (cheddarCheckBox->isChecked())
            order.cheeses << "체다";

        Message message;
        message.type = MessageType::ORDER_NEW;
        message.data = order.toJson();

        if (networkManager->sendMessage(message)) {
            ActiveOrder activeOrder;
            activeOrder.order = order;
            activeOrder.currentStep = BREAD_STEP;
            activeOrder.isProcessing = false;
            activeOrder.status = "대기 중";

            activeOrders[nextOrderId] = activeOrder;
            updateOrderStatusTable(nextOrderId, "빵 준비 대기 중", "대기 중");
            appendLog(QString("새로운 주문이 접수되었습니다. (주문 ID: %1)").arg(nextOrderId));

            nextOrderId++;
            resetOrderForm();
        } else {
            appendLog("주문 전송 실패: 서버 연결을 확인해주세요.");
        }
    } catch (const std::exception& e) {
        qDebug() << "Order submit error:" << e.what();
        appendLog(QString("주문 처리 중 오류 발생: %1").arg(e.what()));
    }
}
void OrderManagerGUI::handleNetworkMessage(const Message& message)
{
    try {
        qDebug() << "Received message type:" << static_cast<int>(message.type);
        qDebug() << "Message data:" << message.data;

        switch (message.type) {
        case MessageType::DEVICE_STATUS_UPDATE: {
            DeviceStatusMessage status = DeviceStatusMessage::fromJson(message.data);
            QString statusStr = status.status == DeviceStatus::ON ? "작동 중" : "대기 중";

            // 디바이스 상태에 따른 주문 상태 업데이트 추가
            if (!status.currentTask.isEmpty()) {
                // 작업 시작 시
                for (auto it = activeOrders.begin(); it != activeOrders.end(); ++it) {
                    if (!it.value().isProcessing) {
                        QString currentStepModule;
                        switch (it.value().currentStep) {
                        case BREAD_STEP: currentStepModule = "Bread"; break;
                        case CHEESE_STEP: currentStepModule = "Cheese"; break;
                        case EGG_STEP: currentStepModule = "Egg"; break;
                        case JAM_STEP: currentStepModule = "Jam"; break;
                        default: continue;
                        }

                        if (status.moduleType == currentStepModule) {
                            Message orderUpdate;
                            orderUpdate.type = MessageType::ORDER_STATUS_UPDATE;
                            QJsonObject updateData;
                            updateData["orderId"] = it.key();
                            updateData["module"] = status.moduleType;
                            updateData["status"] = static_cast<int>(OrderStatus::PROCESSING);
                            orderUpdate.data = updateData;
                            handleNetworkMessage(orderUpdate);
                            break;
                        }
                    }
                }
            } else {
                // 작업 완료 시
                for (auto it = activeOrders.begin(); it != activeOrders.end(); ++it) {
                    if (it.value().isProcessing) {
                        QString currentStepModule;
                        switch (it.value().currentStep) {
                        case BREAD_STEP: currentStepModule = "Bread"; break;
                        case CHEESE_STEP: currentStepModule = "Cheese"; break;
                        case EGG_STEP: currentStepModule = "Egg"; break;
                        case JAM_STEP: currentStepModule = "Jam"; break;
                        default: continue;
                        }

                        if (status.moduleType == currentStepModule) {
                            Message orderUpdate;
                            orderUpdate.type = MessageType::ORDER_STATUS_UPDATE;
                            QJsonObject updateData;
                            updateData["orderId"] = it.key();
                            updateData["module"] = status.moduleType;
                            updateData["status"] = static_cast<int>(OrderStatus::COMPLETED);
                            orderUpdate.data = updateData;
                            handleNetworkMessage(orderUpdate);
                            break;
                        }
                    }
                }
            }

            appendLog(QString("%1 장치 %2: %3 - %4")
                          .arg(status.moduleType)
                          .arg(status.deviceIndex)
                          .arg(statusStr)
                          .arg(status.currentTask));
            break;
        }
        case MessageType::ORDER_STATUS_UPDATE: {
            int orderId = message.data["orderId"].toInt();
            QString module = message.data["module"].toString();
            OrderStatus status = static_cast<OrderStatus>(message.data["status"].toInt());

            if (!activeOrders.contains(orderId)) {
                qDebug() << "Order not found in activeOrders:" << orderId;
                return;
            }

            ActiveOrder& activeOrder = activeOrders[orderId];
            QString stepText;
            QString statusText;

            if (status == OrderStatus::PROCESSING) {
                activeOrder.isProcessing = true;
                statusText = "처리 중";

                switch (activeOrder.currentStep) {
                case BREAD_STEP: stepText = "빵 준비 중"; break;
                case CHEESE_STEP: stepText = "치즈 준비 중"; break;
                case EGG_STEP: stepText = "계란 준비 중"; break;
                case JAM_STEP: stepText = "잼 준비 중"; break;
                default: stepText = "알 수 없는 단계";
                }
            }
            else if (status == OrderStatus::COMPLETED) {
                activeOrder.isProcessing = false;

                switch (activeOrder.currentStep) {
                case BREAD_STEP:
                    activeOrder.currentStep = CHEESE_STEP;
                    stepText = "치즈 준비 대기 중";
                    break;
                case CHEESE_STEP:
                    activeOrder.currentStep = EGG_STEP;
                    stepText = "계란 준비 대기 중";
                    break;
                case EGG_STEP:
                    activeOrder.currentStep = JAM_STEP;
                    stepText = "잼 준비 대기 중";
                    break;
                case JAM_STEP:
                    activeOrder.currentStep = COMPLETE_STEP;
                    stepText = "주문 완료";
                    statusText = "완료";
                    break;
                default:
                    stepText = "알 수 없는 단계";
                }

                if (statusText.isEmpty()) {
                    statusText = "대기 중";
                }
            }

            if (!stepText.isEmpty()) {
                updateOrderStatusTable(orderId, stepText, statusText);
                appendLog(QString("주문 %1: %2 - %3").arg(orderId).arg(stepText).arg(statusText));
            }
            break;
        }
        default:
            qDebug() << "Unknown message type received:" << static_cast<int>(message.type);
            break;
        }
    } catch (const std::exception& e) {
        qDebug() << "Network message handling error:" << e.what();
        appendLog(QString("네트워크 메시지 처리 중 오류: %1").arg(e.what()));
    }
}

void OrderManagerGUI::handleNetworkError(const QString& error)
{
    updateNetworkStatus("오류: " + error, "red");
    startServerButton->setText("서버 시작");
    startServerButton->setEnabled(true);
    portSpinBox->setEnabled(true);
}

void OrderManagerGUI::handleNetworkConnection()
{
    appendLog("클라이언트가 연결되었습니다.");
    updateNetworkStatus("클라이언트 연결됨", "green");
}

void OrderManagerGUI::handleNetworkDisconnection()
{
    appendLog("클라이언트 연결이 끊어졌습니다.");
    updateNetworkStatus("클라이언트 연결 끊김", "orange");
}

void OrderManagerGUI::updateNetworkStatus(const QString& status, const QString& color)
{
    if (networkStatusLabel) {
        networkStatusLabel->setText(status);
        networkStatusLabel->setStyleSheet(QString("QLabel { color: %1; }").arg(color));
        appendLog(status);
    }
}

void OrderManagerGUI::updateOrderStatusTable(int orderId, const QString& step, const QString& status)
{
    // 테이블이 없으면 리턴
    if (!orderStatusTable) {
        qDebug() << "Order status table is null";
        return;
    }

    qDebug() << "Updating table for order:" << orderId << "Step:" << step << "Status:" << status;

    int row = -1;
    // 기존 행 찾기
    for (int i = 0; i < orderStatusTable->rowCount(); ++i) {
        QTableWidgetItem* idItem = orderStatusTable->item(i, 0);
        if (idItem && idItem->text().toInt() == orderId) {
            row = i;
            break;
        }
    }

    // 새 행 추가
    if (row == -1) {
        row = orderStatusTable->rowCount();
        orderStatusTable->insertRow(row);
    }

    // 주문 ID 설정 (새 행인 경우에만)
    if (!orderStatusTable->item(row, 0)) {
        QTableWidgetItem* idItem = new QTableWidgetItem(QString::number(orderId));
        idItem->setTextAlignment(Qt::AlignCenter);
        orderStatusTable->setItem(row, 0, idItem);
    }

    // 기존 아이템이 있으면 삭제
    if (orderStatusTable->item(row, 1)) {
        delete orderStatusTable->item(row, 1);
    }
    if (orderStatusTable->item(row, 2)) {
        delete orderStatusTable->item(row, 2);
    }

    // 새 아이템 생성 및 설정
    QTableWidgetItem* stepItem = new QTableWidgetItem(step);
    QTableWidgetItem* statusItem = new QTableWidgetItem(status);

    stepItem->setTextAlignment(Qt::AlignCenter);
    statusItem->setTextAlignment(Qt::AlignCenter);

    // 아이템 설정
    orderStatusTable->setItem(row, 1, stepItem);
    orderStatusTable->setItem(row, 2, statusItem);

    // 테이블 새로고침
    orderStatusTable->viewport()->update();
}

void OrderManagerGUI::resetOrderForm()
{
    try {
        // 빵 선택 초기화
        breadButtonGroup->setExclusive(false);
        ryeBreadRadioButton->setChecked(false);
        whiteBreadRadioButton->setChecked(false);
        breadButtonGroup->setExclusive(true);

        // 계란 선택 초기화
        eggButtonGroup->setExclusive(false);
        hardBoiledEggRadioButton->setChecked(false);
        softBoiledEggRadioButton->setChecked(false);
        eggButtonGroup->setExclusive(true);

        // 잼 선택 초기화
        strawberryJamCheckBox->setChecked(false);
        appleJamCheckBox->setChecked(false);
        jamAmountSlider->setValue(50);

        // 치즈 선택 초기화
        mozzarellaCheckBox->setChecked(false);
        cheddarCheckBox->setChecked(false);
    } catch (const std::exception& e) {
        qDebug() << "Form reset error:" << e.what();
    }
}

void OrderManagerGUI::appendLog(const QString& message)
{
    QString timestamp = QTime::currentTime().toString("hh:mm:ss");
    QString logMessage = QString("[%1] %2").arg(timestamp).arg(message);

    if (logTextEdit) {
        logTextEdit->append(logMessage);

        // 오류 메시지는 빨간색으로 표시
        if (message.startsWith("오류:") || message.startsWith("주문 오류:")) {
            QTextCursor cursor = logTextEdit->textCursor();
            cursor.movePosition(QTextCursor::End);
            cursor.movePosition(QTextCursor::StartOfBlock, QTextCursor::KeepAnchor);
            QTextCharFormat format;
            format.setForeground(Qt::red);
            cursor.mergeCharFormat(format);
        }

        // 성공 메시지는 녹색으로 표시
        if (message.contains("접수되었습니다")) {
            QTextCursor cursor = logTextEdit->textCursor();
            cursor.movePosition(QTextCursor::End);
            cursor.movePosition(QTextCursor::StartOfBlock, QTextCursor::KeepAnchor);
            QTextCharFormat format;
            format.setForeground(Qt::darkGreen);
            cursor.mergeCharFormat(format);
        }

        // 로그를 최신 내용이 보이도록 스크롤
        logTextEdit->verticalScrollBar()->setValue(
            logTextEdit->verticalScrollBar()->maximum()
            );
    }
}
