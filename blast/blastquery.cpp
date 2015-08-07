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


#include "blastquery.h"
#include "../program/settings.h"
#include "../graph/path.h"
#include "../graph/debruijnnode.h"

BlastQuery::BlastQuery(QString name, QString sequence) :
    m_name(name), m_sequence(sequence), m_searchedFor(false), m_shown(true)
{
    autoSetSequenceType();
}


//This function looks at the query sequence to decide if it is
//a nucleotide or protein sequence.
void BlastQuery::autoSetSequenceType()
{
    //If the sequence contains a letter that's in the protein
    //alphabet but not in the extended DNA alphabet, then it's
    //a protein
    if (m_sequence.contains('e') || m_sequence.contains('E') ||
            m_sequence.contains('f') || m_sequence.contains('F') ||
            m_sequence.contains('i') || m_sequence.contains('I') ||
            m_sequence.contains('l') || m_sequence.contains('L') ||
            m_sequence.contains('p') || m_sequence.contains('P') ||
            m_sequence.contains('q') || m_sequence.contains('Q'))
    {
        m_sequenceType = PROTEIN;
        return;
    }

    //If the code got here, then it's a bit trickier.  It could
    //possibly be an extended alphabet DNA sequence or a protein
    //sequence without particular amino acids.

    //Look to see if A, C, G and T make up 75% or more of
    //the sequence.  If so, it's DNA.  If not, it's
    //protein.
    int length = m_sequence.length();
    int nuclLetters = m_sequence.count('a') + m_sequence.count('A') +
            m_sequence.count('c') + m_sequence.count('C') +
            m_sequence.count('g') + m_sequence.count('G') +
            m_sequence.count('t') + m_sequence.count('T');

    if (double(nuclLetters) / length >= 0.75)
        m_sequenceType = NUCLEOTIDE;
    else
        m_sequenceType = PROTEIN;
}


QString BlastQuery::getTypeString() const
{
    if (m_sequenceType == NUCLEOTIDE)
        return "nucl";
    else
        return "prot";
}


void BlastQuery::clearSearchResults()
{
    m_searchedFor = false;
    m_hits.clear();
}





//This function tries to find the paths through the graph which cover the query.
void BlastQuery::findQueryPaths()
{
    int queryLength = m_sequence.length();
    if (m_sequenceType == PROTEIN)
        queryLength *= 3;

    //Find all possible path starts within an acceptable distance from the query
    //start.
    QList<BlastHit *> possibleStarts;
    double acceptableStartFraction = 1.0 - g_settings->queryRequiredCoverage;
    for (int i = 0; i < m_hits.size(); ++i)
    {
        BlastHit * hit = m_hits[i].data();
        if (hit->m_queryStartFraction <= acceptableStartFraction)
            possibleStarts.push_back(hit);
    }

    //Find all possible path ends.
    QList<BlastHit *> possibleEnds;
    double acceptableEndFraction = g_settings->queryRequiredCoverage;
    for (int i = 0; i < m_hits.size(); ++i)
    {
        BlastHit * hit = m_hits[i].data();
        if (hit->m_queryEndFraction >= acceptableEndFraction)
            possibleEnds.push_back(hit);
    }

    //For each possible start, find paths to each possible end.
    QList<Path> possiblePaths;
    for (int i = 0; i < possibleStarts.size(); ++i)
    {
        BlastHit * start = possibleStarts[i];
        GraphLocation startLocation = start->getHitStart();

        for (int j = 0; j < possibleEnds.size(); ++j)
        {
            BlastHit * end = possibleEnds[j];
            GraphLocation endLocation = end->getHitEnd();

            //Assuming there is a path from the start hit to the end hit,
            //determine the ideal length.  This is the query length minus the
            //parts of the query not covered by the start and end.
            int partialQueryLength = queryLength;
            int pathStart = start->m_queryStart - 1;
            int pathEnd = end->m_queryEnd;
            if (m_sequenceType == PROTEIN)
            {
                pathStart *= 3;
                pathEnd *= 3;
            }
            partialQueryLength -= pathStart;
            partialQueryLength -= queryLength - pathEnd;

            int minLength = int(partialQueryLength * (1.0 - g_settings->queryAllowedLengthDiscrepancy) + 0.5);
            int maxLength = int(partialQueryLength * (1.0 + g_settings->queryAllowedLengthDiscrepancy) + 0.5);

            possiblePaths.append(Path::getAllPossiblePaths(startLocation,
                                                           endLocation,
                                                           g_settings->maxQueryPathNodes - 1,
                                                           minLength,
                                                           maxLength));
        }
    }

    //We now want to throw out any paths for which the hits do not cover a
    //sufficient amount of the query.
    QList<Path> sufficientCoveragePaths;
    for (int i = 0; i < possiblePaths.size(); ++i)
    {
        QList<BlastHit *> pathHits = possiblePaths[i].getBlastHitsForQuery(this);
        double fractionCovered = fractionCoveredByHits(&pathHits);
        if (fractionCovered >= g_settings->queryRequiredCoverage)
            sufficientCoveragePaths.push_back(possiblePaths[i]);
    }

    //We now want to throw out any paths which are sub-paths of other, larger
    //paths.
    m_paths = QList<Path>();
    for (int i = 0; i < sufficientCoveragePaths.size(); ++i)
    {
        bool throwOut = false;
        for (int j = 0; j < sufficientCoveragePaths.size(); ++j)
        {
            //No need to compare a path with itself.
            if (i == j)
                continue;

            if (sufficientCoveragePaths[i].hasNodeSubset(sufficientCoveragePaths[j]))
            {
                throwOut = true;
                break;
            }
        }
        if (!throwOut)
            m_paths.push_back(sufficientCoveragePaths[i]);
    }

    //Now we sort the paths from best to worst.  Since I can't normally use a
    //member function in std::sort, I just do a simple bubble sort here.
    bool swapped = true;
    int j = 0;
    while (swapped)
    {
        swapped = false;
        j++;
        for (int i = 0; i < m_paths.size() - j; i++)
        {
            if (comparePaths(m_paths[i+1], m_paths[i]))
            {
                m_paths.swap(i, i+1);
                swapped = true;
            }
        }
    }
}


//This function compares two paths using their hits for this query.  It is used
//for sorting the query's paths.  It compares first using the e-value product
//of all the hits, and if they are the same, it uses the length discrepancy.
bool BlastQuery::comparePaths(Path a, Path b)
{
    long double aEValueProduct = getPathEValueProduct(a);
    long double bEValueProduct = getPathEValueProduct(b);

    if (aEValueProduct != bEValueProduct)
        return aEValueProduct < bEValueProduct;

    //If the code got here, then the two paths have the same evalue product,
    //probably because they contain the same hits.  In this case, we use their
    //length discrepency.

    return getRelativeLengthDiscrepancy(a) < getRelativeLengthDiscrepancy(b);
}

//This function looks at all of the hits in the path for this query and
//multiplies the evalues together.
long double BlastQuery::getPathEValueProduct(Path path)
{
    long double eValueProduct = 1.0;
    QList<BlastHit *> pathHits = path.getBlastHitsForQuery(this);
    for (int j = 0; j < pathHits.size(); ++j)
        eValueProduct *= pathHits[j]->m_eValue;

    return eValueProduct;
}


//This function looks at the length of the given path and compares it to how
//long the path should be for the hits it contains (i.e. if the path perfectly
//matched up the query).
double BlastQuery::getRelativeLengthDiscrepancy(Path path)
{
    QList<BlastHit *> pathHits = path.getBlastHitsForQuery(this);
    if (pathHits.empty())
        return std::numeric_limits<double>::max();

    int queryStart = pathHits.front()->m_queryStart;
    int queryEnd = pathHits.back()->m_queryEnd;
    int hitQueryLength = queryEnd - queryStart;
    if (m_sequenceType == PROTEIN)
        hitQueryLength *= 3;

    int discrepancy = abs(path.getLength() - hitQueryLength);
    return double(discrepancy) / hitQueryLength;
}



//This function returns the fraction of the query that is covered by BLAST hits.
//If a list of BLAST hits is passed to the function, it only looks in those
//hits.  If no such list is passed, it looks in all hits for this query.
double BlastQuery::fractionCoveredByHits(QList<BlastHit *> * hitsToCheck) const
{
    int hitBases = 0;
    int queryLength = getLength();
    if (queryLength == 0)
        return 0.0;

    for (int i = 0; i < queryLength; ++i)
    {
        //Add one to the index because BLAST results use 1-based indexing.
        if (hitsToCheck == 0)
        {
            if (positionInAnyHit(i+1))
                ++hitBases;
        }
        else
        {
            if (positionInHitList(i+1, hitsToCheck))
                ++hitBases;
        }
    }

    return double(hitBases) / queryLength;
}


//This accepts a position with 1-based indexing, which is what BLAST results
//use.
bool BlastQuery::positionInAnyHit(int position) const
{
    for (int i = 0; i < m_hits.size(); ++i)
    {
        BlastHit * hit = m_hits[i].data();
        if (position >= hit->m_queryStart && position <= hit->m_queryEnd)
            return true;
    }
    return false;
}

bool BlastQuery::positionInHitList(int position, QList<BlastHit *> * hitsToCheck) const
{
    for (int i = 0; i < hitsToCheck->size(); ++i)
    {
        BlastHit * hit = (*hitsToCheck)[i];
        if (position >= hit->m_queryStart && position <= hit->m_queryEnd)
            return true;
    }
    return false;
}


//This function returns the paths in string form, if any exist.
QString BlastQuery::getPathsString(int max) const
{
    if (m_paths.empty())
        return "-";

    QString pathsString;

    int count = std::min(m_paths.size(), max);
    for (int i = 0; i < count; ++i)
    {
        pathsString += m_paths[i].getString(true);
        if (i < count - 1)
            pathsString += "; ";
    }

    return pathsString;
}
