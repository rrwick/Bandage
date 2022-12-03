//Copyright 2017 Ryan Wick

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


#ifndef ASSEMBLYFOREST_H
#define ASSEMBLYFOREST_H

#include <QObject>
#include <vector>

#include "../ogdf/basic/Graph.h"
#include "../ogdf/basic/GraphAttributes.h"
#pragma once

#include <QString>
#include <QMap>
#include "../program/globals.h"
#include "../ui/mygraphicsscene.h"
#include <QPair>
#include "RandomForestEdge.h"
#include "RandomForestNode.h"
#include "../program/globals.h"

class RandomForestNode;
class RandomForestEdge;
class MyProgressDialog;
class OgdfNode;

class AssemblyForest : public QObject
{
    Q_OBJECT

public:
    AssemblyForest();
    ~AssemblyForest();
    bool loadRandomForestFromFile(QString filename, QString* errormsg);
    void AssemblyForest::buildOgdfGraphFromNodesAndEdges();

    QMap<QString, RandomForestNode*> m_roots;
    QMap<QString, RandomForestNode*> m_nodes;
    QMap<QPair<RandomForestNode*, RandomForestNode*>, RandomForestEdge*> m_edges;
    QMap<QString, int> m_classes;

    ogdf::Graph* m_ogdfGraph;
    ogdf::EdgeArray<double>* m_edgeArray;
    ogdf::GraphAttributes* m_graphAttributes;
    void cleanUp();
    void addGraphicsItemsToScene(MyGraphicsScene* scene);
    void recalculateAllNodeWidths();
    void addClass(QString className);
    void resetAllNodeColours();
    QString getClassFigureInfo();

private:

    void processChild(RandomForestNode* parent, QString childName);
    void addEdgeToOgdfGraph(RandomForestNode* startNode, RandomForestNode* lastNode);


};

#endif // ASSEMBLYFOREST_H
