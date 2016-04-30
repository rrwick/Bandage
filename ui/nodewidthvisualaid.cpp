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


#include "nodewidthvisualaid.h"
#include <QPainter>
#include <QSizePolicy>
#include "../graph/graphicsitemnode.h"

NodeWidthVisualAid::NodeWidthVisualAid(QWidget *parent) : QWidget(parent)
{
    QSizePolicy policy = sizePolicy();
    policy.setHeightForWidth(true);
    setSizePolicy(policy);
}

NodeWidthVisualAid::~NodeWidthVisualAid()
{

}

void NodeWidthVisualAid::paintEvent(QPaintEvent * /*event*/)
{
    QPainter painter(this);
    painter.fillRect(0, 0, width(), height(), QBrush(Qt::white));

    QPen gridLinePen(QBrush(QColor(50,50,50)), 0.5);
    painter.setPen(gridLinePen);


    painter.drawLine(0, height() * 0.5, width(), height() * 0.5);
    painter.drawLine(0, height() - 1, width(), height() - 1);
    painter.drawLine(width() * 0.0, 0, width() * 0.0, height());
    painter.drawLine(width() * 0.5, 0, width() * 0.5, height());

    gridLinePen = QPen(QBrush(Qt::gray), 0.5);
    painter.setPen(gridLinePen);

    painter.drawLine(0, height() * 0.125, width(), height() * 0.125);
    painter.drawLine(0, height() * 0.25, width(), height() * 0.25);
    painter.drawLine(0, height() * 0.375, width(), height() * 0.375);
    painter.drawLine(0, height() * 0.625, width(), height() * 0.625);
    painter.drawLine(0, height() * 0.75, width(), height() * 0.75);
    painter.drawLine(0, height() * 0.875, width(), height() * 0.875);
    painter.drawLine(width() * 0.125, 0, width() * 0.125, height());
    painter.drawLine(width() * 0.25, 0, width() * 0.25, height());
    painter.drawLine(width() * 0.375, 0, width() * 0.375, height());
    painter.drawLine(width() * 0.625, 0, width() * 0.625, height());
    painter.drawLine(width() * 0.75, 0, width() * 0.75, height());
    painter.drawLine(width() * 0.875, 0, width() * 0.875, height());

    painter.setRenderHint(QPainter::Antialiasing, true);

    QPainterPath graphLine;
    graphLine.moveTo(0.0, height() - GraphicsItemNode::getNodeWidth(0.0, m_depthPower, m_depthEffectOnWidth, width() / 2.0));

    for (int i = 0; i < width(); ++i)
    {
        double x = double(i);
        double depthRelativeToMeanDrawnDepth = 2.0 * x / width();
        double y = height() -GraphicsItemNode::getNodeWidth(depthRelativeToMeanDrawnDepth, m_depthPower, m_depthEffectOnWidth, width() / 2.0);
        graphLine.lineTo(x, y);
    }

    QPen waterLinePen(QBrush(Qt::blue), 2.0);
    painter.setPen(waterLinePen);
    painter.drawPath(graphLine);
}
