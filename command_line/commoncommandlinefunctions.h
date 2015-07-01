//Copyright 2015 Ryan Wick

//This file is part of Bandage

//Bandage is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//Bandage is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

//You should have received a copy of the GNU General Public License
//along with Bandage.  If not, see <http://www.gnu.org/licenses/>.


#ifndef COMMANDCOMMANDLINEFUNCTIONS_H
#define COMMANDCOMMANDLINEFUNCTIONS_H

#include "../program/globals.h"
#include <QString>

bool loadAssemblyGraph(QString filename);
void layoutGraph();
bool checkForHelp(QStringList arguments);
QString checkOptionForInt(QString option, QStringList * arguments, int min, int max);
void checkOptionWithoutValue(QString option, QStringList * arguments);
QString checkForExcessArguments(QStringList arguments);

#endif // COMMANDCOMMANDLINEFUNCTIONS_H
