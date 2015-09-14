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
#include <QProcess>
#include <math.h>

QSharedPointer<Settings> g_settings;
QSharedPointer<Memory> g_memory;
MyGraphicsView * g_graphicsView;
double g_absoluteZoom;
QSharedPointer<BlastSearch> g_blastSearch;
QString m_tempDirectory;
QSharedPointer<AssemblyGraph> g_assemblyGraph;


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

QString formatDoubleForDisplay(double num, int decimalPlacesToDisplay)
{
    //Add a bit for rounding
    double addValue = 0.5 / pow(10, decimalPlacesToDisplay);
    num += addValue;

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


bool parseSciNotation(QString numString, double * coefficient, int * exponent)
{
    QStringList parts = numString.split('e');
    if (parts.size() != 2)
        return false;

    bool ok;
    *coefficient = parts[0].toDouble(&ok);
    if (!ok)
        return false;

    *exponent = parts[1].toInt(&ok);
    if (!ok)
        return false;

    if (*coefficient < 0.0)
        return false;

    while (*coefficient > 10.0)
    {
        *coefficient /= 10.0;
        ++*exponent;
    }
    while (*coefficient < 1.0)
    {
        *coefficient *= 10.0;
        --*exponent;
    }

    return ok;
}

bool lessThan(QString a, QString b)
{
    double aC, bC;
    int aE, bE;
    parseSciNotation(a, &aC, &aE);
    parseSciNotation(b, &bC, &bE);
    return lessThan(aC, aE, bC, bE);
}

//This function compares two numbers in scientific notation.
//It returns true if a < b.
//It assumes the numbers are standardised (i.e. the coefficient is between 1.0
//and 10.0).
bool lessThan(double aC, int aE, double bC, double bE)
{
    if (aE < bE)
        return true;
    if (aE > bE)
        return false;
    return aC < bC;
}
