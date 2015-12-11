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
    arguments.pop_front();

    if (!checkIfFileExists(graphFilename))
    {
        err << "Bandage error: " << graphFilename << " does not exist." << endl;
        return 1;
    }

    QString error = checkForInvalidInfoOptions(arguments);
    if (error.length() > 0)
    {
        err << "Bandage error: " << error << endl;
        return 1;
    }

    bool tsv;
    parseInfoOptions(arguments, &tsv);

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
    int firstQuartile = 0;
    int median = 0;
    int thirdQuartile = 0;
    int longestNode = 0;
    g_assemblyGraph->getNodeStats(&n50, &shortestNode, &firstQuartile, &median, &thirdQuartile, &longestNode);

    int componentCount = 0;
    int largestComponentLength = 0;
    g_assemblyGraph->getGraphComponentCountAndLargestComponentSize(&componentCount, &largestComponentLength);

    double medianReadDepthByBase = g_assemblyGraph->getMedianReadDepthByBase();

    if (tsv)
    {
        out << graphFilename << "\t";
        out << nodeCount << "\t";
        out << edgeCount << "\t";
        out << totalLength << "\t";
        out << deadEnds << "\t";
        out << percentageDeadEnds << "%\t";
        out << componentCount << "\t";
        out << largestComponentLength << "\t";
        out << n50 << "\t";
        out << shortestNode << "\t";
        out << firstQuartile << "\t";
        out << median << "\t";
        out << thirdQuartile << "\t";
        out << longestNode << "\t";
        out << medianReadDepthByBase << "\n";
    }
    else
    {
        out << "Node count:               " << nodeCount << "\n";
        out << "Edge count:               " << edgeCount << "\n";
        out << "Total length (bp):        " << totalLength << "\n";
        out << "Dead ends:                " << deadEnds << "\n";
        out << "Percentage dead ends:     " << percentageDeadEnds << "%\n";
        out << "Connected components:     " << componentCount << "\n";
        out << "Largest component (bp):   " << largestComponentLength << "\n";
        out << "N50 (bp):                 " << n50 << "\n";
        out << "Shortest node (bp):       " << shortestNode << "\n";
        out << "Lower quartile node (bp): " << firstQuartile << "\n";
        out << "Median node (bp):         " << median << "\n";
        out << "Upper quartile node (bp): " << thirdQuartile << "\n";
        out << "Longest node (bp):        " << longestNode << "\n";
        out << "Median read depth:        " << medianReadDepthByBase << "\n";
    }

    return 0;
}



void printInfoUsage(QTextStream * out, bool all)
{
    *out << endl;
    *out << "Bandage info takes a graph file as input and outputs the following statistics" << endl;
    *out << "about the graph:" << endl;
    *out << "  * Node count: The number of nodes in the graph. Only positive nodes are" << endl;
    *out << "        counted (i.e. each complementary pair counts as one)." << endl;
    *out << "  * Edge count: The number of edges in the graph. Only one edge in each" << endl;
    *out << "        complementary pair is counted." << endl;
    *out << "  * Total length: The total number of base pairs in the graph." << endl;
    *out << "  * Dead ends: The number of instances where an end of a node does not connect" << endl;
    *out << "        to any other nodes." << endl;
    *out << "  * Percentage dead ends: The proportion of possible dead ends. The maximum" << endl;
    *out << "        number of dead ends is twice the number of nodes (occurs when there are" << endl;
    *out << "        no edges), so this value is the number of dead ends divided by twice the" << endl;
    *out << "        node count." << endl;
    *out << "  * Connected components: The number of regions of the graph which are" << endl;
    *out << "        disconnected from each other." << endl;
    *out << "  * Largest component: The total number of base pairs in the largest connected" << endl;
    *out << "        component." << endl;
    *out << "  * N50: Nodes that are this length or greater will collectively add up to at" << endl;
    *out << "        least half of the total length." << endl;
    *out << "  * Shortest node: The length of the shortest node in the graph." << endl;
    *out << "  * Lower quartile node: The median node length for the shorter half of the" << endl;
    *out << "        nodes." << endl;
    *out << "  * Median node: The median node length for the graph." << endl;
    *out << "  * Upper quartile node: The median node length for the longer half of the" << endl;
    *out << "        nodes." << endl;
    *out << "  * Longest node: The length of the longest node in the graph." << endl;
    *out << "  * Median read depth: The median read depth of the graph, by base." << endl;
    *out << endl;
    *out << "Usage:    Bandage info <graph> [options]" << endl;
    *out << endl;
    *out << "Positional parameters:" << endl;
    *out << "          <graph>             A graph file of any type supported by Bandage" << endl;
    *out << endl;
    *out << "Options:  --tsv               Output the information in a single tab-delimited" << endl;
    *out << "                              line starting with the graph file" << endl;
    *out << endl;
    printCommonHelp(out);
    if (all)
        printSettingsUsage(out);
}



QString checkForInvalidInfoOptions(QStringList arguments)
{
    checkOptionWithoutValue("--tsv", &arguments);

    QString error = checkForInvalidOrExcessSettings(&arguments);
    if (error.length() > 0) return error;

    return checkForInvalidOrExcessSettings(&arguments);
}



void parseInfoOptions(QStringList arguments, bool * tsv)
{
    int tsvIndex = arguments.indexOf("--tsv");
    *tsv = (tsvIndex > -1);
}
