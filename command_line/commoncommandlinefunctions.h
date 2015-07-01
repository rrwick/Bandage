#ifndef COMMANDCOMMANDLINEFUNCTIONS_H
#define COMMANDCOMMANDLINEFUNCTIONS_H

#include "../program/globals.h"
#include <QString>

bool loadAssemblyGraph(QString filename);
void layoutGraph();
QString checkOptionForInt(QString option, QStringList * arguments, int min, int max);

#endif // COMMANDCOMMANDLINEFUNCTIONS_H
