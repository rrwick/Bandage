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
#include <QStringList>
#include "../program/globals.h"
#include "graphlocation.h"

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
    static Path makeFromString(QString pathString, bool circular,
                               QList<DeBruijnNode *> * nodesInGraph,
                               QStringList * nodesNotInGraph,
                               PathStringFailure * pathStringFailure);

    static QList<Path> getAllPossiblePaths(GraphLocation startLocation,
                                           GraphLocation endLocation,
                                           int nodeSearchDepth,
                                           int minDistance, int maxDistance);

    QList<DeBruijnNode *> m_nodes;
    QList<DeBruijnEdge *> m_edges;

    GraphLocation m_startLocation;
    GraphLocation m_endLocation;

    bool addNode(DeBruijnNode * newNode, bool strandSpecific);
    bool isEmpty() {return m_nodes.empty();}
    bool isCircular();
    QByteArray getPathSequence();
    QString getFasta();
    bool checkForOtherEdges();
    QString getString(bool spaces) const;
    int getLength();
    QList<Path> extendPathInAllPossibleWays();

    bool canNodeFitOnEnd(DeBruijnNode * node, Path * extendedPath);
    bool canNodeFitAtStart(DeBruijnNode * node, Path * extendedPath);

    QList<BlastHit *> getBlastHitsForQuery(BlastQuery * query);
    double getMeanReadDepth();
    
    bool areIdentical(Path other);
    bool haveSameNodes(Path other);
    bool hasNodeSubset(Path other);

    void extendPathToIncludeEntirityOfNodes();

private:
    void buildUnambiguousPathFromNodes(QList<DeBruijnNode *> nodes,
                                       bool strandSpecific);
};

#endif // PATH_H
