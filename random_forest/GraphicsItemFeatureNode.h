#ifndef GRAPHICSITEMFUTURENODE_H
#define GRAPHICSITEMFUTURENODE_H

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
#include "../program/settings.h"
#include "../program/globals.h"
#include "../painting/CommonGraphicsItemNode.h"

class RandomForestNode;
class Path;

class GraphicsItemFeatureNode : public CommonGraphicsItemNode
{
public:
    GraphicsItemFeatureNode(RandomForestNode* featureNode,
        ogdf::GraphAttributes* graphAttributes,
        CommonGraphicsItemNode* parent = 0);

    RandomForestNode* m_featureNode;

    QRectF boundingRect() const;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*);

    QPainterPath shape() const;
    QPainterPath shapeRect() const;
    QPainterPath shapeTriangl() const;
    void setColour();
    QStringList getNodeText();
    void mousePressEvent(QGraphicsSceneMouseEvent* event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event);
private:
    void fixEdgePaths(std::vector<GraphicsItemFeatureNode*>* nodes);
    std::vector<QColor> m_presetColours;
    std::vector<QColor> getPresetColours();
    void drawNodeText(QPainter* painter, QStringList nodeText);
    void drawTextPathAtLocation(QPainter* painter, QPainterPath textPath);

};
#endif // GRAPHICSITEMFUTURENODE_H

