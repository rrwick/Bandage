#include "GraphicsItemFeatureNode.h"
#include "../graph/ogdfnode.h"
#include <QPainter>
#include "RandomForestNode.h"
#include "GraphicsItemFeatureEdge.h"
#include <QGraphicsScene>
#include "../ui/mygraphicsview.h"
#include "../ui/mygraphicsscene.h"
#include <set>
#include "RandomForestEdge.h"

GraphicsItemFeatureNode::GraphicsItemFeatureNode(RandomForestNode* featureNode,
    ogdf::GraphAttributes* graphAttributes, CommonGraphicsItemNode* parent) :
    CommonGraphicsItemNode(g_graphicsViewFeaturesForest, parent), m_featureNode(featureNode)

{
    m_width = g_settings->averageFeatureNodeWidth;

    OgdfNode* pathOgdfNode = featureNode->getOgdfNode();
    if (pathOgdfNode != 0)
    {
        for (size_t i = 0; i < pathOgdfNode->m_ogdfNodes.size(); ++i)
        {
            ogdf::node ogdfNode = pathOgdfNode->m_ogdfNodes[i];
            QPointF point(graphAttributes->x(ogdfNode), graphAttributes->y(ogdfNode));
            m_linePoints.push_back(point);
        }
    }

    remakePath();
}

QRectF GraphicsItemFeatureNode::boundingRect() const
{
    //double extraSize = g_settings->selectionThickness / 2.0;
    QRectF bound = shape().boundingRect();

    bound.setTop(bound.top() - 0.5);
    bound.setBottom(bound.bottom() + 0.5);
    bound.setLeft(bound.left() - 0.5);
    bound.setRight(bound.right() + 0.5);

    return bound;
}

QPainterPath GraphicsItemFeatureNode::shape() const
{
    int width = g_settings->averageFeatureNodeWidth;
    QPainterPath mainNodePath;
    mainNodePath.addEllipse(m_linePoints[0].toPoint(), width, width);
    return mainNodePath;
}

QPainterPath GraphicsItemFeatureNode::shapeRect() const
{
    int width = g_settings->averageFeatureNodeWidth;
    QRect r(m_linePoints[0].toPoint().x(), m_linePoints[0].toPoint().y(), width, width);
    r.moveCenter(m_linePoints[0].toPoint());
    QPainterPath mainNodePath;
    mainNodePath.addRect(r);
    return mainNodePath;
}

void GraphicsItemFeatureNode::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*)
{
    int width = g_settings->averageFeatureNodeWidth;
    int x = m_linePoints[0].x();
    int y = m_linePoints[0].y();
    //QPainter painter;
    setColour();
    QBrush brush;
    QColor outlineColour = g_settings->outlineColour;
    double outlineThickness = g_settings->outlineThickness;
    if (isSelected()) {
        outlineColour = g_settings->selectionColour;
        outlineThickness = g_settings->selectionThickness;
    }
    if (outlineThickness > 0.0)
    {
        QPen outlinePen(QBrush(outlineColour), outlineThickness, Qt::SolidLine,
            Qt::SquareCap, Qt::RoundJoin);
        painter->setPen(outlinePen);
    }
    painter->setBrush(m_colour);
    if (g_settings->displayFeatureClassLikeFigure) {
        if (m_featureNode->getClassInd() == 0) {
            QRect r(x, y, width, width);
            r.moveCenter(m_linePoints[0].toPoint());
            
            QPainterPath outlinePath;
            outlinePath.addRect(r);
            painter->fillPath(outlinePath, brush);
            painter->drawRect(r);
        }
        else if (m_featureNode->getClassInd() == 1) {
            QRect r(x, y, width, width);
            r.moveCenter(m_linePoints[0].toPoint());

            QPainterPath outlinePath;
            outlinePath.moveTo(r.left() + (r.width() / 2), r.top());
            outlinePath.lineTo(r.bottomLeft());
            outlinePath.lineTo(r.bottomRight());
            outlinePath.lineTo(r.left() + (r.width() / 2), r.top());

            painter->fillPath(outlinePath, brush);
            painter->drawPath(outlinePath);
        }
        else {
            QPainterPath outlinePath = shape();
            QRect r(x, y, width, width);
            r.moveCenter(m_linePoints[0].toPoint());
            painter->fillPath(outlinePath, brush);
            painter->drawEllipse(r);
        }
    }
    else {
        QPainterPath outlinePath = shape();
        QRect r(x, y, width, width);
        r.moveCenter(m_linePoints[0].toPoint());
        painter->fillPath(outlinePath, brush);
        painter->drawEllipse(r);
    }

    if (g_settings->displayFeatureIdLabels ||
        g_settings->displayFeatureClassLabels ||
        g_settings->displayFeatureCustomLabels)
    {
        QStringList nodeText = getNodeText();
        drawNodeText(painter, nodeText);
    }

    return;
}

void GraphicsItemFeatureNode::drawNodeText(QPainter* painter, QStringList nodeText) {
    QPainterPath textPath;

    QFontMetrics metrics(g_settings->labelFont);
    double fontHeight = metrics.ascent();

    for (int i = 0; i < nodeText.size(); ++i)
    {
        QString text = nodeText.at(i);
        int stepsUntilLast = nodeText.size() - 1 - i;
        double shiftLeft = -metrics.width(text) / 2.0;
        textPath.addText(shiftLeft, -stepsUntilLast * fontHeight, g_settings->labelFont, text);
    }

    drawTextPathAtLocation(painter, textPath);
}

void GraphicsItemFeatureNode::drawTextPathAtLocation(QPainter* painter, QPainterPath textPath)
{
    QRectF textBoundingRect = textPath.boundingRect();
    double textHeight = textBoundingRect.height();
    QPointF offset(0.0, textHeight / 2.0);
    QPointF centre(m_linePoints[0].x() - textBoundingRect.width() / 2.0 - m_width / 2.0 - 1.0, m_linePoints[0].y());

    double zoom = g_absoluteZoom;
    if (zoom == 0.0)
        zoom = 1.0;

    double zoomAdjustment = 1.0 / (1.0 + ((zoom - 1.0) * g_settings->textZoomScaleFactor));
    double inverseZoomAdjustment = 1.0 / zoomAdjustment;

    painter->translate(centre);
    painter->rotate(-m_graphicsView->getRotation());
    painter->scale(zoomAdjustment, zoomAdjustment);
    painter->translate(offset);

    if (g_settings->textOutline)
    {
        painter->setPen(QPen(g_settings->textOutlineColour,
            g_settings->textOutlineThickness * 2.0,
            Qt::SolidLine,
            Qt::SquareCap,
            Qt::RoundJoin));
        painter->drawPath(textPath);
    }

    painter->fillPath(textPath, QBrush(g_settings->textColour));
    painter->translate(-offset);
    painter->scale(inverseZoomAdjustment, inverseZoomAdjustment);
    painter->rotate(m_graphicsView->getRotation());
    painter->translate(-centre);
}

void GraphicsItemFeatureNode::setColour() {
    int blastColourIndex = m_featureNode->getBlastColourInd();
    int classInd = m_featureNode->getClassInd();
    switch (g_settings->featureColourScheme)
    {
    case UNIFORM_COLOURS:
        m_colour.setRgb(255, 130, 5);
        break;
    case CLASS_COLOURS:
        if (m_presetColours.empty()) {
            m_presetColours = getPresetColours();
        }
        classInd %= m_presetColours.size();
        m_colour = m_presetColours[classInd];
        break;
    case BLAST_HITS_SOLID_COLOUR:
        if (blastColourIndex == -1) {
            m_colour = g_settings->noBlastHitsColour;
        } else {
            if (m_presetColours.empty()) {
                m_presetColours = getPresetColours();
            }
            m_presetColours = getPresetColours();
           
            blastColourIndex %= m_presetColours.size();
            m_colour = m_presetColours[blastColourIndex];
        }
        break;
    case BLAST_HITS_CLASS_COLOURS:
        if (blastColourIndex == -1) {
            m_colour = g_settings->noBlastHitsColour;
        }
        else {
            if (m_presetColours.empty()) {
                m_presetColours = getPresetColours();
            }
            classInd %= m_presetColours.size();
            m_colour = m_presetColours[classInd];
        }
        break;
    case CUSTOM_COLOURS:
        m_colour = m_featureNode->getCustomColour();
        break;
    }
}

std::vector<QColor> GraphicsItemFeatureNode::getPresetColours()
{
    std::vector<QColor> presetColours;

    presetColours.push_back(QColor("#306FF8"));
    presetColours.push_back(QColor("#86BB18"));
    presetColours.push_back(QColor("#DF123A"));
    presetColours.push_back(QColor("#181E2A"));
    presetColours.push_back(QColor("#F91BBD"));
    presetColours.push_back(QColor("#3CB2A4"));
    presetColours.push_back(QColor("#D29AC1"));
    presetColours.push_back(QColor("#E2922E"));
    presetColours.push_back(QColor("#22501B"));
    presetColours.push_back(QColor("#57297D"));
    presetColours.push_back(QColor("#3FA0E6"));
    presetColours.push_back(QColor("#770739"));
    presetColours.push_back(QColor("#6A390C"));
    presetColours.push_back(QColor("#25AB5D"));
    presetColours.push_back(QColor("#ACAF61"));
    presetColours.push_back(QColor("#F0826F"));
    presetColours.push_back(QColor("#E94A80"));
    presetColours.push_back(QColor("#C187F2"));
    presetColours.push_back(QColor("#7E5764"));
    presetColours.push_back(QColor("#037290"));
    presetColours.push_back(QColor("#D65114"));
    presetColours.push_back(QColor("#08396A"));
    presetColours.push_back(QColor("#99ABBE"));
    presetColours.push_back(QColor("#F270C0"));
    presetColours.push_back(QColor("#F056F9"));
    presetColours.push_back(QColor("#8E8D00"));
    presetColours.push_back(QColor("#70010F"));
    presetColours.push_back(QColor("#9C1E9A"));
    presetColours.push_back(QColor("#471B1F"));
    presetColours.push_back(QColor("#A00B6D"));
    presetColours.push_back(QColor("#38C037"));
    presetColours.push_back(QColor("#282C16"));
    presetColours.push_back(QColor("#15604D"));
    presetColours.push_back(QColor("#2E75D6"));
    presetColours.push_back(QColor("#A09DEB"));
    presetColours.push_back(QColor("#8454D7"));
    presetColours.push_back(QColor("#301745"));
    presetColours.push_back(QColor("#A45704"));
    presetColours.push_back(QColor("#4D8C0E"));
    presetColours.push_back(QColor("#C09860"));
    presetColours.push_back(QColor("#009C73"));
    presetColours.push_back(QColor("#FD6453"));
    presetColours.push_back(QColor("#C11C4B"));
    presetColours.push_back(QColor("#183B8B"));
    presetColours.push_back(QColor("#5E6706"));
    presetColours.push_back(QColor("#E42005"));
    presetColours.push_back(QColor("#4873AF"));
    presetColours.push_back(QColor("#6CA563"));
    presetColours.push_back(QColor("#5E0F54"));
    presetColours.push_back(QColor("#FE2065"));
    presetColours.push_back(QColor("#5BB4D2"));
    presetColours.push_back(QColor("#3F4204"));
    presetColours.push_back(QColor("#521839"));
    presetColours.push_back(QColor("#9A7706"));
    presetColours.push_back(QColor("#77AB8C"));
    presetColours.push_back(QColor("#105E04"));
    presetColours.push_back(QColor("#98290F"));
    presetColours.push_back(QColor("#B849D4"));
    presetColours.push_back(QColor("#FC8426"));
    presetColours.push_back(QColor("#341B03"));
    presetColours.push_back(QColor("#E3278C"));
    presetColours.push_back(QColor("#F28F93"));
    presetColours.push_back(QColor("#D1A21F"));
    presetColours.push_back(QColor("#277E46"));
    presetColours.push_back(QColor("#285C60"));
    presetColours.push_back(QColor("#76B945"));
    presetColours.push_back(QColor("#E75D65"));
    presetColours.push_back(QColor("#84ADDC"));
    presetColours.push_back(QColor("#153C2B"));
    presetColours.push_back(QColor("#FD10D9"));
    presetColours.push_back(QColor("#C095D5"));
    presetColours.push_back(QColor("#052B48"));
    presetColours.push_back(QColor("#B365FC"));
    presetColours.push_back(QColor("#97AA75"));
    presetColours.push_back(QColor("#C78C9C"));
    presetColours.push_back(QColor("#FD4838"));
    presetColours.push_back(QColor("#F181E2"));
    presetColours.push_back(QColor("#815A1A"));
    presetColours.push_back(QColor("#BB2093"));
    presetColours.push_back(QColor("#691822"));
    presetColours.push_back(QColor("#C41A12"));
    presetColours.push_back(QColor("#728A1F"));
    presetColours.push_back(QColor("#375B73"));
    presetColours.push_back(QColor("#97022C"));
    presetColours.push_back(QColor("#95B44D"));
    presetColours.push_back(QColor("#EB8DBB"));
    presetColours.push_back(QColor("#83ACAB"));
    presetColours.push_back(QColor("#E37D51"));
    presetColours.push_back(QColor("#D78A68"));
    presetColours.push_back(QColor("#4A41A2"));
    presetColours.push_back(QColor("#8A0C79"));
    presetColours.push_back(QColor("#133102"));
    presetColours.push_back(QColor("#237A78"));
    presetColours.push_back(QColor("#ADB03B"));
    presetColours.push_back(QColor("#289E26"));
    presetColours.push_back(QColor("#7683EC"));
    presetColours.push_back(QColor("#4E1E04"));
    presetColours.push_back(QColor("#BB17B2"));
    presetColours.push_back(QColor("#EB6A81"));
    presetColours.push_back(QColor("#47B4E8"));
    presetColours.push_back(QColor("#0A6191"));
    presetColours.push_back(QColor("#4EADB2"));
    presetColours.push_back(QColor("#442965"));
    presetColours.push_back(QColor("#FE784B"));
    presetColours.push_back(QColor("#55BD8D"));
    presetColours.push_back(QColor("#742B03"));
    presetColours.push_back(QColor("#8C38AA"));
    presetColours.push_back(QColor("#F758A6"));
    presetColours.push_back(QColor("#A32526"));
    presetColours.push_back(QColor("#442C2E"));
    presetColours.push_back(QColor("#F06A97"));
    presetColours.push_back(QColor("#3A1527"));
    presetColours.push_back(QColor("#503509"));
    presetColours.push_back(QColor("#2A67B4"));
    presetColours.push_back(QColor("#243644"));
    presetColours.push_back(QColor("#A74006"));
    presetColours.push_back(QColor("#335900"));
    presetColours.push_back(QColor("#A07484"));
    presetColours.push_back(QColor("#490216"));
    presetColours.push_back(QColor("#B19BCB"));
    presetColours.push_back(QColor("#75B75A"));
    presetColours.push_back(QColor("#BE71EB"));
    presetColours.push_back(QColor("#024A2E"));
    presetColours.push_back(QColor("#A097AB"));
    presetColours.push_back(QColor("#7A287E"));
    presetColours.push_back(QColor("#6A1444"));
    presetColours.push_back(QColor("#212449"));
    presetColours.push_back(QColor("#B07017"));
    presetColours.push_back(QColor("#227D57"));
    presetColours.push_back(QColor("#1B8CAF"));
    presetColours.push_back(QColor("#016438"));
    presetColours.push_back(QColor("#EA64CF"));
    presetColours.push_back(QColor("#B5310E"));
    presetColours.push_back(QColor("#B00765"));
    presetColours.push_back(QColor("#5F42B3"));
    presetColours.push_back(QColor("#EF9649"));
    presetColours.push_back(QColor("#25717F"));
    presetColours.push_back(QColor("#BCA309"));
    presetColours.push_back(QColor("#FA35A6"));
    presetColours.push_back(QColor("#F63D54"));
    presetColours.push_back(QColor("#E83D6C"));
    presetColours.push_back(QColor("#8362F2"));
    presetColours.push_back(QColor("#33BC4A"));
    presetColours.push_back(QColor("#194A85"));
    presetColours.push_back(QColor("#E24215"));
    presetColours.push_back(QColor("#6D71FE"));
    presetColours.push_back(QColor("#3E52AF"));
    presetColours.push_back(QColor("#1E9E89"));
    presetColours.push_back(QColor("#740860"));
    presetColours.push_back(QColor("#4B7BEE"));
    presetColours.push_back(QColor("#8742C0"));
    presetColours.push_back(QColor("#DD8EC6"));
    presetColours.push_back(QColor("#CD202C"));
    presetColours.push_back(QColor("#FD82C2"));
    presetColours.push_back(QColor("#3C2874"));
    presetColours.push_back(QColor("#F9742B"));
    presetColours.push_back(QColor("#013B10"));
    presetColours.push_back(QColor("#D12867"));
    presetColours.push_back(QColor("#F743C3"));
    presetColours.push_back(QColor("#B98EEC"));
    presetColours.push_back(QColor("#D260EC"));
    presetColours.push_back(QColor("#671C06"));
    presetColours.push_back(QColor("#37A968"));
    presetColours.push_back(QColor("#3B9529"));
    presetColours.push_back(QColor("#2A0E33"));
    presetColours.push_back(QColor("#51B237"));
    presetColours.push_back(QColor("#95B61B"));
    presetColours.push_back(QColor("#B195E2"));
    presetColours.push_back(QColor("#68B49A"));
    presetColours.push_back(QColor("#182339"));
    presetColours.push_back(QColor("#FC4822"));
    presetColours.push_back(QColor("#D79621"));
    presetColours.push_back(QColor("#90761B"));
    presetColours.push_back(QColor("#777315"));
    presetColours.push_back(QColor("#E389E9"));
    presetColours.push_back(QColor("#35BD64"));
    presetColours.push_back(QColor("#C17910"));
    presetColours.push_back(QColor("#3386ED"));
    presetColours.push_back(QColor("#E82C2E"));
    presetColours.push_back(QColor("#AC925F"));
    presetColours.push_back(QColor("#F227C8"));
    presetColours.push_back(QColor("#F43E67"));
    presetColours.push_back(QColor("#55AEEB"));
    presetColours.push_back(QColor("#F518E3"));
    presetColours.push_back(QColor("#AB0643"));
    presetColours.push_back(QColor("#8DA1F3"));
    presetColours.push_back(QColor("#5C9C14"));
    presetColours.push_back(QColor("#381F27"));
    presetColours.push_back(QColor("#6BB7B5"));
    presetColours.push_back(QColor("#9842BE"));
    presetColours.push_back(QColor("#4897D6"));
    presetColours.push_back(QColor("#8958E4"));
    presetColours.push_back(QColor("#8F0065"));
    presetColours.push_back(QColor("#A10A5E"));
    presetColours.push_back(QColor("#076315"));
    presetColours.push_back(QColor("#FA5EF9"));
    presetColours.push_back(QColor("#A33402"));
    presetColours.push_back(QColor("#A0ABC4"));
    presetColours.push_back(QColor("#2B6EFE"));
    presetColours.push_back(QColor("#9A9EE7"));

    return presetColours;
}

QStringList GraphicsItemFeatureNode::getNodeText()
{
    QStringList nodeText;
    if (g_settings->displayFeatureCustomLabels)
        nodeText << m_featureNode->getCustomLabelForDisplay();
    if (g_settings->displayFeatureIdLabels)
    {
        QString id = m_featureNode->getName();
        nodeText << id;
    }
    if (g_settings->displayFeatureClassLabels)
        nodeText << m_featureNode->getClass();

    return nodeText;
}

void GraphicsItemFeatureNode::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    updateGrabIndex(event);
}

//When this node graphics item is moved, each of the connected edge
//graphics items will need to be adjusted accordingly.
void GraphicsItemFeatureNode::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    QPointF difference = event->pos() - event->lastPos();

    //If this node is selected, then move all of the other selected nodes too.
    //If it is not selected, then only move this node.
    std::vector<GraphicsItemFeatureNode*> nodesToMove;
    MyGraphicsScene* graphicsScene = dynamic_cast<MyGraphicsScene*>(scene());
    if (isSelected())
        nodesToMove = graphicsScene->getSelectedGraphicsItemFeatureNode();
    else
        nodesToMove.push_back(this);

    for (size_t i = 0; i < nodesToMove.size(); ++i)
    {
        nodesToMove[i]->shiftPoints(difference);
        nodesToMove[i]->remakePath();
    }
    graphicsScene->possiblyExpandSceneRectangle(&nodesToMove);

    fixEdgePaths(&nodesToMove);
}

void GraphicsItemFeatureNode::fixEdgePaths(std::vector<GraphicsItemFeatureNode*>* nodes)
{
    std::set<RandomForestEdge*> edgesToFix;

    if (nodes == 0)
    {
        const std::vector<RandomForestEdge*>* edges = m_featureNode->getEdges();
        for (size_t j = 0; j < edges->size(); ++j)
            edgesToFix.insert((*edges)[j]);
    }
    else
    {
        for (size_t i = 0; i < nodes->size(); ++i)
        {
            RandomForestNode* node = (*nodes)[i]->m_featureNode;
            const std::vector<RandomForestEdge*>* edges = node->getEdges();
            for (size_t j = 0; j < edges->size(); ++j)
                edgesToFix.insert((*edges)[j]);
        }
    }

    for (std::set<RandomForestEdge*>::iterator i = edgesToFix.begin(); i != edgesToFix.end(); ++i)
    {
        RandomForestEdge* randomForestEdge = *i;
        GraphicsItemFeatureEdge* graphicsItemEdge = randomForestEdge->getGraphicsItemFeatureEdge();

        //If this edge has a graphics item, adjust it now.
        if (graphicsItemEdge != 0)
            graphicsItemEdge->calculateAndSetPath();
    }
}
