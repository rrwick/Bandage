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


#include "mygraphicsscene.h"
#include "../graph/debruijnnode.h"
#include "../graph/debruijnedge.h"
#include "../graph/graphicsitemnode.h"
#include "../graph/graphicsitemedge.h"
#include "../graph/debruijnnode.h"

MyGraphicsScene::MyGraphicsScene(QObject *parent) :
    QGraphicsScene(parent)
{
}


bool compareNodePointers(DeBruijnNode * a, DeBruijnNode * b)
{
    QString aName = a->getName();
    QString bName = b->getName();

    QString aName2 = aName;
    aName2.chop(1);
    QString bName2 = bName;
    bName2.chop(1);

    bool ok1, ok2;
    long long aNum = aName2.toLongLong(&ok1);
    long long bNum = bName2.toLongLong(&ok2);

    //If the node names are essentially numbers, then sort them as numbers
    if (ok1 && ok2 && aNum != bNum)
        return aNum < bNum;

    return aName < bName;
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


//This function works like getSelectedNodes, but only positive nodes are
//returned.  If a negative node is selected, its positive complement is in the
//results.  If both nodes in a pair are selected, then only the positive node
//of the pair is in the results.
std::vector<DeBruijnNode *> MyGraphicsScene::getSelectedPositiveNodes()
{
    std::vector<DeBruijnNode *> selectedNodes = getSelectedNodes();

    //First turn all of the nodes to positive nodes.
    std::vector<DeBruijnNode *> allPositive;
    for (size_t i = 0; i < selectedNodes.size(); ++i)
    {
        DeBruijnNode * node = selectedNodes[i];
        if (node->isNegativeNode())
            node = node->getReverseComplement();
        allPositive.push_back(node);
    }

    //Now remove duplicates.  Since the nodes are sorted, all duplicates should
    //be adjacent to each other.
    std::vector<DeBruijnNode *> uniquePositive;
    for (size_t i = 0; i < allPositive.size(); ++i)
    {
        DeBruijnNode * node = allPositive[i];
        DeBruijnNode * previousNode = 0;
        if (i > 0)
            previousNode = allPositive[i-1];
        if (node != previousNode)
            uniquePositive.push_back(node);
    }

    return uniquePositive;
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

//This function, like getOneSelectedNode, returns a single selected node from
//graph and a 0 if it can't be done.  However, it will always return the
//positive node in the pair, and if two complementary nodes are selected, it
//will still work.
DeBruijnNode * MyGraphicsScene::getOnePositiveSelectedNode()
{
    std::vector<DeBruijnNode *> selectedNodes = getSelectedNodes();
    if (selectedNodes.size() == 0)
        return 0;

    else if (selectedNodes.size() == 1)
    {
        DeBruijnNode * selectedNode = selectedNodes[0];
        if (selectedNode->isPositiveNode())
            return selectedNode;
        else
            return selectedNode->getReverseComplement();
    }

    else if (selectedNodes.size() == 2)
    {
        DeBruijnNode * selectedNode1 = selectedNodes[0];
        DeBruijnNode * selectedNode2 = selectedNodes[1];
        if (selectedNode1->getReverseComplement() == selectedNode2)
        {
            if (selectedNode1->isPositiveNode())
                return selectedNode1;
            else
                return selectedNode2;
        }
        else
            return 0;
    }

    return 0;
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


//Expands the scene rectangle a bit beyond the items so they aren't drawn right to the edge.
void MyGraphicsScene::setSceneRectangle()
{
    QRectF boundingRect = itemsBoundingRect();
    double width = boundingRect.width();
    double height = boundingRect.height();
    double margin = std::max(width, height);
    margin *= 0.05; //5% margin

    setSceneRect(boundingRect.left() - margin, boundingRect.top() - margin,
                 width + 2 * margin, height + 2 * margin);
}



//After the user drags nodes, it may be necessary to expand the scene rectangle
//if the nodes were moved out of the existing rectangle.
void MyGraphicsScene::possiblyExpandSceneRectangle(std::vector<GraphicsItemNode *> * movedNodes)
{
    QRectF currentSceneRect = sceneRect();
    QRectF newSceneRect = currentSceneRect;

    for (size_t i = 0; i < movedNodes->size(); ++i)
    {
        GraphicsItemNode * node = (*movedNodes)[i];
        QRectF nodeRect = node->boundingRect();
        newSceneRect = newSceneRect.united(nodeRect);
    }

    if (newSceneRect != currentSceneRect)
        setSceneRect(newSceneRect);
}

