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


#include "debruijnedge.h"
#include <cmath>
#include "../program/settings.h"
#include "ogdfnode.h"
#include <QApplication>

DeBruijnEdge::DeBruijnEdge(DeBruijnNode *startingNode, DeBruijnNode *endingNode) :
    m_startingNode(startingNode), m_endingNode(endingNode), m_graphicsItemEdge(0), m_drawn(false)
{
}



//This function assumes that the parameter node pointer is one of the two nodes
//in this edge, and it returns the other one.
DeBruijnNode * DeBruijnEdge::getOtherNode(DeBruijnNode * node)
{
    if (node == m_startingNode)
        return m_endingNode;
    else
        return m_startingNode;
}


//This function determines whether the edge should be drawn to the screen.
bool DeBruijnEdge::edgeIsVisible()
{
    //If the program is in double mode, then draw any edge where both of its
    //nodes are drawn.
    if (g_settings->doubleMode)
        return m_startingNode->m_drawn && m_endingNode->m_drawn;

    //If the program is in single mode, then draw any edge where both of its
    //nodes or their reverse complements are drawn.
    else
    {
        bool drawEdge = (m_startingNode->m_drawn || m_startingNode->m_reverseComplement->m_drawn)
                && (m_endingNode->m_drawn || m_endingNode->m_reverseComplement->m_drawn);
        if (!drawEdge)
            return false;

        //But it is also necessary to avoid drawing both an edge and its
        //reverse complement edge, so some choices are made to ensure
        //that only one is drawn.

        //If both nodes have a positive number, show this edge, and not
        //the reverse complement where both nodes are negative.
        if (m_startingNode->isPositiveNode() && m_endingNode->isPositiveNode())
            return true;
        if (m_startingNode->isNegativeNode() && m_endingNode->isNegativeNode() < 0)
            return false;

        //If the code got here, then one node is positive and the other
        //negative.  In this case, just choose the one with the first name
        //alphabetically - an arbitrary choice, but at least it is
        //consistent.
        return (m_startingNode->m_name > m_reverseComplement->m_startingNode->m_name);
    }
}


void DeBruijnEdge::addToOgdfGraph(ogdf::Graph * ogdfGraph)
{
    ogdf::node firstEdgeOgdfNode;
    ogdf::node secondEdgeOgdfNode;

    if (m_startingNode->inOgdf())
        firstEdgeOgdfNode = m_startingNode->m_ogdfNode->getLast();
    else if (m_startingNode->m_reverseComplement->inOgdf())
        firstEdgeOgdfNode = m_startingNode->m_reverseComplement->m_ogdfNode->getFirst();
    else
        return; //Ending node or its reverse complement isn't in OGDF

    if (m_endingNode->inOgdf())
        secondEdgeOgdfNode = m_endingNode->m_ogdfNode->getFirst();
    else if (m_endingNode->m_reverseComplement->inOgdf())
        secondEdgeOgdfNode = m_endingNode->m_reverseComplement->m_ogdfNode->getLast();
    else
        return; //Ending node or its reverse complement isn't in OGDF

    ogdfGraph->newEdge(firstEdgeOgdfNode, secondEdgeOgdfNode);
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
                              std::vector<DeBruijnNode *> pathSoFar)
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
int DeBruijnEdge::timesNodeInPath(DeBruijnNode * node, std::vector<DeBruijnNode *> * path)
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
                                   bool includeReverseComplement)
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
    if (includeReverseComplement && nextNode->m_reverseComplement == target)
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
                                                              bool forward)
{
    std::vector<DeBruijnEdge *> nextEdges;
    for (size_t i = 0; i < nextNode->m_edges.size(); ++i)
    {
        DeBruijnEdge * edge = nextNode->m_edges[i];

        //If forward, we're looking for edges that lead away from
        //nextNode.  If backward, we're looking for edges that lead
        //into nextNode.
        if ((forward && edge->m_startingNode == nextNode) ||
                (!forward && edge->m_endingNode == nextNode))
            nextEdges.push_back(edge);
    }

    return nextEdges;
}
