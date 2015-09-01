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
#include "../program/settings.h"
#include "graphicsitemnode.h"
#include <math.h>
#include "../blast/blasthit.h"
#include "../blast/blastquery.h"
#include "assemblygraph.h"
#include <set>
#include <QApplication>

DeBruijnNode::DeBruijnNode(QString name, double readDepth, QByteArray sequence) :
    m_name(name),
    m_readDepth(readDepth),
    m_readDepthRelativeToMeanDrawnReadDepth(1.0),
    m_sequence(sequence),
    m_contiguityStatus(NOT_CONTIGUOUS),
    m_reverseComplement(0),
    m_ogdfNode(0),
    m_graphicsItemNode(0),
    m_specialNode(false),
    m_drawn(false),
    m_highestDistanceInNeighbourSearch(0),
    m_customColour(QColor(190, 190, 190)),
    m_csvData()
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
    resetContiguityStatus();
    setAsNotDrawn();
    setAsNotSpecial();
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
    int numberOfGraphEdges = ceil(double(getLength()) / getBasePairsPerSegment());
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



//This function determines the contiguity of nodes relative to this one.
//It has two steps:
// -First, for each edge leaving this node, all paths outward are found.
//  Any nodes in any path are MAYBE_CONTIGUOUS, and nodes in all of the
//  paths are CONTIGUOUS.
// -Second, it is necessary to check in the opposite direction - for each
//  of the MAYBE_CONTIGUOUS nodes, do they have a path that unambiguously
//  leads to this node?  If so, then they are CONTIGUOUS.
void DeBruijnNode::determineContiguity()
{
    upgradeContiguityStatus(STARTING);

    //A set is used to store all nodes found in the paths, as the nodes
    //that show up as MAYBE_CONTIGUOUS will have their paths checked
    //to this node.
    std::set<DeBruijnNode *> allCheckedNodes;

    //For each path leaving this node, find all possible paths
    //outward.  Nodes in any of the paths for an edge are
    //MAYBE_CONTIGUOUS.  Nodes in all of the paths for an edge
    //are CONTIGUOUS.
    for (size_t i = 0; i < m_edges.size(); ++i)
    {
        DeBruijnEdge * edge = m_edges[i];
        bool outgoingEdge = (this == edge->getStartingNode());

        std::vector< std::vector <DeBruijnNode *> > allPaths;
        edge->tracePaths(outgoingEdge, g_settings->contiguitySearchSteps, &allPaths, this);

        //Set all nodes in the paths as MAYBE_CONTIGUOUS
        for (size_t j = 0; j < allPaths.size(); ++j)
        {
            QApplication::processEvents();
            for (size_t k = 0; k < allPaths[j].size(); ++k)
            {
                DeBruijnNode * node = allPaths[j][k];
                node->upgradeContiguityStatus(MAYBE_CONTIGUOUS);
                allCheckedNodes.insert(node);
            }
        }

        //Set all common nodes as CONTIGUOUS_STRAND_SPECIFIC
        std::vector<DeBruijnNode *> commonNodesStrandSpecific = getNodesCommonToAllPaths(&allPaths, false);
        for (size_t j = 0; j < commonNodesStrandSpecific.size(); ++j)
            (commonNodesStrandSpecific[j])->upgradeContiguityStatus(CONTIGUOUS_STRAND_SPECIFIC);

        //Set all common nodes (when including reverse complement nodes)
        //as CONTIGUOUS_EITHER_STRAND
        std::vector<DeBruijnNode *> commonNodesEitherStrand = getNodesCommonToAllPaths(&allPaths, true);
        for (size_t j = 0; j < commonNodesEitherStrand.size(); ++j)
        {
            DeBruijnNode * node = commonNodesEitherStrand[j];
            node->upgradeContiguityStatus(CONTIGUOUS_EITHER_STRAND);
            node->getReverseComplement()->upgradeContiguityStatus(CONTIGUOUS_EITHER_STRAND);
        }
    }

    //For each node that was checked, then we check to see if any
    //of its paths leads unambiuously back to the starting node (this node).
    for (std::set<DeBruijnNode *>::iterator i = allCheckedNodes.begin(); i != allCheckedNodes.end(); ++i)
    {
        QApplication::processEvents();
        DeBruijnNode * node = *i;
        ContiguityStatus status = node->getContiguityStatus();

        //First check without reverse complement target for
        //strand-specific contiguity.
        if (status != CONTIGUOUS_STRAND_SPECIFIC &&
                node->doesPathLeadOnlyToNode(this, false))
            node->upgradeContiguityStatus(CONTIGUOUS_STRAND_SPECIFIC);

        //Now check including the reverse complement target for
        //either strand contiguity.
        if (status != CONTIGUOUS_STRAND_SPECIFIC &&
                status != CONTIGUOUS_EITHER_STRAND &&
                node->doesPathLeadOnlyToNode(this, true))
        {
            node->upgradeContiguityStatus(CONTIGUOUS_EITHER_STRAND);
            node->getReverseComplement()->upgradeContiguityStatus(CONTIGUOUS_EITHER_STRAND);
        }
    }
}


//This function differs from the above by including all reverse complement
//nodes in the path search.
std::vector<DeBruijnNode *> DeBruijnNode::getNodesCommonToAllPaths(std::vector< std::vector <DeBruijnNode *> > * paths,
                                                                   bool includeReverseComplements) const
{
    std::vector<DeBruijnNode *> commonNodes;

    //If there are no paths, then return the empty vector.
    if (paths->size() == 0)
        return commonNodes;

    //If there is only one path in path, then they are all common nodes
    commonNodes = (*paths)[0];
    if (paths->size() == 1)
        return commonNodes;

    //If there are two or more paths, it's necessary to find the intersection.
    for (size_t i = 1; i < paths->size(); ++i)
    {
        QApplication::processEvents();
        std::vector <DeBruijnNode *> * path = &((*paths)[i]);

        //If we are including reverse complements in the search,
        //then it is necessary to build a new vector that includes
        //reverse complement nodes and then use that vector.
        std::vector <DeBruijnNode *> pathWithReverseComplements;
        if (includeReverseComplements)
        {
            for (size_t j = 0; j < path->size(); ++j)
            {
                DeBruijnNode * node = (*path)[j];
                pathWithReverseComplements.push_back(node);
                pathWithReverseComplements.push_back(node->getReverseComplement());
            }
            path = &pathWithReverseComplements;
        }

        //Combine the commonNodes vector with the path vector,
        //excluding any repeats.
        std::sort(commonNodes.begin(), commonNodes.end());
        std::sort(path->begin(), path->end());
        std::vector<DeBruijnNode *> newCommonNodes;
        std::set_intersection(commonNodes.begin(), commonNodes.end(), path->begin(), path->end(), std::back_inserter(newCommonNodes));
        commonNodes = newCommonNodes;
    }

    return commonNodes;
}


//This function checks whether this node has any path leading outward that
//unambiguously leads to the given node.
//It checks a number of steps as set by the contiguitySearchSteps setting.
//If includeReverseComplement is true, then this function returns true if
//all paths lead either to the node or its reverse complement node.
bool DeBruijnNode::doesPathLeadOnlyToNode(DeBruijnNode * node, bool includeReverseComplement)
{
    for (size_t i = 0; i < m_edges.size(); ++i)
    {
        DeBruijnEdge * edge = m_edges[i];
        bool outgoingEdge = (this == edge->getStartingNode());

        std::vector<DeBruijnNode *> pathSoFar;
        pathSoFar.push_back(this);
        if (edge->leadsOnlyToNode(outgoingEdge, g_settings->contiguitySearchSteps, node, pathSoFar, includeReverseComplement))
            return true;
    }

    return false;
}


//This function only upgrades a node's status, never downgrades.
void DeBruijnNode::upgradeContiguityStatus(ContiguityStatus newStatus)
{
    if (newStatus < m_contiguityStatus)
        m_contiguityStatus = newStatus;
}



//It is expected that the argument connectedNode is either in incomingNodes or
//outgoingNodes.  If that node is the only one in whichever container it is in,
//this function returns true.
bool DeBruijnNode::isOnlyPathInItsDirection(DeBruijnNode * connectedNode,
                                            std::vector<DeBruijnNode *> * incomingNodes,
                                            std::vector<DeBruijnNode *> * outgoingNodes) const
{
    std::vector<DeBruijnNode *> * container;
    if (std::find(incomingNodes->begin(), incomingNodes->end(), connectedNode) != incomingNodes->end())
        container = incomingNodes;
    else
        container = outgoingNodes;

    return (container->size() == 1 && (*container)[0] == connectedNode);
}
bool DeBruijnNode::isNotOnlyPathInItsDirection(DeBruijnNode * connectedNode,
                                 std::vector<DeBruijnNode *> * incomingNodes,
                                 std::vector<DeBruijnNode *> * outgoingNodes) const
{
    return !isOnlyPathInItsDirection(connectedNode, incomingNodes, outgoingNodes);
}


QByteArray DeBruijnNode::getFasta() const
{
    QByteArray fasta = ">";

    fasta += "NODE_";
    fasta += m_name;
    fasta += "_length_";
    fasta += QByteArray::number(getLength());
    fasta += "_cov_";
    fasta += QByteArray::number(getReadDepth());
    fasta += "\n";

    int charactersRemaining = m_sequence.length();
    int currentIndex = 0;
    while (charactersRemaining > 70)
    {
        fasta += m_sequence.mid(currentIndex, 70);
        fasta += "\n";
        charactersRemaining -= 70;
        currentIndex += 70;
    }
    fasta += m_sequence.mid(currentIndex);
    fasta += "\n";

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
            if (otherNode->isPositiveNode())
                otherNode->m_drawn = true;
            else
                otherNode->getReverseComplement()->m_drawn = true;
        }
        otherNode->labelNeighbouringNodesAsDrawn(nodeDistance-1, this);
    }
}



std::vector<BlastHitPart> DeBruijnNode::getBlastHitPartsForThisNode(double scaledNodeLength) const
{
    std::vector<BlastHitPart> returnVector;

    for (size_t i = 0; i < m_blastHits.size(); ++i)
    {
        std::vector<BlastHitPart> hitParts = m_blastHits[i]->getBlastHitParts(false, scaledNodeLength);
        returnVector.insert(returnVector.end(), hitParts.begin(), hitParts.end());
    }

    return returnVector;
}

std::vector<BlastHitPart> DeBruijnNode::getBlastHitPartsForThisNodeOrReverseComplement(double scaledNodeLength) const
{
    const DeBruijnNode * positiveNode = this;
    const DeBruijnNode * negativeNode = getReverseComplement();
    if (isNegativeNode())
        std::swap(positiveNode, negativeNode);

    //Look for blast hit parts on both the positive and the negative node,
    //since hits were previously filtered such that startPos < endPos,
    //hence we need to look at both positive and negative nodes to recover all hits.
    std::vector<BlastHitPart> returnVector;
    for (size_t i = 0; i < positiveNode->m_blastHits.size(); ++i)
    {
        std::vector<BlastHitPart> hitParts = positiveNode->m_blastHits[i]->getBlastHitParts(false, scaledNodeLength);
        returnVector.insert(returnVector.end(), hitParts.begin(), hitParts.end());
    }
    for (size_t i = 0; i < negativeNode->m_blastHits.size(); ++i)
    {
        std::vector<BlastHitPart> hitParts = negativeNode->m_blastHits[i]->getBlastHitParts(true, scaledNodeLength);
        returnVector.insert(returnVector.end(), hitParts.begin(), hitParts.end());
    }

    return returnVector;
}



bool DeBruijnNode::isPositiveNode() const
{
    QChar lastChar = m_name.at(m_name.length() - 1);
    return lastChar == '+';
}

bool DeBruijnNode::isNegativeNode() const
{
    QChar lastChar = m_name.at(m_name.length() - 1);
    return lastChar == '-';
}



//This function checks to see if the passed node leads into
//this node.  If so, it returns the connecting edge.  If not,
//it returns a null pointer.
DeBruijnEdge * DeBruijnNode::doesNodeLeadIn(DeBruijnNode * node) const
{
    for (size_t i = 0; i < m_edges.size(); ++i)
    {
        DeBruijnEdge * edge = m_edges[i];
        if (edge->getStartingNode() == node && edge->getEndingNode() == this)
            return edge;
    }
    return 0;
}

//This function checks to see if the passed node leads away from
//this node.  If so, it returns the connecting edge.  If not,
//it returns a null pointer.
DeBruijnEdge * DeBruijnNode::doesNodeLeadAway(DeBruijnNode * node) const
{
    for (size_t i = 0; i < m_edges.size(); ++i)
    {
        DeBruijnEdge * edge = m_edges[i];
        if (edge->getStartingNode() == this && edge->getEndingNode() == node)
            return edge;
    }
    return 0;
}


bool DeBruijnNode::isNodeConnected(DeBruijnNode * node) const
{
    for (size_t i = 0; i < m_edges.size(); ++i)
    {
        DeBruijnEdge * edge = m_edges[i];
        if (edge->getStartingNode() == node || edge->getEndingNode() == node)
            return true;
    }
    return false;
}



std::vector<DeBruijnEdge *> DeBruijnNode::getEnteringEdges() const
{
    std::vector<DeBruijnEdge *> returnVector;
    for (size_t i = 0; i < m_edges.size(); ++i)
    {
        DeBruijnEdge * edge = m_edges[i];
        if (this == edge->getEndingNode())
            returnVector.push_back(edge);
    }
    return returnVector;
}
std::vector<DeBruijnEdge *> DeBruijnNode::getLeavingEdges() const
{
    std::vector<DeBruijnEdge *> returnVector;
    for (size_t i = 0; i < m_edges.size(); ++i)
    {
        DeBruijnEdge * edge = m_edges[i];
        if (this == edge->getStartingNode())
            returnVector.push_back(edge);
    }
    return returnVector;
}



std::vector<DeBruijnNode *> DeBruijnNode::getDownstreamNodes() const
{
    std::vector<DeBruijnEdge *> leavingEdges = getLeavingEdges();

    std::vector<DeBruijnNode *> returnVector;
    for (size_t i = 0; i < leavingEdges.size(); ++i)
        returnVector.push_back(leavingEdges[i]->getEndingNode());

    return returnVector;
}


std::vector<DeBruijnNode *> DeBruijnNode::getUpstreamNodes() const
{
    std::vector<DeBruijnEdge *> enteringEdges = getEnteringEdges();

    std::vector<DeBruijnNode *> returnVector;
    for (size_t i = 0; i < enteringEdges.size(); ++i)
        returnVector.push_back(enteringEdges[i]->getStartingNode());

    return returnVector;
}



int DeBruijnNode::getBasePairsPerSegment() const
{
    if (g_settings->nodeLengthMode == AUTO_NODE_LENGTH)
        return g_settings->autoBasePairsPerSegment;
    else
        return g_settings->manualBasePairsPerSegment;
}
