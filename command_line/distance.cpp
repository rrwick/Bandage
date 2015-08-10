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
    *out << "Bandage distance takes two queries as input and will output (to stdout) the" << endl;
    *out << "possible orientations and distances between them in the graph." << endl;
    *out << endl;
    *out << "Usage:    Bandage distance <graph> <query1> <query2> [options]" << endl;
    *out << endl;
    *out << "Options:  --allquerypaths     Use all possible query paths in the graph for the" << endl;
    *out << "                              distance search. If this option is not used," << endl;
    *out << "                              Bandage will use only the single best path for" << endl;
    *out << "                              each query." << endl;
    *out << "          --onlyshortest      Only output the shortest path in the graph as a" << endl;
    *out << "                              single number (default: output all paths, sorted" << endl;
    *out << "                              from shortest to longest, in CSV format)" << endl;
    *out << "          --showpaths         Include graph paths in the CSV output (not" << endl;
    *out << "                              compatible with --onlyshortest option, default:" << endl;
    *out << "                              no paths in output)" << endl;
    *out << endl;
    *out << "          The following options are available in the full set set of Bandage" << endl;
    *out << "          command line options (viewable via --helpall) but are particularly" << endl;
    *out << "          relevant for a 'distance between queries' search." << endl;
    *out << endl;
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
    *out << "          --distnodes <int>   Maximum number of nodes allowed in a 'distance" << endl;
    *out << "                              between queries' search (default: " + QString::number(g_settings->distancePathSearchDepth + 1) + ")" << endl;
    *out << "          --distmin <int>     Minimum path distance allowed in a 'distance" << endl;
    *out << "                              between queries' search (default: " + QString::number(g_settings->minDistancePathLength) + ")" << endl;
    *out << "          --distmax <int>     Maximum path distance allowed in a 'distance" << endl;
    *out << "                              between queries' search (default: " + QString::number(g_settings->maxDistancePathLength) + ")" << endl;
    *out << "          --distor1           Allow 1-> 2-> query orientation" << endl;
    *out << "          --distor2           Allow 2-> 1-> query orientation" << endl;
    *out << "          --distor3           Allow 1-> <-2 query orientation" << endl;
    *out << "          --distor4           Allow <-1 2-> query orientation" << endl;
    *out << "                              (default: if none of the above four options are" << endl;
    *out << "                              used, then all four orientations are allowed)" << endl;
}



QString checkForInvalidDistanceOptions(QStringList arguments)
{
    return checkForExcessArguments(arguments);
}



void parseDistanceOptions(QStringList /*arguments*/)
{


}


