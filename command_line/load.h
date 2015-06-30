#ifndef LOAD_H
#define LOAD_H

#include <QStringList>
#include "../ui/mainwindow.h"
#include <QTextStream>
#include <QApplication>

int bandageLoad(QApplication * a, QStringList arguments);
void printLoadUsage();
void voidPrintInvalidLoadOptions(QStringList invalidOptions);

#endif // LOAD_H
