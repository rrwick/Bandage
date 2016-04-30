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


#ifndef TABLEWIDGETITEMSHOWN_H
#define TABLEWIDGETITEMSHOWN_H

#include <QTableWidgetItem>

class TableWidgetItemShown : public QTableWidgetItem
{
public:
    TableWidgetItemShown(bool shown);

    bool m_shown;

    virtual bool operator<(QTableWidgetItem const &other) const;
};

#endif // TABLEWIDGETITEMSHOWN_H
