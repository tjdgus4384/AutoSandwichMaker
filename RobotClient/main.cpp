#include "robotcontrolgui.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    RobotControlGUI w;
    w.show();
    return a.exec();
}
