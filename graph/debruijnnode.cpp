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


#include "debruijnnode.h"
#include "debruijnedge.h"
#include "ogdfnode.h"
#include "../program/globals.h"
#include "../program/settings.h"
#include "graphicsitemnode.h"
#include <math.h>
#include "../blast/blasthit.h"

DeBruijnNode::DeBruijnNode(int number, int length, double coverage, QByteArray sequence) :
    m_number(number), m_length(length), m_coverage(coverage),
    m_coverageRelativeToMeanDrawnCoverage(1.0), m_sequence(sequence),
    m_contiguityStatus(NOT_CONTIGUOUS), m_reverseComplement(0), m_ogdfNode(0),
    m_graphicsItemNode(0), m_startingNode(false), m_drawn(false), m_highestDistanceInNeighbourSearch(0),
    m_customColour(QColor(190, 190, 190))
{
}


DeBruijnNode::~DeBruijnNode()
{
    if (m_ogdfNode != 0)
        delete m_ogdfNode;
}



//This function adds an edge to the Node, but only if the edge hasn't already
//been added.
void DeBruijnNode::addEdge(DeBruijnEdge * edge)
{
    if (std::find(m_edges.begin(), m_edges.end(), edge) == m_edges.end())
        m_edges.push_back(edge);
}


//This function resets the node to the state it would be in after a graph
//file was loaded - no contiguity status and no OGDF nodes.
void DeBruijnNode::resetNode()
{
    if (m_ogdfNode != 0)
        delete m_ogdfNode;
    m_ogdfNode = 0;
    m_graphicsItemNode = 0;
    m_contiguityStatus = NOT_CONTIGUOUS;
    m_startingNode = false;
    m_drawn = false;
    m_highestDistanceInNeighbourSearch = 0;
}


void DeBruijnNode::addToOgdfGraph(ogdf::Graph * ogdfGraph)
{
    //If this node or its reverse complement is already in OGDF, then
    //it's not necessary to make the node.
    if (thisOrReverseComplementInOgdf())
        return;

    //Create the OgdfNode object
    m_ogdfNode = new OgdfNode();

    //Each node in the Velvet sense is made up of multiple nodes in the
    //OGDF sense.  This way, Velvet nodes appear as lines whose length
    //corresponds to the sequence length.
    int numberOfGraphEdges = ceil(double(m_length) / g_settings->basePairsPerSegment);
    if (numberOfGraphEdges == 0)
        numberOfGraphEdges = 1;
    int numberOfGraphNodes = numberOfGraphEdges + 1;

    ogdf::node newNode = 0;
    ogdf::node previousNode = 0;
    for (int i = 0; i < numberOfGraphNodes; ++i)
    {
        newNode = ogdfGraph->newNode();
        m_ogdfNode->addOgdfNode(newNode);

        if (i > 0)
            ogdfGraph->newEdge(previousNode, newNode);

        previousNode = newNode;
    }
}



void DeBruijnNode::determineContiguity(DeBruijnNode * previousNode)
{
    //If this is the start of the contiguity determination, travel out in both directions.
    if (previousNode == 0)
    {
        setContiguityStatus(STARTING);
        for (size_t i = 0; i < m_edges.size(); ++i)
        {
            DeBruijnNode * otherNode = m_edges[i]->getOtherNode(this);
            otherNode->determineContiguity(this);
        }
        return;
    }

    //If this is not the start, determine whether the previous node came from the incoming or outgoing connections.
    ContiguityStatus startingStatus = m_contiguityStatus;
    std::vector<DeBruijnNode *> incomingNodes;
    std::vector<DeBruijnNode *> outgoingNodes;
    for (size_t i = 0; i < m_edges.size(); ++i)
    {
        DeBruijnNode * otherNode = m_edges[i]->getOtherNode(this);
        if (this == m_edges[i]->m_startingNode)
            outgoingNodes.push_back(otherNode);
        else //This node is the ending node of the edge
            incomingNodes.push_back(otherNode);
    }
    std::vector<DeBruijnNode *> * forwardNodes;
    std::vector<DeBruijnNode *> * backwardNodes;
    if (std::find(incomingNodes.begin(), incomingNodes.end(), previousNode) != incomingNodes.end())
    {
        //previousNode is in incomingEdges
        forwardNodes = &outgoingNodes;
        backwardNodes = &incomingNodes;
    }
    else
    {
        //previousNode is in outgoing
        forwardNodes = &incomingNodes;
        backwardNodes = &outgoingNodes;
    }

    // If the previous node is contiguous (or the starting node) and the
    // only path backward is to the previous node, then this node is
    // definitely contiguous.
    if ((previousNode->m_contiguityStatus == CONTIGUOUS || previousNode->m_contiguityStatus == STARTING) &&
            backwardNodes->size() == 1 && (*backwardNodes)[0] == previousNode)
        setContiguityStatus(CONTIGUOUS);

    // If the previous node is contiguous, but there are multiple paths
    // backwards, then this node is contiguous until branching.
    else if ((previousNode->m_contiguityStatus == CONTIGUOUS || previousNode->m_contiguityStatus == STARTING) &&
             backwardNodes->size() > 1)
        setContiguityStatus(CONTIGUOUS_UNTIL_BRANCHING);

    // If the previous node is contiguous until branching, but this is
    // the only path from that node, then this node is also contiguous
    // until branching.
    else if (previousNode->m_contiguityStatus == CONTIGUOUS_UNTIL_BRANCHING &&
             previousNode->isOnlyPathInItsDirection(this, &incomingNodes, &outgoingNodes))
        setContiguityStatus(CONTIGUOUS_UNTIL_BRANCHING);

    // If the previous node is contiguous until branching, and it has
    // branched, then this node is maybe contiguous.
    else if (previousNode->m_contiguityStatus == CONTIGUOUS_UNTIL_BRANCHING &&
             previousNode->isNotOnlyPathInItsDirection(this, &incomingNodes, &outgoingNodes))
        setContiguityStatus(MAYBE_CONTIGUOUS);

    // If the previous node is maybe contiguous, then this node is also maybe contiguous.
    else if (previousNode->m_contiguityStatus == MAYBE_CONTIGUOUS)
        setContiguityStatus(MAYBE_CONTIGUOUS);


    //If this node's status has changed during this function, call this function
    //on each of the forward connected nodes.
    if (startingStatus == m_contiguityStatus)
        return;
    for (size_t i = 0; i < forwardNodes->size(); ++i)
        (*forwardNodes)[i]->determineContiguity(this);
}


//This function only upgrades a node's status, never downgrades.
void DeBruijnNode::setContiguityStatus(ContiguityStatus newStatus)
{
    if (newStatus < m_contiguityStatus)
        m_contiguityStatus = newStatus;

    //if (g_settings->doubleMode)
    if (m_graphicsItemNode != 0)
        m_graphicsItemNode->setNodeColour();
}



//It is expected that the argument connectedNode is either in incomingNodes or
//outgoingNodes.  If that node is the only one in whichever container it is in,
//this function returns true.
bool DeBruijnNode::isOnlyPathInItsDirection(DeBruijnNode * connectedNode,
                                            std::vector<DeBruijnNode *> * incomingNodes,
                                            std::vector<DeBruijnNode *> * outgoingNodes)
{
    std::vector<DeBruijnNode *> * container;
    if (std::find(incomingNodes->begin(), incomingNodes->end(), connectedNode) != incomingNodes->end())
        container = incomingNodes;
    else
        container = outgoingNodes;

    return (container->size() == 1 && (*container)[0] == connectedNode);
}


QByteArray DeBruijnNode::getFasta()
{
    QByteArray fasta = ">NODE_";
    fasta += QString::number(m_number);
    fasta += "_length_";
    fasta += QString::number(m_length);
    fasta += "_cov_";
    fasta += QString::number(m_coverage);
    fasta += "\n";

    int charactersOnLine = 0;
    for (int i = 0; i < m_sequence.length(); ++i)
    {
        fasta += m_sequence.at(i);
        ++charactersOnLine;
        if (charactersOnLine >= 60)
        {
            fasta += "\n";
            charactersOnLine = 0;
        }
    }

    return fasta;
}


//This function recursively labels all nodes as drawn that are within a
//certain distance of this node.  Whichever node called this will
//definitely be drawn, so that one is excluded from the recursive call.
void DeBruijnNode::labelNeighbouringNodesAsDrawn(int nodeDistance, DeBruijnNode * callingNode)
{
    if (m_highestDistanceInNeighbourSearch > nodeDistance)
        return;
    m_highestDistanceInNeighbourSearch = nodeDistance;

    if (nodeDistance == 0)
        return;

    DeBruijnNode * otherNode;
    for (size_t i = 0; i < m_edges.size(); ++i)
    {
        otherNode = m_edges[i]->getOtherNode(this);

        if (otherNode == callingNode)
            continue;

        if (g_settings->doubleMode)
            otherNode->m_drawn = true;
        else //single mode
        {
            if (otherNode->m_number > 0)
                otherNode->m_drawn = true;
            else
                otherNode->m_reverseComplement->m_drawn = true;
        }
        otherNode->labelNeighbouringNodesAsDrawn(nodeDistance-1, this);
    }
}


std::vector<BlastHitPart> DeBruijnNode::getBlastHitPartsForThisNodeOrReverseComplement()
{
    std::vector<BlastHitPart> returnVector;

    for (size_t i = 0; i < m_blastHits.size(); ++i)
    {
        std::vector<BlastHitPart> hitParts = m_blastHits[i]->getBlastHitParts(false);
        returnVector.insert(returnVector.end(), hitParts.begin(), hitParts.end());
    }
    for (size_t i = 0; i < m_reverseComplement->m_blastHits.size(); ++i)
    {
        std::vector<BlastHitPart> hitParts = m_reverseComplement->m_blastHits[i]->getBlastHitParts(true);
        returnVector.insert(returnVector.end(), hitParts.begin(), hitParts.end());
    }

    return returnVector;
}
