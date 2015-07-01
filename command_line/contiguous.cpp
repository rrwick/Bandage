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


#include "contiguous.h"
#include "command_line/commoncommandlinefunctions.h"
#include "../program/settings.h"

int bandageContiguous(QStringList arguments)
{
    if (checkForHelp(arguments))
    {
        printContiguousUsage();
        return 0;
    }

    if (arguments.size() < 2)
    {
        printContiguousUsage();
        return 1;
    }

    QString graphFile = arguments.at(0);
    arguments.pop_front();

    QString targetFile = arguments.at(0);
    arguments.pop_front();

    QString error = checkForInvalidContiguousOptions(arguments);
    if (error.length() > 0)
    {
        QTextStream(stdout) << "" << endl << "Error: " << error << endl;
        printContiguousUsage();
        return 1;
    }

    parseContiguousOptions(arguments);

    g_settings = new Settings();

    bool loadSuccess = loadAssemblyGraph(graphFile);
    if (!loadSuccess)
        return 1;









    return 0;
}



void printContiguousUsage()
{
    QTextStream(stdout) << "" << endl;
    QTextStream(stdout) << "Usage:   Bandage contiguous <graph> <target> [options]" << endl << endl;
    QTextStream(stdout) << "Options: " << endl << endl;
}



QString checkForInvalidContiguousOptions(QStringList arguments)
{
    return checkForExcessArguments(arguments);
}



void parseContiguousOptions(QStringList arguments)
{


}

