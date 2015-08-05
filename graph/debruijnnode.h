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


#ifndef DEBRUIJNNODE_H
#define DEBRUIJNNODE_H

#include <QByteArray>
#include <vector>
#include <ogdf/basic/Graph.h>
#include "../program/globals.h"
#include <QColor>
#include "../blast/blasthitpart.h"

class OgdfNode;
class DeBruijnEdge;
class GraphicsItemNode;
class BlastHit;

class DeBruijnNode
{
public:
    DeBruijnNode(QString name, double readDepth, QByteArray sequence);
    ~DeBruijnNode();

    QString m_name;
    double m_readDepth;
    double m_readDepthRelativeToMeanDrawnReadDepth;
    QByteArray m_sequence;
    ContiguityStatus m_contiguityStatus;
    DeBruijnNode * m_reverseComplement;
    OgdfNode * m_ogdfNode;
    GraphicsItemNode * m_graphicsItemNode;
    std::vector<DeBruijnEdge *> m_edges;
    bool m_startingNode;
    bool m_drawn;
    int m_highestDistanceInNeighbourSearch;
    QColor m_customColour;
    QString m_customLabel;
    std::vector<BlastHit *> m_blastHits;

    int getLength() {return m_sequence.length();}
    void addEdge(DeBruijnEdge * edge);
    void resetNode();
    bool hasGraphicsItem() {return m_graphicsItemNode != 0;}
    bool inOgdf() {return m_ogdfNode != 0;}
    bool notInOgdf() {return !inOgdf();}
    bool thisOrReverseComplementInOgdf() {return (inOgdf() || m_reverseComplement->inOgdf());}
    bool thisOrReverseComplementNotInOgdf() {return !thisOrReverseComplementInOgdf();}
    void addToOgdfGraph(ogdf::Graph * ogdfGraph);
    void determineContiguity();
    void setContiguityStatus(ContiguityStatus newStatus);
    bool isOnlyPathInItsDirection(DeBruijnNode * connectedNode,
                                  std::vector<DeBruijnNode *> * incomingNodes,
                                  std::vector<DeBruijnNode *> * outgoingNodes);
    bool isNotOnlyPathInItsDirection(DeBruijnNode * connectedNode,
                                     std::vector<DeBruijnNode *> * incomingNodes,
                                     std::vector<DeBruijnNode *> * outgoingNodes)
    {return !isOnlyPathInItsDirection(connectedNode, incomingNodes, outgoingNodes);}
    QByteArray getFasta();
    void labelNeighbouringNodesAsDrawn(int nodeDistance, DeBruijnNode * callingNode);

    bool thisNodeHasBlastHits();
    bool thisNodeOrReverseComplementHasBlastHits();
    std::vector<BlastHitPart> getBlastHitPartsForThisNode(double scaledNodeLength);
    std::vector<BlastHitPart> getBlastHitPartsForThisNodeOrReverseComplement(double scaledNodeLength);
    std::vector<DeBruijnNode *> getNodesCommonToAllPaths(std::vector< std::vector <DeBruijnNode *> > * paths,
                                                         bool includeReverseComplements);
    bool doesPathLeadOnlyToNode(DeBruijnNode * node, bool includeReverseComplement);
    bool isPositiveNode();
    bool isNegativeNode();

    DeBruijnEdge * doesNodeLeadIn(DeBruijnNode * node);
    DeBruijnEdge * doesNodeLeadAway(DeBruijnNode * node);
    bool isNodeConnected(DeBruijnNode * node);

    std::vector<DeBruijnEdge *> getEnteringEdges();
    std::vector<DeBruijnEdge *> getLeavingEdges();

    std::vector<DeBruijnNode *> getDownstreamNodes();
    std::vector<DeBruijnNode *> getUpstreamNodes();

private:
    int getBasePairsPerSegment();
};

#endif // DEBRUIJNNODE_H
