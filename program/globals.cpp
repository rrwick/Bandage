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
