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


#include "graphicsviewzoom.h"
#include <QMouseEvent>
#include <QApplication>
#include <QScrollBar>
#include <qmath.h>
#include "../program/globals.h"
#include "../program/settings.h"

GraphicsViewZoom::GraphicsViewZoom(QGraphicsView* view)
    : QObject(view), _view(view)
{
    g_absoluteZoom = 1.0;
    _view->viewport()->installEventFilter(this);
    _view->setMouseTracking(true);
    _modifiers = Qt::ControlModifier;
    _zoom_factor_base = g_settings->zoomFactor;
}

void GraphicsViewZoom::gentle_zoom(double factor, ZoomSource zoomSource) {

    if (g_absoluteZoom * factor >= g_settings->maxZoom)
        factor = g_settings->maxZoom / g_absoluteZoom;
    if (g_absoluteZoom * factor <= g_settings->minZoom)
        factor = g_settings->minZoom / g_absoluteZoom;

    g_absoluteZoom *= factor;

    _view->scale(factor, factor);

    if (zoomSource == MOUSE_WHEEL)
    {
        _view->centerOn(target_scene_pos);


        QPointF delta_viewport_pos = target_viewport_pos - QPointF(_view->viewport()->width() / 2.0,
                                                                   _view->viewport()->height() / 2.0);
        QPointF viewport_center = _view->mapFromScene(target_scene_pos) - delta_viewport_pos;
        _view->centerOn(_view->mapToScene(viewport_center.toPoint()));
    }

    if (zoomSource != SPIN_BOX)
        emit zoomed();
}

void GraphicsViewZoom::set_modifiers(Qt::KeyboardModifiers modifiers) {
    _modifiers = modifiers;

}

void GraphicsViewZoom::set_zoom_factor_base(double value) {
    _zoom_factor_base = value;
}

bool GraphicsViewZoom::eventFilter(QObject *object, QEvent *event) {
    if (event->type() == QEvent::MouseMove) {
        QMouseEvent* mouse_event = static_cast<QMouseEvent*>(event);
        QPointF delta = target_viewport_pos - mouse_event->pos();
        if (qAbs(delta.x()) > 5 || qAbs(delta.y()) > 5) {
            target_viewport_pos = mouse_event->pos();
            target_scene_pos = _view->mapToScene(mouse_event->pos());
        }
    } else if (event->type() == QEvent::Wheel) {
        QWheelEvent* wheel_event = static_cast<QWheelEvent*>(event);
        if (QApplication::keyboardModifiers() == _modifiers) {
            if (wheel_event->orientation() == Qt::Vertical) {
                double angle = wheel_event->angleDelta().y();
                double factor = qPow(_zoom_factor_base, angle);
                gentle_zoom(factor, MOUSE_WHEEL);
                return true;
            }
        }
    }
    Q_UNUSED(object)
    return false;
}
