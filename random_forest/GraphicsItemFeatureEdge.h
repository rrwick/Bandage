#ifndef GRAPHICSITEMFUTUREEDGE_H
#define GRAPHICSITEMFUTUREEDGE_H

#include "RandomForestNode.h"
#include <QGraphicsItem>

class GraphicsItemFeatureNode;

class GraphicsItemFeatureEdge : public QGraphicsPathItem
{
public:
    GraphicsItemFeatureEdge(RandomForestNode* startingNode, RandomForestNode* endingNode, QGraphicsItem* parent = 0);

    RandomForestNode* m_startingNode;
    RandomForestNode* m_endingNode;
    QPointF m_startingLocation;
    QPointF m_endingLocation;

    void paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*);
    void calculateAndSetPath();

private:
};
#endif //GRAPHICSITEMFUTUREEDGE_H
