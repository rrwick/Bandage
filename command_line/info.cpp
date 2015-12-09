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


#include "info.h"
#include "commoncommandlinefunctions.h"
#include "../graph/assemblygraph.h"



int bandageInfo(QStringList arguments)
{
    QTextStream out(stdout);
    QTextStream err(stderr);

    if (checkForHelp(arguments))
    {
        printInfoUsage(&out, false);
        return 0;
    }

    if (checkForHelpAll(arguments))
    {
        printInfoUsage(&out, true);
        return 0;
    }

    if (arguments.size() < 1)
    {
        printInfoUsage(&err, false);
        return 1;
    }

    QString graphFilename = arguments.at(0);
    if (!checkIfFileExists(graphFilename))
    {
        err << "Bandage error: " << graphFilename << " does not exist." << endl;
        return 1;
    }

    bool loadSuccess = g_assemblyGraph->loadGraphFromFile(graphFilename);
    if (!loadSuccess)
    {
        err << "Bandage error: could not load " << graphFilename << endl;
        return 1;
    }

    int nodeCount = g_assemblyGraph->m_nodeCount;
    int edgeCount = g_assemblyGraph->m_edgeCount;
    int totalLength = g_assemblyGraph->m_totalLength;
    int deadEnds = g_assemblyGraph->getDeadEndCount();
    double percentageDeadEnds = 100.0 * double(deadEnds) / (2 * nodeCount);

    int n50 = 0;
    int shortestNode = 0;
    double firstQuartile = 0.0;
    double median = 0.0;
    double thirdQuartile = 0.0;
    int longestNode = 0;
    g_assemblyGraph->getNodeStats(&n50, &shortestNode, &firstQuartile, &median, &thirdQuartile, &longestNode);

    out << "Node count:               " << nodeCount << "\n";
    out << "Edge count:               " << edgeCount << "\n";
    out << "Dead ends:                " << deadEnds << "\n";
    out << "Percentage dead ends:     " << percentageDeadEnds << "%\n";
    out << "Total length (bp):        " << totalLength << "\n";
    out << "N50 node (bp):            " << n50 << "\n";
    out << "Shortest node (bp):       " << shortestNode << "\n";
    out << "Lower quartile node (bp): " << firstQuartile << "\n";
    out << "Median node (bp):         " << median << "\n";
    out << "Upper quartile node (bp): " << thirdQuartile << "\n";
    out << "Longest node (bp):        " << longestNode << "\n";

    return 0;
}



void printInfoUsage(QTextStream * out, bool all)
{
    *out << endl;
    *out << "Bandage info takes a graph file as input and outputs some statistics about the" << endl;
    *out << "graph." << endl;
    *out << endl;
    *out << "Usage:    Bandage info <graphfile>" << endl;
    *out << endl;
    printCommonHelp(out);
    if (all)
        printSettingsUsage(out);
}
