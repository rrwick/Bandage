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
#include <QCommandLineParser>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QApplication::setApplicationName("Bandage");
    QApplication::setApplicationVersion("0.6.0");

    QCommandLineParser parser;
    parser.setApplicationDescription("Bandage");
    parser.addHelpOption();
    parser.addVersionOption();

    parser.process(a);






    QString filename;
    MainWindow w(filename);
    w.show();

    return a.exec();
}
