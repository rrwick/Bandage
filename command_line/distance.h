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


#ifndef DISTANCE_H
#define DISTANCE_H


#include <QStringList>
#include "../ui/mainwindow.h"
#include <QTextStream>
#include <QApplication>

int bandageDistance(QStringList arguments);
void printDistanceUsage(QTextStream * out);
QString checkForInvalidDistanceOptions(QStringList arguments);
void parseDistanceOptions(QStringList arguments, bool * allQueryPaths,
                          bool * onlyShortest, bool * showPaths);


#endif // DISTANCE_H
