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


#include "load.h"
#include "commoncommandlinefunctions.h"

int bandageLoad(QApplication * a, QStringList arguments)
{
    if (checkForHelp(arguments))
    {
        printLoadUsage();
        return 0;
    }

    if (arguments.size() == 0)
    {
        printLoadUsage();
        return 1;
    }

    QString filename = arguments.at(0);
    arguments.pop_front();


    QString error = checkForInvalidLoadOptions(arguments);
    if (error.length() > 0)
    {
        QTextStream(stdout) << "" << endl << "Error: " << error << endl;
        printLoadUsage();
        return 1;
    }

    bool drawGraph;
    parseLoadOptions(arguments, &drawGraph);

    MainWindow w(filename, drawGraph);
    w.show();
    return a->exec();
}


void printLoadUsage()
{
    QTextStream(stdout) << "" << endl;
    QTextStream(stdout) << "Usage:   Bandage load <graph>" << endl << endl;
    QTextStream(stdout) << "Options: -d    draw graph after loading" << endl << endl;
}



QString checkForInvalidLoadOptions(QStringList arguments)
{
    checkOptionWithoutValue("-d", &arguments);

    return checkForExcessArguments(arguments);
}



void parseLoadOptions(QStringList arguments, bool * drawGraph)
{
    int drawIndex = arguments.indexOf("-d");
    *drawGraph = (drawIndex > -1);
}
