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


#include "nodewidthvisualaid.h"
#include <QPainter>
#include <QSizePolicy>

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
    painter.setRenderHint(QPainter::Antialiasing, true);
}
