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


#ifndef BLASTQUERY_H
#define BLASTQUERY_H

#include <QObject>
#include <QString>
#include <QColor>

class BlastQuery : public QObject
{
    Q_OBJECT

public:
    BlastQuery() {}
    BlastQuery(QString name, QString sequence) : m_name(name), m_sequence(sequence),
        m_hits(0), m_searchedFor(false)
    {m_length = sequence.length();}

    QString m_name;
    QString m_sequence;
    int m_length;
    int m_hits;
    bool m_searchedFor;
    QColor m_colour;

public slots:
    void setColour(QColor newColour) {m_colour = newColour;}
};

#endif // BLASTQUERY_H
