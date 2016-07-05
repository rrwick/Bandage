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


#ifndef BLASTHIT_H
#define BLASTHIT_H

class DeBruijnNode;
class BlastQuery;

#include <QString>
#include "blasthitpart.h"
#include <vector>
#include "../graph/path.h"
#include "../graph/graphlocation.h"
#include "../program/scinot.h"

class BlastHit
{
public:
    BlastHit(BlastQuery * query, DeBruijnNode * node,
             double percentIdentity, int alignmentLength,
             int numberMismatches, int numberGapOpens,
             int queryStart, int queryEnd,
             int nodeStart, int nodeEnd, SciNot eValue, double bitScore);
    BlastHit();

    BlastQuery * m_query;
    DeBruijnNode * m_node;
    double m_percentIdentity;
    int m_alignmentLength;
    int m_numberMismatches;
    int m_numberGapOpens;
    int m_queryStart;
    int m_queryEnd;
    int m_nodeStart;
    int m_nodeEnd;
    SciNot m_eValue;
    double m_bitScore;

    double m_nodeStartFraction;
    double m_nodeEndFraction;
    double m_queryStartFraction;
    double m_queryEndFraction;

    Path m_queryPath;

    std::vector<BlastHitPart> getBlastHitParts(bool reverse, double scaledNodeLength);
    bool onForwardStrand() {return m_queryStart < m_queryEnd;}
    double getNodeCentreFraction() {return (m_nodeStartFraction + m_nodeEndFraction) / 2.0;}
    static bool compareTwoBlastHitPointers(BlastHit * a, BlastHit * b);
    double getQueryCoverageFraction();
    GraphLocation getHitStart() const;
    GraphLocation getHitEnd() const;
    QByteArray getNodeSequence() const;
    int getNodeLength() const {return m_nodeEnd - m_nodeStart + 1;}
};

#endif // BLASTHIT_H
