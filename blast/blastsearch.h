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


#ifndef BLASTSEARCH_H
#define BLASTSEARCH_H

#include "blasthit.h"
#include "blastqueries.h"
#include <vector>
#include <QString>
#include <QList>
#include <QSharedPointer>
#include "../program/scinot.h"

//This is a class to hold all BLAST search related stuff.
//An instance of it is made available to the whole program
//as a global.

class BlastSearch
{
public:
    BlastSearch();
    ~BlastSearch();

    BlastQueries m_blastQueries;
    QString m_blastOutput;
    bool m_cancelBuildBlastDatabase;
    bool m_cancelRunBlastSearch;
    QProcess * m_makeblastdb;
    QProcess * m_blast;
    QString m_tempDirectory;
    QList< QSharedPointer<BlastHit> > m_allHits;

    void clearBlastHits();
    void cleanUp();
    void buildHitsFromBlastOutput();
    void findQueryPaths();
    static QString getNodeNameFromString(QString nodeString);
    bool findProgram(QString programName, QString * command);
    void clearSomeQueries(std::vector<BlastQuery *> queriesToRemove);
    void emptyTempDirectory();
    QString doAutoBlastSearch();
    int loadBlastQueriesFromFastaFile(QString fullFileName);
    QString cleanQueryName(QString queryName);
    void blastQueryChanged(QString queryName);
};

#endif // BLASTSEARCH_H
