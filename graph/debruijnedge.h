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


#ifndef DEBRUIJNEDGE_H
#define DEBRUIJNEDGE_H

#include "../ogdf/basic/Graph.h"
#include "debruijnnode.h"

class GraphicsItemEdge;

class DeBruijnEdge
{
public:
    //CREATORS
    DeBruijnEdge(DeBruijnNode * startingNode, DeBruijnNode * endingNode);

    //ACCESSORS
    bool isStartingNode(DeBruijnNode * node) const {return node == m_startingNode;}
    DeBruijnNode * getStartingNode() const {return m_startingNode;}
    DeBruijnNode * getEndingNode() const {return m_endingNode;}
    GraphicsItemEdge * getGraphicsItemEdge() const {return m_graphicsItemEdge;}
    DeBruijnEdge * getReverseComplement() const {return m_reverseComplement;}
    bool isDrawn() const {return m_drawn;}
    int getOverlap() const {return m_overlap;}
    EdgeOverlapType getOverlapType() const {return m_overlapType;}
    DeBruijnNode * getOtherNode(const DeBruijnNode * node) const;
    bool testExactOverlap(int overlap) const;
    void tracePaths(bool forward,
                    int stepsRemaining,
                    std::vector<std::vector<DeBruijnNode *> > * allPaths,
                    DeBruijnNode * startingNode,
                    std::vector<DeBruijnNode *> pathSoFar = std::vector<DeBruijnNode *>()) const;
    bool leadsOnlyToNode(bool forward,
                         int stepsRemaining,
                         DeBruijnNode * target,
                         std::vector<DeBruijnNode *> pathSoFar,
                         bool includeReverseComplement) const;
    QByteArray getGfaLinkLine() const;
    bool isPositiveEdge() const;
    bool isNegativeEdge() const {return !isPositiveEdge();}
    bool isOwnReverseComplement() const {return this == getReverseComplement();}
    static bool compareEdgePointers(DeBruijnEdge * a, DeBruijnEdge * b);

    //MODIFERS
    void setGraphicsItemEdge(GraphicsItemEdge * gie) {m_graphicsItemEdge = gie;}
    void setReverseComplement(DeBruijnEdge * rc) {m_reverseComplement = rc;}
    void setOverlap(int ol) {m_overlap = ol;}
    void setOverlapType(EdgeOverlapType olt) {m_overlapType = olt;}
    void reset() {m_graphicsItemEdge = 0; m_drawn = false;}
    void determineIfDrawn() {m_drawn = edgeIsVisible();}
    void setExactOverlap(int overlap) {m_overlap = overlap; m_overlapType = EXACT_OVERLAP;}
    void autoDetermineExactOverlap();
    void addToOgdfGraph(ogdf::Graph * ogdfGraph, ogdf::EdgeArray<double> * edgeArray) const;

private:
    DeBruijnNode * m_startingNode;
    DeBruijnNode * m_endingNode;
    GraphicsItemEdge * m_graphicsItemEdge;
    DeBruijnEdge * m_reverseComplement;
    bool m_drawn;
    EdgeOverlapType m_overlapType;
    int m_overlap;

    bool edgeIsVisible() const;
    int timesNodeInPath(DeBruijnNode * node, std::vector<DeBruijnNode *> * path) const;
    std::vector<DeBruijnEdge *> findNextEdgesInPath(DeBruijnNode * nextNode,
                                                    bool forward) const;
};

#endif // DEBRUIJNEDGE_H
