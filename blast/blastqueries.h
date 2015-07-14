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


#ifndef BLASTQUERIES_H
#define BLASTQUERIES_H

#include <vector>
#include "blastquery.h"
#include <QFile>
#include "../program/globals.h"
#include <QSharedPointer>

//This class manages all BLAST queries. It holds BlastQuery
//objects itself, and it creates/modifies/deletes the temp
//files which hold the queries for use in BLAST.
//There are two separate temp files, one for nucleotide
//queries (for blastn) and one for protein queries (for
//tblasn).

class BlastQueries
{
public:
    BlastQueries();
    ~BlastQueries();

    std::vector<BlastQuery *> m_queries;

    BlastQuery * getQueryFromName(QString queryName);

    void createTempQueryFiles();
    void addQuery(BlastQuery * newQuery);
    void clearAllQueries();
    void clearSomeQueries(std::vector<BlastQuery *> queriesToRemove);
    void searchOccurred();
    void clearSearchResults();
    int getQueryCount(SequenceType sequenceType);

    std::vector<QColor> presetColours;

private:
    QSharedPointer<QFile> m_tempNuclFile;
    QSharedPointer<QFile> m_tempProtFile;

    void deleteTempFiles();
    void updateTempFiles();
    bool tempNuclFileExists();
    bool tempProtFileExists();
    void createPresetColours();
    void writeTempFile(QSharedPointer<QFile> file, SequenceType sequenceType);

};

#endif // BLASTQUERIES_H
