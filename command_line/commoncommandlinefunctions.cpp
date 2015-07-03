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


#include "commoncommandlinefunctions.h"
#include "../graph/assemblygraph.h"
#include <ogdf/energybased/FMMMLayout.h>
#include "../program/graphlayoutworker.h"
#include "../program/settings.h"

bool loadAssemblyGraph(QString filename)
{
    g_assemblyGraph = new AssemblyGraph();
    return g_assemblyGraph->loadGraphFromFile(filename);
}



//Unlike the equivalent function in MainWindow, this does the graph layout in the main thread.
void layoutGraph()
{
    ogdf::FMMMLayout fmmm;
    GraphLayoutWorker * graphLayoutWorker = new GraphLayoutWorker(&fmmm, g_assemblyGraph->m_graphAttributes,
                                                                  g_settings->graphLayoutQuality, g_settings->segmentLength);
    graphLayoutWorker->layoutGraph();
}

bool checkForHelp(QStringList arguments)
{
    int h1 = arguments.indexOf("-h");
    int h2 = arguments.indexOf("-help");
    int h3 = arguments.indexOf("--help");

    return (h1 != -1 || h2 != -1 || h3 != -1);
}



//Returns empty string if everything is okay and an error
//message if there's a problem.  If everything is okay, it
//also removes the option and its value from arguments.
QString checkOptionForInt(QString option, QStringList * arguments, int min, int max)
{
    int optionIndex = arguments->indexOf(option);

    //If the option isn't found, that's fine.
    if (optionIndex == -1)
        return "";

    int integerIndex = optionIndex + 1;

    //If nothing follows the option, that's a problem.
    if (integerIndex >= arguments->size())
        return option + " must be followed by an integer";

    //If the thing following the option isn't an integer, that's a problem.
    bool optionIsInt;
    int optionInt = arguments->at(integerIndex).toInt(&optionIsInt);
    if (!optionIsInt)
        return option + " must be followed by an integer";

    //Check the range of the option.
    if (optionInt < min || optionInt > max)
        return "Value of " + option + " must be between "
                + QString::number(min) + " and " + QString::number(max) +
                " (inclusive)";

    //If the code got here, the option and its integer are okay.
    //Remove them from the arguments.
    arguments->removeAt(integerIndex);
    arguments->removeAt(optionIndex);

    return "";
}


//Returns empty string if everything is okay and an error
//message if there's a problem.  If everything is okay, it
//also removes the option and its value from arguments.
QString checkOptionForFloat(QString option, QStringList * arguments, double min, double max)
{
    int optionIndex = arguments->indexOf(option);

    //If the option isn't found, that's fine.
    if (optionIndex == -1)
        return "";

    int floatIndex = optionIndex + 1;

    //If nothing follows the option, that's a problem.
    if (floatIndex >= arguments->size())
        return option + " must be followed by a number";

    //If the thing following the option isn't a number, that's a problem.
    bool optionIsFloat;
    double optionFloat = arguments->at(floatIndex).toDouble(&optionIsFloat);
    if (!optionIsFloat)
        return option + " must be followed by a number";

    //Check the range of the option.
    if (optionFloat < min || optionFloat > max)
        return "Value of " + option + " must be between "
                + QString::number(min) + " and " + QString::number(max) +
                " (inclusive)";

    //If the code got here, the option and its number are okay.
    //Remove them from the arguments.
    arguments->removeAt(floatIndex);
    arguments->removeAt(optionIndex);

    return "";
}


//Returns empty string if everything is okay and an error
//message if there's a problem.  If everything is okay, it
//also removes the option and its value from arguments.
QString checkOptionForString(QString option, QStringList * arguments, QStringList validOptionsList)
{
    int optionIndex = arguments->indexOf(option);

    //If the option isn't found, that's fine.
    if (optionIndex == -1)
        return "";

    int stringIndex = optionIndex + 1;

    QString validOptions;
    for (int i = 0; i < validOptionsList.size(); ++i)
    {
        validOptions += validOptionsList.at(i);
        if (i == validOptionsList.size() - 2)
            validOptions += " or ";
        else if (i < validOptionsList.size() - 2)
            validOptions += ", ";
    }

    //If nothing follows the option, that's a problem.
    if (stringIndex >= arguments->size())
        return option + " must be followed by " + validOptions;

    //If the thing following the option isn't a valid choice, that's a problem.
    QString value = arguments->at(stringIndex);
    if (!validOptionsList.contains(value, Qt::CaseInsensitive))
        return option + " must be followed by " + validOptions;

    //If the code got here, the option and its string are okay.
    //Remove them from the arguments.
    arguments->removeAt(stringIndex);
    arguments->removeAt(optionIndex);

    return "";
}


QString checkOptionForHexColour(QString option, QStringList * arguments)
{
    int optionIndex = arguments->indexOf(option);

    //If the option isn't found, that's fine.
    if (optionIndex == -1)
        return "";

    int hexIndex = optionIndex + 1;

    //If nothing follows the option, that's a problem.
    if (hexIndex >= arguments->size())
        return option + " must be followed by a 6 character HTML-style hex colour";

    //If the thing following the option isn't a hex colour, that's a problem.
    QString hexName = "#" + arguments->at(hexIndex);
    QColor colour(hexName);
    if (!colour.isValid())
        return option + " must be followed by a 6 character HTML-style hex colour";

    //If the code got here, the option and its hex colour are okay.
    //Remove them from the arguments.
    arguments->removeAt(hexIndex);
    arguments->removeAt(optionIndex);

    return "";
}


//This function simply removes an option from arguments if it is found.
void checkOptionWithoutValue(QString option, QStringList * arguments)
{
    int optionIndex = arguments->indexOf(option);

    //If the option isn't found, that's fine.
    if (optionIndex == -1)
        return ;

    //If the code got here, the option is okay.
    //Remove it from the arguments.
    arguments->removeAt(optionIndex);
}


//This function checks to make sure either both or neither of the options
//are used.  It can also optionally check to make sure the second is larger
//than the first.
QString checkTwoOptionsForFloats(QString option1, QString option2, QStringList * arguments,
                                 double min1, double max1, double min2, double max2,
                                 bool secondMustBeEqualOrLarger)
{
    //First check each option independently
    QStringList argumentsCopy = *arguments;
    QString option1Error = checkOptionForFloat(option1, &argumentsCopy, min1, max1);
    if (option1Error != "")
        return option1Error;
    QString option2Error = checkOptionForFloat(option2, &argumentsCopy, min2, max2);
    if (option2Error != "")
        return option2Error;

    //Now make sure either both or neither are present.
    if (isOptionPresent(option1, arguments) != isOptionPresent(option2, arguments))
        return option1 + " and " + option2 + " must be used together";

    if (secondMustBeEqualOrLarger)
    {
        if (getFloatOption(option2, arguments) < getFloatOption(option1, arguments))
            return option2 + " must be greater than or equal to " + option1;
    }

    //Now remove the options from the arguments before finishing.
    checkOptionForFloat(option1, arguments, min1, max1);
    checkOptionForFloat(option2, arguments, min2, max2);
    return "";
}



bool isOptionPresent(QString option, QStringList * arguments)
{
    return (arguments->indexOf(option) > -1);
}



int getIntOption(QString option, QStringList * arguments)
{
    int optionIndex = arguments->indexOf(option);
    if (optionIndex == -1)
        return 0;

    int integerIndex = optionIndex + 1;
    if (integerIndex >= arguments->size())
        return 0;

    return arguments->at(integerIndex).toInt();
}

double getFloatOption(QString option, QStringList * arguments)
{
     int optionIndex = arguments->indexOf(option);
     if (optionIndex == -1)
         return 0;

     int floatIndex = optionIndex + 1;
     if (floatIndex >= arguments->size())
         return 0;

     return arguments->at(floatIndex).toDouble();
}

NodeColourScheme getColourSchemeOption(QString option, QStringList * arguments)
{
    int optionIndex = arguments->indexOf(option);
    if (optionIndex == -1)
        return RANDOM_COLOURS;

    int colourIndex = optionIndex + 1;
    if (colourIndex >= arguments->size())
        return RANDOM_COLOURS;

    QString colourString = arguments->at(colourIndex).toLower();
    if (colourString == "random")
        return RANDOM_COLOURS;
    else if (colourString == "uniform")
        return ONE_COLOUR;
    else if (colourString == "coverage")
        return COVERAGE_COLOUR;

    //Random colours is the default
    return RANDOM_COLOURS;
}


QColor getHexColourOption(QString option, QStringList * arguments)
{
    int optionIndex = arguments->indexOf(option);
    if (optionIndex == -1)
        return QColor();

    int hexIndex = optionIndex + 1;
    if (hexIndex >= arguments->size())
        return QColor();

    return QColor("#" + arguments->at(hexIndex));
}





//This function generates an error if excess arguments are left after
//parsing.
QString checkForExcessArguments(QStringList arguments)
{
    if (arguments.size() == 0)
        return "";

    QString invalidOptionText = "Invalid option";
    if (arguments.size() > 1)
        invalidOptionText += "s";
    invalidOptionText += ": ";
    for (int i = 0; i < arguments.size(); ++i)
    {
        invalidOptionText += arguments.at(i);
        if (i < arguments.size() - 1)
            invalidOptionText += ", ";
    }

    return invalidOptionText;
}
