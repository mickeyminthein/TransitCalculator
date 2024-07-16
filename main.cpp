#include <QCoreApplication>
#include "transitcalculator.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    TransitCalculator *tranCalculator = new TransitCalculator();
    tranCalculator->computeDistances();
    return a.exec();
}
