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


#include "blastsearch.h"
#include "../graph/assemblygraph.h"

BlastSearch::BlastSearch()
{
}

BlastSearch::~BlastSearch()
{
    cleanUp();
}

void BlastSearch::clearBlastHits()
{
    m_hits.clear();
    m_blastQueries.clearSearchResults();
    m_blastOutput = "";
}

void BlastSearch::cleanUp()
{
    clearBlastHits();
    m_blastQueries.clearAllQueries();
}

//This function uses the contents of m_blastOutput to construct
//the BlastHit objects.
void BlastSearch::buildHitsFromBlastOutput()
{
    QStringList blastHitList = m_blastOutput.split("\n", QString::SkipEmptyParts);

    for (int i = 0; i < blastHitList.size(); ++i)
    {
        QString hit = blastHitList[i];
        QStringList alignmentParts = hit.split('\t');

        if (alignmentParts.size() < 12)
            return;

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
        double eValue = alignmentParts[10].toDouble();
        int bitScore = alignmentParts[11].toInt();

        //Only save BLAST hits that are on forward strands.
        if (nodeStart > nodeEnd)
            continue;

        QString nodeName = getNodeNameFromString(nodeLabel);
        DeBruijnNode * node;
        if (g_assemblyGraph->m_deBruijnGraphNodes.contains(nodeName))
            node = g_assemblyGraph->m_deBruijnGraphNodes[nodeName];
        else
            return;

        BlastQuery * query = g_blastSearch->m_blastQueries.getQueryFromName(queryName);
        if (query == 0)
            return;

        g_blastSearch->m_hits.push_back(BlastHit(query, node, percentIdentity, alignmentLength,
                                                 numberMismatches, numberGapOpens, queryStart, queryEnd,
                                                 nodeStart, nodeEnd, eValue, bitScore));

        ++(query->m_hits);
    }
}



QString BlastSearch::getNodeNameFromString(QString nodeString)
{
    QStringList nodeStringParts = nodeString.split("_");
    return nodeStringParts[1];
}



bool BlastSearch::findProgram(QString programName, QString * command)
{
    QString findCommand = "which " + programName;
#ifdef Q_OS_WIN32
    findCommand = "WHERE " + programName;
#endif

    QProcess find;

    //On Mac, it's necessary to do some stuff with the PATH variable in order
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
#endif

    return (find.exitCode() == 0);
}





void BlastSearch::clearSomeQueries(std::vector<BlastQuery *> queriesToRemove)
{
    //Remove any hits that are for queries that will be deleted.
    std::vector<BlastHit>::iterator i;
    for (i = m_hits.begin(); i != m_hits.end(); )
    {
        BlastQuery * hitQuery = i->m_query;
        bool hitIsForDeletedQuery = (std::find(queriesToRemove.begin(), queriesToRemove.end(), hitQuery) != queriesToRemove.end());
        if (hitIsForDeletedQuery)
            i = m_hits.erase(i);
        else
            ++i;
    }

    //Now actually delete the queries.
    m_blastQueries.clearSomeQueries(queriesToRemove);
}



