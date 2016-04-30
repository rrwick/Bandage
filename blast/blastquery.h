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


#ifndef BLASTQUERY_H
#define BLASTQUERY_H

#include <QObject>
#include <QString>
#include <QColor>
#include "../program/globals.h"
#include "blasthit.h"
#include <QList>
#include <QSharedPointer>
#include "blastquerypath.h"

class BlastQuery : public QObject
{
    Q_OBJECT

public:
    //CREATORS
    BlastQuery() {}
    BlastQuery(QString name, QString sequence);

    //ACCESSORS
    QString getName() const {return m_name;}
    QString getSequence() const {return m_sequence;}
    int getLength() const {return m_sequence.length();}
    bool hasHits() const {return m_hits.size() > 0;}
    int hitCount() const {return m_hits.size();}
    QList< QSharedPointer<BlastHit> > getHits() const {return m_hits;}
    bool wasSearchedFor() const {return m_searchedFor;}
    QColor getColour() const {return m_colour;}
    SequenceType getSequenceType() const {return m_sequenceType;}
    QList<BlastQueryPath> getPaths() const {return m_paths;}
    int getPathCount() const {return m_paths.size();}
    QString getTypeString() const;
    double fractionCoveredByHits(const QList<BlastHit *> * hitsToCheck = 0) const;
    bool isShown() const {return m_shown;}
    bool isHidden() const {return !m_shown;}

    //MODIFIERS
    void setName(QString newName) {m_name = newName;}
    void addHit(QSharedPointer<BlastHit> newHit) {m_hits.push_back(newHit);}
    void clearSearchResults();
    void setAsSearchedFor() {m_searchedFor = true;}
    void findQueryPaths();

public slots:
    void setColour(QColor newColour) {m_colour = newColour;}
    void setShown(bool newShown) {m_shown = newShown;}

private:
    QString m_name;
    QString m_sequence;
    QList< QSharedPointer<BlastHit> > m_hits;
    bool m_searchedFor;
    QColor m_colour;
    SequenceType m_sequenceType;
    QList<BlastQueryPath> m_paths;
    bool m_shown;

    void autoSetSequenceType();
    bool positionInAnyHit(int position) const;
    bool positionInHitList(int position, const QList<BlastHit *> * hitsToCheck) const;
};

#endif // BLASTQUERY_H
