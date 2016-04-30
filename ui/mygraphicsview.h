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


#ifndef MYGRAPHICSVIEW_H
#define MYGRAPHICSVIEW_H

#include <QGraphicsView>
#include <QPoint>
#include <QLineF>

class GraphicsViewZoom;
class DeBruijnNode;

class MyGraphicsView : public QGraphicsView
{
    Q_OBJECT
public:
    // CREATORS
    explicit MyGraphicsView(QObject *parent = 0);

    //ACCESSORS
    double getRotation() const {return m_rotation;}

    //MODIFERS
    void setRotation(double newRotation);
    void changeRotation(double rotationChange);
    void undoRotation();


    GraphicsViewZoom * m_zoom;
    QPoint m_previousPos;

    void setAntialiasing(bool antialiasingOn);
    bool isPointVisible(QPointF p);
    QPointF findIntersectionWithViewportBoundary(QLineF line);
    QLineF findVisiblePartOfLine(QLineF line, bool * success);

protected:
    void mousePressEvent(QMouseEvent * event);
    void mouseReleaseEvent(QMouseEvent * event);
    void mouseMoveEvent(QMouseEvent * event);
    void keyPressEvent(QKeyEvent * event);
    void mouseDoubleClickEvent(QMouseEvent * event);

private:
    double m_rotation;

    double distance(double x1, double y1, double x2, double y2);
    double angleBetweenTwoLines(QPointF line1Start, QPointF line1End, QPointF line2Start, QPointF line2End);
    void getFourViewportCornersInSceneCoordinates(QPointF * c1, QPointF * c2, QPointF * c3, QPointF * c4);
    bool differentSidesOfLine(QPointF p1, QPointF p2, QLineF line);
    bool differentSidesOfLine(QPointF p1, QPointF p2, QPointF p3, QPointF p4, QLineF line);
    bool sideOfLine(QPointF p, QLineF line);

signals:
    void doubleClickedNode(DeBruijnNode * node);
    void copySelectedSequencesToClipboard();
    void saveSelectedSequencesToFile();
};

#endif // MYGRAPHICSVIEW_H
