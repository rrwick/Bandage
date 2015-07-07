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
    QTextStream out(stdout);
    QTextStream err(stdout);

    if (checkForHelp(arguments))
    {
        printLoadUsage(&out, false);
        return 0;
    }

    if (isOptionPresent("--helpall", &arguments))
    {
        printLoadUsage(&out, true);
        return 0;
    }

    if (arguments.size() == 0)
    {
        printLoadUsage(&err, false);
        return 1;
    }

    QString filename = arguments.at(0);
    arguments.pop_front();


    QString error = checkForInvalidLoadOptions(arguments);
    if (error.length() > 0)
    {
        err << "Bandage error: " << error << endl;
        return 1;
    }

    bool drawGraph;
    parseLoadOptions(arguments, &drawGraph);

    MainWindow w(filename, drawGraph);
    w.show();
    return a->exec();
}


void printLoadUsage(QTextStream * out, bool all)
{
    *out << endl;
    *out << "Usage:    Bandage load <graph>" << endl << endl;
    *out << "Options:  --draw              draw graph after loading" << endl;
    *out << endl;
    printCommonHelp(out);
    if (all)
        printSettingsUsage(out);
}



QString checkForInvalidLoadOptions(QStringList arguments)
{
    checkOptionWithoutValue("--draw", &arguments);

    QString error = checkForInvalidOrExcessSettings(&arguments);
    if (error.length() > 0) return error;

    return checkForInvalidOrExcessSettings(&arguments);
}



void parseLoadOptions(QStringList arguments, bool * drawGraph)
{
    int drawIndex = arguments.indexOf("--draw");
    *drawGraph = (drawIndex > -1);

    parseSettings(arguments);
}
