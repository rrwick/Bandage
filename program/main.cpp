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


#include "../ui/mainwindow.h"
#include <QApplication>
#include <QStringList>
#include <QString>
#include <QCommandLineParser>
#include <QTextStream>
#include "../command_line/load.h"
#include "../command_line/info.h"
#include "../command_line/image.h"
#include "../command_line/querypaths.h"
#include "../command_line/reduce.h"
#include "../command_line/contiguous.h"
#include "../command_line/commoncommandlinefunctions.h"
#include "../program/settings.h"
#include "../program/memory.h"
#include "../program/globals.h"
#include "../blast/blastsearch.h"
#include "../graph/assemblygraph.h"
#include "../ui/mygraphicsview.h"

#ifndef Q_OS_WIN32
#include <sys/ioctl.h>
#endif //Q_OS_WIN32

void printUsage(QTextStream * out, bool all)
{
    QStringList text;

    text << "";
    text << "Usage:    Bandage <command> [options]";
    text << "";
    text << "Commands: <blank>      Launch the Bandage GUI";
    text << "load         Launch the Bandage GUI and load a graph file";
    text << "info         Display information about a graph";
    text << "image        Generate an image file of a graph";
    text << "querypaths   Output graph paths for BLAST queries";
    text << "reduce       Save a subgraph of a larger graph";
//    text << "          contiguous   extract all sequences contiguous with a target sequence";
    text << "";
    text << "Options:  --help       View this help message";
    text << "--helpall    View all command line settings";
    text << "--version    View Bandage version number";
    text << "";

    if (all)
        getSettingsUsage(&text);
    getOnlineHelpMessage(&text);

    outputText(text, out);
}

int main(int argc, char *argv[])
{
    QStringList arguments = getArgumentList(argc, argv);

    QString first;
    if (arguments.size() > 0)
        first = arguments[0];

    //Create the application.
    bool guiNeeded = (first == "") || (first.toLower() == "load");
    if (!guiNeeded)
        qputenv("QT_QPA_PLATFORM", QByteArrayLiteral("minimal"));
    QApplication * a = new QApplication(argc, argv);

    //Create the important global objects.
    g_settings.reset(new Settings());
    g_memory.reset(new Memory());
    g_blastSearch.reset(new BlastSearch());
    g_assemblyGraph.reset(new AssemblyGraph());
    g_graphicsView = new MyGraphicsView();

    //Save the terminal width (useful for displaying help text neatly).
    #ifndef Q_OS_WIN32
    struct winsize ws;
    ioctl(0, TIOCGWINSZ, &ws);
    g_memory->terminalWidth = ws.ws_col;
    if (g_memory->terminalWidth < 50) g_memory->terminalWidth = 50;
    if (g_memory->terminalWidth > 300) g_memory->terminalWidth = 300;
    #endif //Q_OS_WIN32

    QApplication::setApplicationName("Bandage");
    QApplication::setApplicationVersion("0.7.2");

    QTextStream out(stdout);
    QTextStream err(stderr);

    //If the first argument was a recognised command, move to that command's function.
    if (arguments.size() > 0)
    {
        if (checkForVersion(arguments))
        {
            out << "Version: " << QApplication::applicationVersion() << endl;
            return 0;
        }

        if (first.toLower() == "load")
        {
            arguments.pop_front();
            g_memory->commandLineCommand = BANDAGE_LOAD;
            return bandageLoad(a, arguments);
        }
        else if (first.toLower() == "info")
        {
            arguments.pop_front();
            g_memory->commandLineCommand = BANDAGE_INFO;
            return bandageInfo(arguments);
        }
        else if (first.toLower() == "image")
        {
            arguments.pop_front();
            g_memory->commandLineCommand = BANDAGE_IMAGE;
            return bandageImage(arguments);
        }
        else if (first.toLower() == "querypaths")
        {
            arguments.pop_front();
            g_memory->commandLineCommand = BANDAGE_QUERY_PATHS;
            return bandageQueryPaths(arguments);
        }
        else if (first.toLower() == "reduce")
        {
            arguments.pop_front();
            g_memory->commandLineCommand = BANDAGE_REDUCE;
            return bandageReduce(arguments);
        }
//        else if (first == "contiguous")
//        {
//            arguments.pop_front();
//            return bandageContiguous(arguments);
//        }

        //Since a recognised command was not seen, we now check to see if the user
        //was looking for help information.
        else if (checkForHelp(arguments))
        {
            out << "" << endl;
            out << getBandageTitleAsciiArt() << endl;
            out << "Version: " << QApplication::applicationVersion() << endl;
            printUsage(&out, false);
            return 0;
        }
        else if (checkForHelpAll(arguments))
        {
            out << "" << endl;
            out << getBandageTitleAsciiArt() << endl;
            out << "Version: " << QApplication::applicationVersion() << endl;
            printUsage(&out, true);
            return 0;
        }
    }

    //If the code got here, we assume the user is simply launching Bandage,
    //with or without some options to specify settings.

    //Check the settings.
    QStringList argumentsCopy = arguments;
    QString error = checkForInvalidOrExcessSettings(&argumentsCopy);
    if (error.length() > 0)
    {
        outputText("Bandage error: " + error, &err);
        return 1;
    }

    //If the code got here, then the settings are good.  Parse them now and
    //run the program.
    parseSettings(arguments);
    MainWindow w;
    w.show();
    return a->exec();
}
