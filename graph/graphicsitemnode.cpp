//Copyright 2017 Ryan Wick

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


#include "graphicsitemnode.h"
#include "debruijnnode.h"
#include "ogdfnode.h"
#include <QPainterPathStroker>
#include "../graph/ogdfnode.h"
#include "../program/settings.h"
#include <QPainter>
#include <QPen>
#include <QMessageBox>
#include "debruijnedge.h"
#include "graphicsitemedge.h"
#include "../ogdf/basic/GraphAttributes.h"
#include <math.h>
#include <QFontMetrics>
#include <QSize>
#include <stdlib.h>
#include <QGraphicsScene>
#include "../ui/mygraphicsscene.h"
#include <set>
#include "../ui/mygraphicsview.h"
#include <QTransform>
#include "../blast/blasthit.h"
#include "../blast/blastquery.h"
#include "../blast/blasthitpart.h"
#include "assemblygraph.h"
#include <math.h>
#include <QFontMetrics>
#include "../program/memory.h"
#include <cmath>

GraphicsItemNode::GraphicsItemNode(DeBruijnNode * deBruijnNode,
                                   ogdf::GraphAttributes * graphAttributes, CommonGraphicsItemNode * parent) :
    CommonGraphicsItemNode(g_graphicsView, parent), m_deBruijnNode(deBruijnNode)

{
    m_hasArrow = g_settings->doubleMode || g_settings->arrowheadsInSingleMode;
    setWidth();
    //m_width = g_settings->averageNodeWidth;

    OgdfNode * pathOgdfNode = deBruijnNode->getOgdfNode();
    if (pathOgdfNode != 0)
    {
        for (size_t i = 0; i < pathOgdfNode->m_ogdfNodes.size(); ++i)
        {
            ogdf::node ogdfNode = pathOgdfNode->m_ogdfNodes[i];
            QPointF point(graphAttributes->x(ogdfNode), graphAttributes->y(ogdfNode));
            m_linePoints.push_back(point);
        }
    }
    else
    {
        pathOgdfNode = deBruijnNode->getReverseComplement()->getOgdfNode();
        for (int i = int(pathOgdfNode->m_ogdfNodes.size()) - 1; i >= 0; --i)
        {
            ogdf::node ogdfNode = pathOgdfNode->m_ogdfNodes[i];
            QPointF point(graphAttributes->x(ogdfNode), graphAttributes->y(ogdfNode));
            m_linePoints.push_back(point);
        }
    }

    //If we are in double mode and this node's complement is also drawn,
    //then we should shift the points so the two nodes are not drawn directly
    //on top of each other.
    if (g_settings->doubleMode && deBruijnNode->getReverseComplement()->isDrawn())
        shiftPointsLeft();

    remakePath();
}


//This constructor makes a new GraphicsItemNode by copying the line points of
//the given node.
GraphicsItemNode::GraphicsItemNode(DeBruijnNode* deBruijnNode,
                                   GraphicsItemNode* toCopy,
                                   CommonGraphicsItemNode* parent) :
                                   CommonGraphicsItemNode(g_graphicsView, parent), m_deBruijnNode(deBruijnNode)
{
    m_hasArrow = toCopy->m_hasArrow;
    m_linePoints = toCopy->m_linePoints;
    setWidth();
    remakePath();
}

//This constructor makes a new GraphicsItemNode with a specific collection of
//line points.
GraphicsItemNode::GraphicsItemNode(DeBruijnNode * deBruijnNode,
                                   std::vector<QPointF> linePoints,
                                   CommonGraphicsItemNode * parent) :
    CommonGraphicsItemNode(g_graphicsView, parent), m_deBruijnNode(deBruijnNode)
{
    m_hasArrow = g_settings->doubleMode;
    m_linePoints = linePoints;
    setWidth();
    remakePath();
}

void GraphicsItemNode::paint(QPainter * painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    //This code lets me see the node's bounding box.
    //I use it for debugging graphics issues.
//    painter->setBrush(Qt::NoBrush);
//    painter->setPen(QPen(Qt::black, 1.0));
//    painter->drawRect(boundingRect());

    if (m_deBruijnNode->isNodeUnion()) {
        QPainterPath outlinePath = shape();
        int width = g_settings->averageNodeWidth;
        int x = m_linePoints[0].x();
        int y = m_linePoints[0].y();
        //QPainter painter;
        QColor fillColor = QColor(200, 200, 200);
        QBrush brush;
        //brush.setStyle(Qt::Sol);
        //brush.setColor(fillColor);
        painter->setBrush(fillColor);
        painter->setPen(QPen(Qt::black, 1.0));
        QRect r(x, y, width, width);
        r.moveCenter(m_linePoints[0].toPoint());
        painter->fillPath(outlinePath, brush);
        painter->drawEllipse(r);
        return;
    }

    QPainterPath outlinePath = shape();

    //Fill the node's colour
    QBrush brush(m_colour);
    if (g_settings->propagateTaxColour && m_deBruijnNode->getTax(g_settings->taxRank) == NULL && 
        m_colour != QColor(200, 200, 200) && g_settings->nodeColourScheme == COLOUR_BY_TAX) {
        brush.setStyle(Qt::Dense2Pattern);
        painter->setBrush(brush);
        painter->setPen(QPen(Qt::black, 1.0));
    }
    painter->fillPath(outlinePath, brush);

    bool nodeHasBlastHits;
    if (g_settings->doubleMode)
        nodeHasBlastHits = m_deBruijnNode->thisNodeHasBlastHits();
    else
        nodeHasBlastHits = m_deBruijnNode->thisNodeOrReverseComplementHasBlastHits();

    //If the node contains a BLAST hit, draw that on top.
    if (nodeHasBlastHits && (g_settings->nodeColourScheme == BLAST_HITS_RAINBOW_COLOUR ||
            g_settings->nodeColourScheme == BLAST_HITS_SOLID_COLOUR ||
            g_settings->nodeColourScheme == BLAST_HITS_CLASS_COLOURS))
    {
        std::vector<BlastHitPart> parts;

        //The scaled node length is passed to the function which makes the
        //BlastHitPart objects, because we don't want those parts to be much
        //less than 1 pixel in size, which isn't necessary and can cause weird
        //visual artefacts.
        double scaledNodeLength = getNodePathLength() * g_absoluteZoom;

        if (g_settings->doubleMode)
        {
            if (m_deBruijnNode->thisNodeHasBlastHits())
                parts = m_deBruijnNode->getBlastHitPartsForThisNode(scaledNodeLength);
        }
        else
        {
            if (m_deBruijnNode->thisNodeOrReverseComplementHasBlastHits())
                parts = m_deBruijnNode->getBlastHitPartsForThisNodeOrReverseComplement(scaledNodeLength);
        }

        QPen partPen;
        partPen.setWidthF(m_width);
        partPen.setCapStyle(Qt::FlatCap);
        partPen.setJoinStyle(Qt::BevelJoin);

        //If the node has an arrow, then it's necessary to use the outline
        //as a clipping path so the colours don't extend past the edge of the
        //node.
        if (m_hasArrow)
            painter->setClipPath(outlinePath);

        for (size_t i = 0; i < parts.size(); ++i)
        {
            partPen.setColor(parts[i].m_colour);
            painter->setPen(partPen);

            painter->drawPath(makePartialPath(parts[i].m_nodeFractionStart,
                                              parts[i].m_nodeFractionEnd));
        }
        painter->setClipping(false);
    }


    //Draw the node outline
    QColor outlineColour = g_settings->outlineColour;
    double outlineThickness = g_settings->outlineThickness;
    if (isSelected())
    {
        outlineColour = g_settings->selectionColour;
        outlineThickness = g_settings->selectionThickness;
    }
    if (outlineThickness > 0.0)
    {
        outlinePath = outlinePath.simplified();
        QPen outlinePen(QBrush(outlineColour), outlineThickness, Qt::SolidLine,
                        Qt::SquareCap, Qt::RoundJoin);
        painter->setPen(outlinePen);
        painter->drawPath(outlinePath);
    }


    //Draw the path highlighting outline, if appropriate
    if (g_memory->pathDialogIsVisible)
        exactPathHighlightNode(painter);


    //Draw the query path, if appropriate
    if (g_memory->queryPathDialogIsVisible)
        queryPathHighlightNode(painter);


    //Draw node labels if there are any to display.
    if (anyNodeDisplayText())
    {
        QStringList nodeText = getNodeText();
        drawNodeText(painter, nodeText);
    }

    //Draw BLAST hit labels, if appropriate.
    if (g_settings->displayBlastHits && nodeHasBlastHits)
    {
        std::vector<QString> blastHitText;
        std::vector<QPointF> blastHitLocation;

        if (g_settings->doubleMode)
            getBlastHitsTextAndLocationThisNode(&blastHitText, &blastHitLocation);
        else
            getBlastHitsTextAndLocationThisNodeOrReverseComplement(&blastHitText, &blastHitLocation);

        for (size_t i = 0; i < blastHitText.size(); ++i)
        {
            QString text = blastHitText[i];
            QPointF centre = blastHitLocation[i];

            QPainterPath textPath;
            QFontMetrics metrics(g_settings->labelFont);
            double shiftLeft = -metrics.boundingRect(text).width() / 2.0;
            textPath.addText(shiftLeft, 0.0, g_settings->labelFont, text);

            drawTextPathAtLocation(painter, textPath, centre);
        }
    }
}

void GraphicsItemNode::setNodeColour()
{
    if (g_settings->nodeColourScheme == SAVE_COLOURS && m_deBruijnNode->m_lastColor.isValid()) {
        m_colour = m_deBruijnNode->m_lastColor;
        return;
    }
    switch (g_settings->nodeColourScheme)
    {
    case UNIFORM_COLOURS:
        if (m_deBruijnNode->isSpecialNode())
            m_colour = g_settings->uniformNodeSpecialColour;
        else if (usePositiveNodeColour())
            m_colour = g_settings->uniformPositiveNodeColour;
        else
            m_colour = g_settings->uniformNegativeNodeColour;
        break;
    case RANDOM_COMPONENT_COLOURS:
    {
        if (m_deBruijnNode->getNameWithoutSign().endsWith("_start")) {
            QColor redColour;
            redColour.setRgb(255, 0, 0);
            m_colour = redColour;
        }
        else
        {
            if (g_hicSettings->isTargetComponent(m_deBruijnNode->getComponentId())) {
                QColor orangeColour;
                orangeColour.setRgb(255, 130, 5);
                m_colour = orangeColour;
                DeBruijnNode* revCompNode = m_deBruijnNode->getReverseComplement();
                if (revCompNode != 0)
                {
                    GraphicsItemNode* revCompGraphNode = revCompNode->getGraphicsItemNode();
                    if (revCompGraphNode != 0)
                        revCompGraphNode->m_colour = orangeColour;
                }
                break;
            }
            std::srand(m_deBruijnNode->getComponentId());
            int hue = rand() % 360;
            QColor posColour;
            posColour.setHsl(hue,
                g_settings->randomColourPositiveSaturation,
                g_settings->randomColourPositiveLightness);
            posColour.setAlpha(g_settings->randomColourPositiveOpacity);

            QColor negColour;
            negColour.setHsl(hue,
                g_settings->randomColourNegativeSaturation,
                g_settings->randomColourNegativeLightness);
            negColour.setAlpha(g_settings->randomColourNegativeOpacity);

            QColor colour1, colour2;
            if (m_deBruijnNode->isPositiveNode())
            {
                colour1 = posColour;
                colour2 = negColour;
            }
            else
            {
                colour1 = negColour;
                colour2 = posColour;
            }

            m_colour = colour1;
            DeBruijnNode* revCompNode = m_deBruijnNode->getReverseComplement();
            if (revCompNode != 0)
            {
                GraphicsItemNode* revCompGraphNode = revCompNode->getGraphicsItemNode();
                if (revCompGraphNode != 0)
                    revCompGraphNode->m_colour = colour2;
            }
        }
        break;
    }
    case COLOUR_BY_TAX:
    {
        QColor colour = QColor(200, 200, 200);
        if (m_deBruijnNode->getTax() != NULL) {
            int rank = g_settings->taxRank;
            tax* curTax = m_deBruijnNode->getTax()->getTaxHierarchy(rank);
            if (curTax != NULL) {
                colour = curTax->getColor();
            }
        }
        else if (g_settings->propagateTaxColour) {
            colour = propagateColour();
        }
        m_colour = colour;
        DeBruijnNode* revCompNode = m_deBruijnNode->getReverseComplement();
        if (revCompNode != 0)
        {
            GraphicsItemNode* revCompGraphNode = revCompNode->getGraphicsItemNode();
            if (revCompGraphNode != 0)
                revCompGraphNode->m_colour = colour;
        }
        break;
    }
    case (RANDOM_COLOURS || SAVE_COLOURS):
    {
        //Make a colour with a random hue.  Assign a colour to both this node and
        //it complement so their hue matches.
        int hue = rand() % 360;
        QColor posColour;
        posColour.setHsl(hue,
                         g_settings->randomColourPositiveSaturation,
                         g_settings->randomColourPositiveLightness);
        posColour.setAlpha(g_settings->randomColourPositiveOpacity);

        QColor negColour;
        negColour.setHsl(hue,
                         g_settings->randomColourNegativeSaturation,
                         g_settings->randomColourNegativeLightness);
        negColour.setAlpha(g_settings->randomColourNegativeOpacity);

        QColor colour1, colour2;
        if (m_deBruijnNode->isPositiveNode())
        {
            colour1 = posColour;
            colour2 = negColour;
        }
        else
        {
            colour1 = negColour;
            colour2 = posColour;
        }

        m_colour = colour1;
        DeBruijnNode * revCompNode = m_deBruijnNode->getReverseComplement();
        if (revCompNode != 0)
        {
            GraphicsItemNode * revCompGraphNode = revCompNode->getGraphicsItemNode();
            if (revCompGraphNode != 0)
                revCompGraphNode->m_colour = colour2;
        }
        break;
    }

    case DEPTH_COLOUR:
    {
        m_colour = getDepthColour();
        break;
    }

    case BLAST_HITS_RAINBOW_COLOUR:
    {
        m_colour = g_settings->noBlastHitsColour;
        break;
    }

    case BLAST_HITS_SOLID_COLOUR:
    {
        m_colour = g_settings->noBlastHitsColour;
        break;
    }

    case BLAST_HITS_CLASS_COLOURS:
    {
        m_colour = g_settings->noBlastHitsColour;
        break;
    }
    case CUSTOM_COLOURS:
    {
        m_colour = m_deBruijnNode->getCustomColourForDisplay();
        break;
    }

    default: //CONTIGUITY COLOUR
    {
        //For single nodes, display the colour of whichever of the
        //twin nodes has the greatest contiguity status.
        ContiguityStatus contiguityStatus = m_deBruijnNode->getContiguityStatus();
        if (!m_hasArrow)
        {
            ContiguityStatus twinContiguityStatus = m_deBruijnNode->getReverseComplement()->getContiguityStatus();
            if (twinContiguityStatus < contiguityStatus)
                contiguityStatus = twinContiguityStatus;
        }

        switch (contiguityStatus)
        {
        case STARTING:
            m_colour = g_settings->contiguityStartingColour;
            break;
        case CONTIGUOUS_STRAND_SPECIFIC:
            m_colour = g_settings->contiguousStrandSpecificColour;
            break;
        case CONTIGUOUS_EITHER_STRAND:
            m_colour = g_settings->contiguousEitherStrandColour;
            break;
        case MAYBE_CONTIGUOUS:
            m_colour = g_settings->maybeContiguousColour;
            break;
        default: //NOT_CONTIGUOUS
            m_colour = g_settings->notContiguousColour;
            break;
        }
    }
    }
    m_deBruijnNode->m_lastColor = m_colour;
}

QColor GraphicsItemNode::propagateColour() {
    int rank = g_settings->taxRank;
    QColor color = QColor(200, 200, 200);
    if (m_deBruijnNode->getTax(rank) == NULL) {
        tax* prevNodeTax = NULL;
        bool flag = false;
        for (DeBruijnEdge* edge : m_deBruijnNode->getEnteringEdges()) {
            DeBruijnNode* prevNode = edge->getStartingNode();
            tax* prevTax = prevNode->getTax(rank);
            if (prevTax != NULL) {
                if (!flag) {
                    flag = true;
                    prevNodeTax = prevTax;
                }
                else {
                    if (prevNodeTax->getTaxId() != prevTax->getTaxId()) {
                        flag = false;
                        return color;
                    }
                }
            }
        }
        flag = false;
        tax* nextNodeTax = NULL;
        for (DeBruijnEdge* edge : m_deBruijnNode->getLeavingEdges()) {
            DeBruijnNode* nextNode = edge->getEndingNode();
            tax* nextTax = nextNode->getTax(rank);
            if (nextTax != NULL) {
                if (!flag) {
                    flag = true;
                    nextNodeTax = nextTax;
                }
                else if (nextNodeTax->getTaxId() != nextTax->getTaxId()) {
                    flag = false;
                    return color;
                }
            }
        }
        if (flag && prevNodeTax!= NULL && nextNodeTax != NULL && prevNodeTax->getTaxId() == nextNodeTax->getTaxId()) {
            return prevNodeTax->getColor();
        }
        return color;
    }
    else {
        return m_deBruijnNode->getTax(rank)->getColor();
    }
}

QPainterPath GraphicsItemNode::shape() const
{
    if (m_deBruijnNode->isNodeUnion()) {
        int width = g_settings->averageNodeWidth;
        QPainterPath mainNodePath;
        mainNodePath.addEllipse(m_linePoints[0].toPoint(), width, width);
        return mainNodePath;
    }
    //If there is only one segment and it is shorter than half its
    //width, then the arrow head will not be made with 45 degree
    //angles, but rather whatever angle is made by going from the
    //end to the back corners (the final node will be a triangle).
    if (m_hasArrow
            && m_linePoints.size() == 2
            && distance(getLast(), getSecondLast()) < m_width / 2.0)
    {
        QLineF backline = QLineF(getSecondLast(), getLast()).normalVector();
        backline.setLength(m_width / 2.0);
        QPointF backVector = backline.p2() - backline.p1();
        QPainterPath trianglePath;
        trianglePath.moveTo(getLast());
        trianglePath.lineTo(getSecondLast() + backVector);
        trianglePath.lineTo(getSecondLast() - backVector);
        trianglePath.lineTo(getLast());
        return trianglePath;
    }

    //Create a path that outlines the main node shape.
    QPainterPathStroker stroker;
    stroker.setWidth(m_width);
    stroker.setCapStyle(Qt::FlatCap);
    stroker.setJoinStyle(Qt::RoundJoin);
    QPainterPath mainNodePath = stroker.createStroke(m_path);

    if (!m_hasArrow)
        return mainNodePath;

    //If the node has an arrow head, subtract the part of its
    //final segment to give it a pointy end.
    //NOTE: THIS APPROACH CAN LEAD TO WEIRD EFFECTS WHEN THE NODE'S
    //POINTY END OVERLAPS WITH ANOTHER PART OF THE NODE.  PERHAPS THERE
    //IS A BETTER WAY TO MAKE ARROWHEADS?
    QLineF frontline = QLineF(getLast(), getSecondLast()).normalVector();
    frontline.setLength(m_width / 2.0);
    QPointF frontVector = frontline.p2() - frontline.p1();
    QLineF arrowheadLine(getLast(), getSecondLast());
    arrowheadLine.setLength(1.42 * (m_width / 2.0));
    arrowheadLine.setAngle(arrowheadLine.angle() + 45.0);
    QPointF arrow1 = arrowheadLine.p2();
    arrowheadLine.setAngle(arrowheadLine.angle() - 90.0);
    QPointF arrow2 = arrowheadLine.p2();
    QLineF lastSegmentLine(getSecondLast(), getLast());
    lastSegmentLine.setLength(0.01);
    QPointF additionalForwardBit = lastSegmentLine.p2() - lastSegmentLine.p1();
    QPainterPath subtractionPath;
    subtractionPath.moveTo(getLast());
    subtractionPath.lineTo(arrow1);
    subtractionPath.lineTo(getLast() + frontVector + additionalForwardBit);
    subtractionPath.lineTo(getLast() - frontVector + additionalForwardBit);
    subtractionPath.lineTo(arrow2);
    subtractionPath.lineTo(getLast());
    return mainNodePath.subtracted(subtractionPath);
}

void GraphicsItemNode::mousePressEvent(QGraphicsSceneMouseEvent * event)
{
    updateGrabIndex(event);
}

//When this node graphics item is moved, each of the connected edge
//graphics items will need to be adjusted accordingly.
void GraphicsItemNode::mouseRoundEvent(QGraphicsSceneMouseEvent* event) {

}

//When this node graphics item is moved, each of the connected edge
//graphics items will need to be adjusted accordingly.
void GraphicsItemNode::mouseMoveEvent(QGraphicsSceneMouseEvent * event)
{
    if (g_settings->roundMode) {
        QPointF lastPos = event->lastPos(); //B
        QPointF newPos = event->pos(); //C
        QPointF centralPos; //A
        if (m_grabIndex > m_linePoints.size() / 2) {
            centralPos = m_linePoints[0];
        }
        else {
            centralPos = m_linePoints[m_linePoints.size() - 1];
        }

        MyGraphicsScene* graphicsScene = dynamic_cast<MyGraphicsScene*>(scene());

        std::vector<GraphicsItemNode*> nodesToMove;
        nodesToMove.push_back(this);
        double alpha = angleBetweenTwoLines(centralPos, lastPos, centralPos, newPos);
        roundPoints(centralPos, alpha);
        remakePath();
        graphicsScene->possiblyExpandSceneRectangle(&nodesToMove);

        fixEdgePaths(&nodesToMove);
    }
    else {
        QPointF difference = event->pos() - event->lastPos();

        //If this node is selected, then move all of the other selected nodes too.
        //If it is not selected, then only move this node.
        std::vector<GraphicsItemNode*> nodesToMove;
        MyGraphicsScene* graphicsScene = dynamic_cast<MyGraphicsScene*>(scene());
        if (isSelected())
            nodesToMove = graphicsScene->getSelectedGraphicsItemNodes();
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
}



//This function remakes edge paths.  If nodes is passed, it will remake the
//edge paths for all of the nodes.  If nodes isn't passed, then it will just
//do it for this node.
void GraphicsItemNode::fixEdgePaths(std::vector<GraphicsItemNode *> * nodes)
{
    std::set<DeBruijnEdge *> edgesToFix;

    if (nodes == 0)
    {
        const std::vector<DeBruijnEdge *> * edges = m_deBruijnNode->getEdgesPointer();
        for (size_t j = 0; j < edges->size(); ++j)
            edgesToFix.insert((*edges)[j]);
    }
    else
    {
        for (size_t i = 0; i < nodes->size(); ++i)
        {
            DeBruijnNode * node = (*nodes)[i]->m_deBruijnNode;
            const std::vector<DeBruijnEdge *> * edges = node->getEdgesPointer();
            for (size_t j = 0; j < edges->size(); ++j)
                edgesToFix.insert((*edges)[j]);
        }
    }

    for (std::set<DeBruijnEdge *>::iterator i = edgesToFix.begin(); i != edgesToFix.end(); ++i)
    {
        DeBruijnEdge * deBruijnEdge = *i;
        GraphicsItemEdge * graphicsItemEdge = deBruijnEdge->getGraphicsItemEdge();

        //If this edge has a graphics item, adjust it now.
        if (graphicsItemEdge != 0)
            graphicsItemEdge->calculateAndSetPath();

        //If this edge does not have a graphics item, then perhaps its
        //reverse complment does.  Only do this check if the graph was drawn
        //on single mode.
        else if (!g_settings->doubleMode)
        {
            graphicsItemEdge = deBruijnEdge->getReverseComplement()->getGraphicsItemEdge();
            if (graphicsItemEdge != 0)
                graphicsItemEdge->calculateAndSetPath();
        }
    }
}

bool GraphicsItemNode::usePositiveNodeColour()
{
    return !m_hasArrow || m_deBruijnNode->isPositiveNode();
}

QStringList GraphicsItemNode::getNodeText()
{
    QStringList nodeText;
    if (g_settings->displayNodeCustomLabels)
        nodeText << m_deBruijnNode->getCustomLabelForDisplay();
    if (g_settings->displayNodeNames)
    {
        QString nodeName = m_deBruijnNode->getName();
        if (!g_settings->doubleMode)
            nodeName.chop(1);
        nodeText << nodeName;
    }
    if (g_settings->displayNodeLengths)
        nodeText << formatIntForDisplay(m_deBruijnNode->getLength()) + " bp";
    if (g_settings->displayNodeDepth)
        nodeText << formatDepthForDisplay(m_deBruijnNode->getDepth());
    if (g_settings->displayNodeCsvData && m_deBruijnNode->hasCsvData())
        nodeText << m_deBruijnNode->getCsvLine(g_settings->displayNodeCsvDataCol);
    if (g_settings->displayTaxIdName && m_deBruijnNode->getTax() != NULL) {
        tax* curTax = m_deBruijnNode->getTax();
        if (curTax != NULL) {
            nodeText << curTax->getName() << QString::number(curTax->getTaxId());
        }
    }
    else {
        if (g_settings->displayTaxNameRank && m_deBruijnNode->getTax() != NULL) {
            tax* curTax = m_deBruijnNode->getTax(g_settings->taxRank);
            if (curTax != NULL) {
                nodeText << curTax->getName();
            }
        }
        if (g_settings->displayTaxIdRank && m_deBruijnNode->getTax() != NULL) {
            tax* curTax = m_deBruijnNode->getTax(g_settings->taxRank);
            if (curTax != NULL) {
                nodeText << QString::number(curTax->getTaxId());
            }
        }
    }
        
    return nodeText;
}

QSize GraphicsItemNode::getNodeTextSize(QString text)
{
    QFontMetrics fontMetrics(g_settings->labelFont);
    return fontMetrics.size(0, text);
}

QColor GraphicsItemNode::getDepthColour()
{
    double depth = m_deBruijnNode->getDepth();
    double lowValue;
    double highValue;
    if (g_settings->autoDepthValue)
    {
        lowValue = g_assemblyGraph->m_firstQuartileDepth;
        highValue = g_assemblyGraph->m_thirdQuartileDepth;
    }
    else
    {
        lowValue = g_settings->lowDepthValue;
        highValue = g_settings->highDepthValue;
    }

    if (depth <= lowValue)
        return g_settings->lowDepthColour;
    if (depth >= highValue)
        return g_settings->highDepthColour;

    double fraction = (depth - lowValue) / (highValue - lowValue);

    int redDifference = g_settings->highDepthColour.red() - g_settings->lowDepthColour.red();
    int greenDifference = g_settings->highDepthColour.green() - g_settings->lowDepthColour.green();
    int blueDifference = g_settings->highDepthColour.blue() - g_settings->lowDepthColour.blue();
    int alphaDifference = g_settings->highDepthColour.alpha() - g_settings->lowDepthColour.alpha();

    int red = int(g_settings->lowDepthColour.red() + (fraction * redDifference) + 0.5);
    int green = int(g_settings->lowDepthColour.green() + (fraction * greenDifference) + 0.5);
    int blue = int(g_settings->lowDepthColour.blue() + (fraction * blueDifference) + 0.5);
    int alpha = int(g_settings->lowDepthColour.alpha() + (fraction * alphaDifference) + 0.5);

    return QColor(red, green, blue, alpha);
}

void GraphicsItemNode::setWidth()
{
    m_width = getNodeWidth(m_deBruijnNode->getDepthRelativeToMeanDrawnDepth(), g_settings->depthPower,
                           g_settings->depthEffectOnWidth, g_settings->averageNodeWidth);
    if (m_width < 0.0)
        m_width = 0.0;
}

//The bounding rectangle of a node has to be a little bit bigger than
//the node's path, because of the outline.  The selection outline is
//the largest outline we can expect, so use that to define the bounding
//rectangle.
QRectF GraphicsItemNode::boundingRect() const
{
    double extraSize = g_settings->selectionThickness / 2.0;
    QRectF bound = shape().boundingRect();

    bound.setTop(bound.top() - extraSize);
    bound.setBottom(bound.bottom() + extraSize);
    bound.setLeft(bound.left() - extraSize);
    bound.setRight(bound.right() + extraSize);

    return bound;
}


double GraphicsItemNode::getNodeWidth(double depthRelativeToMeanDrawnDepth, double depthPower,
                                      double depthEffectOnWidth, double averageNodeWidth)
{
    if (depthRelativeToMeanDrawnDepth < 0.0)
        depthRelativeToMeanDrawnDepth = 0.0;
    double widthRelativeToAverage = (pow(depthRelativeToMeanDrawnDepth, depthPower) - 1.0) * depthEffectOnWidth + 1.0;
    return averageNodeWidth * widthRelativeToAverage;
}

void GraphicsItemNode::getBlastHitsTextAndLocationThisNode(std::vector<QString> * blastHitText,
                                                       std::vector<QPointF> * blastHitLocation)
{
    const std::vector<BlastHit *> * blastHits = m_deBruijnNode->getBlastHitsPointer();
    for (size_t i = 0; i < blastHits->size(); ++i)
    {
        BlastHit * hit = (*blastHits)[i];
        blastHitText->push_back(hit->m_query->getName());
        blastHitLocation->push_back(findLocationOnPath(hit->getNodeCentreFraction()));
    }
}

void GraphicsItemNode::getBlastHitsTextAndLocationThisNodeOrReverseComplement(std::vector<QString> * blastHitText,
                                                                          std::vector<QPointF> * blastHitLocation)
{
    getBlastHitsTextAndLocationThisNode(blastHitText, blastHitLocation);

    const std::vector<BlastHit *> * blastHits = m_deBruijnNode->getReverseComplement()->getBlastHitsPointer();
    for (size_t i = 0; i < blastHits->size(); ++i)
    {
        BlastHit * hit = (*blastHits)[i];
        blastHitText->push_back(hit->m_query->getName());
        blastHitLocation->push_back(findLocationOnPath(1.0 - hit->getNodeCentreFraction()));
    }
}

bool GraphicsItemNode::anyNodeDisplayText()
{
    return g_settings->displayNodeCustomLabels ||
            g_settings->displayNodeNames ||
            g_settings->displayNodeLengths ||
            g_settings->displayNodeDepth ||
            g_settings->displayNodeCsvData ||
            g_settings->displayTaxIdName ||
            g_settings->displayTaxIdRank ||
            g_settings->displayTaxNameRank ;
}

//This function outlines and shades the appropriate part of a node if it is
//in the user-specified path.
void GraphicsItemNode::exactPathHighlightNode(QPainter* painter)
{
    if (g_memory->userSpecifiedPath.containsNode(m_deBruijnNode))
        pathHighlightNode2(painter, m_deBruijnNode, false, &g_memory->userSpecifiedPath);

    if (!g_settings->doubleMode &&
        g_memory->userSpecifiedPath.containsNode(m_deBruijnNode->getReverseComplement()))
        pathHighlightNode2(painter, m_deBruijnNode->getReverseComplement(), true, &g_memory->userSpecifiedPath);
}

//This function outlines and shades the appropriate part of a node if it is
//in the user-specified path.
void GraphicsItemNode::queryPathHighlightNode(QPainter* painter)
{
    if (g_memory->queryPaths.size() == 0)
        return;

    for (int i = 0; i < g_memory->queryPaths.size(); ++i)
    {
        Path* path = &(g_memory->queryPaths[i]);
        if (path->containsNode(m_deBruijnNode))
            pathHighlightNode2(painter, m_deBruijnNode, false, path);

        if (!g_settings->doubleMode &&
            path->containsNode(m_deBruijnNode->getReverseComplement()))
            pathHighlightNode2(painter, m_deBruijnNode->getReverseComplement(), true, path);
    }
}

void GraphicsItemNode::pathHighlightNode2(QPainter* painter,
    DeBruijnNode* node,
    bool reverse,
    Path* path)
{
    int numberOfTimesInMiddle = path->numberOfOccurrencesInMiddleOfPath(node);
    for (int i = 0; i < numberOfTimesInMiddle; ++i)
        pathHighlightNode3(painter, shape());

    bool isStartingNode = path->isStartingNode(node);
    bool isEndingNode = path->isEndingNode(node);

    //If this is the only node in the path, then we limit the highlighting to the appropriate region.
    if (isStartingNode && isEndingNode && path->getNodeCount() == 1)
    {
        pathHighlightNode3(painter, buildPartialHighlightPath(path->getStartFraction(), path->getEndFraction(), reverse));
        return;
    }

    if (isStartingNode)
        pathHighlightNode3(painter, buildPartialHighlightPath(path->getStartFraction(), 1.0, reverse));

    if (isEndingNode)
        pathHighlightNode3(painter, buildPartialHighlightPath(0.0, path->getEndFraction(), reverse));
}

void GraphicsItemNode::pathHighlightNode3(QPainter* painter,
    QPainterPath highlightPath)
{
    QBrush shadingBrush(g_settings->pathHighlightShadingColour);
    painter->fillPath(highlightPath, shadingBrush);

    highlightPath = highlightPath.simplified();
    QPen outlinePen(QBrush(g_settings->pathHighlightOutlineColour),
        g_settings->selectionThickness, Qt::SolidLine,
        Qt::SquareCap, Qt::RoundJoin);
    painter->setPen(outlinePen);
    painter->drawPath(highlightPath);
}

QPainterPath GraphicsItemNode::buildPartialHighlightPath(double startFraction,
    double endFraction,
    bool reverse)
{
    if (reverse)
    {
        startFraction = 1.0 - startFraction;
        endFraction = 1.0 - endFraction;
        std::swap(startFraction, endFraction);
    }

    QPainterPath partialPath = makePartialPath(startFraction,
        endFraction);

    QPainterPathStroker stroker;

    //If the node has an arrow, we need a path intersection with the
    //shape to make sure the arrowhead is part of the path.  Adding a bit
    //to the width seems to help with the intersection.
    if (m_hasArrow)
        stroker.setWidth(m_width + 0.1);
    else
        stroker.setWidth(m_width);

    stroker.setCapStyle(Qt::FlatCap);
    stroker.setJoinStyle(Qt::RoundJoin);
    QPainterPath highlightPath = stroker.createStroke(partialPath);

    if (m_hasArrow)
        highlightPath = highlightPath.intersected(shape());

    return highlightPath;
}