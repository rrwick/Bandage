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
    QTextStream err(stderr);

    if (checkForHelp(arguments))
    {
        printLoadUsage(&out, false);
        return 0;
    }

    if (checkForHelpAll(arguments))
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

    if (!checkIfFileExists(filename))
    {
        err << "Bandage error: " << filename << " does not exist" << endl;
        return 1;
    }

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
    *out << "Bandage load will open the Bandage GUI and immediately load the specified graph" << endl;
    *out << "file." << endl;
    *out << endl;
    *out << "Usage:    Bandage load <graph> [options]" << endl;
    *out << endl;
    *out << "Positional parameters:" << endl;
    *out << "          <graph>             A graph file of any type supported by Bandage" << endl;
    *out << endl;
    *out << "Options:  --draw              Draw graph after loading" << endl;
    *out << endl;
    printCommonHelp(out);
    if (all)
        printSettingsUsage(out);
    *out << "Online Bandage help: https://github.com/rrwick/Bandage/wiki" << endl;
    *out << endl;
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
