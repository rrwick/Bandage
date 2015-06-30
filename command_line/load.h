#ifndef LOAD_H
#define LOAD_H

#include <QStringList>
#include "../ui/mainwindow.h"
#include <QTextStream>
#include <QApplication>

int launchBandageAndLoadFile(QApplication * a, QStringList arguments);
void printLoadUsage();

#endif // LOAD_H
