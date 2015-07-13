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
#include "../command_line/image.h"
#include "../command_line/contiguous.h"
#include "../command_line/commoncommandlinefunctions.h"
#include "../program/settings.h"

void printUsage(QTextStream * out, bool all)
{
    *out << endl;
    *out << "Usage:   Bandage <command> [options]" << endl;
    *out << endl;
    *out << "Command: <blank>      launch Bandage GUI" << endl;
    *out << "         load         launch Bandage GUI and load a graph file" << endl;
    *out << "         image        generate an image file of a graph" << endl;
//    *out << "         contiguous   extract all sequences contiguous with a target sequence" << endl;
    *out << endl;
    *out << "Options: --help       view this help message" << endl;
    *out << "         --helpall    view all command line settings" << endl;
    *out << "         --version    view Bandage version number" << endl;
    *out << endl;
    if (all)
        printSettingsUsage(out);
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QApplication::setApplicationName("Bandage");
    QApplication::setApplicationVersion("0.6.0");

    QTextStream out(stdout);
    QTextStream err(stdout);

    g_settings.reset(new Settings());

    QStringList arguments = QCoreApplication::arguments();
    arguments.pop_front();

    if (checkForVersion(arguments))
    {
        out << "Version: " << QApplication::applicationVersion() << endl;
        return 0;
    }

    //If the first argument was a recognised command, move to that command's function.
    if (arguments.size() > 0)
    {
        QString first = arguments.at(0);
        if (first == "load")
        {
            arguments.pop_front();
            return bandageLoad(&a, arguments);
        }
        else if (first == "image")
        {
            arguments.pop_front();
            return bandageImage(arguments);
        }
//        else if (first == "contiguous")
//        {
//            arguments.pop_front();
//            return bandageContiguous(arguments);
//        }
    }

    //Since a recognised command was not seen, we now check to see if the user
    //was looking for help or version information.
    if (checkForHelp(arguments))
    {
        out << "" << endl;
        out << "Program: Bandage" << endl;
        out << "Version: " << QApplication::applicationVersion() << endl;
        printUsage(&out, false);
        return 0;
    }
    if (checkForHelpAll(arguments))
    {
        out << "" << endl;
        out << "Program: Bandage" << endl;
        out << "Version: " << QApplication::applicationVersion() << endl;
        printUsage(&out, true);
        return 0;
    }

    //If the code got here, we assume the user is simply launching Bandage,
    //with or without some options to specify settings.

    //Check the settings.
    QStringList argumentsCopy = arguments;
    QString error = checkForInvalidOrExcessSettings(&argumentsCopy);
    if (error.length() > 0)
    {
        err << "Bandage error: " + error << endl;
        return 1;
    }

    //If the code got here, then the settings are good.  Parse them now and
    //run the program.
    parseSettings(arguments);
    MainWindow w;
    w.show();
    return a.exec();
}
