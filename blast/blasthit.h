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


#ifndef BLASTHIT_H
#define BLASTHIT_H

class DeBruijnNode;
class BlastTarget;

#include <QString>
#include "blasthitpart.h"

class BlastHit
{
public:
    BlastHit(DeBruijnNode * node, int nodeStart, int nodeEnd,
                BlastTarget * target, int targetStart, int targetEnd);
    BlastHit();
    ~BlastHit();

    DeBruijnNode * m_node;
    int m_nodeStart;
    int m_nodeEnd;

    BlastTarget * m_target;
    int m_targetStart;
    int m_targetEnd;

    double m_nodeStartFraction;
    double m_nodeEndFraction;
    double m_targetStartFraction;
    double m_targetEndFraction;

    std::vector<BlastHitPart> getBlastHitParts(bool reverse);
    bool onForwardStrand() {return m_targetStart < m_targetEnd;}
};

#endif // BLASTHIT_H
