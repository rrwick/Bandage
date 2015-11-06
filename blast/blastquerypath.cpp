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


#include "blastquerypath.h"

#include "blastquery.h"
#include "../graph/debruijnnode.h"
#include <limits>

BlastQueryPath::BlastQueryPath(Path path, BlastQuery * query) :
    m_path(path), m_query(query)
{

    //This function follows the path, returning the BLAST hits it finds for the
    //query.  It requires that the hits occur in order, i.e. that each hit in
    //the path begins later in the query than the previous hit.

    BlastHit * previousHit = 0;
    QList<DeBruijnNode *> pathNodes = m_path.getNodes();
    for (int i = 0; i < pathNodes.size(); ++i)
    {
        DeBruijnNode * node = pathNodes[i];

        QList<BlastHit *> hitsThisNode;
        QList< QSharedPointer<BlastHit> > queryHits = query->getHits();
        for (int j = 0; j < queryHits.size(); ++j)
        {
            BlastHit * hit = queryHits[j].data();
            if (hit->m_node->getName() == node->getName())
                hitsThisNode.push_back(hit);
        }

        std::sort(hitsThisNode.begin(), hitsThisNode.end(),
                  BlastHit::compareTwoBlastHitPointers);

        for (int j = 0; j < hitsThisNode.size(); ++j)
        {
            BlastHit * hit = hitsThisNode[j];

            //First check to make sure the hits are within the path.  This means
            //if we are in the first or last nodes of the path, we need to make
            //sure that our hit is contained within the start/end positions.
            if ( (i != 0 || hit->m_nodeStart >= m_path.getStartLocation().getPosition()) &&
                    (i != pathNodes.size()-1 || hit->m_nodeEnd <= m_path.getEndLocation().getPosition()))
            {
                //Now make sure that the hit follows the previous hit in the
                //query.
                if (previousHit == 0 ||
                        hit->m_queryStart > previousHit->m_queryStart)
                {
                    m_hits.push_back(hit);
                    previousHit = hit;
                }
            }
        }
    }
}





double BlastQueryPath::getMeanHitPercIdentity() const
{
    int totalHitLength = 0;
    double sum = 0.0;

    for (int i = 0; i < m_hits.size(); ++i)
    {
        int hitLength = m_hits[i]->m_alignmentLength;
        totalHitLength += hitLength;

        double hitIdentity = m_hits[i]->m_percentIdentity;
        sum += hitIdentity * hitLength;
    }

    if (totalHitLength == 0)
        return 0.0;
    else
        return sum / totalHitLength;
}




//This function looks at all of the hits in the path for this query and
//multiplies the evalues together.
SciNot BlastQueryPath::getEvalueProduct() const
{
    double coefficientProduct = 1.0;
    int exponentSum = 0;

    for (int i = 0; i < m_hits.size(); ++i)
    {
        coefficientProduct *= m_hits[i]->m_eValue.getCoefficient();
        exponentSum += m_hits[i]->m_eValue.getExponent();
    }

    return SciNot(coefficientProduct, exponentSum);
}


//This function looks at the length of the given path and compares it to how
//long the path should be for the hits it contains (i.e. if the path perfectly
//matched up the query).
double BlastQueryPath::getRelativeLengthDiscrepancy() const
{
    if (m_hits.empty())
        return std::numeric_limits<double>::max();

    int hitQueryLength = getHitQueryLength();

    int discrepancy = m_path.getLength() - hitQueryLength;
    return double(discrepancy) / hitQueryLength;
}


//This function gets the length of the path relative to the how long it should
//be.  A value of 1 means a perfect match; less than 1 means it is too short;
//more than 1 means it is too long.
double BlastQueryPath::getRelativePathLength() const
{
    return double(m_path.getLength()) / getHitQueryLength();
}


//This function gets the difference between how long the path is vs how long it
//should be.  A value of 0 means a perfect match; less than 0 means it is too
//short; more than 0 means it is too long.
int BlastQueryPath::getAbsolutePathLengthDifference() const
{
    return m_path.getLength() - getHitQueryLength();
}


QString BlastQueryPath::getAbsolutePathLengthDifferenceString(bool commas) const
{
    int lengthDisc = getAbsolutePathLengthDifference();
    QString lengthDiscSign = "";
    if (lengthDisc > 0)
        lengthDiscSign = "+";
    if (commas)
        return lengthDiscSign + formatIntForDisplay(lengthDisc);
    else
        return lengthDiscSign + QString::number(lengthDisc);
}


//This function returns the fraction of the query that is covered by the entire
//path.
double BlastQueryPath::getPathQueryCoverage() const
{
    if (m_hits.empty())
        return 0.0;

    int queryStart = m_hits.front()->m_queryStart;
    int queryEnd = m_hits.back()->m_queryEnd;
    int queryLength = m_query->getLength();

    int notIncluded = queryStart - 1;
    notIncluded += queryLength - queryEnd;

    return 1.0 - notIncluded / double(queryLength);
}


//This function returns the fraction of the query that is covered by hits in the
//path.
double BlastQueryPath::getHitsQueryCoverage() const
{
    return m_query->fractionCoveredByHits(&m_hits);
}


//This function returns the length of the query which is covered by the path.
//It is returned in bp, whether or not the query is a protein or nucleotide
//sequence.
int BlastQueryPath::getHitQueryLength() const
{
    int queryStart = m_hits.front()->m_queryStart;
    int queryEnd = m_hits.back()->m_queryEnd;
    int hitQueryLength = queryEnd - queryStart + 1;

    if (m_query->getSequenceType() == PROTEIN)
        hitQueryLength *= 3;

    return hitQueryLength;
}



int BlastQueryPath::getTotalHitMismatches() const
{
    int total = 0;
    for (int i = 0; i < m_hits.size(); ++i)
        total += m_hits[i]->m_numberMismatches;
    return total;
}

int BlastQueryPath::getTotalHitGapOpens() const
{
    int total = 0;
    for (int i = 0; i < m_hits.size(); ++i)
        total += m_hits[i]->m_numberGapOpens;
    return total;
}


//This function is used for sorting the paths for a query from best to worst.
//it uses < to mean 'better than'.
bool BlastQueryPath::operator<(BlastQueryPath const &other) const
{
    //First we compare using the E-value product.  This seems to value stronger
    //hits as well as paths with fewer, longer hits.
    SciNot aEValueProduct = getEvalueProduct();
    SciNot bEValueProduct = other.getEvalueProduct();
    if (aEValueProduct != bEValueProduct)
        return aEValueProduct < bEValueProduct;

    //If the code got here, then the two paths have the same e-value product,
    //possibly because they contain the same hits, or possibly because they both
    //contain hits so strong as to have an e-value of zero.

    //Now we compare using mean percent identity.
    double aMeanPercIdentity = getMeanHitPercIdentity();
    double bMeanPercIdentity = other.getMeanHitPercIdentity();
    if (aMeanPercIdentity != bMeanPercIdentity)
        return aMeanPercIdentity > bMeanPercIdentity;

    //Now we use the absolute value of the length discrepancy.
    double aLengthDiscrepancy = fabs(getRelativeLengthDiscrepancy());
    double bLengthDiscrepancy = fabs(other.getRelativeLengthDiscrepancy());
    if (aLengthDiscrepancy != bLengthDiscrepancy)
        return aLengthDiscrepancy < bLengthDiscrepancy;

    //Now we use fraction of query covered by hits.
    double aHitsQueryCoverage = getHitsQueryCoverage();
    double bHitsQueryCoverage = other.getHitsQueryCoverage();
    if (aHitsQueryCoverage != bHitsQueryCoverage)
        return aHitsQueryCoverage > bHitsQueryCoverage;

    return false;
}

