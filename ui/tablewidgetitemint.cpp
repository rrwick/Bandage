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


#include "tablewidgetitemint.h"

TableWidgetItemInt::TableWidgetItemInt(QString text, int value) :
    QTableWidgetItem(text), m_int(value)
{
}



bool TableWidgetItemInt::operator<(QTableWidgetItem const &other) const
{
    TableWidgetItemInt const *item = dynamic_cast<TableWidgetItemInt const*>(&other);
    if(item)
        return m_int < item->m_int;
    else
        return QTableWidgetItem::operator<(other);
}
