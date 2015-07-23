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
    m_name(name), m_sequence(sequence), m_searchedFor(false)
{
    m_length = sequence.length();
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


QString BlastQuery::getTypeString()
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


//This function tries to find a path through the graph which covers the maximal
//amount of the query.
void BlastQuery::findQueryPath()
{
    //Determine the acceptable path length range for this query.
    int queryLength = m_sequence.length();
    if (m_sequenceType == PROTEIN)
        queryLength *= 3;
    int minLength = int(queryLength * (1.0 - g_settings->queryAllowedLengthDiscrepancy) + 0.5);
    int maxLength = int(queryLength * (1.0 + g_settings->queryAllowedLengthDiscrepancy) + 0.5);


    //Find all possible path starts within an acceptable distance from the query
    //start.
    QList<BlastHit *> possibleStarts;
    double acceptableStartFraction = 1.0 - g_settings->queryRequiredCoverage;
    for (int i = 0; i < m_hits.size(); ++i)
    {
        BlastHit * hit = m_hits[i].data();
        if (hit->m_queryStartFraction < acceptableStartFraction)
            possibleStarts.push_back(hit);
    }

    //Find all possible path ends.
    QList<BlastHit *> possibleEnds;
    double acceptableEndFraction = g_settings->queryRequiredCoverage;
    for (int i = 0; i < m_hits.size(); ++i)
    {
        BlastHit * hit = m_hits[i].data();
        if (hit->m_queryEndFraction > acceptableEndFraction)
            possibleEnds.push_back(hit);
    }

    //For each possible start, find paths to each possible end.
    QList<Path> possiblePaths;
    for (int i = 0; i < possibleStarts.size(); ++i)
    {
        BlastHit * start = possibleStarts[i];
        DeBruijnNode * startNode = start->m_node;
        int startPosition = start->m_nodeStart;

        for (int j = 0; j < possibleEnds.size(); ++j)
        {
            BlastHit * end = possibleEnds[j];
            DeBruijnNode * endNode = end->m_node;
            int endPosition = end->m_nodeEnd;

            possiblePaths.append(Path::getAllPossiblePaths(startNode,
                                                           startPosition,
                                                           endNode, endPosition,
                                                           g_settings->queryPathSearchDepth,
                                                           minLength,
                                                           maxLength));
        }
    }

    //We now want to throw out any paths which are sub-paths of other, larger
    //paths.
    m_paths = QList<Path>();
    for (int i = 0; i < possiblePaths.size(); ++i)
    {
        bool subpathOfAnyOther = false;
        for (int j = 0; j < possiblePaths.size(); ++j)
        {
            //No need to compare a path with itself.
            if (i == j)
                continue;

            if (possiblePaths[i].hasNodeSubset(possiblePaths[j]))
            {
                subpathOfAnyOther = true;
                break;
            }
        }
        if (!subpathOfAnyOther)
            m_paths.push_back(possiblePaths[i]);
    }

    //If there were no paths which succeeded, then this query gets no path (i.e.
    //an empty path)
    if (m_paths.empty())
    {
        m_bestPath = Path();
        return;
    }

    //If there are multiple paths, we choose the best one by summing the bit
    //scores for the hits on each path and choosing the largest.
    double bestSummedBitScore = 0.0;
    double bestSummedBitScoreIndex = 0.0;
    for (int i = 0; i < m_paths.size(); ++i)
    {
        double summedBitScore = 0.0;
        QList<BlastHit *> pathHits = m_paths[i].getBlastHitsForQuery(this);
        for (int j = 0; j < pathHits.size(); ++j)
            summedBitScore += pathHits[j]->m_bitScore;

        if (summedBitScore > bestSummedBitScore)
        {
            bestSummedBitScore = summedBitScore;
            bestSummedBitScoreIndex = i;
        }
    }
    m_bestPath = m_paths[bestSummedBitScoreIndex];
}


double BlastQuery::fractionCoveredByHits()
{
    int hitBases = 0;
    for (int i =0; i < m_length; ++i)
    {
        if (positionInAHit(i))
            ++hitBases;
    }

    return double(hitBases) / m_length;
}


bool BlastQuery::positionInAHit(int position)
{
    for (int i = 0; i < m_hits.size(); ++i)
    {
        BlastHit * hit = m_hits[i].data();
        if (position >= hit->m_queryStart && position <= hit->m_queryEnd)
            return true;
    }
    return false;
}
