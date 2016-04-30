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


#ifndef GRAPHICSITEMEDGE_H
#define GRAPHICSITEMEDGE_H

#include <QGraphicsPathItem>
#include <QPainterPath>


#include "../ogdf/basic/Graph.h"
#include "../ogdf/basic/GraphAttributes.h"
#include <QPointF>

class DeBruijnEdge;

class GraphicsItemEdge : public QGraphicsPathItem
{
public:
    GraphicsItemEdge(DeBruijnEdge * deBruijnEdge, QGraphicsItem * parent = 0);

    DeBruijnEdge * m_deBruijnEdge;
    QPointF m_startingLocation;
    QPointF m_endingLocation;
    QPointF m_beforeStartingLocation;
    QPointF m_afterEndingLocation;
    QPointF m_controlPoint1;
    QPointF m_controlPoint2;

    void paint(QPainter * painter, const QStyleOptionGraphicsItem *, QWidget *);
    QPainterPath shape() const;
    QPointF extendLine(QPointF start, QPointF end, double extensionLength);
    void calculateAndSetPath();
    void setControlPointLocations();
    void setStartingPoints(QPointF startingLocation, QPointF beforeStartingLocation) {m_startingLocation = startingLocation; m_beforeStartingLocation = beforeStartingLocation;}
    void setEndingPoints(QPointF endingLocation, QPointF afterEndingLocation) {m_endingLocation = endingLocation; m_afterEndingLocation = afterEndingLocation;}
    void makeSpecialPathConnectingNodeToSelf();
    void makeSpecialPathConnectingNodeToReverseComplement();
};

#endif // GRAPHICSITEMEDGE_H
