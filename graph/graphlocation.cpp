//Copyright 2016 Ryan Wick

//This file is part of Bandage.

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


#include "graphlocation.h"

#include "debruijnnode.h"
#include "assemblygraph.h"
#include "../program/globals.h"



GraphLocation::GraphLocation() :
    m_node(0), m_position(0)
{

}

GraphLocation::GraphLocation(DeBruijnNode * node, int position) :
    m_node(node), m_position(position)
{
}


GraphLocation GraphLocation::startOfNode(DeBruijnNode * node)
{
    GraphLocation location(node, 1);
    if (location.isValid())
        return location;
    else
        return GraphLocation::null();
}


GraphLocation GraphLocation::endOfNode(DeBruijnNode * node)
{
    if (node == 0)
        return GraphLocation::null();

    int pos = node->getLength();
    GraphLocation location(node, pos);
    if (location.isValid())
        return location;
    else
        return GraphLocation::null();
}


GraphLocation GraphLocation::null()
{
    return GraphLocation(0, 0);
}

bool GraphLocation::isValid() const
{
    if (isNull())
        return false;
    if (m_position < 1)
        return false;
    return m_position <= m_node->getLength();
}


bool GraphLocation::isNull() const
{
    return (m_node == 0 || m_position == 0);
}


GraphLocation GraphLocation::reverseComplementLocation() const
{
    int newPos = m_node->getLength() - m_position + 1;
    GraphLocation newLocation(m_node->getReverseComplement(), newPos);

    //For Velvet graphs, the reverse complement location is shifted by the k-mer
    //size and may not even be on the same node!
    if (g_assemblyGraph->m_graphFileType == LAST_GRAPH)
        newLocation.moveLocation(-g_assemblyGraph->m_kmer + 1);

    if (newLocation.isValid())
        return newLocation;
    else
        return GraphLocation::null();
}



void GraphLocation::moveLocation(int change)
{
    if (change > 0)
        moveForward(change);
    else if (change < 0)
        moveBackward(-change);
}


void GraphLocation::moveForward(int change)
{
    //See if there are enough bases left in this node to move by the
    //required amount.  If so, we're done!
    int basesLeftInNode = m_node->getLength() - m_position;
    if (change <= basesLeftInNode)
    {
        m_position += change;
        return;
    }

    //If there aren't enough bases left, then we recursively try with the
    //next nodes.
    std::vector<DeBruijnNode *> downstreamNodes = m_node->getDownstreamNodes();
    for (size_t i = 0; i < downstreamNodes.size(); ++i)
    {
        DeBruijnNode * node = downstreamNodes[i];
        GraphLocation nextNodeLocation = GraphLocation::startOfNode(node);
        nextNodeLocation.moveForward(change - basesLeftInNode - 1);

        if (nextNodeLocation.isValid())
        {
            m_node = nextNodeLocation.getNode();
            m_position = nextNodeLocation.getPosition();
            return;
        }
    }

    //If the code got here, then we failed to move and we make this a null
    //position.
    m_node = 0;
    m_position = 0;
    return;
}

void GraphLocation::moveBackward(int change)
{
    //See if there are enough bases left in this node to move by the
    //required amount.  If so, we're done!
    int basesLeftInNode = m_position - 1;
    if (change <= basesLeftInNode)
    {
        m_position -= change;
        return;
    }

    //If there aren't enough bases left, then we recursively try with the
    //next nodes.
    std::vector<DeBruijnNode *> upstreamNodes = m_node->getUpstreamNodes();
    for (size_t i = 0; i < upstreamNodes.size(); ++i)
    {
        DeBruijnNode * node = upstreamNodes[i];
        GraphLocation nextNodeLocation = GraphLocation::endOfNode(node);
        nextNodeLocation.moveBackward(change - basesLeftInNode - 1);

        if (nextNodeLocation.isValid())
        {
            m_node = nextNodeLocation.getNode();
            m_position = nextNodeLocation.getPosition();
            return;
        }
    }

    //If the code got here, then we failed to move and we make this a null
    //position.
    m_node = 0;
    m_position = 0;
    return;
}


char GraphLocation::getBase() const
{
    if (!isValid())
        return '\0';
    else
        return m_node->getBaseAt(m_position - 1);
}



bool GraphLocation::isAtStartOfNode() const
{
    return (isValid() && m_position == 1);
}

bool GraphLocation::isAtEndOfNode() const
{
    return (isValid() && m_position == m_node->getLength());
}

bool GraphLocation::operator==(GraphLocation const &other) const
{
    return (m_node == other.m_node && m_position == other.m_position);
}
