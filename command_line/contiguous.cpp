//Copyright 2016 Ryan Wick

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
#include "commoncommandlinefunctions.h"
#include "../program/settings.h"
#include "../graph/assemblygraph.h"

int bandageContiguous(QStringList arguments)
{
    QTextStream out(stdout);
    QTextStream err(stderr);

    if (checkForHelp(arguments))
    {
        printContiguousUsage(&out, false);
        return 0;
    }

    if (checkForHelpAll(arguments))
    {
        printContiguousUsage(&out, true);
        return 0;
    }

    if (arguments.size() < 2)
    {
        printContiguousUsage(&err, false);
        return 1;
    }

    QString graphFile = arguments.at(0);
    arguments.pop_front();

    QString targetFile = arguments.at(0);
    arguments.pop_front();

    QString error = checkForInvalidContiguousOptions(arguments);
    if (error.length() > 0)
    {
        err << "" << endl << "Error: " << error << endl;
        printContiguousUsage(&err, false);
        return 1;
    }

    parseContiguousOptions(arguments);

    bool loadSuccess = g_assemblyGraph->loadGraphFromFile(graphFile);
    if (!loadSuccess)
        return 1;

    return 0;
}



void printContiguousUsage(QTextStream * out, bool all)
{
    QStringList text;

    text << "";
    text << "Usage:   Bandage contiguous <graph> <target> [options]";
    text << "";
    text << "Options:";
    text << "";

    getCommonHelp(&text);
    if (all)
        getSettingsUsage(&text);
    getOnlineHelpMessage(&text);

    outputText(text, out);
}



QString checkForInvalidContiguousOptions(QStringList arguments)
{
    return checkForExcessArguments(arguments);
}



void parseContiguousOptions(QStringList /*arguments*/)
{


}

