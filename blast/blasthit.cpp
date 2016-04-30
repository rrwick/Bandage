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


#include "blasthit.h"

#include "../graph/debruijnnode.h"
#include "blastquery.h"
#include "../program/settings.h"
#include "../program/globals.h"
#include <math.h>

BlastHit::BlastHit(BlastQuery * query, DeBruijnNode * node,
                   double percentIdentity, int alignmentLength,
                   int numberMismatches, int numberGapOpens,
                   int queryStart, int queryEnd,
                   int nodeStart, int nodeEnd,
                   SciNot eValue, double bitScore) :
    m_query(query), m_node(node),
    m_percentIdentity(percentIdentity), m_alignmentLength(alignmentLength),
    m_numberMismatches(numberMismatches), m_numberGapOpens(numberGapOpens),
    m_queryStart(queryStart), m_queryEnd(queryEnd),
    m_nodeStart(nodeStart), m_nodeEnd(nodeEnd),
    m_eValue(eValue), m_bitScore(bitScore)
{
    int nodeLength = m_node->getLength();
    int queryLength = m_query->getLength();

    m_nodeStartFraction = double(nodeStart - 1) / nodeLength;
    m_nodeEndFraction = double(nodeEnd) / nodeLength;
    m_queryStartFraction = double(queryStart - 1) / queryLength;
    m_queryEndFraction = double(queryEnd) / queryLength;
}

std::vector<BlastHitPart> BlastHit::getBlastHitParts(bool reverse, double scaledNodeLength)
{
    std::vector<BlastHitPart> returnVector;

    //If the colour scheme is Blast rainbow, then this function generates lots
    //of BlastHitParts - each small and with a different colour of the rainbow.
    if (g_settings->nodeColourScheme == BLAST_HITS_RAINBOW_COLOUR)
    {
        double scaledHitLength = (m_nodeEndFraction - m_nodeStartFraction) * scaledNodeLength;

        int partCount = ceil(g_settings->blastRainbowPartsPerQuery * fabs(m_queryStartFraction - m_queryEndFraction));

        //If there are way more parts than the scaled hit length, that means
        //that a single part will be much less than a pixel in length.  This
        //isn't desirable, so reduce the partCount in these cases.
        if (partCount > scaledHitLength * 2.0)
            partCount = int(scaledHitLength * 2.0);

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
            returnVector.push_back(BlastHitPart(m_query->getColour(), 1.0 - m_nodeStartFraction, 1.0 - m_nodeEndFraction));
        else
            returnVector.push_back(BlastHitPart(m_query->getColour(), m_nodeStartFraction, m_nodeEndFraction));
    }

    return returnVector;
}


bool BlastHit::compareTwoBlastHitPointers(BlastHit * a, BlastHit * b)
{
    return a->m_queryStart < b->m_queryStart;
}


double BlastHit::getQueryCoverageFraction()
{
    int queryRegionSize = m_queryEnd - m_queryStart + 1;
    int queryLength = m_query->getLength();

    if (queryLength == 0)
        return 0.0;
    else
        return double(queryRegionSize) / queryLength;
}


GraphLocation BlastHit::getHitStart() const
{
    return GraphLocation(m_node, m_nodeStart);
}

GraphLocation BlastHit::getHitEnd() const
{
    return GraphLocation(m_node, m_nodeEnd);
}


//This function returns the node sequence for this hit.
QByteArray BlastHit::getNodeSequence() const
{
    int nodeSequenceLength = m_nodeEnd - m_nodeStart + 1;
    return m_node->getSequence().mid(m_nodeStart-1, nodeSequenceLength);
}
