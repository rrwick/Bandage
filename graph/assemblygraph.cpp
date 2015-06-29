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


#include "assemblygraph.h"
#include <QMapIterator>
#include "../program/globals.h"
#include "../program/settings.h"
#include <limits>
#include <algorithm>
#include "../graph/debruijnnode.h"
#include "../graph/debruijnedge.h"
#include "../graph/graphicsitemnode.h"

AssemblyGraph::AssemblyGraph() :
    m_contiguitySearchDone(false)
{
    m_ogdfGraph = new ogdf::Graph();
    m_graphAttributes = new ogdf::GraphAttributes(*m_ogdfGraph, ogdf::GraphAttributes::nodeGraphics |
                                                  ogdf::GraphAttributes::edgeGraphics);
}

AssemblyGraph::~AssemblyGraph()
{
    delete m_graphAttributes;
    delete m_ogdfGraph;
}


void AssemblyGraph::cleanUp()
{
    QMapIterator<long long, DeBruijnNode*> i(m_deBruijnGraphNodes);
    while (i.hasNext())
    {
        i.next();
        delete i.value();
    }
    m_deBruijnGraphNodes.clear();

    for (size_t i = 0; i < m_deBruijnGraphEdges.size(); ++i)
        delete m_deBruijnGraphEdges[i];
    m_deBruijnGraphEdges.clear();

    m_contiguitySearchDone = false;

    clearGraphInfo();
}





//This function makes a double edge: in one direction for the given nodes
//and the opposite direction for their reverse complements.  It adds the
//new edges to the vector here and to the nodes themselves.
void AssemblyGraph::createDeBruijnEdge(long long node1Number, long long node2Number)
{
    //Quit if any of the nodes don't exist.
    if (!m_deBruijnGraphNodes.contains(node1Number) ||
            !m_deBruijnGraphNodes.contains(node2Number) ||
            !m_deBruijnGraphNodes.contains(-node1Number) ||
            !m_deBruijnGraphNodes.contains(-node2Number))
        return;

    DeBruijnNode * node1 = m_deBruijnGraphNodes[node1Number];
    DeBruijnNode * node2 = m_deBruijnGraphNodes[node2Number];
    DeBruijnNode * negNode1 = m_deBruijnGraphNodes[-node1Number];
    DeBruijnNode * negNode2 = m_deBruijnGraphNodes[-node2Number];

    //Quit if the edge already exists
    for (size_t i = 0; i < node1->m_edges.size(); ++i)
    {
        if (node1->m_edges[i]->m_startingNode == node1 &&
                node1->m_edges[i]->m_endingNode == node2)
            return;
    }

    //Usually, an edge has a different pair, but it is possible
    //for an edge to be its own pair.
    bool isOwnPair = (node1 == negNode2 && node2 == negNode1);

    DeBruijnEdge * forwardEdge = new DeBruijnEdge(node1, node2);
    DeBruijnEdge * backwardEdge;

    if (isOwnPair)
        backwardEdge = forwardEdge;
    else
        backwardEdge = new DeBruijnEdge(negNode2, negNode1);

    forwardEdge->m_reverseComplement = backwardEdge;
    backwardEdge->m_reverseComplement = forwardEdge;

    m_deBruijnGraphEdges.push_back(forwardEdge);
    if (!isOwnPair)
        m_deBruijnGraphEdges.push_back(backwardEdge);

    node1->addEdge(forwardEdge);
    node2->addEdge(forwardEdge);
    negNode1->addEdge(backwardEdge);
    negNode2->addEdge(backwardEdge);
}




void AssemblyGraph::clearOgdfGraphAndResetNodes()
{
    QMapIterator<long long, DeBruijnNode*> i(m_deBruijnGraphNodes);
    while (i.hasNext())
    {
        i.next();
        i.value()->resetNode();
    }

    m_ogdfGraph->clear();
}






//http://www.code10.info/index.php?option=com_content&view=article&id=62:articledna-reverse-complement&catid=49:cat_coding_algorithms_bioinformatics&Itemid=74
QByteArray AssemblyGraph::getReverseComplement(QByteArray forwardSequence)
{
    QByteArray reverseComplement;

    for (int i = forwardSequence.length() - 1; i >= 0; --i)
    {
        char letter = forwardSequence.at(i);

        switch (letter)
        {
        case 'A': reverseComplement.append('T'); break;
        case 'T': reverseComplement.append('A'); break;
        case 'G': reverseComplement.append('C'); break;
        case 'C': reverseComplement.append('G'); break;
        case 'a': reverseComplement.append('t'); break;
        case 't': reverseComplement.append('a'); break;
        case 'g': reverseComplement.append('c'); break;
        case 'c': reverseComplement.append('g'); break;
        case 'R': reverseComplement.append('Y'); break;
        case 'Y': reverseComplement.append('R'); break;
        case 'S': reverseComplement.append('S'); break;
        case 'W': reverseComplement.append('W'); break;
        case 'K': reverseComplement.append('M'); break;
        case 'M': reverseComplement.append('K'); break;
        case 'r': reverseComplement.append('y'); break;
        case 'y': reverseComplement.append('r'); break;
        case 's': reverseComplement.append('s'); break;
        case 'w': reverseComplement.append('w'); break;
        case 'k': reverseComplement.append('m'); break;
        case 'm': reverseComplement.append('k'); break;
        case 'B': reverseComplement.append('V'); break;
        case 'D': reverseComplement.append('H'); break;
        case 'H': reverseComplement.append('D'); break;
        case 'V': reverseComplement.append('B'); break;
        case 'b': reverseComplement.append('v'); break;
        case 'd': reverseComplement.append('h'); break;
        case 'h': reverseComplement.append('d'); break;
        case 'v': reverseComplement.append('b'); break;
        case 'N': reverseComplement.append('N'); break;
        case 'n': reverseComplement.append('n'); break;
        case '.': reverseComplement.append('.'); break;
        case '-': reverseComplement.append('-'); break;
        case '?': reverseComplement.append('?'); break;
        }
    }

    return reverseComplement;
}




void AssemblyGraph::resetEdges()
{
    for (size_t i = 0; i < m_deBruijnGraphEdges.size(); ++i)
        m_deBruijnGraphEdges[i]->reset();
}


double AssemblyGraph::getMeanDeBruijnGraphCoverage(bool drawnNodesOnly)
{
    int nodeCount = 0;
    long double coverageSum = 0.0;
    long long totalLength = 0;

    QMapIterator<long long, DeBruijnNode*> i(m_deBruijnGraphNodes);
    while (i.hasNext())
    {
        i.next();
        DeBruijnNode * node = i.value();

        if (drawnNodesOnly && !node->m_drawn)
            continue;

        ++nodeCount;
        totalLength += node->m_length;
        coverageSum += node->m_length * node->m_coverage;

    }

    if (totalLength == 0)
        return 0.0;
    else
        return coverageSum / totalLength;
}

double AssemblyGraph::getMaxDeBruijnGraphCoverageOfDrawnNodes()
{
    double maxCoverage = 1.0;

    QMapIterator<long long, DeBruijnNode*> i(m_deBruijnGraphNodes);
    while (i.hasNext())
    {
        i.next();

        if (i.value()->m_graphicsItemNode != 0 && i.value()->m_coverage > maxCoverage)
            maxCoverage = i.value()->m_coverage;
    }

    return maxCoverage;
}


void AssemblyGraph::resetNodeContiguityStatus()
{
    QMapIterator<long long, DeBruijnNode*> i(m_deBruijnGraphNodes);
    while (i.hasNext())
    {
        i.next();
        i.value()->m_contiguityStatus = NOT_CONTIGUOUS;
    }
    m_contiguitySearchDone = false;
}

void AssemblyGraph::resetAllNodeColours()
{
    QMapIterator<long long, DeBruijnNode*> i(m_deBruijnGraphNodes);
    while (i.hasNext())
    {
        i.next();
        if (i.value()->m_graphicsItemNode != 0)
            i.value()->m_graphicsItemNode->setNodeColour();
    }
}

void AssemblyGraph::clearAllBlastHitPointers()
{
    QMapIterator<long long, DeBruijnNode*> i(m_deBruijnGraphNodes);
    while (i.hasNext())
    {
        i.next();
        DeBruijnNode * node = i.value();
        node->m_blastHits.clear();
    }
}



void AssemblyGraph::determineGraphInfo()
{
    m_shortestContig = std::numeric_limits<long long>::max();
    m_longestContig = 0;
    int nodeCount = 0;
    long long totalLength = 0;
    std::vector<double> nodeCoverages;

    QMapIterator<long long, DeBruijnNode*> i(m_deBruijnGraphNodes);
    while (i.hasNext())
    {
        i.next();
        long long nodeLength = i.value()->m_length;

        if (nodeLength < m_shortestContig)
            m_shortestContig = nodeLength;
        if (nodeLength > m_longestContig)
            m_longestContig = nodeLength;

        //Only add up the length for positive nodes
        if (i.value()->m_number > 0)
        {
            totalLength += nodeLength;
            ++nodeCount;
        }

        nodeCoverages.push_back(i.value()->m_coverage);
    }

    //Count up the edges.  Edges that are their own pairs will
    //not be counted, as these won't show up in single mode.
    int edgeCount = 0;
    for (size_t i = 0; i < m_deBruijnGraphEdges.size(); ++i)
    {
        DeBruijnEdge * edge = m_deBruijnGraphEdges[i];
        if (edge != edge->m_reverseComplement)
            ++edgeCount;
    }
    edgeCount /= 2;

    m_nodeCount = nodeCount;
    m_edgeCount = edgeCount;
    m_totalLength = totalLength;
    m_meanCoverage = getMeanDeBruijnGraphCoverage();

    std::sort(nodeCoverages.begin(), nodeCoverages.end());

    double firstQuartileIndex = nodeCoverages.size() / 4.0;
    double medianIndex = nodeCoverages.size() / 2.0;
    double thirdQuartileIndex = nodeCoverages.size() * 3.0 / 4.0;

    m_firstQuartileCoverage = getValueUsingFractionalIndex(&nodeCoverages, firstQuartileIndex);
    m_medianCoverage = getValueUsingFractionalIndex(&nodeCoverages, medianIndex);
    m_thirdQuartileCoverage = getValueUsingFractionalIndex(&nodeCoverages, thirdQuartileIndex);

    //Set the auto base pairs per segment
    int totalSegments = m_nodeCount * g_settings->meanSegmentsPerNode;
    g_settings->autoBasePairsPerSegment = m_totalLength / totalSegments;
}

double AssemblyGraph::getValueUsingFractionalIndex(std::vector<double> * doubleVector, double index)
{
    if (doubleVector->size() == 0)
        return 0.0;
    if (doubleVector->size() == 1)
        return (*doubleVector)[0];

    int wholePart = floor(index);

    if (wholePart < 0)
        return (*doubleVector)[0];
    if (wholePart >= int(doubleVector->size()) - 1)
        return (*doubleVector)[doubleVector->size() - 1];

    double fractionalPart = index - wholePart;

    double piece1 = (*doubleVector)[wholePart];
    double piece2 = (*doubleVector)[wholePart+1];

    return piece1 * (1.0 - fractionalPart) + piece2 * fractionalPart;
}

void AssemblyGraph::clearGraphInfo()
{
    m_totalLength = 0;
    m_shortestContig = 0;
    m_longestContig = 0;

    m_meanCoverage = 0.0;
    m_firstQuartileCoverage = 0.0;
    m_medianCoverage = 0.0;
    m_thirdQuartileCoverage = 0.0;
}
