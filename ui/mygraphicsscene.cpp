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


#include "mygraphicsscene.h"
#include "../graph/debruijnnode.h"
#include "../graph/debruijnedge.h"
#include "../graph/graphicsitemnode.h"
#include "../graph/graphicsitemedge.h"

MyGraphicsScene::MyGraphicsScene(QObject *parent) :
    QGraphicsScene(parent)
{
}


bool compareNodePointers(DeBruijnNode * a, DeBruijnNode * b)
{
    QString aName = a->m_name;
    QString bName = b->m_name;

    QString aName2 = aName;
    aName2.chop(1);
    QString bName2 = aName;
    bName2.chop(1);

    bool ok1, ok2;
    long long aNum = aName2.toLongLong(&ok1);
    long long bNum = bName2.toLongLong(&ok2);

    //If the node names are essentially numbers, then sort them as numbers
    if (ok1 && ok2 && aNum != bNum)
        return aNum < bNum;

    return aName > bName;
}


//This function returns all of the selected nodes, sorted by their node number.
std::vector<DeBruijnNode *> MyGraphicsScene::getSelectedNodes()
{
    std::vector<DeBruijnNode *> returnVector;

    QList<QGraphicsItem *> selection = selectedItems();
    for (int i = 0; i < selection.size(); ++i)
    {
        QGraphicsItem * selectedItem = selection[i];
        GraphicsItemNode * selectedNodeItem = dynamic_cast<GraphicsItemNode *>(selectedItem);
        if (selectedNodeItem != 0)
            returnVector.push_back(selectedNodeItem->m_deBruijnNode);
    }

    std::sort(returnVector.begin(), returnVector.end(), compareNodePointers);

    return returnVector;
}

//This function returns all of the selected graphics item nodes, unsorted.
std::vector<GraphicsItemNode *> MyGraphicsScene::getSelectedGraphicsItemNodes()
{
    std::vector<GraphicsItemNode *> returnVector;

    QList<QGraphicsItem *> selection = selectedItems();
    for (int i = 0; i < selection.size(); ++i)
    {
        QGraphicsItem * selectedItem = selection[i];
        GraphicsItemNode * selectedNodeItem = dynamic_cast<GraphicsItemNode *>(selectedItem);
        if (selectedNodeItem != 0)
            returnVector.push_back(selectedNodeItem);
    }

    return returnVector;
}


std::vector<DeBruijnEdge *> MyGraphicsScene::getSelectedEdges()
{
    std::vector<DeBruijnEdge *> returnVector;

    QList<QGraphicsItem *> selection = selectedItems();
    for (int i = 0; i < selection.size(); ++i)
    {
        QGraphicsItem * selectedItem = selection[i];
        GraphicsItemEdge * selectedEdgeItem = dynamic_cast<GraphicsItemEdge *>(selectedItem);
        if (selectedEdgeItem != 0)
            returnVector.push_back(selectedEdgeItem->m_deBruijnEdge);
    }

    return returnVector;
}



DeBruijnNode * MyGraphicsScene::getOneSelectedNode()
{
    std::vector<DeBruijnNode *> selectedNodes = getSelectedNodes();
    if (selectedNodes.size() == 0)
        return 0;
    else
        return selectedNodes[0];
}

DeBruijnEdge * MyGraphicsScene::getOneSelectedEdge()
{
    std::vector<DeBruijnEdge *> selectedEdges = getSelectedEdges();
    if (selectedEdges.size() == 0)
        return 0;
    else
        return selectedEdges[0];
}

double MyGraphicsScene::getTopZValue()
{
    double topZ = 0.0;

    QList<QGraphicsItem *> allItems = items();
    for (int i = 0; i < allItems.size(); ++i)
    {
        QGraphicsItem * item = allItems[i];
        if (i == 0)
            topZ = item->zValue();
        else
        {
            double z = item->zValue();
            if (z > topZ)
                topZ = z;
        }
    }

    return topZ;
}
