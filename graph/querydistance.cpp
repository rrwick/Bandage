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


#include "querydistance.h"

QueryDistance::QueryDistance(Path path, QString orientation) :
    m_path(path), m_orientation(orientation)
{
    m_distance = m_path.getLength();
}


bool QueryDistance::operator<(QueryDistance const &other) const
{
    return m_distance < other.m_distance;
}

bool QueryDistance::operator==(QueryDistance const &other) const
{
    return m_distance == other.m_distance &&
            m_orientation == other.m_orientation &&
            m_path.areIdentical(other.m_path);
}
