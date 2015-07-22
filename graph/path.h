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


#ifndef PATH_H
#define PATH_H

#include "debruijnnode.h"
#include "debruijnedge.h"
#include <QByteArray>
#include <QList>
#include <vector>
#include <QString>

class Path
{
public:
    Path() {}

    //Named constructors
    static Path makeFromUnorderedNodes(QList<DeBruijnNode *> nodes, bool strandSpecific);
    static Path makeFromUnorderedNodes(std::vector<DeBruijnNode *> nodes, bool strandSpecific);
    static Path makeFromOrderedNodes(QList<DeBruijnNode *> nodes, bool circular);

    QList<DeBruijnNode *> m_nodes;
    QList<DeBruijnEdge *> m_edges;

    int m_startPosition;
    int m_endPosition;

    bool addNode(DeBruijnNode * newNode, bool strandSpecific);
    bool isEmpty() {return m_nodes.empty();}
    bool isCircular();
    QByteArray getPathSequence();
    QString getFasta();
    bool checkForOtherEdges();

    bool canNodeFitOnEnd(DeBruijnNode * node);
    bool canNodeFitAtStart(DeBruijnNode * node);

private:
    void buildUnambiguousPathFromNodes(QList<DeBruijnNode *> nodes, bool strandSpecific);
};

#endif // PATH_H
