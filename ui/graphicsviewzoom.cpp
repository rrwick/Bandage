//Copyright 2016 Ryan Wick

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


#include "graphicsviewzoom.h"
#include <QMouseEvent>
#include <QApplication>
#include <QScrollBar>
#include <qmath.h>
#include "../program/settings.h"
#include "mygraphicsview.h"

GraphicsViewZoom::GraphicsViewZoom(MyGraphicsView * view) :
    QObject(view), m_view(view)
{
    g_absoluteZoom = 1.0;
    m_view->viewport()->installEventFilter(this);
    m_view->setMouseTracking(true);
    m_modifiers = Qt::ControlModifier;
    m_zoomFactorBase = g_settings->zoomFactor;
}

void GraphicsViewZoom::gentleZoom(double factor, ZoomSource zoomSource)
{

    if (g_absoluteZoom * factor >= g_settings->maxZoom)
        factor = g_settings->maxZoom / g_absoluteZoom;
    if (g_absoluteZoom * factor <= g_settings->minZoom)
        factor = g_settings->minZoom / g_absoluteZoom;

    g_absoluteZoom *= factor;
    m_view->scale(factor, factor);

    if (zoomSource == MOUSE_WHEEL)
    {
        m_view->centerOn(targetScenePos);


        QPointF deltaViewportPos = targetViewportPos - QPointF(m_view->viewport()->width() / 2.0,
                                                                   m_view->viewport()->height() / 2.0);
        QPointF viewport_center = m_view->mapFromScene(targetScenePos) - deltaViewportPos;
        m_view->centerOn(m_view->mapToScene(viewport_center.toPoint()));
    }

    if (zoomSource != SPIN_BOX)
        emit zoomed();
}

void GraphicsViewZoom::setModifiers(Qt::KeyboardModifiers modifiers)
{
    m_modifiers = modifiers;
}

void GraphicsViewZoom::setZoomFactorBase(double value)
{
    m_zoomFactorBase = value;
}

bool GraphicsViewZoom::eventFilter(QObject *object, QEvent *event)
{
    if (event->type() == QEvent::MouseMove)
    {
        QMouseEvent * mouseEvent = static_cast<QMouseEvent*>(event);
        QPointF delta = targetViewportPos - mouseEvent->pos();
        if (qAbs(delta.x()) > 5 || qAbs(delta.y()) > 5)
        {
            targetViewportPos = mouseEvent->pos();
            targetScenePos = m_view->mapToScene(mouseEvent->pos());
        }
    }
    else if (event->type() == QEvent::Wheel)
    {
        QWheelEvent * wheelEvent = static_cast<QWheelEvent*>(event);
        if (QApplication::keyboardModifiers() == m_modifiers)
        {
            if (wheelEvent->orientation() == Qt::Vertical)
            {
                double angle = wheelEvent->angleDelta().y();
                double factor = qPow(m_zoomFactorBase, angle);
                gentleZoom(factor, MOUSE_WHEEL);
                return true;
            }
        }
    }
    else if (event->type() == QEvent::NativeGesture)
    {
        QNativeGestureEvent * gestureEvent = static_cast<QNativeGestureEvent *>(event);
        if (gestureEvent->gestureType() == Qt::ZoomNativeGesture)
        {
            double factor = 1.0 + gestureEvent->value();
            gentleZoom(factor, GESTURE);
            return true;
        }
        else if (gestureEvent->gestureType() == Qt::RotateNativeGesture)
        {
            double angle = gestureEvent->value();
            m_view->changeRotation(angle);
        }
    }

    Q_UNUSED(object)
    return false;
}
