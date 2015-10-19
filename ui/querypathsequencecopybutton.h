//Copyright 2015 Ryan Wick

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


#ifndef QUERYPATHSEQUENCECOPYBUTTON_H
#define QUERYPATHSEQUENCECOPYBUTTON_H

#include <QPushButton>

class QueryPathSequenceCopyButton : public QPushButton
{
    Q_OBJECT

public:
    QueryPathSequenceCopyButton(QByteArray pathSequence, QString pathStart);

private:
    QByteArray m_pathSequence;

private slots:
    void copySequenceToClipboard();

};

#endif // QUERYPATHSEQUENCECOPYBUTTON_H
