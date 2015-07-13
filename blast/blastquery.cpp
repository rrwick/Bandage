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

BlastQuery::BlastQuery(QString name, QString sequence) :
    m_name(name), m_sequence(sequence),
    m_hits(0), m_searchedFor(false)
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
    m_hits = 0;
}
