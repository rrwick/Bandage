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


#include "mygraphicsview.h"
#include <QMouseEvent>
#include "../program/globals.h"
#include "../program/settings.h"
#include <QFont>
#include "graphicsviewzoom.h"
#include <qmath.h>
#include <QMessageBox>
#include <math.h>

MyGraphicsView::MyGraphicsView(QObject * /*parent*/) :
    QGraphicsView(), m_rotation(0.0)
{
    setDragMode(QGraphicsView::RubberBandDrag);
    setRenderHints(QPainter::Antialiasing);
    setViewportUpdateMode(QGraphicsView::MinimalViewportUpdate); //CAN I CHANGE THIS LATER TO BE MORE EFFICIENT?
}



void MyGraphicsView::mousePressEvent(QMouseEvent * event)
{
    if (event->modifiers() == Qt::CTRL)
        setDragMode(QGraphicsView::ScrollHandDrag);
    else if (event->button() == Qt::RightButton)
        g_settings->nodeDragging = ONE_PIECE;

    m_previousPos = event->pos();

    QGraphicsView::mousePressEvent(event);
}

void MyGraphicsView::mouseReleaseEvent(QMouseEvent * event)
{
    QGraphicsView::mouseReleaseEvent(event);
    setDragMode(QGraphicsView::RubberBandDrag);
    g_settings->nodeDragging = NEARBY_PIECES;
}

void MyGraphicsView::mouseMoveEvent(QMouseEvent * event)
{
    //If the user drags the right mouse button while holding control,
    //the view rotates.
    bool rightButtonDown = event->buttons() & Qt::RightButton;
    if (event->modifiers() == Qt::CTRL &&
            rightButtonDown)
    {
        QPointF viewCentre(width() / 2.0, height() / 2.0);
        double angle = angleBetweenTwoLines(viewCentre, m_previousPos, viewCentre, event->pos());
        angle *= 57.295779513; //convert to degrees

        rotate(angle);
        m_rotation += angle;

        m_previousPos = event->pos();
    }
    else
        QGraphicsView::mouseMoveEvent(event);
}

//Adapted from:
//http://stackoverflow.com/questions/2663570/how-to-calculate-both-positive-and-negative-angle-between-two-lines
double MyGraphicsView::angleBetweenTwoLines(QPointF line1Start, QPointF line1End, QPointF line2Start, QPointF line2End)
{
    double a = line1End.x() - line1Start.x();
    double b = line1End.y() - line1Start.y();
    double c = line2End.x() - line2Start.x();
    double d = line2End.y() - line2Start.y();

    double atanA = atan2(a, b);
    double atanB = atan2(c, d);

    return atanA - atanB;
}

double MyGraphicsView::distance(double x1, double y1, double x2, double y2)
{
    double xDiff = x1 - x2;
    double yDiff = y1 - y2;
    return sqrt(xDiff * xDiff + yDiff * yDiff);
}

void MyGraphicsView::keyPressEvent(QKeyEvent * event)
{
    //This function uses angle in the same way that the mouse wheel code
    //in GraphicsViewZoom does.  This keeps the zoom steps consistent
    //between keyboard and mouse wheel.
    int angle = 0;


    bool shiftPressed = event->modifiers().testFlag(Qt::ShiftModifier);

    if (event->matches(QKeySequence::ZoomIn) ||
            event->key() == Qt::Key_Equal ||
            event->key() == Qt::Key_Plus)
    {
        if (shiftPressed)
        {
            rotate(1.0);
            m_rotation += 1.0;
        }
        else
            angle = 120;
    }
    else if (event->matches(QKeySequence::ZoomOut) ||
             event->key() == Qt::Key_Minus ||
             event->key() == Qt::Key_Underscore)
    {
        if (shiftPressed)
        {
            rotate(-1.0);
            m_rotation -= 1.0;
        }
        else
            angle = -120;
    }

    if (angle != 0)
    {
        double factor = qPow(m_zoom->_zoom_factor_base, angle);
        m_zoom->gentle_zoom(factor, KEYBOARD);
    }

    QGraphicsView::keyPressEvent(event);
}

void MyGraphicsView::setAntialiasing(bool antialiasingOn)
{
    if (antialiasingOn)
    {
        setRenderHints(QPainter::Antialiasing);
        g_settings->labelFont.setStyleStrategy(QFont::PreferDefault);
    }
    else
    {
        setRenderHints(0);
        g_settings->labelFont.setStyleStrategy(QFont::NoAntialias);
    }
}
