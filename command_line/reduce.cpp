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


#include "reduce.h"
#include "commoncommandlinefunctions.h"
#include "../program/globals.h"
#include "../ui/mygraphicsscene.h"
#include "../ui/mygraphicsview.h"
#include "../graph/assemblygraph.h"
#include <vector>
#include "../program/settings.h"
#include <QPainter>
#include <QSvgGenerator>
#include <QDir>
#include "../blast/blastsearch.h"

int bandageReduce(QStringList arguments)
{
    QTextStream out(stdout);
    QTextStream err(stderr);

    if (checkForHelp(arguments))
    {
        printReduceUsage(&out, false);
        return 0;
    }

    if (checkForHelpAll(arguments))
    {
        printReduceUsage(&out, true);
        return 0;
    }

    if (arguments.size() < 2)
    {
        printReduceUsage(&err, false);
        return 1;
    }

    QString inputFilename = arguments.at(0);
    arguments.pop_front();

    if (!checkIfFileExists(inputFilename))
    {
        err << "Bandage error: " << inputFilename << " does not exist" << endl;
        return 1;
    }

    QString outputFilename = arguments.at(0);
    arguments.pop_front();
    if (!outputFilename.endsWith(".gfa"))
        outputFilename += ".gfa";

    QString error = checkForInvalidReduceOptions(arguments);
    if (error.length() > 0)
    {
        err << "Bandage error: " << error << endl;
        return 1;
    }

    bool loadSuccess = g_assemblyGraph->loadGraphFromFile(inputFilename);
    if (!loadSuccess)
    {
        err << "Bandage error: could not load " << inputFilename << endl;
        return 1;
    }

    parseSettings(arguments);

    bool blastUsed = isOptionPresent("--query", &arguments);
    if (blastUsed)
    {
        if (!createBlastTempDirectory())
        {
            err << "Error creating temporary directory for BLAST files" << endl;
            return 1;
        }

        QString blastError = g_blastSearch->doAutoBlastSearch();

        if (blastError != "")
        {
            err << blastError << endl;
            return 1;
        }
    }

    QString errorTitle;
    QString errorMessage;
    std::vector<DeBruijnNode *> startingNodes = g_assemblyGraph->getStartingNodes(&errorTitle, &errorMessage,
                                                                                  g_settings->doubleMode,
                                                                                  g_settings->startingNodes,
                                                                                  "all");

    if (errorMessage != "")
    {
        err << errorMessage << endl;
        return 1;
    }

    g_assemblyGraph->buildOgdfGraphFromNodesAndEdges(startingNodes, g_settings->nodeDistance);

    bool success = g_assemblyGraph->saveVisibleGraphToGfa(outputFilename);
    if (!success)
    {
        err << "Bandage was unable to save the graph file." << endl;
        return 1;
    }

    return 0;
}


void printReduceUsage(QTextStream * out, bool all)
{
    *out << endl;
    *out << "Bandage reduce takes an input graph and saves a reduced subgraph using the graph" << endl;
    *out << "scope settings. The saved graph will be in GFA format." << endl;
    *out << endl;
    *out << "If a graph scope is not specified, then the 'entire' scope will be used, in" << endl;
    *out << "which case this will simply convert the input graph to GFA format." << endl;
    *out << endl;
    *out << "Usage:    Bandage reduce <inputgraph> <outputgraph> [options]" << endl;
    *out << endl;
    *out << "Positional parameters:" << endl;
    *out << "          <inputgraph>        A graph file of any type supported by Bandage" << endl;
    *out << "          <outputgraph>       The filename for the GFA graph to be made (if it" << endl;
    *out << "                              does not end in '.gfa', that extension will be" << endl;
    *out << "                              added)" << endl;
    *out << endl;

    *out << "Options:  ";
    printCommonHelp(out, false);

    if (!all)
    {
        *out << "Settings: ";
        printGraphScopeOptions(out);
        *out << endl;
    }

    if (all)
        printSettingsUsage(out);
    *out << "Online Bandage help: https://github.com/rrwick/Bandage/wiki" << endl;
    *out << endl;
}



QString checkForInvalidReduceOptions(QStringList arguments)
{
    return checkForInvalidOrExcessSettings(&arguments);
}

