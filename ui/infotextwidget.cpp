//Copyright 2016 Ryan Wick

//This file is part of Grovolve.

//Grovolve is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//Grovolve is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

//You should have received a copy of the GNU General Public License
//along with Grovolve.  If not, see <http://www.gnu.org/licenses/>.


#include "infotextwidget.h"
#include <QPainter>
#include <QToolTip>
#include <QMouseEvent>


InfoTextWidget::InfoTextWidget(QWidget * parent) :
    QWidget(parent)
{
    setFixedSize(16, 16);
    setMouseTracking(true);
}

InfoTextWidget::InfoTextWidget(QWidget * parent, QString infoText) :
    QWidget(parent)
{
    setFixedSize(16, 16);
    setMouseTracking(true);
    setInfoText(infoText);
}

void InfoTextWidget::setInfoText(QString infoText)
{
    //Convert text to a rich text format, which will let QToolTip wrap the text.
    m_infoText = "<html>" + infoText + "</html>";
}


void InfoTextWidget::paintEvent(QPaintEvent * /*event*/)
{
    QPainter painter(this);

    QPixmap infoIcon;
    if (isEnabled())
        infoIcon = QPixmap(":/icons/information-256.png");
    else
        infoIcon = QPixmap(":/icons/information-256-inactive.png");

    //This scaling using the device pixel ratio was necessary to make the labels look right on a MacBook retina display.
    double scaledWidth = width() * devicePixelRatio();
    double scaledHeight = height() * devicePixelRatio();
    double scaledSize = std::min(scaledWidth, scaledHeight);

    QPixmap scaledInfoIcon = infoIcon.scaled(scaledSize, scaledSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);

    scaledInfoIcon.setDevicePixelRatio(devicePixelRatio());

    double topLeftX = scaledWidth / 2.0 - scaledSize / 2.0;
    double topLeftY = scaledHeight / 2.0 - scaledSize / 2.0;

    painter.drawPixmap(topLeftX, topLeftY, scaledInfoIcon);
}


void InfoTextWidget::mousePressEvent(QMouseEvent * event)
{
    QToolTip::showText(event->globalPos(), m_infoText);
}
