#ifndef COMMONGRAPHICSITEMNODE_H
#define COMMONGRAPHICSITEMNODE_H

#include <QGraphicsItem>
#include <QGraphicsSceneMouseEvent>
#include <vector>
#include <QPointF>
#include <QColor>
#include <QFont>
#include <QString>
#include <QPainterPath>
#include <QStringList>
#include <algorithm>
#include <iostream>
#include "../ui/mygraphicsview.h"

class Path;

class CommonGraphicsItemNode : public QGraphicsItem
{
public:

    CommonGraphicsItemNode(MyGraphicsView* graphicsView, QGraphicsItem* parent = 0);

    double m_width;
    bool m_hasArrow;
    std::vector<QPointF> m_linePoints;
    size_t m_grabIndex;
    QColor m_colour;
    QPainterPath m_path;
    MyGraphicsView* m_graphicsView;

    void shiftPoints(QPointF difference);
    void remakePath();
    void setWidth(double width) { m_width = width; };
    void updateGrabIndex(QGraphicsSceneMouseEvent* event);
    double distance(QPointF p1, QPointF p2) const;
    
    QPointF getFirst() const { return m_linePoints[0]; }
    QPointF getSecond() const {
        if (m_linePoints.size() > 1) return m_linePoints[1];
        else return m_linePoints[0];
    }
    QPointF getLast() const { return m_linePoints[m_linePoints.size() - 1]; }
    QPointF getSecondLast() const
    {
        if (m_linePoints.size() > 1) return m_linePoints[m_linePoints.size() - 2];
        else return m_linePoints[0];
    }
    bool isBig() const { return m_linePoints.size() >= 3; }
    bool isOne() const { return m_linePoints.size() == 1; }
    QPointF getMiddle() const { return m_linePoints[m_linePoints.size() / 2]; }
    QPointF getBeforeMiddle() const
    {
        if (m_linePoints.size() >= 3)
            return m_linePoints[(m_linePoints.size() / 2) - 1];
        else
            return m_linePoints[0];
    }
    QPointF getAfterMiddle() const
    {
        if (m_linePoints.size() >= 3)
            return m_linePoints[(m_linePoints.size() / 2) + 1];
        else
            return m_linePoints[m_linePoints.size() - 1];
    }
    std::vector<QPointF> getCentres() const;
    QPointF getCentre(std::vector<QPointF> linePoints) const;

    QPainterPath makePartialPath(double startFraction, double endFraction);
    double getNodePathLength();
    QPointF findLocationOnPath(double fraction);
    QPointF findIntermediatePoint(QPointF p1, QPointF p2, double p1Value,
        double p2Value, double targetValue);
    void shiftPointsLeft();
    void shiftPointsRight();
    void drawNodeText(QPainter* painter, QStringList nodeText);
    void drawTextPathAtLocation(QPainter* painter, QPainterPath textPath, QPointF centre);
    void roundPoints(QPointF centralPoint, double alpha);
    double angleBetweenTwoLines(QPointF line1Start, QPointF line1End, QPointF line2Start, QPointF line2End);

private:
    void shiftPointSideways(bool left);
};

#endif //COMMONGRAPHICSITEMNODE_H

