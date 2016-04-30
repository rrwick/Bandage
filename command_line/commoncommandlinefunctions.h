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


#ifndef COMMANDCOMMANDLINEFUNCTIONS_H
#define COMMANDCOMMANDLINEFUNCTIONS_H

#include "../program/globals.h"
#include <QString>
#include <QStringList>
#include <QColor>
#include <QTextStream>
#include "../program/scinot.h"
#include <QDateTime>
#include <QStringList>
#include "../program/settings.h"

QStringList getArgumentList(int argc, char *argv[]);

bool checkForHelp(QStringList arguments);
bool checkForHelpAll(QStringList arguments);
bool checkForVersion(QStringList arguments);

QString checkOptionForInt(QString option, QStringList * arguments, IntSetting setting, bool offOkay);
QString checkOptionForFloat(QString option, QStringList * arguments, FloatSetting setting, bool offOkay);
QString checkOptionForSciNot(QString option, QStringList * arguments, SciNotSetting setting, bool offOkay);
QString checkOptionForString(QString option, QStringList * arguments,
                             QStringList validOptionsList,
                             QString validDescription = "");
QString checkOptionForColour(QString option, QStringList * arguments);
QString checkOptionForFile(QString option, QStringList * arguments);
bool checkIfFileExists(QString filename);
void checkOptionWithoutValue(QString option, QStringList * arguments);
QString checkTwoOptionsForFloats(QString option1, QString option2,
                                 QStringList * arguments,
                                 FloatSetting setting1,
                                 FloatSetting setting2,
                                 bool secondMustBeEqualOrLarger = false);

bool isOptionPresent(QString option, QStringList * arguments);
bool isOptionAndValuePresent(QString option, QString value,
                             QStringList * arguments);

int getIntOption(QString option, QStringList * arguments);
double getFloatOption(QString option, QStringList * arguments);
SciNot getSciNotOption(QString option, QStringList * arguments);
QColor getColourOption(QString option, QStringList * arguments);
NodeColourScheme getColourSchemeOption(QString option, QStringList * arguments);
GraphScope getGraphScopeOption(QString option, QStringList * arguments);
QString getStringOption(QString option, QStringList * arguments);

QString checkForInvalidOrExcessSettings(QStringList * arguments);
QString checkForExcessArguments(QStringList arguments);

void parseSettings(QStringList arguments);

void getCommonHelp(QStringList * text);
void getSettingsUsage(QStringList *text);

bool createBlastTempDirectory();
void deleteBlastTempDirectory();

QString getElapsedTime(QDateTime start, QDateTime end);

void getGraphScopeOptions(QStringList * text);

QStringList wrapText(QString text, int width, int firstLineIndent, int laterLineIndent);
QString rstrip(const QString& str);


QString getRangeAndDefault(IntSetting setting);
QString getRangeAndDefault(IntSetting setting, QString defaultVal);
QString getRangeAndDefault(FloatSetting setting);
QString getRangeAndDefault(FloatSetting setting, QString defaultVal);
QString getRangeAndDefault(SciNotSetting setting);
QString getRangeAndDefault(int min, int max, int defaultVal);
QString getRangeAndDefault(int min, int max, int defaultVal);
QString getRangeAndDefault(int min, int max, QString defaultVal);
QString getRangeAndDefault(double min, double max, double defaultVal);
QString getRangeAndDefault(double min, double max, QString defaultVal);
QString getRangeAndDefault(QString min, QString max, QString defaultVal);
QString getDefaultColour(QColor colour);

QString getBandageTitleAsciiArt();
bool isOption(QString text);
bool isSectionHeader(QString text);
bool isListItem(QString text);
bool isCommand(QString text);
bool isError(QString text);
void outputText(QString text, QTextStream * out);
void outputText(QStringList text, QTextStream * out);
void getOnlineHelpMessage(QStringList * text);

#endif // COMMANDCOMMANDLINEFUNCTIONS_H
