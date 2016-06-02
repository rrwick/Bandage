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


#include "buildblastdatabaseworker.h"
#include <QProcess>
#include "../program/globals.h"
#include "../program/settings.h"
#include <QFile>
#include <QTextStream>
#include <QMapIterator>
#include "../graph/debruijnnode.h"
#include "../graph/assemblygraph.h"
#include "blastsearch.h"

BuildBlastDatabaseWorker::BuildBlastDatabaseWorker(QString makeblastdbCommand) :
    m_makeblastdbCommand(makeblastdbCommand)
{
}

void BuildBlastDatabaseWorker::buildBlastDatabase()
{
    g_blastSearch->m_cancelBuildBlastDatabase = false;

    QFile file(g_blastSearch->m_tempDirectory + "all_nodes.fasta");
    file.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream out(&file);

    QMapIterator<QString, DeBruijnNode*> i(g_assemblyGraph->m_deBruijnGraphNodes);
    while (i.hasNext())
    {
        if (g_blastSearch->m_cancelBuildBlastDatabase)
        {
            emit finishedBuild("Build cancelled.");
            return;
        }

        i.next();
        DeBruijnNode * node = i.value();
        out << node->getFasta(true, false, false);
    }
    file.close();

    // Make sure the graph has sequences to BLAST.
    bool atLeastOneSequence = false;
    QMapIterator<QString, DeBruijnNode*> j(g_assemblyGraph->m_deBruijnGraphNodes);
    while (j.hasNext())
    {
        j.next();
        DeBruijnNode * node = j.value();
        if (!node->sequenceIsMissing())
        {
            atLeastOneSequence = true;
            break;
        }
    }
    if (!atLeastOneSequence)
    {
        m_error = "Cannot build the BLAST database as this graph contains no sequences";
        emit finishedBuild(m_error);
        return;
    }

    QString fullMakeblastdbCommand = m_makeblastdbCommand + " -in " + g_blastSearch->m_tempDirectory + "all_nodes.fasta " + "-dbtype nucl";
    g_blastSearch->m_makeblastdb = new QProcess();
    g_blastSearch->m_makeblastdb->start(fullMakeblastdbCommand);

    bool finished = g_blastSearch->m_makeblastdb->waitForFinished(-1);

    if (g_blastSearch->m_makeblastdb->exitCode() != 0 || !finished)
    {
        m_error = "There was a problem building the BLAST database";
        QString stdErr = g_blastSearch->m_makeblastdb->readAllStandardError();
        if (stdErr.length() > 0)
            m_error += ":\n\n" + stdErr;
        else
            m_error += ".";
    }
    else if (g_blastSearch->m_cancelBuildBlastDatabase)
        m_error = "Build cancelled.";
    else
        m_error = "";

    emit finishedBuild(m_error);

    g_blastSearch->m_makeblastdb->deleteLater();
    g_blastSearch->m_makeblastdb = 0;
}
