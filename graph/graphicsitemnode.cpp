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
#include <ogdf/basic/GraphAttributes.h>
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
#include <cmath>
#include <QFontMetrics>

GraphicsItemNode::GraphicsItemNode(DeBruijnNode * deBruijnNode,
                                   ogdf::GraphAttributes * graphAttributes, QGraphicsItem * parent) :
    QGraphicsItem(parent), m_deBruijnNode(deBruijnNode),
    m_hasArrow(g_settings->doubleMode)

{
    setWidth();

    OgdfNode * pathOgdfNode = deBruijnNode->m_ogdfNode;
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
        pathOgdfNode = deBruijnNode->m_reverseComplement->m_ogdfNode;
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
    if (g_settings->doubleMode && deBruijnNode->m_reverseComplement->m_drawn)
        shiftPointsLeft();

    remakePath();
}


void GraphicsItemNode::paint(QPainter * painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    //This code lets me see the node's bounding box.
    //I use it for debugging graphics issues.
//    painter->setBrush(Qt::NoBrush);
//    painter->setPen(QPen(Qt::black, 1.0));
//    painter->drawRect(boundingRect());

    QPainterPath outlinePath = shape();

    //Fill the node's colour
    QBrush brush(m_colour);
    painter->fillPath(outlinePath, brush);

    bool nodeHasBlastHits;
    if (g_settings->doubleMode)
        nodeHasBlastHits = m_deBruijnNode->thisNodeHasBlastHits();
    else
        nodeHasBlastHits = m_deBruijnNode->thisNodeOrReverseComplementHasBlastHits();

    //If the node contains a BLAST hit, draw that on top.
    if (nodeHasBlastHits && (g_settings->nodeColourScheme == BLAST_HITS_RAINBOW_COLOUR ||
            g_settings->nodeColourScheme == BLAST_HITS_SOLID_COLOUR))
    {
        std::vector<BlastHitPart> parts;

        if (g_settings->doubleMode)
        {
            if (m_deBruijnNode->thisNodeHasBlastHits())
                parts = m_deBruijnNode->getBlastHitPartsForThisNode();
        }
        else
        {
            if (m_deBruijnNode->thisNodeOrReverseComplementHasBlastHits())
                parts = m_deBruijnNode->getBlastHitPartsForThisNodeOrReverseComplement();
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

            painter->drawPath(makePartialPath(parts[i].m_nodeFractionStart, parts[i].m_nodeFractionEnd));
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
    if (g_settings->pathDialogIsVisible && pathHighlightNode(m_deBruijnNode))
    {
        outlineColour = g_settings->outlineColour;
        outlineThickness = g_settings->selectionThickness;
    }
    if (outlineThickness > 0.0)
    {
        outlinePath = outlinePath.simplified();
        QPen outlinePen(QBrush(outlineColour), outlineThickness, Qt::SolidLine, Qt::SquareCap, Qt::RoundJoin);
        painter->setPen(outlinePen);
        painter->drawPath(outlinePath);
    }


    //Draw node labels if there are any to display.
    if (g_settings->anyNodeDisplayText())
    {
        QStringList nodeText = getNodeText();
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

        std::vector<QPointF> centres;
        if (g_settings->positionTextNodeCentre)
            centres.push_back(getCentre(m_linePoints));
        else
            centres = getCentres();

        for (size_t i = 0; i < centres.size(); ++i)
            drawTextPathAtLocation(painter, textPath, centres[i]);
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
            double shiftLeft = -metrics.width(text) / 2.0;
            textPath.addText(shiftLeft, 0.0, g_settings->labelFont, text);

            drawTextPathAtLocation(painter, textPath, centre);
        }
    }
}


bool GraphicsItemNode::pathHighlightNode(DeBruijnNode * node)
{
    if (g_settings->doubleMode)
        return g_settings->userSpecifiedPathNodes.contains(node);
    else
        return (g_settings->userSpecifiedPathNodes.contains(node) || g_settings->userSpecifiedPathNodes.contains(node->m_reverseComplement));
}


void GraphicsItemNode::drawTextPathAtLocation(QPainter * painter, QPainterPath textPath, QPointF centre)
{
    QRectF textBoundingRect = textPath.boundingRect();
    double textHeight = textBoundingRect.height();
    QPointF offset(0.0, textHeight / 2.0);

    painter->translate(centre);
    painter->rotate(-g_graphicsView->m_rotation);
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
    painter->rotate(g_graphicsView->m_rotation);
    painter->translate(-centre);
}



void GraphicsItemNode::setNodeColour()
{
    switch (g_settings->nodeColourScheme)
    {
    case ONE_COLOUR:
        if (m_deBruijnNode->m_startingNode)
            m_colour = g_settings->uniformNodeSpecialColour;
        else if (usePositiveNodeColour())
            m_colour = g_settings->uniformPositiveNodeColour;
        else
            m_colour = g_settings->uniformNegativeNodeColour;
        break;

    case RANDOM_COLOURS:
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
        DeBruijnNode * revCompNode = m_deBruijnNode->m_reverseComplement;
        if (revCompNode != 0)
        {
            GraphicsItemNode * revCompGraphNode = revCompNode->m_graphicsItemNode;
            if (revCompGraphNode != 0)
                revCompGraphNode->m_colour = colour2;
        }
        break;
    }

    case COVERAGE_COLOUR:
    {
        m_colour = getCoverageColour();
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

    case CUSTOM_COLOURS:
    {
        m_colour = m_deBruijnNode->m_customColour;
        break;
    }

    default: //CONTIGUITY COLOUR
    {
        //For single nodes, display the colour of whichever of the
        //twin nodes has the greatest contiguity status.
        ContiguityStatus contiguityStatus = m_deBruijnNode->m_contiguityStatus;
        if (!m_hasArrow)
        {
            ContiguityStatus twinContiguityStatus = m_deBruijnNode->m_reverseComplement->m_contiguityStatus;
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
}


QPainterPath GraphicsItemNode::shape() const
{
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
    m_grabIndex = 0;
    QPointF grabPoint = event->pos();

    double closestPointDistance = distance(grabPoint, m_linePoints[0]);
    for (size_t i = 1; i < m_linePoints.size(); ++i)
    {
        double pointDistance = distance(grabPoint, m_linePoints[i]);
        if (pointDistance < closestPointDistance)
        {
            closestPointDistance = pointDistance;
            m_grabIndex = i;
        }
    }
}


//When this node graphics item is moved, each of the connected edge
//graphics items will need to be adjusted accordingly.
void GraphicsItemNode::mouseMoveEvent(QGraphicsSceneMouseEvent * event)
{
    QPointF difference = event->pos() - event->lastPos();

    //If this node is selected, then move all of the other selected nodes too.
    //If it is not selected, then only move this node.
    std::vector<GraphicsItemNode *> nodesToMove;
    MyGraphicsScene * graphicsScene = dynamic_cast<MyGraphicsScene *>(scene());
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

    //It is now necessary to remake the paths for each edge that is connected
    //to a moved node.
    std::set<DeBruijnEdge *> edgesToFix;
    for (size_t i = 0; i < nodesToMove.size(); ++i)
    {
        DeBruijnNode * node = nodesToMove[i]->m_deBruijnNode;
        for (size_t j = 0; j < node->m_edges.size(); ++j)
            edgesToFix.insert(node->m_edges[j]);
    }

    for (std::set<DeBruijnEdge *>::iterator i = edgesToFix.begin(); i != edgesToFix.end(); ++i)
    {
        DeBruijnEdge * deBruijnEdge = *i;
        GraphicsItemEdge * graphicsItemEdge = deBruijnEdge->m_graphicsItemEdge;

        //If this edge has a graphics item, adjust it now.
        if (graphicsItemEdge != 0)
            graphicsItemEdge->calculateAndSetPath();

        //If this edge does not have a graphics item, then perhaps its
        //reverse complment does.  Only do this check if the graph was drawn
        //on single mode
        else if (!g_settings->doubleMode)
        {
            graphicsItemEdge = deBruijnEdge->m_reverseComplement->m_graphicsItemEdge;
            if (graphicsItemEdge != 0)
                graphicsItemEdge->calculateAndSetPath();
        }
    }
}


void GraphicsItemNode::shiftPoints(QPointF difference)
{
    prepareGeometryChange();

    if (isSelected()) //Move all pieces for selected nodes
    {
        for (size_t i = 0; i < m_linePoints.size(); ++i)
            m_linePoints[i] += difference;
    }
    else if (g_settings->nodeDragging == ONE_PIECE)
        m_linePoints[m_grabIndex] += difference;

    else if (g_settings->nodeDragging == NEARBY_PIECES)
    {
        for (size_t i = 0; i < m_linePoints.size(); ++i)
        {
            int indexDistance = abs(int(i) - int(m_grabIndex));
            double dragStrength = pow(2.0, -1.0 * pow(double(indexDistance), 1.8) / g_settings->dragStrength); //constants chosen for dropoff of drag strength
            m_linePoints[i] += difference * dragStrength;
        }
    }
}

void GraphicsItemNode::remakePath()
{
    QPainterPath path;

    path.moveTo(m_linePoints[0]);
    for (size_t i = 1; i < m_linePoints.size(); ++i)
        path.lineTo(m_linePoints[i]);

    m_path = path;
}


QPainterPath GraphicsItemNode::makePartialPath(double startFraction, double endFraction)
{
    if (endFraction < startFraction)
        std::swap(startFraction, endFraction);

    double totalLength = getNodePathLength();

    QPainterPath path;
    bool pathStarted = false;
    double lengthSoFar = 0.0;
    for (size_t i = 0; i < m_linePoints.size() - 1; ++i)
    {
        QPointF point1 = m_linePoints[i];
        QPointF point2 = m_linePoints[i + 1];
        QLineF line(point1, point2);

        double point1Fraction = lengthSoFar / totalLength;
        lengthSoFar += line.length();
        double point2Fraction = lengthSoFar / totalLength;

        //If the path hasn't yet begun and this segment is before
        //the starting fraction, do nothing.
        if (!pathStarted && point2Fraction < startFraction)
            continue;

        //If the path hasn't yet begun but this segment covers the starting
        //fraction, start the path now.
        if (!pathStarted && point2Fraction >= startFraction)
        {
            pathStarted = true;
            path.moveTo(findIntermediatePoint(point1, point2, point1Fraction, point2Fraction, startFraction));
        }

        //If the path is in progress and this segment hasn't yet reached the end,
        //just continue the path.
        if (pathStarted && point2Fraction < endFraction)
            path.lineTo(point2);

        //If the path is in progress and this segment passes the end, finish the line.
        if (pathStarted && point2Fraction >= endFraction)
        {
            path.lineTo(findIntermediatePoint(point1, point2, point1Fraction, point2Fraction, endFraction));
            return path;
        }
    }

    return path;
}


double GraphicsItemNode::getNodePathLength()
{
    double totalLength = 0.0;
    for (size_t i = 0; i < m_linePoints.size() - 1; ++i)
    {
        QLineF line(m_linePoints[i], m_linePoints[i + 1]);
        totalLength += line.length();
    }
    return totalLength;
}


//This function will find the point that is a certain fraction of the way along the node's path.
QPointF GraphicsItemNode::findLocationOnPath(double fraction)
{
    double totalLength = getNodePathLength();

    double lengthSoFar = 0.0;
    for (size_t i = 0; i < m_linePoints.size() - 1; ++i)
    {
        QPointF point1 = m_linePoints[i];
        QPointF point2 = m_linePoints[i + 1];
        QLineF line(point1, point2);

        double point1Fraction = lengthSoFar / totalLength;
        lengthSoFar += line.length();
        double point2Fraction = lengthSoFar / totalLength;

        //If point2 hasn't yet reached the target, do nothing.
        if (point2Fraction < fraction)
            continue;

        //If the path hasn't yet begun but this segment covers the starting
        //fraction, start the path now.
        if (point2Fraction >= fraction)
            return findIntermediatePoint(point1, point2, point1Fraction, point2Fraction, fraction);
    }

    //The code shouldn't get here, as the target point should have been found in the above loop.
    return QPointF();
}

QPointF GraphicsItemNode::findIntermediatePoint(QPointF p1, QPointF p2, double p1Value, double p2Value, double targetValue)
{
    QPointF difference = p2 - p1;
    double fraction = (targetValue - p1Value) / (p2Value - p1Value);
    return difference * fraction + p1;
}

double GraphicsItemNode::distance(QPointF p1, QPointF p2) const
{
    double xDiff = p1.x() - p2.x();
    double yDiff = p1.y() - p2.y();
    return sqrt(xDiff * xDiff + yDiff * yDiff);
}


bool GraphicsItemNode::usePositiveNodeColour()
{
    return !m_hasArrow || m_deBruijnNode->isPositiveNode();
}




//This function returns the nodes' visible centres.  If the entire node is visible,
//then there is just one visible centre.  If none of the node is visible, then
//there are no visible centres.  If multiple parts of the node are visible, then there
//are multiple visible centres.
std::vector<QPointF> GraphicsItemNode::getCentres() const
{
    std::vector<QPointF> centres;
    std::vector<QPointF> currentRun;

    QPointF lastP;
    bool lastPointVisible = false;

    for (size_t i = 0; i < m_linePoints.size(); ++i)
    {
        QPointF p = m_linePoints[i];
        bool pVisible = g_graphicsView->isPointVisible(p);

        //If this point is visible, but the last wasn't, a new run is started.
        if (pVisible && !lastPointVisible)
        {
            //If this is not the first point, then we need to find the intermediate
            //point that lies on the visible boundary and start the path with that.
            if (i > 0)
                currentRun.push_back(g_graphicsView->findIntersectionWithViewportBoundary(QLineF(p, lastP)));
            currentRun.push_back(p);
        }

        //If this point is visible, add it to the current run.
        if (pVisible && lastPointVisible)
            currentRun.push_back(p);

        //If the last point was visible and this one isn't, then a run has ended.
        else if (!pVisible && lastPointVisible)
        {
            //We need to find the intermediate point that is on the visible boundary.
            currentRun.push_back(g_graphicsView->findIntersectionWithViewportBoundary(QLineF(p, lastP)));

            centres.push_back(getCentre(currentRun));
            currentRun.clear();
        }

        lastPointVisible = pVisible;
        lastP = p;
    }

    //If there is a current run, add its centre
    if (currentRun.size() > 0)
        centres.push_back(getCentre(currentRun));

    return centres;
}



//This function finds the centre point on the path defined by linePoints.
QPointF GraphicsItemNode::getCentre(std::vector<QPointF> linePoints) const
{
    if (linePoints.size() == 0)
        return QPointF();
    if (linePoints.size() == 1)
        return linePoints[0];

    double pathLength = 0.0;
    for (size_t i = 0; i < linePoints.size() - 1; ++i)
        pathLength += distance(linePoints[i], linePoints[i+1]);

    double endToCentre = pathLength / 2.0;

    double lengthSoFar = 0.0;
    for (size_t i = 0; i < linePoints.size() - 1; ++i)
    {
        QPointF a = linePoints[i];
        QPointF b = linePoints[i+1];
        double segmentLength = distance(a, b);

        //If this segment will push the distance over halfway, then it
        //contains the centre point.
        if (lengthSoFar + segmentLength >= endToCentre)
        {
            double additionalLengthNeeded = endToCentre - lengthSoFar;
            double fractionOfCurrentSegment = additionalLengthNeeded / segmentLength;
            return (b - a) * fractionOfCurrentSegment + a;
        }

        lengthSoFar += segmentLength;
    }

    //Code should never get here.
    return QPointF();
}




QStringList GraphicsItemNode::getNodeText()
{
    QStringList nodeText;

    if (g_settings->displayNodeCustomLabels && m_deBruijnNode->m_customLabel.length() > 0)
        nodeText << m_deBruijnNode->m_customLabel;
    if (g_settings->displayNodeNames)
    {
        QString nodeName = m_deBruijnNode->m_name;
        if (!g_settings->doubleMode)
            nodeName.chop(1);
        nodeText << nodeName;
    }
    if (g_settings->displayNodeLengths)
        nodeText << formatIntForDisplay(m_deBruijnNode->m_length) + " bp";
    if (g_settings->displayNodeCoverages)
        nodeText << formatDoubleForDisplay(m_deBruijnNode->m_coverage, 1) + "x";
    if (g_settings->displayNodeCsvData)
    {
        if (m_deBruijnNode->m_csvData.length() > g_settings->displayNodeCsvDataCol)
            nodeText << m_deBruijnNode->m_csvData.at(g_settings->displayNodeCsvDataCol);
        else
            nodeText << "-";
    }

    return nodeText;
}



QSize GraphicsItemNode::getNodeTextSize(QString text)
{
    QFontMetrics fontMetrics(g_settings->labelFont);
    return fontMetrics.size(0, text);
}


QColor GraphicsItemNode::getCoverageColour()
{
    double coverage = m_deBruijnNode->m_coverage;
    double lowValue;
    double highValue;
    if (g_settings->autoCoverageValue)
    {
        lowValue = g_assemblyGraph->m_firstQuartileCoverage;
        highValue = g_assemblyGraph->m_thirdQuartileCoverage;
    }
    else
    {
        lowValue = g_settings->lowCoverageValue;
        highValue = g_settings->highCoverageValue;
    }

    if (coverage <= lowValue)
        return g_settings->lowCoverageColour;
    if (coverage >= highValue)
        return g_settings->highCoverageColour;

    double fraction = (coverage - lowValue) / (highValue - lowValue);

    int redDifference = g_settings->highCoverageColour.red() - g_settings->lowCoverageColour.red();
    int greenDifference = g_settings->highCoverageColour.green() - g_settings->lowCoverageColour.green();
    int blueDifference = g_settings->highCoverageColour.blue() - g_settings->lowCoverageColour.blue();
    int alphaDifference = g_settings->highCoverageColour.alpha() - g_settings->lowCoverageColour.alpha();

    int red = int(g_settings->lowCoverageColour.red() + (fraction * redDifference) + 0.5);
    int green = int(g_settings->lowCoverageColour.green() + (fraction * greenDifference) + 0.5);
    int blue = int(g_settings->lowCoverageColour.blue() + (fraction * blueDifference) + 0.5);
    int alpha = int(g_settings->lowCoverageColour.alpha() + (fraction * alphaDifference) + 0.5);

    return QColor(red, green, blue, alpha);
}



void GraphicsItemNode::setWidth()
{
    m_width = getNodeWidth(m_deBruijnNode->m_coverageRelativeToMeanDrawnCoverage, g_settings->coveragePower,
                           g_settings->coverageEffectOnWidth, g_settings->averageNodeWidth);
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


double GraphicsItemNode::getNodeWidth(double coverageRelativeToMeanDrawnCoverage, double coveragePower,
                                      double coverageEffectOnWidth, double averageNodeWidth)
{
    double widthRelativeToAverage = (pow(coverageRelativeToMeanDrawnCoverage, coveragePower) - 1.0) * coverageEffectOnWidth + 1.0;
    return averageNodeWidth * widthRelativeToAverage;
}



//This function shifts all the node's points to the left (relative to its
//direction).  This is used in double mode to prevent nodes from displaying
//directly on top of their complement nodes.
void GraphicsItemNode::shiftPointsLeft()
{
    //The collection of line points should be at least
    //two large.  But just to be safe, quit now if it
    //is not.
    size_t linePointsSize = m_linePoints.size();
    if (linePointsSize < 2)
        return;

    //Shift by a quarter of the segment length.  This should make
    //nodes one half segment length separated from their complements.
    double shiftDistance = g_settings->segmentLength / 4.0;

    for (size_t i = 0; i < linePointsSize; ++i)
    {
        QPointF point = m_linePoints[i];
        QLineF nodeDirection;

        //If the point is on the end, then determine the node direction
        //using this point and its adjacent point.
        if (i == 0)
        {
            QPointF nextPoint = m_linePoints[i+1];
            nodeDirection = QLineF(point, nextPoint);
        }
        else if (i == linePointsSize - 1)
        {
            QPointF previousPoint = m_linePoints[i-1];
            nodeDirection = QLineF(previousPoint, point);
        }

        // If the point is in the middle, then determine the node direction
        //using both adjacent points.
        else
        {
            QPointF previousPoint = m_linePoints[i-1];
            QPointF nextPoint = m_linePoints[i+1];
            nodeDirection = QLineF(previousPoint, nextPoint);
        }

        QLineF shiftLine = nodeDirection.normalVector().unitVector();
        shiftLine.setLength(shiftDistance);
        QPointF shiftVector = shiftLine.p2() - shiftLine.p1();
        QPointF newPoint = point + shiftVector;
        m_linePoints[i] = newPoint;
    }
}



void GraphicsItemNode::getBlastHitsTextAndLocationThisNode(std::vector<QString> * blastHitText,
                                                       std::vector<QPointF> * blastHitLocation)
{
    for (size_t i = 0; i < m_deBruijnNode->m_blastHits.size(); ++i)
    {
        BlastHit * hit = m_deBruijnNode->m_blastHits[i];
        blastHitText->push_back(hit->m_query->m_name);
        blastHitLocation->push_back(findLocationOnPath(hit->getNodeCentreFraction()));
    }

}

void GraphicsItemNode::getBlastHitsTextAndLocationThisNodeOrReverseComplement(std::vector<QString> * blastHitText,
                                                                          std::vector<QPointF> * blastHitLocation)
{
    getBlastHitsTextAndLocationThisNode(blastHitText, blastHitLocation);

    for (size_t i = 0; i < m_deBruijnNode->m_reverseComplement->m_blastHits.size(); ++i)
    {
        BlastHit * hit = m_deBruijnNode->m_reverseComplement->m_blastHits[i];
        blastHitText->push_back(hit->m_query->m_name);
        blastHitLocation->push_back(findLocationOnPath(1.0 - hit->getNodeCentreFraction()));
    }
}
