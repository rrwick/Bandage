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


#ifndef BLASTQUERYPATH_H
#define BLASTQUERYPATH_H

#include "../graph/path.h"
#include <QList>
#include "blasthit.h"
#include "../program/scinot.h"

class BlastQuery;

class BlastQueryPath
{
public:

    //CREATORS
    BlastQueryPath(Path path, BlastQuery * query);

    //ACCESSORS
    Path getPath() const {return m_path;}
    QList<BlastHit *> getHits() const {return m_hits;}
    SciNot getEvalueProduct() const;
    double getMeanHitPercIdentity() const;
    double getRelativeLengthDiscrepancy() const;
    double getRelativePathLength() const;
    int getAbsolutePathLengthDifference() const;
    QString getAbsolutePathLengthDifferenceString(bool commas) const;
    double getPathQueryCoverage() const;
    double getHitsQueryCoverage() const;
    int getTotalHitMismatches() const;
    int getTotalHitGapOpens() const;

    bool operator<(BlastQueryPath const &other) const;

private:
    Path m_path;
    BlastQuery * m_query;
    QList<BlastHit *> m_hits;

    int getHitQueryLength() const;
    int getHitOverlap(BlastHit * hit1, BlastHit * hit2) const;
};

#endif // BLASTQUERYPATH_H
