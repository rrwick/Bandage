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


#ifndef GRAPHICSITEMNODE_H
#define GRAPHICSITEMNODE_H

#include <QGraphicsItem>
#include <QGraphicsSceneMouseEvent>
#include <ogdf/basic/GraphAttributes.h>
#include <vector>
#include <QPointF>
#include <QColor>
#include <QFont>
#include <QString>
#include <QPainterPath>
#include <QStringList>

class DeBruijnNode;

class GraphicsItemNode : public QGraphicsItem
{
public:
    GraphicsItemNode(DeBruijnNode * deBruijnNode,
                     ogdf::GraphAttributes * graphAttributes,
                     QGraphicsItem * parent = 0);

    DeBruijnNode * m_deBruijnNode;
    double m_width;
    bool m_hasArrow;
    std::vector<QPointF> m_linePoints;
    size_t m_grabIndex;
    QColor m_colour;
    QPainterPath m_path;

    void mousePressEvent(QGraphicsSceneMouseEvent * event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent * event);
    void paint(QPainter * painter, const QStyleOptionGraphicsItem *, QWidget *);
    QPainterPath shape() const;
    void shiftPoints(QPointF difference);
    void remakePath();
    double distance(QPointF p1, QPointF p2) const;
    bool usePositiveNodeColour();
    QPointF getFirst() const {return m_linePoints[0];}
    QPointF getSecond() const {return m_linePoints[1];}
    QPointF getLast() const {return m_linePoints[m_linePoints.size()-1];}
    QPointF getSecondLast() const {return m_linePoints[m_linePoints.size()-2];}
    std::vector<QPointF> getCentres() const;
    QPointF getCentre(std::vector<QPointF> linePoints) const;
    void setNodeColour();
    QStringList getNodeText();
    QSize getNodeTextSize(QString text);
    QColor getCoverageColour();
    void setWidth();
    QPainterPath makePartialPath(double startFraction, double endFraction);
    double getNodePathLength();
    QPointF findLocationOnPath(double fraction);
    QPointF findIntermediatePoint(QPointF p1, QPointF p2, double p1Value, double p2Value, double targetValue);
    QRectF boundingRect() const;
    static double getNodeWidth(double coverageRelativeToMeanDrawnCoverage, double coveragePower,
                               double coverageEffectOnWidth, double averageNodeWidth);
    void shiftPointsLeft();

};

#endif // GRAPHICSITEMNODE_H
