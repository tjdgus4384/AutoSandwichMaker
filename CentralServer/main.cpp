// main.cpp
#include <QApplication>
#include "ordermanagergui.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    OrderManagerGUI w;
    w.show();

    return a.exec();
}
