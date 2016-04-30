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


#ifndef COLOURBUTTON_H
#define COLOURBUTTON_H

#include <QPushButton>
#include <QColor>
#include <QString>

class ColourButton : public QPushButton
{
    Q_OBJECT

public:
    ColourButton(QWidget * parent = 0);

    QColor m_colour;
    QString m_name;

    void setColour(QColor newColour);

signals:
    void colourChosen(QColor colour);

public slots:
    void chooseColour();
};

#endif // COLOURBUTTON_H
