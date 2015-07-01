#ifndef IMAGE_H
#define IMAGE_H


#include <QStringList>
#include "../ui/mainwindow.h"
#include <QTextStream>
#include <QApplication>

int bandageImage(QStringList arguments);
void printImageUsage();
QString checkForInvalidOptions(QStringList arguments);
void parseOptions(QStringList arguments, int * width, int * height);

#endif // IMAGE_H
