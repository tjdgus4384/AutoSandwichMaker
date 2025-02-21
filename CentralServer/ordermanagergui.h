// ordermanagergui.h
#ifndef ORDERMANAGERGUI_H
#define ORDERMANAGERGUI_H

#include <QMainWindow>
#include <QPushButton>
#include <QRadioButton>
#include <QCheckBox>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSlider>
#include <QTextEdit>
#include <QTableWidget>
#include <QButtonGroup>
#include <QHeaderView>
#include <QScrollBar>
#include <QSpinBox>
#include <QTime>
#include <QMessageBox>
#include <QDebug>
#include "networkmanager.h"
#include "message.h"

class OrderManagerGUI : public QMainWindow
{
    Q_OBJECT

public:
    explicit OrderManagerGUI(QWidget *parent = nullptr);
    ~OrderManagerGUI() override;
    QString validateOrder();

private slots:
    void onStartServerClicked();
    void onOrderSubmit();
    void handleNetworkMessage(const Message& message);
    void handleNetworkError(const QString& error);
    void handleNetworkConnection();
    void handleNetworkDisconnection();

private:
    // GUI 요소
    QWidget *centralWidget;
    QVBoxLayout *mainLayout;

    // 서버 제어
    QSpinBox *portSpinBox;
    QPushButton *startServerButton;
    QLabel *networkStatusLabel;

    // 주문 입력
    QLabel *breadLabel;
    QRadioButton *ryeBreadRadioButton;
    QRadioButton *whiteBreadRadioButton;
    QButtonGroup *breadButtonGroup;

    QLabel *eggLabel;
    QRadioButton *hardBoiledEggRadioButton;
    QRadioButton *softBoiledEggRadioButton;
    QButtonGroup *eggButtonGroup;

    QLabel *jamLabel;
    QCheckBox *strawberryJamCheckBox;
    QCheckBox *appleJamCheckBox;
    QLabel *jamAmountLabel;
    QSlider *jamAmountSlider;

    QLabel *cheeseLabel;
    QCheckBox *mozzarellaCheckBox;
    QCheckBox *cheddarCheckBox;

    QPushButton *submitButton;

    // 주문 상태 및 로그
    QLabel *statusLabel;
    QTableWidget *orderStatusTable;
    QLabel *logLabel;
    QTextEdit *logTextEdit;

    // 네트워크 매니저
    NetworkManager *networkManager;

    // 주문 관리
    int nextOrderId;
    enum OrderStep {
        BREAD_STEP = 0,
        CHEESE_STEP = 1,
        EGG_STEP = 2,
        JAM_STEP = 3,
        COMPLETE_STEP = 4
    };

    struct ActiveOrder {
        OrderMessage order;
        OrderStep currentStep;
        bool isProcessing;
        QString status;
    };

    QMap<int, ActiveOrder> activeOrders;

    // 초기화 함수
    void initializeGUI();
    void setupServerControl();
    void setupOrderInputs();
    void setupOrderStatusTable();
    void setupNetworkConnections();

    // 유틸리티 함수
    void updateNetworkStatus(const QString& status, const QString& color);
    void updateOrderStatusTable(int orderId, const QString& status, const QString& details = "");
    void appendLog(const QString& message);
    void resetOrderForm();
};

#endif
