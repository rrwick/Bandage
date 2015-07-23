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

#include <QByteArray>
#include <QList>
#include <vector>
#include <QString>
#include "../program/globals.h"

class DeBruijnNode;
class DeBruijnEdge;

class BlastHit;
class BlastQuery;

class Path
{
public:
    Path() {}

    //Named constructors
    static Path makeFromUnorderedNodes(QList<DeBruijnNode *> nodes,
                                       bool strandSpecific);
    static Path makeFromUnorderedNodes(std::vector<DeBruijnNode *> nodes,
                                       bool strandSpecific);
    static Path makeFromOrderedNodes(QList<DeBruijnNode *> nodes,
                                     bool circular);


    static QList<Path> getAllPossiblePaths(DeBruijnNode * startNode,
                                           int startPosition,
                                           DeBruijnNode * endNode,
                                           int endPosition, int nodeSearchDepth,
                                           int minDistance, int maxDistance);

    QList<DeBruijnNode *> m_nodes;
    QList<DeBruijnEdge *> m_edges;

    //If start/end type is PART_OF_NODE, then the Path sequence can begin and
    //end in particular indices of the first and last node.  These indices are
    //1-based, i.e. if m_startPosition is 1 the entire first node is included.
    PathStartEndType m_startType;
    int m_startPosition;
    PathStartEndType m_endType;
    int m_endPosition;

    bool addNode(DeBruijnNode * newNode, bool strandSpecific);
    bool isEmpty() {return m_nodes.empty();}
    bool isCircular();
    QByteArray getPathSequence();
    QString getFasta();
    bool checkForOtherEdges();
    QString getString();
    int getLength();
    QList<Path> extendPathInAllPossibleWays();

    bool canNodeFitOnEnd(DeBruijnNode * node);
    bool canNodeFitAtStart(DeBruijnNode * node);

    QList<BlastHit *> getBlastHitsForQuery(BlastQuery * query);
    double getMeanReadDepth();
    
    bool areIdentical(Path other);
    bool hasNodeSubset(Path other);

private:
    void buildUnambiguousPathFromNodes(QList<DeBruijnNode *> nodes,
                                       bool strandSpecific);
};

#endif // PATH_H
