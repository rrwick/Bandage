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
    //CREATORS
    Path() {}
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

    //ACCESSORS
    bool isEmpty() const {return m_nodes.empty();}
    bool isCircular() const;
    bool areIdentical(Path other) const;
    bool haveSameNodes(Path other) const;
    bool hasNodeSubset(Path other) const;
    QByteArray getPathSequence() const;
    QString getFasta() const;
    QString getString(bool spaces) const;
    int getLength() const;
    QList<Path> extendPathInAllPossibleWays() const;
    bool canNodeFitOnEnd(DeBruijnNode * node, Path * extendedPath) const;
    bool canNodeFitAtStart(DeBruijnNode * node, Path * extendedPath) const;
    QList<BlastHit *> getBlastHitsForQuery(BlastQuery * query) const;
    double getMeanReadDepth() const;

    //MODIFERS
    bool addNode(DeBruijnNode * newNode, bool strandSpecific);
    void extendPathToIncludeEntirityOfNodes();

    //STATIC
    static QList<Path> getAllPossiblePaths(GraphLocation startLocation,
                                           GraphLocation endLocation,
                                           int nodeSearchDepth,
                                           int minDistance, int maxDistance);

private:
    GraphLocation m_startLocation;
    GraphLocation m_endLocation;
    QList<DeBruijnNode *> m_nodes;
    QList<DeBruijnEdge *> m_edges;

    void buildUnambiguousPathFromNodes(QList<DeBruijnNode *> nodes,
                                       bool strandSpecific);
    bool checkForOtherEdges();
};

#endif // PATH_H
