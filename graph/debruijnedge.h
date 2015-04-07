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


#ifndef DEBRUIJNEDGE_H
#define DEBRUIJNEDGE_H

#include <ogdf/basic/Graph.h>
#include "debruijnnode.h"

class GraphicsItemEdge;

class DeBruijnEdge
{
public:
    DeBruijnEdge(DeBruijnNode * startingNode, DeBruijnNode * endingNode);

    DeBruijnNode * m_startingNode;
    DeBruijnNode * m_endingNode;
    GraphicsItemEdge * m_graphicsItemEdge;
    DeBruijnEdge * m_reverseComplement;
    bool m_drawn;

    bool isStartingNode(DeBruijnNode * node) {return node == m_startingNode;}
    DeBruijnNode * getOtherNode(DeBruijnNode * node);
    void addToOgdfGraph(ogdf::Graph * ogdfGraph);
    void reset() {m_graphicsItemEdge = 0; m_drawn = false;}
    void determineIfDrawn() {m_drawn = edgeIsVisible();}
    void tracePaths(bool forward,
                    int stepsRemaining,
                    std::vector<std::vector<DeBruijnNode *> > * allPaths,
                    std::vector<DeBruijnNode *> pathSoFar = std::vector<DeBruijnNode *>());

private:
    bool edgeIsVisible();

};

#endif // DEBRUIJNEDGE_H
