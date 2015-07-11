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


#include "runblastsearchworker.h"
#include "../program/globals.h"
#include "../program/settings.h"
#include "blastsearch.h"


RunBlastSearchWorker::RunBlastSearchWorker(QString blastnCommand, QString tblastnCommand, QString parameters) :
    m_blastnCommand(blastnCommand), m_tblastnCommand(tblastnCommand), m_parameters(parameters)
{

}


void RunBlastSearchWorker::runBlastSearch()
{
    bool success;

    if (g_blastSearch->m_blastQueries.nuclQueryCount() > 0)
    {
        g_blastSearch->m_hitsString += runOneBlastSearch(NUCLEOTIDE, &success);
        if (!success)
            return;
    }

    if (g_blastSearch->m_blastQueries.protQueryCount() > 0 && !g_blastSearch->m_cancelRunBlastSearch)
    {
        g_blastSearch->m_hitsString += runOneBlastSearch(PROTEIN, &success);
        if (!success)
            return;
    }

    if (g_blastSearch->m_cancelRunBlastSearch)
        emit finishedSearch("Search cancelled.");
    else
        emit finishedSearch("");
}


QString RunBlastSearchWorker::runOneBlastSearch(SequenceType sequenceType, bool * success)
{
    QString fullBlastCommand;
    if (sequenceType == NUCLEOTIDE)
        fullBlastCommand = m_blastnCommand + " -query " + g_tempDirectory + "nucl_queries.fasta ";
    else
        fullBlastCommand = m_tblastnCommand + " -query " + g_tempDirectory + "prot_queries.fasta ";
    fullBlastCommand += "-db " + g_tempDirectory + "all_nodes.fasta -outfmt 6";

    QString extraCommandLineOptions = m_parameters;
    fullBlastCommand += " " + extraCommandLineOptions;

    g_blastSearch->m_blast = new QProcess();
    g_blastSearch->m_blast->start(fullBlastCommand);

    bool finished = g_blastSearch->m_blast->waitForFinished(-1);

    if (g_blastSearch->m_blast->exitCode() != 0 || !finished)
    {
        if (g_blastSearch->m_cancelRunBlastSearch)
            emit finishedSearch("Search cancelled.");
        else
            emit finishedSearch("There was a problem building the BLAST database.");
        *success = false;
        return "";
    }

    *success = true;
    return g_blastSearch->m_blast->readAll();
}
