#ifndef NODEMOVING_H
#define NODEMOVING_H

#include <QGraphicsItem>
#include <QGraphicsSceneMouseEvent>
#include <QPainterPath>

class NodeMoving : public QGraphicsItem
{
public:
	NodeMoving();
	size_t getGrapIndex(QGraphicsSceneMouseEvent* event, std::vector<QPointF>& linePoints);
	void shiftPoints(QPointF difference, std::vector<QPointF>& linePoints, size_t grabIndex);
	void roundPoints(QPointF referencePont, double alpha, std::vector<QPointF>& linePoints);

	QPainterPath remakePath(std::vector<QPointF>& linePoints);

private:
	double distance(QPointF p1, QPointF p2) const;
};
#endif; //NODEMOVING_H

