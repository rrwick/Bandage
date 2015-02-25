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


#include "blasthit.h"

#include "../graph/debruijnnode.h"
#include "blasttarget.h"
#include "../program/settings.h"
#include "../program/globals.h"
#include <math.h>

BlastHit::BlastHit(DeBruijnNode * node, int nodeStart, int nodeEnd,
                         BlastTarget * target, int targetStart, int targetEnd) :
    m_node(node), m_nodeStart(nodeStart), m_nodeEnd(nodeEnd),
    m_target(target), m_targetStart(targetStart), m_targetEnd(targetEnd)
{
    m_nodeStartFraction = double(nodeStart) / m_node->m_length;
    m_nodeEndFraction = double(nodeEnd) / m_node->m_length;
    m_targetStartFraction = double(targetStart) / m_target->m_length;
    m_targetEndFraction = double(targetEnd) / m_target->m_length;
}

BlastHit::~BlastHit()
{

}


std::vector<BlastHitPart> BlastHit::getBlastHitParts(bool reverse)
{
    std::vector<BlastHitPart> returnVector;

    int partCount = ceil(g_settings->blastPartsPerTarget * fabs(m_targetStartFraction - m_targetEndFraction));

    double nodeSpacing = (m_nodeEndFraction - m_nodeStartFraction) / partCount;
    double targetSpacing = (m_targetEndFraction - m_targetStartFraction) / partCount;

    double nodeFraction = m_nodeStartFraction;
    double targetFraction = m_targetStartFraction;
    for (int i = 0; i < partCount; ++i)
    {
        QColor dotColour;
        dotColour.setHsvF(targetFraction * 0.9, 1.0, 1.0);  //times 0.9 to keep the colour from getting too close to red, as that could confuse the end with the start

        double nextFraction = nodeFraction + nodeSpacing;

        if (reverse)
            returnVector.push_back(BlastHitPart(dotColour, 1.0 - nodeFraction, 1.0 - nextFraction));
        else
            returnVector.push_back(BlastHitPart(dotColour, nodeFraction, nextFraction));

        nodeFraction = nextFraction;
        targetFraction += targetSpacing;
    }

    return returnVector;
}
