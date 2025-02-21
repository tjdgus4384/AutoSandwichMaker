// Order.h

#ifndef ORDER_H
#define ORDER_H

#include <QString>
#include <QStringList>

struct Order {
    int id;
    QString bread;
    QString egg;
    QStringList jams;
    int jamAmount;
    QStringList cheeses;
    int currentStep;
    bool isProcessing;
    QString status;
};

#endif // ORDER_H
