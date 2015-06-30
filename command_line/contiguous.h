#ifndef CONTIGUOUS_H
#define CONTIGUOUS_H

#include <QStringList>
#include "../ui/mainwindow.h"
#include <QTextStream>
#include <QApplication>

int bandageContiguous(QApplication * a, QStringList arguments);
void printContiguousUsage();
void voidPrintInvalidContiguousOptions(QStringList invalidOptions);

#endif // CONTIGUOUS_H
