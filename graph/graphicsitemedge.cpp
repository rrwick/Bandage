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


#include "graphicsitemedge.h"
#include "debruijnedge.h"
#include <QPainterPathStroker>
#include <QPainter>
#include <QPen>
#include "../program/globals.h"
#include "../program/settings.h"
#include "debruijnnode.h"
#include "ogdfnode.h"
#include <QLineF>
#include "graphicsitemnode.h"

GraphicsItemEdge::GraphicsItemEdge(DeBruijnEdge * deBruijnEdge, QGraphicsItem * parent) :
    QGraphicsPathItem(parent), m_deBruijnEdge(deBruijnEdge)

{
    calculateAndSetPath();
}



QPointF GraphicsItemEdge::extendLine(QPointF start, QPointF end, double extensionLength)
{
    double extensionRatio = extensionLength / QLineF(start, end).length();
    QPointF difference = end - start;
    difference *= extensionRatio;
    return end + difference;
}


void GraphicsItemEdge::paint(QPainter * painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    double edgeWidth = g_settings->edgeWidth;
    QColor penColour;
    if (isSelected())
        penColour = g_settings->selectionColour;
    else
        penColour = g_settings->edgeColour;
    QPen edgePen(QBrush(penColour), edgeWidth, Qt::SolidLine, Qt::RoundCap);
    painter->setPen(edgePen);
    painter->drawPath(path());
}



QPainterPath GraphicsItemEdge::shape() const
{
    QPainterPathStroker stroker;
    stroker.setWidth(g_settings->edgeWidth);
    stroker.setCapStyle(Qt::RoundCap);
    stroker.setJoinStyle(Qt::RoundJoin);
    return stroker.createStroke(path());
}



void GraphicsItemEdge::calculateAndSetPath()
{
    setControlPointLocations();

    double edgeDistance = QLineF(m_startingLocation, m_endingLocation).length();

    double extensionLength = g_settings->segmentLength;
    if (extensionLength > edgeDistance / 2.0)
        extensionLength = edgeDistance / 2.0;

    m_controlPoint1 = extendLine(m_beforeStartingLocation, m_startingLocation, extensionLength);
    m_controlPoint2 = extendLine(m_afterEndingLocation, m_endingLocation, extensionLength);


    //If this edge is connecting a node to itself, and that node
    //is made of only one line segment, then a special path is
    //required, otherwise the edge will be mostly hidden underneath
    //the node.
    DeBruijnNode * startingNode = m_deBruijnEdge->m_startingNode;
    if (startingNode == m_deBruijnEdge->m_endingNode)
    {
        GraphicsItemNode * graphicsItemNode = startingNode->m_graphicsItemNode;
        if (graphicsItemNode == 0)
            graphicsItemNode = startingNode->m_reverseComplement->m_graphicsItemNode;
        if (graphicsItemNode != 0 && graphicsItemNode->m_linePoints.size() == 2)
        {
            makeSpecialPathConnectingNodeToSelf();
            return;
        }
    }

    //Otherwise, the path is just a single cubic Bezier curve.
    QPainterPath path;
    path.moveTo(m_startingLocation);
    path.cubicTo(m_controlPoint1, m_controlPoint2, m_endingLocation);

    setPath(path);
}

void GraphicsItemEdge::setControlPointLocations()
{
    DeBruijnNode * startingNode = m_deBruijnEdge->m_startingNode;
    DeBruijnNode * endingNode = m_deBruijnEdge->m_endingNode;

    if (startingNode->hasGraphicsItem())
    {
        m_startingLocation = startingNode->m_graphicsItemNode->getLast();
        m_beforeStartingLocation = startingNode->m_graphicsItemNode->getSecondLast();
    }
    else if (startingNode->m_reverseComplement->hasGraphicsItem())
    {
        m_startingLocation = startingNode->m_reverseComplement->m_graphicsItemNode->getFirst();
        m_beforeStartingLocation = startingNode->m_reverseComplement->m_graphicsItemNode->getSecond();
    }

    if (endingNode->hasGraphicsItem())
    {
        m_endingLocation = endingNode->m_graphicsItemNode->getFirst();
        m_afterEndingLocation = endingNode->m_graphicsItemNode->getSecond();
    }
    else if (endingNode->m_reverseComplement->hasGraphicsItem())
    {
        m_endingLocation = endingNode->m_reverseComplement->m_graphicsItemNode->getLast();
        m_afterEndingLocation = endingNode->m_reverseComplement->m_graphicsItemNode->getSecondLast();
    }
}


//This function handles the special case of an edge that connects a node
//to itself where the node graphics item has only one line segment.
void GraphicsItemEdge::makeSpecialPathConnectingNodeToSelf()
{
    QLineF nodeLine(m_startingLocation, m_endingLocation);
    QLineF normalLine = nodeLine.normalVector();
    QPointF perpendicularShift = normalLine.p2() - normalLine.p1();
    QPointF nodeMidPoint = (m_startingLocation + m_endingLocation) / 2.0;

    QPainterPath path;
    path.moveTo(m_startingLocation);

    path.cubicTo(m_controlPoint1, m_controlPoint1 + perpendicularShift, nodeMidPoint + perpendicularShift);
    path.cubicTo(m_controlPoint2 + perpendicularShift, m_controlPoint2, m_endingLocation);

    setPath(path);
}
