//Copyright 2017 Ryan Wick

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


#include "blastsearch.h"
#include "../graph/assemblygraph.h"
#include <QDir>
#include <QRegularExpression>
#include "buildblastdatabaseworker.h"
#include "runblastsearchworker.h"
#include "../program/settings.h"
#include <QApplication>
#include "../graph/debruijnnode.h"
#include "../program/memory.h"
#include <math.h>

BlastSearch::BlastSearch() :
    m_blastQueries(), m_tempDirectory("bandage_temp/")
{
}

BlastSearch::~BlastSearch()
{
    cleanUp();
}

void BlastSearch::clearBlastHits()
{
    m_allHits.clear();
    m_blastQueries.clearSearchResults();
    m_blastOutput = "";
}

void BlastSearch::cleanUp()
{
    clearBlastHits();
    m_blastQueries.clearAllQueries();
    emptyTempDirectory();
}

//This function uses the contents of m_blastOutput (the raw output from the
//BLAST search) to construct the BlastHit objects.
//It looks at the filters to possibly exclude hits which fail to meet user-
//defined thresholds.
void BlastSearch::buildHitsFromBlastOutput()
{
    QStringList blastHitList = m_blastOutput.split("\n", QString::SkipEmptyParts);

    for (int i = 0; i < blastHitList.size(); ++i)
    {
        QString hitString = blastHitList[i];
        QStringList alignmentParts = hitString.split('\t');

        if (alignmentParts.size() < 12)
            continue;

        QString queryName = alignmentParts[0];
        QString nodeLabel = alignmentParts[1];
        double percentIdentity = alignmentParts[2].toDouble();
        int alignmentLength = alignmentParts[3].toInt();
        int numberMismatches = alignmentParts[4].toInt();
        int numberGapOpens = alignmentParts[5].toInt();
        int queryStart = alignmentParts[6].toInt();
        int queryEnd = alignmentParts[7].toInt();
        int nodeStart = alignmentParts[8].toInt();
        int nodeEnd = alignmentParts[9].toInt();
        SciNot eValue(alignmentParts[10]);
        double bitScore = alignmentParts[11].toDouble();

        //Only save BLAST hits that are on forward strands.
        if (nodeStart > nodeEnd)
            continue;

        QString nodeName = getNodeNameFromString(nodeLabel);
        DeBruijnNode * node;
        if (g_assemblyGraph->m_deBruijnGraphNodes.contains(nodeName))
            node = g_assemblyGraph->m_deBruijnGraphNodes[nodeName];
        else
            continue;

        BlastQuery * query = g_blastSearch->m_blastQueries.getQueryFromName(queryName);
        if (query == 0)
            continue;

        QSharedPointer<BlastHit> hit(new BlastHit(query, node, percentIdentity, alignmentLength,
                                                  numberMismatches, numberGapOpens, queryStart, queryEnd,
                                                  nodeStart, nodeEnd, eValue, bitScore));

        //Check the user-defined filters.
        if (g_settings->blastAlignmentLengthFilter.on &&
                alignmentLength < g_settings->blastAlignmentLengthFilter)
            continue;
        if (g_settings->blastQueryCoverageFilter.on)
        {
            double hitCoveragePercentage = 100.0 * hit->getQueryCoverageFraction();
            if (hitCoveragePercentage < g_settings->blastQueryCoverageFilter)
                continue;
        }
        if (g_settings->blastIdentityFilter.on &&
                percentIdentity < g_settings->blastIdentityFilter)
            continue;
        if (g_settings->blastEValueFilter.on &&
                eValue > g_settings->blastEValueFilter)
                continue;
        if (g_settings->blastBitScoreFilter.on &&
                bitScore < g_settings->blastBitScoreFilter)
            continue;

        m_allHits.push_back(hit);
        query->addHit(hit);
    }
}


//This function looks at each BLAST query and tries to find a path through
//the graph which covers the maximal amount of the query.
void BlastSearch::findQueryPaths()
{
    m_blastQueries.findQueryPaths();
}



QString BlastSearch::getNodeNameFromString(QString nodeString)
{
    QStringList nodeStringParts = nodeString.split("_");

    //The node string format should look like this:
    //NODE_nodename_length_123_cov_1.23

    if (nodeStringParts.size() < 6)
        return "";

    if (nodeStringParts.size() == 6)
        return nodeStringParts[1];

    //If the code got here, there are more than 6 parts.  This means there are
    //underscores in the node name (happens a lot with Trinity graphs).  So we
    //need to pull out the parts which consitute the name.
    int underscoreCount = nodeStringParts.size() - 6;
    QString nodeName = "";
    for (int i = 0; i <= underscoreCount; ++i)
    {
        nodeName += nodeStringParts[1+i];
        if (i < underscoreCount)
            nodeName += "_";
    }
    return nodeName;
}


#ifdef Q_OS_WIN32
//On Windows, we use the WHERE command to find a program.
bool BlastSearch::findProgram(QString programName, QString * command)
{
    QString findCommand = "WHERE " + programName;
    QProcess find;
    find.start(findCommand);
    find.waitForFinished();
    *command = programName;
    return (find.exitCode() == 0);
}

#else
//On Mac/Linux we use the which command to find a program.
bool BlastSearch::findProgram(QString programName, QString * command)
{
    QString findCommand = "which " + programName;
    QProcess find;

    //On Mac, it's necessary to adjust the PATH variable in order
    //for which to work.
#ifdef Q_OS_MAC
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    QStringList envlist = env.toStringList();

    //Add some paths to the process environment
    envlist.replaceInStrings(QRegularExpression("^(?i)PATH=(.*)"), "PATH="
                                                                   "/usr/bin:"
                                                                   "/bin:"
                                                                   "/usr/sbin:"
                                                                   "/sbin:"
                                                                   "/opt/local/bin:"
                                                                   "/usr/local/bin:"
                                                                   "$HOME/bin:"
                                                                   "/usr/local/ncbi/blast/bin:"
                                                                   "\\1");
    find.setEnvironment(envlist);
#endif

    find.start(findCommand);
    find.waitForFinished();

    //On a Mac, we need to use the full path to the program.
#ifdef Q_OS_MAC
    *command = QString(find.readAll()).simplified();
#else
    *command = programName;
#endif

    return (find.exitCode() == 0);
}
#endif



void BlastSearch::clearSomeQueries(std::vector<BlastQuery *> queriesToRemove)
{
    //Remove any hits that are for queries that will be deleted.
    QList< QSharedPointer<BlastHit> >::iterator i = m_allHits.begin();
    while (i != m_allHits.end())
    {
        BlastQuery * hitQuery = (*i)->m_query;
        bool hitIsForDeletedQuery = (std::find(queriesToRemove.begin(), queriesToRemove.end(), hitQuery) != queriesToRemove.end());
        if (hitIsForDeletedQuery)
            i = m_allHits.erase(i);
        else
            ++i;
    }

    //Now actually delete the queries.
    m_blastQueries.clearSomeQueries(queriesToRemove);
}



void BlastSearch::emptyTempDirectory()
{
    //Safety checks
    if (g_blastSearch->m_tempDirectory == "")
        return;
    if (!g_blastSearch->m_tempDirectory.contains("bandage_temp"))
        return;

    QDir tempDirectory(m_tempDirectory);
    tempDirectory.setNameFilters(QStringList() << "*.*");
    tempDirectory.setFilter(QDir::Files);
    foreach(QString dirFile, tempDirectory.entryList())
        tempDirectory.remove(dirFile);
}


//This function carries out the entire BLAST search procedure automatically, without user input.
//It returns an error string which is empty if all goes well.
QString BlastSearch::doAutoBlastSearch()
{
    cleanUp();

    QString makeblastdbCommand;
    if (!findProgram("makeblastdb", &makeblastdbCommand))
        return "Error: The program makeblastdb was not found.  Please install NCBI BLAST to use this feature.";

    BuildBlastDatabaseWorker buildBlastDatabaseWorker(makeblastdbCommand);
    buildBlastDatabaseWorker.buildBlastDatabase();
    if (buildBlastDatabaseWorker.m_error != "")
        return buildBlastDatabaseWorker.m_error;

    loadBlastQueriesFromFastaFile(g_settings->blastQueryFilename);

    QString blastnCommand;
    if (!findProgram("blastn", &blastnCommand))
        return "Error: The program blastn was not found.  Please install NCBI BLAST to use this feature.";
    QString tblastnCommand;
    if (!findProgram("tblastn", &tblastnCommand))
        return "Error: The program tblastn was not found.  Please install NCBI BLAST to use this feature.";

    RunBlastSearchWorker runBlastSearchWorker(blastnCommand, tblastnCommand, g_settings->blastSearchParameters);
    runBlastSearchWorker.runBlastSearch();
    if (runBlastSearchWorker.m_error != "")
        return runBlastSearchWorker.m_error;

    blastQueryChanged("all");

    return "";
}


//This function returns the number of queries loaded from the FASTA file.
int BlastSearch::loadBlastQueriesFromFastaFile(QString fullFileName)
{
    int queriesBefore = int(g_blastSearch->m_blastQueries.m_queries.size());

    std::vector<QString> queryNames;
    std::vector<QByteArray> querySequences;
    AssemblyGraph::readFastaOrFastqFile(fullFileName, &queryNames, &querySequences);

    for (size_t i = 0; i < queryNames.size(); ++i)
    {
        QApplication::processEvents();

        //We only use the part of the query name up to the first space.
        QStringList queryNameParts = queryNames[i].split(" ");
        QString queryName;
        if (queryNameParts.size() > 0)
            queryName = cleanQueryName(queryNameParts[0]);

        g_blastSearch->m_blastQueries.addQuery(new BlastQuery(queryName,
                                                              querySequences[i]));
    }

    int queriesAfter = int(g_blastSearch->m_blastQueries.m_queries.size());
    return queriesAfter - queriesBefore;
}


QString BlastSearch::cleanQueryName(QString queryName)
{
    //Replace whitespace with underscores
    queryName = queryName.replace(QRegExp("\\s"), "_");

    //Remove any dots from the end of the query name.  BLAST doesn't
    //include them in its results, so if we don't remove them, then
    //we won't be able to find a match between the query name and
    //the BLAST hit.
    while (queryName.length() > 0 && queryName[queryName.size() - 1] == '.')
        queryName = queryName.left(queryName.size() - 1);

    return queryName;
}

void BlastSearch::blastQueryChanged(QString queryName)
{
    g_assemblyGraph->clearAllBlastHitPointers();

    std::vector<BlastQuery *> queries;

    //If "all" is selected, then we'll display each of the BLAST queries
    if (queryName == "all")
        queries = g_blastSearch->m_blastQueries.m_queries;

    //If only one query is selected, then just display that one.
    else
    {
        BlastQuery * query = g_blastSearch->m_blastQueries.getQueryFromName(queryName);
        if (query != 0)
            queries.push_back(query);
    }

    //We now filter out any queries that have been hidden by the user.
    std::vector<BlastQuery *> shownQueries;
    for (size_t i = 0; i < queries.size(); ++i)
    {
        BlastQuery * query = queries[i];
        if (query->isShown())
            shownQueries.push_back(query);
    }

    //Add the blast hit pointers to nodes that have a hit for
    //the selected target(s).
    for (size_t i = 0; i < shownQueries.size(); ++i)
    {
        BlastQuery * query = shownQueries[i];
        for (int j = 0; j < g_blastSearch->m_allHits.size(); ++j)
        {
            BlastHit * hit = g_blastSearch->m_allHits[j].data();
            if (hit->m_query == query)
                hit->m_node->addBlastHit(hit);
        }
    }
}
