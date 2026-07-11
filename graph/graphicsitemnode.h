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


#ifndef GRAPHICSITEMNODE_H
#define GRAPHICSITEMNODE_H

#include <QGraphicsItem>
#include <QGraphicsSceneMouseEvent>
#include "../ogdf/basic/GraphAttributes.h"
#include <vector>
#include <QPointF>
#include <QColor>
#include <QFont>
#include <QString>
#include <QPainterPath>
#include <QStringList>
#include <algorithm>
#include <iostream>
#include "../painting/CommonGraphicsItemNode.h"

class DeBruijnNode;

class GraphicsItemNode : public CommonGraphicsItemNode
{
public:
    GraphicsItemNode(DeBruijnNode * deBruijnNode,
                     ogdf::GraphAttributes * graphAttributes,
                     CommonGraphicsItemNode* parent = 0);
    GraphicsItemNode(DeBruijnNode * deBruijnNode,
                     GraphicsItemNode * toCopy,
                     CommonGraphicsItemNode* parent = 0);
    GraphicsItemNode(DeBruijnNode * deBruijnNode,
                     std::vector<QPointF> linePoints,
                     CommonGraphicsItemNode* parent = 0);

    DeBruijnNode * m_deBruijnNode;

    void mousePressEvent(QGraphicsSceneMouseEvent * event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent * event);
    void paint(QPainter * painter, const QStyleOptionGraphicsItem *, QWidget *);
    QPainterPath shape() const;

    bool usePositiveNodeColour();
    void setNodeColour();
    QColor propagateColour();
    QStringList getNodeText();
    QSize getNodeTextSize(QString text);
    QColor getDepthColour();
    void setWidth();
    QRectF boundingRect() const;
    static double getNodeWidth(double depthRelativeToMeanDrawnDepth,
                               double depthPower,
                               double depthEffectOnWidth,
                               double averageNodeWidth);
    void getBlastHitsTextAndLocationThisNode(std::vector<QString> * blastHitText,
                                             std::vector<QPointF> * blastHitLocation);
    void getBlastHitsTextAndLocationThisNodeOrReverseComplement(std::vector<QString> * blastHitText,
                                                                std::vector<QPointF> * blastHitLocation);
    void fixEdgePaths(std::vector<GraphicsItemNode *> * nodes = 0);
    void mouseRoundEvent(QGraphicsSceneMouseEvent* event);

private:
    void exactPathHighlightNode(QPainter* painter);
    void queryPathHighlightNode(QPainter* painter);
    void pathHighlightNode2(QPainter* painter, DeBruijnNode* node, bool reverse, Path* path);
    void pathHighlightNode3(QPainter* painter, QPainterPath highlightPath);
    QPainterPath buildPartialHighlightPath(double startFraction, double endFraction, bool reverse);
    bool anyNodeDisplayText();
    
};

#endif // GRAPHICSITEMNODE_H
