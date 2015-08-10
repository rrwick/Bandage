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


#include "distance.h"
#include "commoncommandlinefunctions.h"
#include "../program/settings.h"
#include "../graph/assemblygraph.h"

int bandageDistance(QStringList arguments)
{
    QTextStream out(stdout);
    QTextStream err(stdout);

    if (checkForHelp(arguments))
    {
        printDistanceUsage(&out);
        return 0;
    }

    if (arguments.size() < 2)
    {
        printDistanceUsage(&err);
        return 1;
    }

    QString graphFile = arguments.at(0);
    arguments.pop_front();

    QString targetFile = arguments.at(0);
    arguments.pop_front();

    QString error = checkForInvalidDistanceOptions(arguments);
    if (error.length() > 0)
    {
        err << "" << endl << "Error: " << error << endl;
        printDistanceUsage(&err);
        return 1;
    }

    parseDistanceOptions(arguments);

    bool loadSuccess = g_assemblyGraph->loadGraphFromFile(graphFile);
    if (!loadSuccess)
        return 1;

    return 0;
}



void printDistanceUsage(QTextStream * out)
{
    *out << endl;
    *out << "Usage:    Bandage distance <graph> <query1> <query2> [options]" << endl << endl;
    *out << "Options:  BLAST query paths" << endl;
    *out << "          ---------------------------------------------------------------------" << endl;
    *out << "          These settings control how Bandage searches for query paths after" << endl;
    *out << "          conducting a BLAST search." << endl;
    *out << "          --reqcov <float>    Required fraction of a BLAST query which must be" << endl;
    *out << "                              covered by a graph path (0.5 to 1.0, default:" << endl;
    *out << "                              " + QString::number(g_settings->queryRequiredCoverage) + ")" << endl;
    *out << "          --lendis <float>    Maximum allowed relative length discrepancy" << endl;
    *out << "                              between a BLAST query and its path in the graph" << endl;
    *out << "                              (0.0 to 0.5, default: " << QString::number(g_settings->queryAllowedLengthDiscrepancy) + ")" << endl;
    *out << "          --pathnodes <int>   The number of allowed nodes in a BLAST query path" << endl;
    *out << "                              (1 to 50, default: " << QString::number(g_settings->maxQueryPathNodes) + ")" << endl;
    *out << "          --maxpaths <int>    The number of BLAST query paths displayed to the" << endl;
    *out << "                              user (1 to 100, default: " << QString::number(g_settings->maxQueryPaths) + ")" << endl;
}



QString checkForInvalidDistanceOptions(QStringList arguments)
{
    return checkForExcessArguments(arguments);
}



void parseDistanceOptions(QStringList /*arguments*/)
{


}


