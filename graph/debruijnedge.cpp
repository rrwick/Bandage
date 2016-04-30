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


#include "debruijnedge.h"
#include <math.h>
#include "../program/settings.h"
#include "ogdfnode.h"
#include <QApplication>
#include "../program/settings.h"
#include "../program/globals.h"
#include "assemblygraph.h"

DeBruijnEdge::DeBruijnEdge(DeBruijnNode *startingNode, DeBruijnNode *endingNode) :
    m_startingNode(startingNode), m_endingNode(endingNode), m_graphicsItemEdge(0),
    m_drawn(false), m_overlapType(UNKNOWN_OVERLAP), m_overlap(0)
{
}



//This function assumes that the parameter node pointer is one of the two nodes
//in this edge, and it returns the other one.
DeBruijnNode * DeBruijnEdge::getOtherNode(const DeBruijnNode * node) const
{
    if (node == m_startingNode)
        return m_endingNode;
    else
        return m_startingNode;
}


//This function determines whether the edge should be drawn to the screen.
bool DeBruijnEdge::edgeIsVisible() const
{
    //If the program is in double mode, then draw any edge where both of its
    //nodes are drawn.
    if (g_settings->doubleMode)
        return m_startingNode->isDrawn() && m_endingNode->isDrawn();

    //If the program is in single mode, then draw any edge where both of its
    //nodes or their reverse complements are drawn.
    else
    {
        bool drawEdge = (m_startingNode->isDrawn() || m_startingNode->getReverseComplement()->isDrawn())
                && (m_endingNode->isDrawn() || m_endingNode->getReverseComplement()->isDrawn());
        if (!drawEdge)
            return false;

        //But it is also necessary to avoid drawing both an edge and its
        //reverse complement edge.
        return isPositiveEdge();
    }
}



//This function says whether an edge is 'positive'.  This is used to distinguish
//an edge from its reverse complement - i.e. half of the graph edges are
//positive and their reverse complements are negative.
//When one node in the edge is positive and the other is negative, then the
//choice is somewhat arbitrary.
bool DeBruijnEdge::isPositiveEdge() const
{
    //If both nodes have a positive number, show this edge, and not
    //the reverse complement where both nodes are negative.
    if (m_startingNode->isPositiveNode() && m_endingNode->isPositiveNode())
        return true;
    if (m_startingNode->isNegativeNode() && m_endingNode->isNegativeNode())
        return false;

    //Edges that are their own reverse complement are considered positive (but
    //will not have a negative counterpart).
    if (isOwnReverseComplement())
        return true;

    //If the code got here, then one node is positive and the other
    //negative.  In this case, just choose the one with the first name
    //alphabetically - an arbitrary choice, but at least it is
    //consistent.
    return (m_startingNode->getName() > m_reverseComplement->m_startingNode->getName());
}


void DeBruijnEdge::addToOgdfGraph(ogdf::Graph * ogdfGraph, ogdf::EdgeArray<double> * edgeArray) const
{
    ogdf::node firstEdgeOgdfNode;
    ogdf::node secondEdgeOgdfNode;

    if (m_startingNode->inOgdf())
        firstEdgeOgdfNode = m_startingNode->getOgdfNode()->getLast();
    else if (m_startingNode->getReverseComplement()->inOgdf())
        firstEdgeOgdfNode = m_startingNode->getReverseComplement()->getOgdfNode()->getFirst();
    else
        return; //Ending node or its reverse complement isn't in OGDF

    if (m_endingNode->inOgdf())
        secondEdgeOgdfNode = m_endingNode->getOgdfNode()->getFirst();
    else if (m_endingNode->getReverseComplement()->inOgdf())
        secondEdgeOgdfNode = m_endingNode->getReverseComplement()->getOgdfNode()->getLast();
    else
        return; //Ending node or its reverse complement isn't in OGDF

    //If this in an edge connected a single-segment node to itself, then we
    //don't want to put it in the OGDF graph, because it would be redundant
    //with the node segment (and created conflict with the node/edge length).
    if (m_startingNode == m_endingNode)
    {
        if (m_startingNode->getNumberOfOgdfGraphEdges(m_startingNode->getDrawnNodeLength()) == 1)
            return;
    }

    ogdf::edge newEdge = ogdfGraph->newEdge(firstEdgeOgdfNode, secondEdgeOgdfNode);
    (*edgeArray)[newEdge] = g_settings->edgeLength;
}








//This function traces all possible paths from this edge.
//It proceeds a number of steps, as determined by a setting.
//If forward is true, it looks in a forward direction (starting nodes to
//ending nodes).  If forward is false, it looks in a backward direction
//(ending nodes to starting nodes).
void DeBruijnEdge::tracePaths(bool forward,
                              int stepsRemaining,
                              std::vector< std::vector <DeBruijnNode *> > * allPaths,
                              DeBruijnNode * startingNode,
                              std::vector<DeBruijnNode *> pathSoFar) const
{
    //This can go for a while, so keep the UI responsive.
    QApplication::processEvents();

    //Find the node in the direction we are tracing.
    DeBruijnNode * nextNode;
    if (forward)
        nextNode = m_endingNode;
    else
        nextNode = m_startingNode;

    //Add that node to the path so far.
    pathSoFar.push_back(nextNode);

    //If there are no steps left, then the path so far is done.
    --stepsRemaining;
    if (stepsRemaining == 0)
    {
        allPaths->push_back(pathSoFar);
        return;
    }

    //If the code got here, then more steps remain.
    //Find the edges that are in the correct direction.
    std::vector<DeBruijnEdge *> nextEdges = findNextEdgesInPath(nextNode, forward);

    //If there are no next edges, then we are finished with the
    //path search, even though steps remain.
    if (nextEdges.size() == 0)
    {
        allPaths->push_back(pathSoFar);
        return;
    }

    //Call this function on all of the next edges.
    //However, we also need to check to see if we are tracing a loop
    //and stop if that is the case.
    for (size_t i = 0; i < nextEdges.size(); ++i)
    {
        DeBruijnEdge * nextEdge = nextEdges[i];

        //Determine the node that this next edge leads to.
        DeBruijnNode * nextNextNode;
        if (forward)
            nextNextNode = nextEdge->m_endingNode;
        else
            nextNextNode = nextEdge->m_startingNode;

        //If that node is the starting node, then we've made
        //a full loop and the path should be considered complete.
        if (nextNextNode == startingNode)
        {
            allPaths->push_back(pathSoFar);
            continue;
        }

        //If that node is already in the path TWICE so far, that means
        //we're caught in a loop, and we should throw this path out.
        //If it appears 0 or 1 times, then continue the path search.
        if (timesNodeInPath(nextNextNode, &pathSoFar) < 2)
            nextEdge->tracePaths(forward, stepsRemaining, allPaths, startingNode, pathSoFar);
    }
}


//This function counts how many times a node appears in a path
int DeBruijnEdge::timesNodeInPath(DeBruijnNode * node, std::vector<DeBruijnNode *> * path) const
{
    int count = 0;
    for (size_t i = 0; i < path->size(); ++i)
    {
        if ( (*path)[i] == node)
            ++count;
    }

    return count;
}



bool DeBruijnEdge::leadsOnlyToNode(bool forward,
                                   int stepsRemaining,
                                   DeBruijnNode * target,
                                   std::vector<DeBruijnNode *> pathSoFar,
                                   bool includeReverseComplement) const
{
    //This can go for a while, so keep the UI responsive.
    QApplication::processEvents();

    //Find the node in the direction we are tracing.
    DeBruijnNode * nextNode;
    if (forward)
        nextNode = m_endingNode;
    else
        nextNode = m_startingNode;

    //Add that node to the path so far.
    pathSoFar.push_back(nextNode);

    //If this path has landed on the node from which the search began,
    //that means we've followed a loop around.  The search has therefore
    //failed because this path could represent circular DNA that does
    //not contain the target.
    if (nextNode == pathSoFar[0])
        return false;

    //If the next node is the target, the search succeeded!
    if (nextNode == target)
        return true;

    //If we are including reverse complements and the next node is
    //the reverse complement of the target, the search succeeded!
    if (includeReverseComplement && nextNode->getReverseComplement() == target)
        return true;

    //If there are no steps left, then the search failed.
    --stepsRemaining;
    if (stepsRemaining == 0)
        return false;

    //If the code got here, then more steps remain.
    //Find the edges that are in the correct direction.
    std::vector<DeBruijnEdge *> nextEdges = findNextEdgesInPath(nextNode, forward);

    //If there are no next edges, then the search failed, even
    //though steps remain.
    if (nextEdges.size() == 0)
        return false;

    //In order for the search to succeed, this function needs to return true
    //for all of the nextEdges.
    //However, we also need to check to see if we are tracing a loop
    //and stop if that is the case.
    for (size_t i = 0; i < nextEdges.size(); ++i)
    {
        DeBruijnEdge * nextEdge = nextEdges[i];

        //Determine the node that this next edge leads to.
        DeBruijnNode * nextNextNode;
        if (forward)
            nextNextNode = nextEdge->m_endingNode;
        else
            nextNextNode = nextEdge->m_startingNode;

        //If that node is already in the path TWICE so far, that means
        //we're caught in a loop, and we should throw this path out.
        //If it appears 0 or 1 times, then continue the path search.
        if (timesNodeInPath(nextNextNode, &pathSoFar) < 2)
        {
            if ( !nextEdge->leadsOnlyToNode(forward, stepsRemaining, target, pathSoFar, includeReverseComplement) )
                return false;
        }
    }

    //If the code got here, then the search succeeded!
    return true;
}


std::vector<DeBruijnEdge *> DeBruijnEdge::findNextEdgesInPath(DeBruijnNode * nextNode,
                                                              bool forward) const
{
    std::vector<DeBruijnEdge *> nextEdges;
    const std::vector<DeBruijnEdge *> * nextNodeEdges = nextNode->getEdgesPointer();
    for (size_t i = 0; i < nextNodeEdges->size(); ++i)
    {
        DeBruijnEdge * edge = (*nextNodeEdges)[i];

        //If forward, we're looking for edges that lead away from
        //nextNode.  If backward, we're looking for edges that lead
        //into nextNode.
        if ((forward && edge->m_startingNode == nextNode) ||
                (!forward && edge->m_endingNode == nextNode))
            nextEdges.push_back(edge);
    }

    return nextEdges;
}


//This function tries to automatically determine the overlap size
//between the two nodes.  It tries each overlap size between the min
//to the max (in settings), assigning the first one it finds.
void DeBruijnEdge::autoDetermineExactOverlap()
{
    m_overlap = 0;
    m_overlapType = AUTO_DETERMINED_EXACT_OVERLAP;

    //Find an appropriate search range
    int minPossibleOverlap = std::min(m_startingNode->getLength(), m_endingNode->getLength());
    if (minPossibleOverlap < g_settings->minAutoFindEdgeOverlap)
        return;
    int min = std::min(minPossibleOverlap, g_settings->minAutoFindEdgeOverlap);
    int max = std::min(minPossibleOverlap, g_settings->maxAutoFindEdgeOverlap);

    //Try each overlap in the range and set the first one found.
    //However, we don't want the search to be biased towards larger
    //or smaller overlaps, so start with a pseudorandom value and loop.
    int testOverlap = min + (rand() % (max - min + 1));
    for (int i = min; i <= max; ++i)
    {
        if (testExactOverlap(testOverlap))
        {
            m_overlap = testOverlap;
            return;
        }

        ++testOverlap;
        if (testOverlap > max)
            testOverlap = min;
    }
}




//This function tries the given overlap between the two nodes.
//If the overlap works perfectly, it returns true.
bool DeBruijnEdge::testExactOverlap(int overlap) const
{
    bool mismatchFound = false;

    int seq1Offset = m_startingNode->getLength() - overlap;

    //Look at each position in the overlap
    for (int j = 0; j < overlap && !mismatchFound; ++j)
    {
        char a = m_startingNode->getBaseAt(seq1Offset + j);
        char b = m_endingNode->getBaseAt(j);
        if (a != b)
            mismatchFound = true;
    }

    return !mismatchFound;
}


QByteArray DeBruijnEdge::getGfaLinkLine() const
{
    DeBruijnNode * startingNode = getStartingNode();
    DeBruijnNode * endingNode = getEndingNode();

    QByteArray gfaLinkLine = "L\t";
    gfaLinkLine += startingNode->getNameWithoutSign() + "\t";
    gfaLinkLine += startingNode->getSign() + "\t";
    gfaLinkLine += endingNode->getNameWithoutSign() + "\t";
    gfaLinkLine += endingNode->getSign() + "\t";

    //When Velvet graphs are saved to GFA, the sequences are extended to include
    //the overlap.  So even though this edge might have no overlap, the GFA link
    //line should.
    if (g_assemblyGraph->m_graphFileType == LAST_GRAPH)
        gfaLinkLine += QString::number(g_assemblyGraph->m_kmer - 1) + "M";
    else
        gfaLinkLine += QString::number(getOverlap()) + "M";

    gfaLinkLine += "\n";
    return gfaLinkLine;
}

bool DeBruijnEdge::compareEdgePointers(DeBruijnEdge * a, DeBruijnEdge * b)
{
    QString aStart = a->getStartingNode()->getName();
    QString bStart = b->getStartingNode()->getName();
    QString aStartNoSign = aStart;
    aStartNoSign.chop(1);
    QString bStartNoSign = bStart;
    bStartNoSign.chop(1);
    bool ok1;
    long long aStartNumber = aStartNoSign.toLongLong(&ok1);
    bool ok2;
    long long bStartNumber = bStartNoSign.toLongLong(&ok2);

    QString aEnd = a->getEndingNode()->getName();
    QString bEnd = b->getEndingNode()->getName();
    QString aEndNoSign = aEnd;
    aEndNoSign.chop(1);
    QString bEndNoSign = bEnd;
    bEndNoSign.chop(1);
    bool ok3;
    long long aEndNumber = aEndNoSign.toLongLong(&ok3);
    bool ok4;
    long long bEndNumber = bEndNoSign.toLongLong(&ok4);


    //If the node names are essentially numbers, then sort them as numbers.
    if (ok1 && ok2 && ok3 && ok4)
    {
        if (aStartNumber != bStartNumber)
            return aStartNumber < bStartNumber;

        if (aStartNumber == bStartNumber)
            return aEndNumber < bEndNumber;
    }

    //If the node names are strings, then just sort them as strings.
    return aStart < bStart;
}
