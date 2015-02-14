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


#ifndef MYGRAPHICSVIEW_H
#define MYGRAPHICSVIEW_H

#include <QGraphicsView>
#include <QPoint>

class GraphicsViewZoom;

class MyGraphicsView : public QGraphicsView
{
    Q_OBJECT
public:
    explicit MyGraphicsView(QObject *parent = 0);

    GraphicsViewZoom * m_zoom;
    QPoint m_previousPos;
    double m_rotation;

    void setAntialiasing(bool antialiasingOn);

protected:
    void mousePressEvent(QMouseEvent * event);
    void mouseReleaseEvent(QMouseEvent * event);
    void mouseMoveEvent(QMouseEvent * event);
    void keyPressEvent(QKeyEvent * event);

private:
    double distance(double x1, double y1, double x2, double y2);
    double angleBetweenTwoLines(QPointF line1Start, QPointF line1End, QPointF line2Start, QPointF line2End);
};

#endif // MYGRAPHICSVIEW_H
