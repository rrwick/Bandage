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


#include "globals.h"
#include <QLocale>
#include <QDir>
#include <QStringList>
#include <QFile>
#include <QTextStream>
#include <QApplication>

Settings * g_settings;
MyGraphicsView * g_graphicsView;
double g_absoluteZoom;
int g_randomColourFactor;
BlastSearch * g_blastSearch;
QString g_tempDirectory;
AssemblyGraph * g_assemblyGraph;


QString formatIntForDisplay(int num)
{
    QLocale locale;
    return locale.toString(num);
}

QString formatIntForDisplay(long long num)
{
    QLocale locale;
    return locale.toString(num);
}

QString formatDoubleForDisplay(double num, double decimalPlacesToDisplay)
{
    QLocale locale;
    QString withCommas = locale.toString(num, 'f');

    QString final;
    bool pastDecimalPoint = false;
    int numbersPastDecimalPoint = 0;
    for (int i = 0; i < withCommas.length(); ++i)
    {
        final += withCommas[i];

        if (pastDecimalPoint)
            ++numbersPastDecimalPoint;

        if (numbersPastDecimalPoint >= decimalPlacesToDisplay)
            return final;

        if (withCommas[i] == locale.decimalPoint())
            pastDecimalPoint = true;
    }
    return final;
}


void emptyTempDirectory()
{
    QDir tempDirectory(g_tempDirectory);
    tempDirectory.setNameFilters(QStringList() << "*.*");
    tempDirectory.setFilter(QDir::Files);
    foreach(QString dirFile, tempDirectory.entryList())
        tempDirectory.remove(dirFile);
}


void readFastaFile(QString filename, std::vector<QString> * names, std::vector<QString> * sequences)
{
    QFile inputFile(filename);
    if (inputFile.open(QIODevice::ReadOnly))
    {
        QString name = "";
        QString sequence = "";

        QTextStream in(&inputFile);
        while (!in.atEnd())
        {
            QApplication::processEvents();

            QString line = in.readLine();

            if (line.length() == 0)
                continue;

            if (line.at(0) == '>')
            {
                //If there is a current sequence, add it to the vectors now.
                if (name.length() > 0)
                {
                    names->push_back(name);
                    sequences->push_back(sequence);
                }

                line.remove(0, 1); //Remove '>' from start
                name = line;
                sequence = "";
            }

            else //It's a sequence line
                sequence += line.simplified();
        }

        //Add the last target to the results now.
        if (name.length() > 0)
        {
            names->push_back(name);
            sequences->push_back(sequence);
        }

        inputFile.close();
    }
}



//Trinity nodes are only unique within a component.  It is therefore necessary to generate
//a unique node number using the transcript, component and node numbers.  These functions
//turn those parts into the full unique node number and back.
long long getFullTrinityNodeNumberFromParts(int transcript, int component, long long node)
{
    bool negative = node < 0;
    node = llabs(node);

    long long transcriptPart = 10000000000000LL * transcript;
    long long componentPart = 10000000000LL * component;
    long long fullNodeNumber = transcriptPart + componentPart + node;

    if (negative)
        fullNodeNumber *= -1LL;

    return fullNodeNumber;
}

long long getFullTrinityNodeNumberFromName(QString name)
{
    QStringList nameParts = name.split("-");

    if (nameParts.size() < 3)
        return 0;

    int transcript = nameParts[0].toInt();
    int component = nameParts[1].toInt();
    long long nodeNumber = nameParts[2].toLongLong();

    return getFullTrinityNodeNumberFromParts(transcript, component, nodeNumber);
}

void getTrinityPartsFromFullNodeNumber(long long fullNodeNumber, int * transcript, int * component, long long * node)
{
    bool negative = fullNodeNumber < 0;
    fullNodeNumber = llabs(fullNodeNumber);

    *transcript = fullNodeNumber / 10000000000000LL;
    long long remainder = fullNodeNumber % 10000000000000LL;

    *component = remainder / 10000000000LL;

    long long nodeNumber = remainder % 10000000000LL;
    if (negative)
        nodeNumber *= -1LL;

    *node = nodeNumber;
}


QString getTrinityNodeNameFromFullNodeNumber(long long fullNodeNumber)
{
    int transcript;
    int component;
    long long node;
    getTrinityPartsFromFullNodeNumber(fullNodeNumber, &transcript, &component, &node);
    QString numberText = QString::number(transcript) + "-" +
            QString::number(component) + "-" + QString::number(node);
    return numberText;
}
