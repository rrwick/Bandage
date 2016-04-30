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


#include "querypathspushbutton.h"

#include "../program/globals.h"

QueryPathsPushButton::QueryPathsPushButton(int pathCount, BlastQuery * query) :
    m_pathCount(pathCount), m_query(query)
{
    setText(formatIntForDisplay(pathCount));
    if (pathCount == 0)
        setEnabled(false);

    connect(this, SIGNAL(clicked(bool)), this, SLOT(clickedSlot()));
}





void QueryPathsPushButton::clickedSlot()
{
    emit showPathsDialog(m_query);
}
