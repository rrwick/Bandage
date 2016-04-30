//Copyright 2016 Ryan Wick

//This file is part of Bandage.

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


#ifndef INFOTEXTWIDGET_H
#define INFOTEXTWIDGET_H

#include <QWidget>
#include <QString>

class InfoTextWidget : public QWidget
{
    Q_OBJECT
public:
    explicit InfoTextWidget(QWidget * parent);
    explicit InfoTextWidget(QWidget * parent, QString infoText);

    void setInfoText(QString infoText);

protected:
    void paintEvent(QPaintEvent *);
    void mousePressEvent(QMouseEvent * event);
    
private:
    QString m_infoText;
};

#endif // INFOTEXTWIDGET_H
