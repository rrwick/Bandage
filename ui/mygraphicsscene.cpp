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
    long long aNum = a->m_number;
    long long bNum = b->m_number;

    if (llabs(aNum) != llabs(bNum))
        return (llabs(aNum) < llabs(bNum));

    return aNum > bNum;
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
    double topZ;

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
