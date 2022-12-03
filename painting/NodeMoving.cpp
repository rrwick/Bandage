#include "NodeMoving.h"
#include "../program/settings.h"

NodeMoving::NodeMoving(){}

size_t NodeMoving::getGrapIndex(QGraphicsSceneMouseEvent* event, std::vector<QPointF>& linePoints)
{
    size_t grabIndex = 0;
    QPointF grabPoint = event->pos();

    double closestPointDistance = distance(grabPoint, linePoints[0]);
    for (size_t i = 1; i < linePoints.size(); ++i)
    {
        double pointDistance = distance(grabPoint, linePoints[i]);
        if (pointDistance < closestPointDistance)
        {
            closestPointDistance = pointDistance;
            grabIndex = i;
        }
    }
    return grabIndex;
}

void NodeMoving::shiftPoints(QPointF difference, std::vector<QPointF>& linePoints, size_t grabIndex)
{
    prepareGeometryChange();

    if (g_settings->nodeDragging == NO_DRAGGING)
        return;

    else if (isSelected()) //Move all pieces for selected nodes
    {
        for (size_t i = 0; i < linePoints.size(); ++i)
            linePoints[i] += difference;
    }

    else if (g_settings->nodeDragging == ONE_PIECE)
        linePoints[grabIndex] += difference;

    else if (g_settings->nodeDragging == NEARBY_PIECES)
    {
        for (size_t i = 0; i < linePoints.size(); ++i)
        {
            int indexDistance = abs(int(i) - int(grabIndex));
            double dragStrength = pow(2.0, -1.0 * pow(double(indexDistance), 1.8) / g_settings->dragStrength); //constants chosen for dropoff of drag strength
            linePoints[i] += difference * dragStrength;
        }
    }
}

void NodeMoving::roundPoints(QPointF centralPoint, double alpha, std::vector<QPointF>& linePoints) {
    double sinA = sin(alpha);
    double cosA = cos(alpha);
    double xA = centralPoint.x();
    double yA = centralPoint.y();
    for (size_t i = 0; i < linePoints.size(); ++i) {
        QPointF point = linePoints[i];
        double x = -sinA * (point.y() - yA) + cosA * (point.x() - xA) + xA;
        double y = cosA * (point.y() - yA) + sinA * (point.x() - xA) + yA;
        point.setX(x);
        point.setY(y);
    }
}

QPainterPath NodeMoving::remakePath(std::vector<QPointF> & linePoints)
{
    QPainterPath path;

    path.moveTo(linePoints[0]);
    if (linePoints.size() <= 2) {
        for (size_t i = 1; i < linePoints.size(); ++i)
            path.lineTo(linePoints[i]);
    }
    else {
        int middleInd = linePoints.size() / 2;
        for (size_t i = 1; i < middleInd - 1; ++i)
            path.lineTo(linePoints[i]);
        path.quadTo(linePoints[middleInd], linePoints[middleInd + 1]);

        for (size_t i = middleInd + 1; i < linePoints.size(); ++i)
            path.lineTo(linePoints[i]);
    }

    return path;
}

double NodeMoving::distance(QPointF p1, QPointF p2) const
{
    double xDiff = p1.x() - p2.x();
    double yDiff = p1.y() - p2.y();
    return sqrt(xDiff * xDiff + yDiff * yDiff);
}
