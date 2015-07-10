#include "buildblastdatabaseworker.h"
#include <QProcess>
#include "../program/globals.h"
#include "../program/settings.h"
#include <QFile>
#include <QTextStream>
#include <QMapIterator>
#include "../graph/debruijnnode.h"
#include "../graph/assemblygraph.h"

BuildBlastDatabaseWorker::BuildBlastDatabaseWorker(QString makeblastdbCommand) :
    m_makeblastdbCommand(makeblastdbCommand)
{
}

void BuildBlastDatabaseWorker::buildBlastDatabase()
{
    QFile file(g_tempDirectory + "all_nodes.fasta");
    file.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream out(&file);

    QMapIterator<QString, DeBruijnNode*> i(g_assemblyGraph->m_deBruijnGraphNodes);
    while (i.hasNext())
    {
        if (g_cancelBuildBlastDatabase)
        {
            emit finishedBuild("Build cancelled.");
            return;
        }

        i.next();
        if (i.value()->m_length > 0)
        {
            out << i.value()->getFasta();
            out << "\n";
        }
    }
    file.close();

    g_makeblastdb = new QProcess();
    g_makeblastdb->start(m_makeblastdbCommand + " -in " + g_tempDirectory + "all_nodes.fasta " + "-dbtype nucl");

    bool finished = g_makeblastdb->waitForFinished(-1);

    if (g_makeblastdb->exitCode() != 0)
        emit finishedBuild("There was a problem building the BLAST database.");
    else if (!finished)
        emit finishedBuild("The BLAST database did not build in the allotted time.\n\n"
                           "Increase the 'Allowed time' setting and try again.");
    else if (g_cancelBuildBlastDatabase)
        emit finishedBuild("Build cancelled.");
    else
        emit finishedBuild("");
}
