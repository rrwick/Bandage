//Copyright 2017 Ryan Wick

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
#include <QPair>



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
        outputText("Bandage error: " + graphFilename + " does not exist.", &err);
        return 1;
    }

    QString error = checkForInvalidInfoOptions(arguments);
    if (error.length() > 0)
    {
        outputText("Bandage error: " + error, &err);
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
    QPair<int, int> overlapRange = g_assemblyGraph->getOverlapRange();
    int smallestOverlap = overlapRange.first;
    int largestOverlap = overlapRange.second;
    int totalLength = g_assemblyGraph->m_totalLength;
    int totalLengthNoOverlaps = g_assemblyGraph->getTotalLengthMinusEdgeOverlaps();
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
    long long totalLengthOrphanedNodes = g_assemblyGraph->getTotalLengthOrphanedNodes();

    double medianDepthByBase = g_assemblyGraph->getMedianDepthByBase();
    long long estimatedSequenceLength = g_assemblyGraph->getEstimatedSequenceLength(medianDepthByBase);

    if (tsv)
    {
        out << graphFilename << "\t";
        out << nodeCount << "\t";
        out << edgeCount << "\t";
        out << smallestOverlap << "\t";
        out << largestOverlap << "\t";
        out << totalLength << "\t";
        out << totalLengthNoOverlaps << "\t";
        out << deadEnds << "\t";
        out << percentageDeadEnds << "%\t";
        out << componentCount << "\t";
        out << largestComponentLength << "\t";
        out << totalLengthOrphanedNodes << "\t";
        out << n50 << "\t";
        out << shortestNode << "\t";
        out << firstQuartile << "\t";
        out << median << "\t";
        out << thirdQuartile << "\t";
        out << longestNode << "\t";
        out << medianDepthByBase << "\t";
        out << estimatedSequenceLength << "\n";
    }
    else
    {
        out << "Node count:                       " << nodeCount << "\n";
        out << "Edge count:                       " << edgeCount << "\n";
        out << "Smallest edge overlap (bp):       " << smallestOverlap << "\n";
        out << "Largest edge overlap (bp):        " << largestOverlap << "\n";
        out << "Total length (bp):                " << totalLength << "\n";
        out << "Total length no overlaps (bp):    " << totalLengthNoOverlaps << "\n";
        out << "Dead ends:                        " << deadEnds << "\n";
        out << "Percentage dead ends:             " << percentageDeadEnds << "%\n";
        out << "Connected components:             " << componentCount << "\n";
        out << "Largest component (bp):           " << largestComponentLength << "\n";
        out << "Total length orphaned nodes (bp): " << largestComponentLength << "\n";
        out << "N50 (bp):                         " << n50 << "\n";
        out << "Shortest node (bp):               " << shortestNode << "\n";
        out << "Lower quartile node (bp):         " << firstQuartile << "\n";
        out << "Median node (bp):                 " << median << "\n";
        out << "Upper quartile node (bp):         " << thirdQuartile << "\n";
        out << "Longest node (bp):                " << longestNode << "\n";
        out << "Median depth:                     " << medianDepthByBase << "\n";
        out << "Estimated sequence length (bp):   " << estimatedSequenceLength << "\n";
    }

    return 0;
}



void printInfoUsage(QTextStream * out, bool all)
{
    QStringList text;

    text << "Bandage info takes a graph file as input and outputs (to stdout) the following statistics about the graph:";
    text << "* Node count: The number of nodes in the graph. Only positive nodes are counted (i.e. each complementary pair counts as one).";
    text << "* Edge count: The number of edges in the graph. Only one edge in each complementary pair is counted.";
    text << "* Smallest edge overlap: The smallest overlap size (in bp) for the edges in the graph.";
    text << "* Largest edge overlap: The smallest overlap size (in bp) for the edges in the graph. For most graphs this will be the same as the smallest edge overlap (i.e. all edges have the same overlap).";
    text << "* Total length: The total number of base pairs in the graph.";
    text << "* Total length no overlaps: The total number of base pairs in the graph, subtracting bases that are duplicated in edge overlaps.";
    text << "* Dead ends: The number of instances where an end of a node does not connect to any other nodes.";
    text << "* Percentage dead ends: The proportion of possible dead ends. The maximum number of dead ends is twice the number of nodes (occurs when there are no edges), so this value is the number of dead ends divided by twice the node count.";
    text << "* Connected components: The number of regions of the graph which are disconnected from each other.";
    text << "* Largest component: The total number of base pairs in the largest connected component.";
    text << "* N50: Nodes that are this length or greater will collectively add up to at least half of the total length.";
    text << "* Shortest node: The length of the shortest node in the graph.";
    text << "* Lower quartile node: The median node length for the shorter half of the nodes.";
    text << "* Median node: The median node length for the graph.";
    text << "* Upper quartile node: The median node length for the longer half of the nodes.";
    text << "* Longest node: The length of the longest node in the graph.";
    text << "* Median depth: The median depth of the graph, by base.";
    text << "* Estimated sequence length: An estimate of the total number of bases in the original sequence, calculated by multiplying each node's length (minus overlaps) by its depth relative to the median.";
    text << "";
    text << "Usage:    Bandage info <graph> [options]";
    text << "";
    text << "Positional parameters:";
    text << "<graph>             A graph file of any type supported by Bandage";
    text << "";
    text << "Options:  --tsv               Output the information in a single tab-delimited line starting with the graph file";
    text << "";

    getCommonHelp(&text);
    if (all)
        getSettingsUsage(&text);
    getOnlineHelpMessage(&text);

    outputText(text, out);
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
