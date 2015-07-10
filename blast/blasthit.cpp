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
#include "blastquery.h"
#include "../program/settings.h"
#include "../program/globals.h"
#include <math.h>

BlastHit::BlastHit(DeBruijnNode * node, int nodeStart, int nodeEnd,
                   BlastQuery * query, int queryStart, int queryEnd,
                   QString eValue) :
    m_node(node), m_nodeStart(nodeStart), m_nodeEnd(nodeEnd),
    m_query(query), m_queryStart(queryStart), m_queryEnd(queryEnd),
    m_eValue(eValue)
{
    m_nodeStartFraction = double(nodeStart) / m_node->m_length;
    m_nodeEndFraction = double(nodeEnd) / m_node->m_length;
    m_queryStartFraction = double(queryStart) / m_query->m_length;
    m_queryEndFraction = double(queryEnd) / m_query->m_length;
}

BlastHit::~BlastHit()
{

}


std::vector<BlastHitPart> BlastHit::getBlastHitParts(bool reverse)
{
    std::vector<BlastHitPart> returnVector;

    //If the colour scheme is Blast rainbow, then this function generates lots
    //of BlastHitParts - each small and with a different colour of the rainbow.
    if (g_settings->nodeColourScheme == BLAST_HITS_RAINBOW_COLOUR)
    {
        int partCount = ceil(g_settings->blastRainbowPartsPerQuery * fabs(m_queryStartFraction - m_queryEndFraction));

        double nodeSpacing = (m_nodeEndFraction - m_nodeStartFraction) / partCount;
        double querySpacing = (m_queryEndFraction - m_queryStartFraction) / partCount;

        double nodeFraction = m_nodeStartFraction;
        double queryFraction = m_queryStartFraction;
        for (int i = 0; i < partCount; ++i)
        {
            QColor dotColour;
            dotColour.setHsvF(queryFraction * 0.9, 1.0, 1.0);  //times 0.9 to keep the colour from getting too close to red, as that could confuse the end with the start

            double nextFraction = nodeFraction + nodeSpacing;

            if (reverse)
                returnVector.push_back(BlastHitPart(dotColour, 1.0 - nodeFraction, 1.0 - nextFraction));
            else
                returnVector.push_back(BlastHitPart(dotColour, nodeFraction, nextFraction));

            nodeFraction = nextFraction;
            queryFraction += querySpacing;
        }
    }

    //If the colour scheme is Blast solid, then this function generates only one
    //BlastHitPart with a colour dependent on the Blast query.
    else
    {
        if (reverse)
            returnVector.push_back(BlastHitPart(m_query->m_colour, 1.0 - m_nodeStartFraction, 1.0 - m_nodeEndFraction));
        else
            returnVector.push_back(BlastHitPart(m_query->m_colour, m_nodeStartFraction, m_nodeEndFraction));
    }

    return returnVector;
}
