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

    double extensionLength = g_settings->edgeLength;
    if (extensionLength > edgeDistance / 2.0)
        extensionLength = edgeDistance / 2.0;

    m_controlPoint1 = extendLine(m_beforeStartingLocation, m_startingLocation, extensionLength);
    m_controlPoint2 = extendLine(m_afterEndingLocation, m_endingLocation, extensionLength);


    //If this edge is connecting a node to itself, and that node
    //is made of only one line segment, then a special path is
    //required, otherwise the edge will be mostly hidden underneath
    //the node.
    DeBruijnNode * startingNode = m_deBruijnEdge->getStartingNode();
    DeBruijnNode * endingNode = m_deBruijnEdge->getEndingNode();
    if (startingNode == endingNode)
    {
        GraphicsItemNode * graphicsItemNode = startingNode->getGraphicsItemNode();
        if (graphicsItemNode == 0)
            graphicsItemNode = startingNode->getReverseComplement()->getGraphicsItemNode();
        if (graphicsItemNode != 0 && graphicsItemNode->m_linePoints.size() == 2)
        {
            makeSpecialPathConnectingNodeToSelf();
            return;
        }
    }

    //If we are in single mode and the edge connects a node to its reverse
    //complement, then we need a special path to make it visible.
    if (startingNode == endingNode->getReverseComplement() &&
            !g_settings->doubleMode)
    {
        makeSpecialPathConnectingNodeToReverseComplement();
        return;
    }

    //Otherwise, the path is just a single cubic Bezier curve.
    QPainterPath path;
    path.moveTo(m_startingLocation);
    path.cubicTo(m_controlPoint1, m_controlPoint2, m_endingLocation);

    setPath(path);
}

void GraphicsItemEdge::setControlPointLocations()
{
    DeBruijnNode * startingNode = m_deBruijnEdge->getStartingNode();
    DeBruijnNode * endingNode = m_deBruijnEdge->getEndingNode();

    if (startingNode->hasGraphicsItem())
    {
        m_startingLocation = startingNode->getGraphicsItemNode()->getLast();
        m_beforeStartingLocation = startingNode->getGraphicsItemNode()->getSecondLast();
    }
    else if (startingNode->getReverseComplement()->hasGraphicsItem())
    {
        m_startingLocation = startingNode->getReverseComplement()->getGraphicsItemNode()->getFirst();
        m_beforeStartingLocation = startingNode->getReverseComplement()->getGraphicsItemNode()->getSecond();
    }

    if (endingNode->hasGraphicsItem())
    {
        m_endingLocation = endingNode->getGraphicsItemNode()->getFirst();
        m_afterEndingLocation = endingNode->getGraphicsItemNode()->getSecond();
    }
    else if (endingNode->getReverseComplement()->hasGraphicsItem())
    {
        m_endingLocation = endingNode->getReverseComplement()->getGraphicsItemNode()->getLast();
        m_afterEndingLocation = endingNode->getReverseComplement()->getGraphicsItemNode()->getSecondLast();
    }
}


//This function handles the special case of an edge that connects a node
//to itself where the node graphics item has only one line segment.
void GraphicsItemEdge::makeSpecialPathConnectingNodeToSelf()
{
    double extensionLength = g_settings->edgeLength;
    m_controlPoint1 = extendLine(m_beforeStartingLocation, m_startingLocation, extensionLength);
    m_controlPoint2 = extendLine(m_afterEndingLocation, m_endingLocation, extensionLength);

    QLineF nodeLine(m_startingLocation, m_endingLocation);
    QLineF normalUnitLine = nodeLine.normalVector().unitVector();
    QPointF perpendicularShift = (normalUnitLine.p2() - normalUnitLine.p1()) * g_settings->edgeLength;
    QPointF nodeMidPoint = (m_startingLocation + m_endingLocation) / 2.0;

    QPainterPath path;
    path.moveTo(m_startingLocation);

    path.cubicTo(m_controlPoint1, m_controlPoint1 + perpendicularShift, nodeMidPoint + perpendicularShift);
    path.cubicTo(m_controlPoint2 + perpendicularShift, m_controlPoint2, m_endingLocation);

    setPath(path);
}

//This function handles the special case of an edge that connects a node to its
//reverse complement and is displayed in single mode.
void GraphicsItemEdge::makeSpecialPathConnectingNodeToReverseComplement()
{
    double extensionLength = g_settings->edgeLength / 2.0;
    m_controlPoint1 = extendLine(m_beforeStartingLocation, m_startingLocation, extensionLength);
    m_controlPoint2 = extendLine(m_afterEndingLocation, m_endingLocation, extensionLength);

    QPointF startToControl = m_controlPoint1 - m_startingLocation;
    QPointF pathMidPoint = m_startingLocation + startToControl * 3.0;

    QLineF normalLine = QLineF(m_controlPoint1, m_startingLocation).normalVector();
    QPointF perpendicularShift = (normalLine.p2() - normalLine.p1()) * 1.5;

    QPainterPath path;
    path.moveTo(m_startingLocation);

    path.cubicTo(m_controlPoint1, pathMidPoint + perpendicularShift, pathMidPoint);
    path.cubicTo(pathMidPoint - perpendicularShift, m_controlPoint2, m_endingLocation);

    setPath(path);
}
