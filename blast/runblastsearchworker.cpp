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


#include "runblastsearchworker.h"
#include "../program/globals.h"
#include "../program/settings.h"
#include "blastsearch.h"
#include "../program/memory.h"


RunBlastSearchWorker::RunBlastSearchWorker(QString blastnCommand, QString tblastnCommand, QString parameters) :
    m_blastnCommand(blastnCommand), m_tblastnCommand(tblastnCommand), m_parameters(parameters)
{

}


void RunBlastSearchWorker::runBlastSearch()
{
    g_blastSearch->m_cancelRunBlastSearch = false;

    bool success;

    if (g_blastSearch->m_blastQueries.getQueryCount(NUCLEOTIDE) > 0)
    {
        g_blastSearch->m_blastOutput += runOneBlastSearch(NUCLEOTIDE, &success);
        if (!success)
            return;
    }

    if (g_blastSearch->m_blastQueries.getQueryCount(PROTEIN) > 0 && !g_blastSearch->m_cancelRunBlastSearch)
    {
        g_blastSearch->m_blastOutput += runOneBlastSearch(PROTEIN, &success);
        if (!success)
            return;
    }

    if (g_blastSearch->m_cancelRunBlastSearch)
    {
        m_error = "BLAST search cancelled.";
        emit finishedSearch(m_error);
        return;
    }

    //If the code got here, then the search completed successfully.
    g_blastSearch->buildHitsFromBlastOutput();
    g_blastSearch->findQueryPaths();
    g_blastSearch->m_blastQueries.searchOccurred();
    m_error = "";
    emit finishedSearch(m_error);
}


QString RunBlastSearchWorker::runOneBlastSearch(SequenceType sequenceType, bool * success)
{
    QString fullBlastCommand;
    if (sequenceType == NUCLEOTIDE)
        fullBlastCommand = m_blastnCommand + " -query " +g_blastSearch-> m_tempDirectory + "nucl_queries.fasta ";
    else
        fullBlastCommand = m_tblastnCommand + " -query " + g_blastSearch->m_tempDirectory + "prot_queries.fasta ";
    fullBlastCommand += "-db " + g_blastSearch->m_tempDirectory + "all_nodes.fasta -outfmt 6";

    QString extraCommandLineOptions = m_parameters;
    fullBlastCommand += " " + extraCommandLineOptions;

    g_blastSearch->m_blast = new QProcess();
    g_blastSearch->m_blast->start(fullBlastCommand);

    bool finished = g_blastSearch->m_blast->waitForFinished(-1);

    if (g_blastSearch->m_blast->exitCode() != 0 || !finished)
    {
        if (g_blastSearch->m_cancelRunBlastSearch)
        {
            m_error = "BLAST search cancelled.";
            emit finishedSearch(m_error);
        }
        else
        {
            m_error = "There was a problem running the BLAST search";
            QString stdErr = g_blastSearch->m_blast->readAllStandardError();
            if (stdErr.length() > 0)
                m_error += ":\n\n" + stdErr;
            else
                m_error += ".";
            emit finishedSearch(m_error);
        }
        *success = false;
        return "";
    }

    QString blastOutput = g_blastSearch->m_blast->readAllStandardOutput();
    g_blastSearch->m_blast->deleteLater();
    g_blastSearch->m_blast = 0;

    *success = true;
    return blastOutput;
}
