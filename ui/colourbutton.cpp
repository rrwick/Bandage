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


#include "colourbutton.h"
#include <QColorDialog>

ColourButton::ColourButton(QWidget * parent) :
    QPushButton(parent)
{
    connect(this, SIGNAL(clicked(bool)), this, SLOT(chooseColour()));
}


void ColourButton::setColour(QColor newColour)
{
    m_colour = newColour;

    const QString COLOR_STYLE("QPushButton { background-color : %1 }");
    setStyleSheet(COLOR_STYLE.arg(m_colour.name()));
}



void ColourButton::chooseColour()
{
    QColor chosenColour = QColorDialog::getColor(m_colour, this, m_name, QColorDialog::ShowAlphaChannel);
    if (chosenColour.isValid())
    {
        setColour(chosenColour);
        emit colourChosen(chosenColour);
    }
}
