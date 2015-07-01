#ifndef CONTIGUOUS_H
#define CONTIGUOUS_H

#include <QStringList>
#include "../ui/mainwindow.h"
#include <QTextStream>
#include <QApplication>

int bandageContiguous(QStringList arguments);
void printContiguousUsage();
QString checkForInvalidContiguousOptions(QStringList arguments);
void parseContiguousOptions(QStringList arguments);

#endif // CONTIGUOUS_H
