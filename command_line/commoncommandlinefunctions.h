#ifndef COMMANDCOMMANDLINEFUNCTIONS_H
#define COMMANDCOMMANDLINEFUNCTIONS_H

#include "../program/globals.h"
#include <QString>

bool loadAssemblyGraph(QString filename);
void layoutGraph();
QString checkOptionForInt(QString option, QStringList * arguments, int min, int max);
void checkOptionWithoutValue(QString option, QStringList * arguments);
QString checkForExcessArguments(QStringList arguments);

#endif // COMMANDCOMMANDLINEFUNCTIONS_H
