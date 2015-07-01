#ifndef IMAGE_H
#define IMAGE_H


#include <QStringList>
#include "../ui/mainwindow.h"
#include <QTextStream>
#include <QApplication>

int bandageImage(QStringList arguments);
void printImageUsage();
void voidPrintInvalidImageOptions(QStringList invalidOptions);

#endif // IMAGE_H
