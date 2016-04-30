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


#include "querypaths.h"
#include "commoncommandlinefunctions.h"
#include "../program/settings.h"
#include "../graph/assemblygraph.h"
#include "../blast/blastsearch.h"
#include <QDateTime>

int bandageQueryPaths(QStringList arguments)
{
    QTextStream out(stdout);
    QTextStream err(stderr);

    if (checkForHelp(arguments))
    {
        printQueryPathsUsage(&out, false);
        return 0;
    }

    if (checkForHelpAll(arguments))
    {
        printQueryPathsUsage(&out, true);
        return 0;
    }

    if (arguments.size() < 3)
    {
        printQueryPathsUsage(&err, false);
        return 1;
    }

    QString graphFilename = arguments.at(0);
    arguments.pop_front();
    if (!checkIfFileExists(graphFilename))
    {
        outputText("Bandage error: " + graphFilename, &err);
        return 1;
    }

    QString queriesFilename = arguments.at(0);
    arguments.pop_front();
    if (!checkIfFileExists(queriesFilename))
    {
        outputText("Bandage error: " + queriesFilename, &err);
        return 1;
    }
    g_settings->blastQueryFilename = queriesFilename;

    //Ensure that the --query option isn't used, as that would overwrite the
    //queries file that is a positional argument.
    if (isOptionPresent("--query", &arguments))
    {
        err << "Bandage error: the --query option cannot be used with Bandage querypaths." << endl;
        return 1;
    }

    QString outputPrefix = arguments.at(0);
    QString tableFilename = outputPrefix + ".tsv";
    QString pathFastaFilename = outputPrefix + "_paths.fasta";
    QString hitsFastaFilename = outputPrefix + "_hits.fasta";
    arguments.pop_front();

    QString error = checkForInvalidQueryPathsOptions(arguments);
    if (error.length() > 0)
    {
        outputText("Bandage error: " + error, &err);
        return 1;
    }

    bool pathFasta = false;
    bool hitsFasta = false;
    parseQueryPathsOptions(arguments, &pathFasta, &hitsFasta);

    //Check to make sure the output files don't already exist.
    QFile tableFile(tableFilename);
    QFile pathsFile(pathFastaFilename);
    QFile hitsFile(hitsFastaFilename);
    if (tableFile.exists())
    {
        outputText("Bandage error: " + tableFilename + " already exists.", &err);
        return 1;
    }
    if (pathFasta && pathsFile.exists())
    {
        outputText("Bandage error: " + pathFastaFilename + " already exists.", &err);
        return 1;
    }
    if (hitsFasta && hitsFile.exists())
    {
        outputText("Bandage error: " + hitsFastaFilename + " already exists.", &err);
        return 1;
    }

    QDateTime startTime = QDateTime::currentDateTime();

    out << endl << "(" << QDateTime::currentDateTime().toString("dd MMM yyyy hh:mm:ss") << ") Loading graph...        " << flush;

    bool loadSuccess = g_assemblyGraph->loadGraphFromFile(graphFilename);
    if (!loadSuccess)
        return 1;
    out << "done" << endl;

    if (!createBlastTempDirectory())
    {
        err << "Error creating temporary directory for BLAST files" << endl;
        return 1;
    }

    out << "(" << QDateTime::currentDateTime().toString("dd MMM yyyy hh:mm:ss") << ") Running BLAST search... " << flush;
    QString blastError = g_blastSearch->doAutoBlastSearch();
    if (blastError != "")
    {
        err << endl << blastError << endl;
        return 1;
    }
    out << "done" << endl;
    out << "(" << QDateTime::currentDateTime().toString("dd MMM yyyy hh:mm:ss") << ") Saving results...       " << flush;

    //Create the table file.
    tableFile.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream tableOut(&tableFile);

    //Write the TSV header line.
    tableOut << "Query\t"
                "Path\t"
                "Length\t"
                "Query covered by path\t"
                "Query covered by hits\t"
                "Mean hit identity\t"
                "Total hit mismatches\t"
                "Total hit gap opens\t"
                "Relative length\t"
                "Length discrepancy\t"
                "E-value product\t";

    //If the user asked for a separate path sequence file, then the last column
    //will be a reference to that sequence ID.  If not, the sequence will go in
    //the table.
    if (pathFasta)
        tableOut << "Sequence ID\n";
    else
        tableOut << "Sequence\n";

    //If a path sequence FASTA file is used, these will store the sequences
    //that will go there.
    QList<QString> pathSequenceIDs;
    QList<QByteArray> pathSequences;

    //If a hits sequence FASTA file is used, these will store the sequences
    //that will go there.
    QList<QString> hitSequenceIDs;
    QList<QByteArray> hitSequences;

    for (size_t i = 0; i < g_blastSearch->m_blastQueries.m_queries.size(); ++i)
    {
        BlastQuery * query = g_blastSearch->m_blastQueries.m_queries[i];
        QList<BlastQueryPath> queryPaths = query->getPaths();

        for (int j = 0; j < queryPaths.size(); ++j)
        {
            BlastQueryPath queryPath = queryPaths[j];
            Path path = queryPath.getPath();

            tableOut << query->getName() << "\t";
            tableOut << path.getString(true) << "\t";
            tableOut << QString::number(path.getLength()) << "\t";
            tableOut << QString::number(100.0 * queryPath.getPathQueryCoverage()) << "%\t";
            tableOut << QString::number(100.0 * queryPath.getHitsQueryCoverage()) << "%\t";
            tableOut << QString::number(queryPath.getMeanHitPercIdentity()) << "%\t";
            tableOut << QString::number(queryPath.getTotalHitMismatches()) << "\t";
            tableOut << QString::number(queryPath.getTotalHitGapOpens()) << "\t";
            tableOut << QString::number(100.0 * queryPath.getRelativePathLength()) << "%\t";
            tableOut << queryPath.getAbsolutePathLengthDifferenceString(false) << "\t";
            tableOut << queryPath.getEvalueProduct().asString(false) << "\t";

            //If we are using a separate file for the path sequences, save the
            //sequence along with its ID to save later, and store the ID here.
            //Otherwise, just include the sequence in this table.
            QByteArray sequence = path.getPathSequence();
            QString pathSequenceID = query->getName() + "_" + QString::number(j+1);
            if (pathFasta)
            {
                pathSequenceIDs.push_back(pathSequenceID);
                pathSequences.push_back(sequence);
                tableOut << pathSequenceID << "\n";
            }
            else
                tableOut << sequence << "\n";

            //If we are also saving the hit sequences, save the hit sequence
            //along with its ID to save later.
            if (hitsFasta)
            {
                QList<BlastHit *> hits = queryPath.getHits();
                for (int k = 0; k < hits.size(); ++k)
                {
                    BlastHit * hit = hits[k];
                    QString hitSequenceID = pathSequenceID + "_" + QString::number(k+1);
                    QByteArray hitSequence = hit->getNodeSequence();
                    hitSequenceIDs.push_back(hitSequenceID);
                    hitSequences.push_back(hitSequence);
                }
            }
        }
    }

    //Write the path sequence FASTA file, if appropriate.
    if (pathFasta)
    {
        pathsFile.open(QIODevice::WriteOnly | QIODevice::Text);
        QTextStream pathsOut(&pathsFile);

        for (int i = 0; i < pathSequenceIDs.size(); ++i)
        {
            pathsOut << ">" + pathSequenceIDs[i] + "\n";
            pathsOut << AssemblyGraph::addNewlinesToSequence(pathSequences[i]);
        }
    }

    //Write the hits sequence FASTA file, if appropriate.
    if (hitsFasta)
    {
        hitsFile.open(QIODevice::WriteOnly | QIODevice::Text);
        QTextStream hitsOut(&hitsFile);

        for (int i = 0; i < hitSequenceIDs.size(); ++i)
        {
            hitsOut << ">" << hitSequenceIDs[i] << "\n";
            hitsOut << AssemblyGraph::addNewlinesToSequence(hitSequences[i]);
        }
    }

    out << "done" << endl;

    out << endl << "Results:      " + tableFilename << endl;
    if (pathFasta)
        out << "              " + pathFastaFilename << endl;
    if (hitsFasta)
        out << "              " + hitsFastaFilename << endl;

    out << endl << "Summary: Total BLAST queries:           " << g_blastSearch->m_blastQueries.getQueryCount() << endl;
    out << "         Queries with found paths:      " << g_blastSearch->m_blastQueries.getQueryCountWithAtLeastOnePath() << endl;
    out << "         Total query paths:             " << g_blastSearch->m_blastQueries.getQueryPathCount() << endl;

    out << endl << "Elapsed time: " << getElapsedTime(startTime, QDateTime::currentDateTime()) << endl;

    deleteBlastTempDirectory();
    return 0;
}


void printQueryPathsUsage(QTextStream * out, bool all)
{
    QStringList text;

    text << "Bandage querypaths searches for queries in the graph using BLAST and outputs the results to a tab-delimited file.";
    text << "";
    text << "Usage:    Bandage querypaths <graph> <queries> <output_prefix> [options]";
    text << "";
    text << "Positional parameters:";
    text << "<graph>             A graph file of any type supported by Bandage";
    text << "<queries>           A FASTA file of one or more BLAST queries";
    text << "<output_prefix>     The output file prefix (used to create the '.tsv' output file, and possibly FASTA files as well, depending on options)";
    text << "";
    text << "Options:  --pathfasta         Put all query path sequences in a multi-FASTA file, not in the TSV file";
    text << "--hitsfasta         Produce a multi-FASTA file of all BLAST hits in the query paths";
    text << "";

    getCommonHelp(&text);
    if (all)
        getSettingsUsage(&text);
    getOnlineHelpMessage(&text);

    outputText(text, out);
}



QString checkForInvalidQueryPathsOptions(QStringList arguments)
{
    checkOptionWithoutValue("--pathfasta", &arguments);
    checkOptionWithoutValue("--hitsfasta", &arguments);

    QString error = checkForInvalidOrExcessSettings(&arguments);
    if (error.length() > 0) return error;

    return checkForInvalidOrExcessSettings(&arguments);
}



void parseQueryPathsOptions(QStringList arguments, bool * pathFasta,
                            bool * hitsFasta)
{
    int pathFastaIndex = arguments.indexOf("--pathfasta");
    *pathFasta = (pathFastaIndex > -1);

    int hitsFastaIndex = arguments.indexOf("--hitsfasta");
    *hitsFasta = (hitsFastaIndex > -1);

    parseSettings(arguments);
}
