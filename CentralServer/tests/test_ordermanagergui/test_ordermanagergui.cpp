#include <QtTest/QtTest>
#include "ordermanagergui.h"
#include <QApplication>
#include <QTextBlock>
#include <QRadioButton>
#include <QCheckBox>
#include <QSlider>
#include <QPushButton>
#include <QTableWidget>
#include <QTextEdit>
#include <QSpinBox>

// OrderManagerGUI 테스트 클래스
class TestOrderManagerGUI : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void testValidateOrder_NoSelection();
    void testValidateOrder_OnlyBreadSelected();
    void testValidateOrder_FullSelection();
    void testOnOrderSubmit_ServerNotRunning();
    void testOnOrderSubmit_ValidOrder();

private:
    QApplication *m_app;
    OrderManagerGUI *m_gui;
};

void TestOrderManagerGUI::initTestCase()
{
    int argc = 0;
    char **argv = nullptr;
    m_app = new QApplication(argc, argv);
    m_gui = new OrderManagerGUI();
    m_gui->show();
}

void TestOrderManagerGUI::cleanupTestCase()
{
    delete m_gui;
    delete m_app;
}

void TestOrderManagerGUI::testValidateOrder_NoSelection()
{
    // 모든 라디오버튼 및 체크박스 해제된 상태에서 validateOrder() 결과 확인
    QString result = m_gui->validateOrder();
    QVERIFY(!result.isEmpty());
    QVERIFY(result.contains("빵 종류"));
    QVERIFY(result.contains("계란 종류"));
}

void TestOrderManagerGUI::testValidateOrder_OnlyBreadSelected()
{
    // 빵 선택만 하고 계란은 선택하지 않은 상태
    // 빵 선택 시: 호밀빵 라디오버튼 체크
    QRadioButton *ryeBread = m_gui->findChild<QRadioButton*>("", Qt::FindDirectChildrenOnly);
    // 위 코드처럼 이름으로 찾기 어려울 경우, 실제 코드에서 objectName을 설정해두는 것을 권장.
    // 여기서는 예시로 assume: ryeBreadRadioButton, hardBoiledEggRadioButton 등에게 objectName을 할당했다고 가정하겠습니다.
    QRadioButton *ryeBreadRB = m_gui->findChild<QRadioButton*>("ryeBreadRadioButton");
    QRadioButton *whiteBreadRB = m_gui->findChild<QRadioButton*>("whiteBreadRadioButton");
    QRadioButton *hardEggRB = m_gui->findChild<QRadioButton*>("hardBoiledEggRadioButton");
    QRadioButton *softEggRB = m_gui->findChild<QRadioButton*>("softBoiledEggRadioButton");

    QVERIFY(ryeBreadRB);
    QVERIFY(whiteBreadRB);
    QVERIFY(hardEggRB);
    QVERIFY(softEggRB);

    ryeBreadRB->setChecked(true);
    hardEggRB->setChecked(false);
    softEggRB->setChecked(false);

    QString result = m_gui->validateOrder();
    QVERIFY(!result.isEmpty());
    QVERIFY(!result.contains("빵 종류"));    // 빵은 선택됨
    QVERIFY(result.contains("계란 종류")); // 계란 종류 미선택
}

void TestOrderManagerGUI::testValidateOrder_FullSelection()
{
    QRadioButton *ryeBreadRB = m_gui->findChild<QRadioButton*>("ryeBreadRadioButton");
    QRadioButton *hardEggRB = m_gui->findChild<QRadioButton*>("hardBoiledEggRadioButton");

    QVERIFY(ryeBreadRB);
    QVERIFY(hardEggRB);

    ryeBreadRB->setChecked(true);
    hardEggRB->setChecked(true);

    QString result = m_gui->validateOrder();
    QVERIFY(result.isEmpty());  // 모든 필수 항목 선택 시 에러 없음
}

void TestOrderManagerGUI::testOnOrderSubmit_ServerNotRunning()
{
    // 서버를 시작하지 않은 상태에서 onOrderSubmit 호출 시 로그에 오류가 남는지 확인
    // 우선 UI 설정
    QRadioButton *ryeBreadRB = m_gui->findChild<QRadioButton*>("ryeBreadRadioButton");
    QRadioButton *hardEggRB = m_gui->findChild<QRadioButton*>("hardBoiledEggRadioButton");

    ryeBreadRB->setChecked(true);
    hardEggRB->setChecked(true);

    // 주문 제출 버튼 클릭 시도
    QPushButton *submitBtn = m_gui->findChild<QPushButton*>("submitButton");
    QVERIFY(submitBtn);

    // 로그 창 초기 상태 확인
    QTextEdit *logText = m_gui->findChild<QTextEdit*>("logTextEdit");
    QVERIFY(logText);

    int initialLogLines = logText->document()->lineCount();

    QTest::mouseClick(submitBtn, Qt::LeftButton);

    // 서버가 켜져있지 않으므로 로그에 에러 메시지가 추가되었는지 확인
    QTRY_VERIFY(logText->document()->lineCount() > initialLogLines);
    QString lastLogLine = logText->document()->findBlockByNumber(logText->document()->lineCount()-1).text();
    QVERIFY(lastLogLine.contains("서버가 실행되고 있지 않습니다"));
}

void TestOrderManagerGUI::testOnOrderSubmit_ValidOrder()
{
    // 이번에는 서버를 켠 뒤 주문이 정상적으로 테이블에 추가되는지 확인.
    QPushButton *startServerBtn = m_gui->findChild<QPushButton*>("startServerButton");
    QSpinBox *portBox = m_gui->findChild<QSpinBox*>("portSpinBox");
    QVERIFY(startServerBtn);
    QVERIFY(portBox);

    // 서버 시작
    QTest::mouseClick(startServerBtn, Qt::LeftButton);
    // 서버 시작 후 상태가 변경되는데 약간의 대기 필요
    QTest::qWait(1000);

    // 필수항목 선택
    QRadioButton *ryeBreadRB = m_gui->findChild<QRadioButton*>("ryeBreadRadioButton");
    QRadioButton *hardEggRB = m_gui->findChild<QRadioButton*>("hardBoiledEggRadioButton");
    QCheckBox *strawberryJamCB = m_gui->findChild<QCheckBox*>("strawberryJamCheckBox");
    QCheckBox *mozzarellaCB = m_gui->findChild<QCheckBox*>("mozzarellaCheckBox");

    ryeBreadRB->setChecked(true);
    hardEggRB->setChecked(true);
    strawberryJamCB->setChecked(true);
    mozzarellaCB->setChecked(true);

    QTableWidget *orderTable = m_gui->findChild<QTableWidget*>("orderStatusTable");
    QVERIFY(orderTable);
    int initialRowCount = orderTable->rowCount();

    QPushButton *submitBtn = m_gui->findChild<QPushButton*>("submitButton");
    QTest::mouseClick(submitBtn, Qt::LeftButton);

    // 주문이 정상적으로 테이블에 추가되는지 확인
    QTRY_VERIFY(orderTable->rowCount() == initialRowCount + 1);
    QTableWidgetItem *idItem = orderTable->item(orderTable->rowCount()-1, 0);
    QVERIFY(idItem);
    QVERIFY(idItem->text().toInt() > 0);
}

QTEST_MAIN(TestOrderManagerGUI)
#include "test_ordermanagergui.moc"