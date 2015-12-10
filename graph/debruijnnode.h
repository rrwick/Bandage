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
    //CREATORS
    DeBruijnNode(QString name, double readDepth, QByteArray sequence, int length = 0);
    ~DeBruijnNode();

    //ACCESSORS
    QString getName() const {return m_name;}
    QString getNameWithoutSign() const {return m_name.left(m_name.length() - 1);}
    QString getSign() const {if (m_name.length() > 0) return m_name.right(1); else return "+";}
    double getReadDepth() const {return m_readDepth;}
    double getReadDepthRelativeToMeanDrawnReadDepth() const {return m_readDepthRelativeToMeanDrawnReadDepth;}
    QByteArray getSequence() const;
    int getLength() const {return m_length;}
    QByteArray getSequenceForGfa() const;
    int getFullLength() const;
    QByteArray getFasta() const;
    QByteArray getFastaNoNewLinesInSequence() const;
    QByteArray getGfaSegmentLine() const;
    char getBaseAt(int i) const {if (i >= 0 && i < m_sequence.length()) return m_sequence.at(i); else return '\0';}
    ContiguityStatus getContiguityStatus() const {return m_contiguityStatus;}
    DeBruijnNode * getReverseComplement() const {return m_reverseComplement;}
    OgdfNode * getOgdfNode() const {return m_ogdfNode;}
    GraphicsItemNode * getGraphicsItemNode() const {return m_graphicsItemNode;}
    bool thisOrReverseComplementHasGraphicsItemNode() const {return (m_graphicsItemNode != 0 || getReverseComplement()->m_graphicsItemNode != 0);}
    bool hasGraphicsItem() const {return m_graphicsItemNode != 0;}
    const std::vector<DeBruijnEdge *> * getEdgesPointer() const {return &m_edges;}
    std::vector<DeBruijnEdge *> getEnteringEdges() const;
    std::vector<DeBruijnEdge *> getLeavingEdges() const;
    std::vector<DeBruijnNode *> getDownstreamNodes() const;
    std::vector<DeBruijnNode *> getUpstreamNodes() const;
    std::vector<DeBruijnNode *> getAllConnectedPositiveNodes() const;
    bool isSpecialNode() const {return m_specialNode;}
    bool isDrawn() const {return m_drawn;}
    bool isNotDrawn() const {return !m_drawn;}
    QColor getCustomColour() const {return m_customColour;}
    QString getCustomLabel() const {return m_customLabel;}
    bool isPositiveNode() const;
    bool isNegativeNode() const;
    bool inOgdf() const {return m_ogdfNode != 0;}
    bool notInOgdf() const {return m_ogdfNode == 0;}
    bool thisOrReverseComplementInOgdf() const {return (inOgdf() || getReverseComplement()->inOgdf());}
    bool thisOrReverseComplementNotInOgdf() const {return !thisOrReverseComplementInOgdf();}
    bool isNodeConnected(DeBruijnNode * node) const;
    const std::vector<BlastHit *> * getBlastHitsPointer() const {return &m_blastHits;}
    bool thisNodeHasBlastHits() const {return m_blastHits.size() > 0;}
    bool thisNodeOrReverseComplementHasBlastHits() const {return m_blastHits.size() > 0 || getReverseComplement()->m_blastHits.size() > 0;}
    DeBruijnEdge * doesNodeLeadIn(DeBruijnNode * node) const;
    DeBruijnEdge * doesNodeLeadAway(DeBruijnNode * node) const;
    std::vector<BlastHitPart> getBlastHitPartsForThisNode(double scaledNodeLength) const;
    std::vector<BlastHitPart> getBlastHitPartsForThisNodeOrReverseComplement(double scaledNodeLength) const;
    bool hasCsvData() const {return !m_csvData.isEmpty();}
    QStringList getAllCsvData() const {return m_csvData;}
    QString getCsvLine(int i) const {if (i < m_csvData.length()) return m_csvData[i]; else return "";}
    bool isInReadDepthRange(double min, double max) const;
    bool sequenceIsMissing() const;
    DeBruijnEdge *getSelfLoopingEdge() const;
    int getDeadEndCount() const;

    //MODIFERS
    void setReadDepthRelativeToMeanDrawnReadDepth(double newVal) {m_readDepthRelativeToMeanDrawnReadDepth = newVal;}
    void appendToSequence(QByteArray additionalSeq) {m_sequence.append(additionalSeq); m_length = m_sequence.length();}
    void upgradeContiguityStatus(ContiguityStatus newStatus);
    void resetContiguityStatus() {m_contiguityStatus = NOT_CONTIGUOUS;}
    void setReverseComplement(DeBruijnNode * rc) {m_reverseComplement = rc;}
    void setGraphicsItemNode(GraphicsItemNode * gin) {m_graphicsItemNode = gin;}
    void setAsSpecial() {m_specialNode = true;}
    void setAsNotSpecial() {m_specialNode = false;}
    void setAsDrawn() {m_drawn = true;}
    void setAsNotDrawn() {m_drawn = false;}
    void setCustomColour(QColor newColour) {m_customColour = newColour;}
    void setCustomLabel(QString newLabel) {m_customLabel = newLabel;}
    void resetNode();
    void addEdge(DeBruijnEdge * edge);
    void removeEdge(DeBruijnEdge * edge);
    void addToOgdfGraph(ogdf::Graph * ogdfGraph);
    void determineContiguity();
    void clearBlastHits() {m_blastHits.clear();}
    void addBlastHit(BlastHit * newHit) {m_blastHits.push_back(newHit);}
    void labelNeighbouringNodesAsDrawn(int nodeDistance, DeBruijnNode * callingNode);
    void setCsvData(QStringList csvData) {m_csvData = csvData;}
    void clearCsvData() {m_csvData.clear();}
    void setReadDepth(double newReadDepth) {m_readDepth = newReadDepth;}
    void setName(QString newName) {m_name = newName;}

private:
    QString m_name;
    double m_readDepth;
    double m_readDepthRelativeToMeanDrawnReadDepth;
    QByteArray m_sequence;
    int m_length;
    ContiguityStatus m_contiguityStatus;
    DeBruijnNode * m_reverseComplement;
    OgdfNode * m_ogdfNode;
    GraphicsItemNode * m_graphicsItemNode;
    std::vector<DeBruijnEdge *> m_edges;
    bool m_specialNode;
    bool m_drawn;
    int m_highestDistanceInNeighbourSearch;
    QColor m_customColour;
    QString m_customLabel;
    std::vector<BlastHit *> m_blastHits;
    QStringList m_csvData;
    QString getNodeNameForFasta() const;
    QByteArray getUpstreamSequence(int upstreamSequenceLength) const;

    int getBasePairsPerSegment() const;
    bool isOnlyPathInItsDirection(DeBruijnNode * connectedNode,
                                  std::vector<DeBruijnNode *> * incomingNodes,
                                  std::vector<DeBruijnNode *> * outgoingNodes) const;
    bool isNotOnlyPathInItsDirection(DeBruijnNode * connectedNode,
                                     std::vector<DeBruijnNode *> * incomingNodes,
                                     std::vector<DeBruijnNode *> * outgoingNodes) const;
    std::vector<DeBruijnNode *> getNodesCommonToAllPaths(std::vector< std::vector <DeBruijnNode *> > * paths,
                                                         bool includeReverseComplements) const;
    bool doesPathLeadOnlyToNode(DeBruijnNode * node, bool includeReverseComplement);
};

#endif // DEBRUIJNNODE_H
