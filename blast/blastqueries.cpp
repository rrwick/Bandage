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


#include "blastqueries.h"
#include "../program/globals.h"
#include "../program/settings.h"
#include <QTextStream>
#include "blastsearch.h"
#include "../program/memory.h"

BlastQueries::BlastQueries() :
    m_tempNuclFile(0), m_tempProtFile(0)
{
    m_presetColours = getPresetColours();
}


BlastQueries::~BlastQueries()
{
    clearAllQueries();
}


void BlastQueries::createTempQueryFiles()
{
    m_tempNuclFile.reset(new QFile(g_blastSearch->m_tempDirectory + "nucl_queries.fasta"));
    m_tempProtFile.reset(new QFile(g_blastSearch->m_tempDirectory + "prot_queries.fasta"));
}

BlastQuery * BlastQueries::getQueryFromName(QString queryName)
{
    for (size_t i = 0; i < m_queries.size(); ++i)
    {
        if (m_queries[i]->getName() == queryName)
            return m_queries[i];
    }
    return 0;
}


void BlastQueries::addQuery(BlastQuery * newQuery)
{
    newQuery->setName(getUniqueName(newQuery->getName()));

    //Give the new query a colour
    int colourIndex = int(m_queries.size());
    colourIndex %= m_presetColours.size();
    newQuery->setColour(m_presetColours[colourIndex]);

    m_queries.push_back(newQuery);
    updateTempFiles();
}


//This function renames the query.  It returns the name given, because that
//might not be exactly the same as the name passed to the function if it
//wasn't unique.
QString BlastQueries::renameQuery(BlastQuery * newQuery, QString newName)
{
    newQuery->setName(getUniqueName(newName));
    updateTempFiles();
    return newQuery->getName();
}


//This function looks at the name, and if it is not unique, it adds a suffix
//to make it unique.  Also make sure it's not "all" or "none", as those will
//conflict with viewing all queries at once or no queries.
QString BlastQueries::getUniqueName(QString name)
{
    //If the query name ends in a semicolon, remove it.  Ending semicolons
    //mess with BLAST.
    if (name.endsWith(';'))
        name.chop(1);

    //The name can't be empty.
    if (name == "")
        name = g_settings->unnamedQueryDefaultName;

    int queryNumber = 2;
    QString finalName = name;
    while (getQueryFromName(finalName) != 0 ||
           finalName == "all" || finalName == "none")
        finalName = name + "_" + QString::number(queryNumber++);
    return finalName;
}

void BlastQueries::clearAllQueries()
{
    for (size_t i = 0; i < m_queries.size(); ++i)
        delete m_queries[i];
    m_queries.clear();
    deleteTempFiles();
}

void BlastQueries::clearSomeQueries(std::vector<BlastQuery *> queriesToRemove)
{
    for (size_t i = 0; i < queriesToRemove.size(); ++i)
    {
        m_queries.erase(std::remove(m_queries.begin(), m_queries.end(), queriesToRemove[i]), m_queries.end());
        delete queriesToRemove[i];
    }

    updateTempFiles();
}

void BlastQueries::deleteTempFiles()
{
    if (tempNuclFileExists())
        m_tempNuclFile->remove();
    if (tempProtFileExists())
        m_tempProtFile->remove();
}

void BlastQueries::updateTempFiles()
{
    deleteTempFiles();

    if (getQueryCount(NUCLEOTIDE) > 0)
        writeTempFile(m_tempNuclFile, NUCLEOTIDE);

    if (getQueryCount(PROTEIN) > 0)
        writeTempFile(m_tempProtFile, PROTEIN);
}


void BlastQueries::writeTempFile(QSharedPointer<QFile> file, SequenceType sequenceType)
{
    file->open(QIODevice::Append | QIODevice::Text);
    QTextStream out(file.data());
    for (size_t i = 0; i < m_queries.size(); ++i)
    {
        if (m_queries[i]->getSequenceType() == sequenceType)
        {
            out << ">" << m_queries[i]->getName() << "\n";
            out << m_queries[i]->getSequence();
            out << "\n";
        }
    }
    file->close();
}


void BlastQueries::searchOccurred()
{
    for (size_t i = 0; i < m_queries.size(); ++i)
        m_queries[i]->setAsSearchedFor();
}


void BlastQueries::clearSearchResults()
{
    for (size_t i = 0; i < m_queries.size(); ++i)
        m_queries[i]->clearSearchResults();
}


int BlastQueries::getQueryCount()
{
    return int(m_queries.size());
}

int BlastQueries::getQueryCountWithAtLeastOnePath()
{
    int count = 0;

    for (size_t i = 0; i < m_queries.size(); ++i)
    {
        if (m_queries[i]->getPathCount() > 0)
            ++count;
    }

    return count;
}

int BlastQueries::getQueryPathCount()
{
    int count = 0;

    for (size_t i = 0; i < m_queries.size(); ++i)
        count += m_queries[i]->getPathCount();
    return count;
}

int BlastQueries::getQueryCount(SequenceType sequenceType)
{
    int count = 0;
    for (size_t i = 0; i < m_queries.size(); ++i)
    {
        if (m_queries[i]->getSequenceType() == sequenceType)
            ++count;
    }
    return count;
}


bool BlastQueries::tempNuclFileExists()
{
    if (m_tempNuclFile.isNull())
        return false;
    return m_tempNuclFile->exists();
}
bool BlastQueries::tempProtFileExists()
{
    if (m_tempProtFile.isNull())
        return false;
    return m_tempProtFile->exists();
}


//This function looks to see if a query pointer is in the list
//of queries.  The query pointer given may or may not still
//actually exist, so it can't be dereferenced.
bool BlastQueries::isQueryPresent(BlastQuery * query)
{
    for (size_t i = 0; i < m_queries.size(); ++i)
    {
        if (query == m_queries[i])
            return true;
    }

    return false;
}


//This function looks at each BLAST query and tries to find a path through
//the graph which covers the maximal amount of the query.
void BlastQueries::findQueryPaths()
{
    for (size_t i = 0; i < m_queries.size(); ++i)
        m_queries[i]->findQueryPaths();
}


