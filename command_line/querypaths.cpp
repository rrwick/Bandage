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


#include "querypaths.h"
#include "commoncommandlinefunctions.h"
#include "../program/settings.h"
#include "../graph/assemblygraph.h"
#include "../blast/blastsearch.h"

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
        err << "Bandage error: " << graphFilename << " does not exist" << endl;
        return 1;
    }

    QString queriesFilename = arguments.at(0);
    arguments.pop_front();
    if (!checkIfFileExists(queriesFilename))
    {
        err << "Bandage error: " << queriesFilename << " does not exist." << endl;
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
    QString tableFilename = outputPrefix + ".csv";
    QString pathFastaFilename = outputPrefix + "_paths.fasta";
    QString hitsFastaFilename = outputPrefix + "_hits.fasta";
    arguments.pop_front();

    QString error = checkForInvalidQueryPathsOptions(arguments);
    if (error.length() > 0)
    {
        err << "Bandage error: " << error << endl;
        return 1;
    }

    bool pathFasta = false;
    bool hitsFasta = false;
    parseQueryPathsOptions(arguments, &pathFasta, &hitsFasta);

    out << endl << "Loading graph...        ";
    bool loadSuccess = g_assemblyGraph->loadGraphFromFile(graphFilename);
    if (!loadSuccess)
        return 1;
    out << "done" << endl;

    if (!createBlastTempDirectory())
    {
        err << "Error creating temporary directory for BLAST files" << endl;
        return 1;
    }

    out << "Running BLAST search... ";
    QString blastError = g_blastSearch->doAutoBlastSearch();
    if (blastError != "")
    {
        err << endl << blastError << endl;
        return 1;
    }
    out << "done" << endl;


    out << "Saving results...       ";

    //TO DO: Write the query path output code here!!!
    //TO DO: Write the query path output code here!!!
    //TO DO: Write the query path output code here!!!
    //TO DO: Write the query path output code here!!!
    //TO DO: Write the query path output code here!!!
    //TO DO: Write the query path output code here!!!
    //TO DO: Write the query path output code here!!!
    //TO DO: Write the query path output code here!!!
    //TO DO: Write the query path output code here!!!
    //TO DO: Write the query path output code here!!!
    //TO DO: Write the query path output code here!!!
    //TO DO: Write the query path output code here!!!
    //TO DO: Write the query path output code here!!!
    //TO DO: Write the query path output code here!!!
    //TO DO: Write the query path output code here!!!
    //TO DO: Write the query path output code here!!!

    out << "done" << endl;


    out << endl << "Results: " + tableFilename << endl;
    if (pathFasta)
        out << "         " + pathFastaFilename << endl;
    if (hitsFasta)
        out << "         " + hitsFastaFilename << endl;

    out << endl << "Summary: Total BLAST queries:      " << g_blastSearch->m_blastQueries.getQueryCount() << endl;
    out << "         Queries with found paths: " << g_blastSearch->m_blastQueries.getQueryCountWithAtLeastOnePath() << endl;
    out << "         Total query paths:        " << g_blastSearch->m_blastQueries.getQueryPathCount() << endl;

    deleteBlastTempDirectory();
    return 0;
}


void printQueryPathsUsage(QTextStream * out, bool all)
{
    *out << endl;
    *out << "Usage:    Bandage querypaths <graph> <queries> <output_prefix> [options]" << endl;
    *out << endl;
    *out << "Options:  --pathfasta         put all query path sequences in a multi-FASTA" << endl;
    *out << "                              file, not in the CSV file" << endl;
    *out << "          --hitsfasta         produce a multi-FASTA file of all BLAST hits in" << endl;
    *out << "                              the query paths" << endl;
    *out << endl;
    printCommonHelp(out);
    if (all)
        printSettingsUsage(out);
    *out << "Online Bandage help: https://github.com/rrwick/Bandage/wiki" << endl;
    *out << endl;
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
