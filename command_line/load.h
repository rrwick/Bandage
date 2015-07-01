#ifndef LOAD_H
#define LOAD_H

#include <QStringList>
#include "../ui/mainwindow.h"
#include <QTextStream>
#include <QApplication>

int bandageLoad(QApplication * a, QStringList arguments);
void printLoadUsage();
QString checkForInvalidLoadOptions(QStringList arguments);
void parseLoadOptions(QStringList arguments, bool *drawGraph);

#endif // LOAD_H
