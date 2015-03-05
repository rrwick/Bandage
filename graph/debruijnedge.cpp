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
        if (m_startingNode->m_number > 0 && m_endingNode->m_number > 0)
            return true;
        if (m_startingNode->m_number < 0 && m_endingNode->m_number < 0)
            return false;

        //If the code got here, then one node is positive and the other
        //negative.  In this case, just choose the one with the larger
        //starting node - an arbitrary choice, but at least it is
        //consistent.
        return (m_startingNode->m_number > m_reverseComplement->m_startingNode->m_number);
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
