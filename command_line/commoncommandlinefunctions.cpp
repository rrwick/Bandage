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

bool checkForHelpAll(QStringList arguments)
{
    return (arguments.indexOf("--helpall") != -1);
}

bool checkForVersion(QStringList arguments)
{
    int v1 = arguments.indexOf("-v");
    int v2 = arguments.indexOf("-version");
    int v3 = arguments.indexOf("--version");

    return (v1 != -1 || v2 != -1 || v3 != -1);
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


QString checkOptionForColour(QString option, QStringList * arguments)
{
    int optionIndex = arguments->indexOf(option);

    //If the option isn't found, that's fine.
    if (optionIndex == -1)
        return "";

    int colIndex = optionIndex + 1;

    //If nothing follows the option, that's a problem.
    if (colIndex >= arguments->size())
        return option + " must be followed by a 6-digit hex colour (e.g. #FFB6C1), an 8-digit hex colour (e.g. #F0D2B48C) or a standard colour name (e.g. skyblue)";

    //If the thing following the option isn't a colour, that's a problem.
    QColor colour(arguments->at(colIndex));
    if (!colour.isValid())
        return option + " must be followed by a 6-digit hex colour (e.g. #FFB6C1), an 8-digit hex colour (e.g. #F0D2B48C) or a standard colour name (e.g. skyblue)";

    //If the code got here, the option and its colour are okay.
    //Remove them from the arguments.
    arguments->removeAt(colIndex);
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


QColor getColourOption(QString option, QStringList * arguments)
{
    int optionIndex = arguments->indexOf(option);
    if (optionIndex == -1)
        return QColor();

    int colIndex = optionIndex + 1;
    if (colIndex >= arguments->size())
        return QColor();

    return QColor(arguments->at(colIndex));
}



//This function checks the values of the Bandage settings.
//If everything is fine, it removes the good arguments/values and returns
//a null string.  If there's a problem, it returns an error message.
QString checkForInvalidOrExcessSettings(QStringList * arguments)
{
    checkOptionWithoutValue("--double", arguments);
    QString error = checkOptionForInt("--bases", arguments, 1, std::numeric_limits<int>::max());
    if (error.length() > 0) return error;
    error = checkOptionForInt("--quality", arguments, 1, 5);
    if (error.length() > 0) return error;

    error = checkOptionForFloat("--nodewidth", arguments, 0.5, 1000);
    if (error.length() > 0) return error;
    error = checkOptionForFloat("--covwidth", arguments, 0.0, 1.0);
    if (error.length() > 0) return error;
    error = checkOptionForFloat("--covpower", arguments, 0.1, 1.0);
    if (error.length() > 0) return error;

    error = checkOptionForFloat("--edgewidth", arguments, 0.1, 1000.0);
    if (error.length() > 0) return error;
    error = checkOptionForFloat("--outline", arguments, 0.0, 1000.0);
    if (error.length() > 0) return error;

    checkOptionWithoutValue("--names", arguments);
    checkOptionWithoutValue("--lengths", arguments);
    checkOptionWithoutValue("--coverages", arguments);
    error = checkOptionForInt("--fontsize", arguments, 1, 100);
    if (error.length() > 0) return error;
    error = checkOptionForFloat("--toutline", arguments, 0.0, 2.0);
    if (error.length() > 0) return error;
    checkOptionWithoutValue("--centre", arguments);

    error = checkOptionForColour("--edgecol", arguments);
    if (error.length() > 0) return error;
    error = checkOptionForColour("--outcol", arguments);
    if (error.length() > 0) return error;
    error = checkOptionForColour("--selcol", arguments);
    if (error.length() > 0) return error;
    error = checkOptionForColour("--textcol", arguments);
    if (error.length() > 0) return error;
    error = checkOptionForColour("--toutcol", arguments);
    if (error.length() > 0) return error;
    checkOptionWithoutValue("--noaa", arguments);

    QStringList validColourOptions;
    validColourOptions << "random" << "uniform" << "coverage";
    error = checkOptionForString("--colour", arguments, validColourOptions);
    if (error.length() > 0) return error;

    error = checkOptionForInt("--ransatpos", arguments, 0, 255);
    if (error.length() > 0) return error;
    error = checkOptionForInt("--ransatneg", arguments, 0, 255);
    if (error.length() > 0) return error;
    error = checkOptionForInt("--ranligpos", arguments, 0, 255);
    if (error.length() > 0) return error;
    error = checkOptionForInt("--ranligneg", arguments, 0, 255);
    if (error.length() > 0) return error;
    error = checkOptionForInt("--ranopapos", arguments, 0, 255);
    if (error.length() > 0) return error;
    error = checkOptionForInt("--ranopaneg", arguments, 0, 255);
    if (error.length() > 0) return error;

    error = checkOptionForColour("--unicolpos", arguments);
    if (error.length() > 0) return error;
    error = checkOptionForColour("--unicolneg", arguments);
    if (error.length() > 0) return error;
    error = checkOptionForColour("--unicolspe", arguments);
    if (error.length() > 0) return error;

    error = checkOptionForColour("--covcollow", arguments);
    if (error.length() > 0) return error;
    error = checkOptionForColour("--covcolhi", arguments);
    if (error.length() > 0) return error;

    error = checkTwoOptionsForFloats("--covvallow", "--covvalhi", arguments, 0.0, 1000000.0, 0.0, 1000000.0, true);
    if (error.length() > 0) return error;

    return checkForExcessArguments(*arguments);
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


void printCommonHelp(QTextStream * out)
{
    *out << "          --help              view this help message" << endl;
    *out << "          --helpall           view all command line settings" << endl;
    *out << endl;
}

void printSettingsUsage(QTextStream * out)
{
    *out << "Settings: The following options configure the Bandage settings that are" << endl;
    *out << "          available in the Bandage GUI." << endl;
    *out << endl;
    *out << "          Colours can be specified using hex values, with or without an alpha " << endl;
    *out << "          channel, (e.g. #FFB6C1 or #7FD2B48C) or using standard colour names" << endl;
    *out << "          (e.g. red, yellowgreen or skyblue).  Note that hex colour names will" << endl;
    *out << "          either need to be enclosed in quotes (e.g. \"#FFB6C1\") or have the" << endl;
    *out << "          hash symbol escaped (e.g. \\#FFB6C1)." << endl;
    *out << endl;
    *out << "          Graph layout" << endl;
    *out << "          ---------------------------------------------------------------------" << endl;
    *out << "          --double            draw graph in double mode (default: off)" << endl;
    *out << "          --bases <int>       base pairs per segment (default: auto)" << endl;
    *out << "                              High values result in longer nodes, small values" << endl;
    *out << "                              in shorter nodes." << endl;
    *out << "          --quality <int>     graph layout quality, 0 (low) to 4 (high)" << endl;
    *out << "                              (default: " + QString::number(g_settings->graphLayoutQuality) + ")" << endl;
    *out << endl;
    *out << "          Node width" << endl;
    *out << "          ---------------------------------------------------------------------" << endl;
    *out << "          Node widths are determined using the following formula:" << endl;
    *out << "              a*b*((c/d)^e-1)+1" << endl;
    *out << "                 a = average node width, b = coverage effect on width" << endl;
    *out << "                 c = node coverage, d = mean coverage" << endl;
    *out << "                 e = power of coverage effect on width" << endl;
    *out << "          --nodewidth <float> Average node width (0.5 to 1000, default: " + QString::number(g_settings->averageNodeWidth) + ")" << endl;
    *out << "          --covwidth <float>  Coverage effect on width (0 to 1, default: " + QString::number(g_settings->coverageEffectOnWidth) + ")" << endl;
    *out << "          --covpower <float>  Power of coverage effect on width (0.1 to 1," << endl;
    *out << "                              default: " + QString::number(g_settings->coveragePower) + ")" << endl;
    *out << endl;
    *out << "          Node labels" << endl;
    *out << "          ---------------------------------------------------------------------" << endl;
    *out << "          --names             Label nodes with name (default: off)" << endl;
    *out << "          --lengths           Label nodes with length (default: off)" << endl;
    *out << "          --coverages         Label nodes with coverage (default: off)" << endl;
    *out << "          --fontsize <int>    Font size for node labels (1 to 100, default: " + QString::number(g_settings->labelFont.pointSize()) + ")" << endl;
    *out << endl;
    *out << "          Graph appearance" << endl;
    *out << "          ---------------------------------------------------------------------" << endl;
    *out << "          --edgecol <col>     Colour for edges (default: " + getColourName(g_settings->edgeColour.name()) + ")" << endl;
    *out << "          --edgewidth <float> Edge width (0.1 to 1000, default: " + QString::number(g_settings->edgeWidth) + ")" << endl;
    *out << "          --outcol <col>      Colour for node outlines (default: " + getColourName(g_settings->outlineColour.name()) + ")" << endl;
    *out << "          --outline <float>   Node outline thickness (0 to 1000, default: " + QString::number(g_settings->outlineThickness) + ") " << endl;
    *out << "          --selcol <col>      Colour for selections (default: " + getColourName(g_settings->selectionColour.name()) + ")" << endl;
    *out << "          --noaa              Disable antialiasing (default: antialiasing on)" << endl;
    *out << endl;
    *out << "          Text appearance" << endl;
    *out << "          ---------------------------------------------------------------------" << endl;
    *out << "          --textcol <col>     Colour for label text (default: " + getColourName(g_settings->textColour.name()) + ")" << endl;
    *out << "          --toutcol <col>     Colour for text outline (default: " + getColourName(g_settings->textOutlineColour.name()) + ")" << endl;
    *out << "          --toutline <float>  Surround text with an outline with this" << endl;
    *out << "                              thickness (default: " + QString::number(g_settings->textOutlineThickness) + ")" << endl;
    *out << "          --centre            Node labels appear at the centre of the node" << endl;
    *out << "                              (default: off, node labels appear over visible" << endl;
    *out << "                              parts of nodes)" << endl;
    *out << endl;
    *out << "          Node colours" << endl;
    *out << "          ---------------------------------------------------------------------" << endl;
    *out << "          --colour <scheme>   Node colouring scheme, from one of the following" << endl;
    *out << "                              options: random, uniform or coverage (default: " << endl;
    *out << "                              random)" << endl;
    *out << endl;
    *out << "          Random colour scheme" << endl;
    *out << "          ---------------------------------------------------------------------" << endl;
    *out << "          These settings only apply when the random colour scheme is used." << endl;
    *out << "          --ransatpos <int>   Positive node saturation (0-255, default: " + QString::number(g_settings->randomColourPositiveSaturation) + ")" << endl;
    *out << "          --ransatneg <int>   Negative node saturation (0-255, default: " + QString::number(g_settings->randomColourNegativeSaturation) + ")" << endl;
    *out << "          --ranligpos <int>   Positive node lightness (0-255, default: " + QString::number(g_settings->randomColourPositiveLightness) + ")" << endl;
    *out << "          --ranligneg <int>   Negative node lightness (0-255, default: " + QString::number(g_settings->randomColourNegativeLightness) + ")" << endl;
    *out << "          --ranopapos <int>   Positive node opacity (0-255, default: " + QString::number(g_settings->randomColourPositiveOpacity) + ")" << endl;
    *out << "          --ranopaneg <int>   Negative node opacity (0-255, default: " + QString::number(g_settings->randomColourNegativeOpacity) + ")" << endl;
    *out << endl;
    *out << "          Uniform colour scheme" << endl;
    *out << "          ---------------------------------------------------------------------" << endl;
    *out << "          These settings only apply when the uniform colour scheme is used." << endl;
    *out << "          --unicolpos <col>   Positive node colour (default: " + getColourName(g_settings->uniformPositiveNodeColour.name()) + ")" << endl;
    *out << "          --unicolneg <col>   Negative node colour (default: " + getColourName(g_settings->uniformNegativeNodeColour.name()) + ")" << endl;
    *out << "          --unicolspe <col>   Special node colour (default: " + getColourName(g_settings->uniformNodeSpecialColour.name()) + ")" << endl;
    *out << endl;
    *out << "          Coverage colour scheme" << endl;
    *out << "          ---------------------------------------------------------------------" << endl;
    *out << "          These settings only apply when the coverage colour scheme is used." << endl;
    *out << "          --covcollow <col>   Colour for nodes with coverage below the low" << endl;
    *out << "                              coverage value (default: " + getColourName(g_settings->lowCoverageColour.name()) + ")" << endl;
    *out << "          --covcolhi <col>    Colour for nodes with coverage above the high" << endl;
    *out << "                              coverage value (default: " + getColourName(g_settings->highCoverageColour.name()) + ")" << endl;
    *out << "          --covvallow <float> Low coverage value (default: auto)" << endl;
    *out << "          --covvalhi <float>  High coverage value (default: auto)" << endl;
    *out << endl;
}


void parseSettings(QStringList arguments)
{
    g_settings->doubleMode = isOptionPresent("--double", &arguments);

    if (isOptionPresent("--bases", &arguments))
    {
        g_settings->manualBasePairsPerSegment = getIntOption("--bases", &arguments);
        g_settings->nodeLengthMode = MANUAL_NODE_LENGTH;
    }

    if (isOptionPresent("--quality", &arguments))
    {
        int quality = getIntOption("--quality", &arguments);
        if (quality < 0)
            quality = 0;
        if (quality > 4)
            quality = 4;
        g_settings->graphLayoutQuality = quality;
    }

    if (isOptionPresent("--nodewidth", &arguments))
        g_settings->averageNodeWidth = getFloatOption("--nodewidth", &arguments);
    if (isOptionPresent("--covwidth", &arguments))
        g_settings->coverageEffectOnWidth = getFloatOption("--covwidth", &arguments);
    if (isOptionPresent("--covpower", &arguments))
        g_settings->coveragePower = getFloatOption("--covpower", &arguments);

    if (isOptionPresent("--edgewidth", &arguments))
        g_settings->edgeWidth = getFloatOption("--edgewidth", &arguments);
    if (isOptionPresent("--outline", &arguments))
        g_settings->outlineThickness = getFloatOption("--outline", &arguments);
    g_settings->antialiasing = !isOptionPresent("--noaa", &arguments);

    if (isOptionPresent("--edgecol", &arguments))
        g_settings->edgeColour = getColourOption("--edgecol", &arguments);
    if (isOptionPresent("--outcol", &arguments))
        g_settings->outlineColour = getColourOption("--outcol", &arguments);
    if (isOptionPresent("--selcol", &arguments))
        g_settings->selectionColour = getColourOption("--selcol", &arguments);
    if (isOptionPresent("--textcol", &arguments))
        g_settings->textColour = getColourOption("--textcol", &arguments);
    if (isOptionPresent("--toutcol", &arguments))
        g_settings->textOutlineColour = getColourOption("--toutcol", &arguments);
    g_settings->positionTextNodeCentre = isOptionPresent("--centre", &arguments);

    g_settings->displayNodeNames = isOptionPresent("--names", &arguments);

    g_settings->displayNodeLengths = isOptionPresent("--lengths", &arguments);

    g_settings->displayNodeCoverages = isOptionPresent("--coverages", &arguments);

    if (isOptionPresent("--fontsize", &arguments))
    {
        int fontsize = getIntOption("--fontsize", &arguments);
        QFont font = g_settings->labelFont;
        font.setPointSize(fontsize);
        g_settings->labelFont = font;
    }

    if (isOptionPresent("--toutline", &arguments))
    {
        double textOutlineThickness = getFloatOption("--toutline", &arguments);
        if (textOutlineThickness == 0.0)
            g_settings->textOutline = false;
        else
        {
            g_settings->textOutline = true;
            g_settings->textOutlineThickness = textOutlineThickness;
        }
    }
    else
    {
        g_settings->textOutline = true;
        g_settings->textOutlineThickness = 0.3;
    }

    if (isOptionPresent("--colour", &arguments))
        g_settings->nodeColourScheme = getColourSchemeOption("--colour", &arguments);

    if (isOptionPresent("--ransatpos", &arguments))
        g_settings->randomColourPositiveSaturation = getIntOption("--ransatpos", &arguments);
    if (isOptionPresent("--ransatneg", &arguments))
        g_settings->randomColourNegativeSaturation = getIntOption("--ransatneg", &arguments);
    if (isOptionPresent("--ranligpos", &arguments))
        g_settings->randomColourPositiveLightness = getIntOption("--ranligpos", &arguments);
    if (isOptionPresent("--ranligneg", &arguments))
        g_settings->randomColourNegativeLightness = getIntOption("--ranligneg", &arguments);
    if (isOptionPresent("--ranopapos", &arguments))
        g_settings->randomColourPositiveOpacity = getIntOption("--ranopapos", &arguments);
    if (isOptionPresent("--ranopaneg", &arguments))
        g_settings->randomColourNegativeOpacity = getIntOption("--ranopaneg", &arguments);

    if (isOptionPresent("--unicolpos", &arguments))
        g_settings->uniformPositiveNodeColour = getColourOption("--unicolpos", &arguments);
    if (isOptionPresent("--unicolneg", &arguments))
        g_settings->uniformNegativeNodeColour = getColourOption("--unicolneg", &arguments);
    if (isOptionPresent("--unicolspe", &arguments))
        g_settings->uniformNodeSpecialColour = getColourOption("--unicolspe", &arguments);

    if (isOptionPresent("--covcollow", &arguments))
        g_settings->lowCoverageColour = getColourOption("--covcollow", &arguments);
    if (isOptionPresent("--covcolhi", &arguments))
        g_settings->highCoverageColour = getColourOption("--covcolhi", &arguments);
    if (isOptionPresent("--covvallow", &arguments))
    {
        g_settings->lowCoverageValue = getFloatOption("--covvallow", &arguments);
        g_settings->autoCoverageValue = false;
    }
    if (isOptionPresent("--covvalhi", &arguments))
    {
        g_settings->highCoverageValue = getFloatOption("--covvalhi", &arguments);
        g_settings->autoCoverageValue = false;
    }
}


//This function will convert a colour to its SVG name, if one exists, or the hex value otherwise.
QString getColourName(QColor colour)
{
    if (colour == QColor(240, 248, 255)) return "aliceblue";
    if (colour == QColor(250, 235, 215)) return "antiquewhite";
    if (colour == QColor( 0, 255, 255)) return "aqua";
    if (colour == QColor(127, 255, 212)) return "aquamarine";
    if (colour == QColor(240, 255, 255)) return "azure";
    if (colour == QColor(245, 245, 220)) return "beige";
    if (colour == QColor(255, 228, 196)) return "bisque";
    if (colour == QColor( 0, 0, 0)) return "black";
    if (colour == QColor(255, 235, 205)) return "blanchedalmond";
    if (colour == QColor( 0, 0, 255)) return "blue";
    if (colour == QColor(138, 43, 226)) return "blueviolet";
    if (colour == QColor(165, 42, 42)) return "brown";
    if (colour == QColor(222, 184, 135)) return "burlywood";
    if (colour == QColor( 95, 158, 160)) return "cadetblue";
    if (colour == QColor(127, 255, 0)) return "chartreuse";
    if (colour == QColor(210, 105, 30)) return "chocolate";
    if (colour == QColor(255, 127, 80)) return "coral";
    if (colour == QColor(100, 149, 237)) return "cornflowerblue";
    if (colour == QColor(255, 248, 220)) return "cornsilk";
    if (colour == QColor(220, 20, 60)) return "crimson";
    if (colour == QColor( 0, 255, 255)) return "cyan";
    if (colour == QColor( 0, 0, 139)) return "darkblue";
    if (colour == QColor( 0, 139, 139)) return "darkcyan";
    if (colour == QColor(184, 134, 11)) return "darkgoldenrod";
    if (colour == QColor( 0, 100, 0)) return "darkgreen";
    if (colour == QColor(169, 169, 169)) return "darkgrey";
    if (colour == QColor(189, 183, 107)) return "darkkhaki";
    if (colour == QColor(139, 0, 139)) return "darkmagenta";
    if (colour == QColor( 85, 107, 47)) return "darkolivegreen";
    if (colour == QColor(255, 140, 0)) return "darkorange";
    if (colour == QColor(153, 50, 204)) return "darkorchid";
    if (colour == QColor(139, 0, 0)) return "darkred";
    if (colour == QColor(233, 150, 122)) return "darksalmon";
    if (colour == QColor(143, 188, 143)) return "darkseagreen";
    if (colour == QColor( 72, 61, 139)) return "darkslateblue";
    if (colour == QColor( 47, 79, 79)) return "darkslategrey";
    if (colour == QColor( 0, 206, 209)) return "darkturquoise";
    if (colour == QColor(148, 0, 211)) return "darkviolet";
    if (colour == QColor(255, 20, 147)) return "deeppink";
    if (colour == QColor( 0, 191, 255)) return "deepskyblue";
    if (colour == QColor(105, 105, 105)) return "dimgrey";
    if (colour == QColor( 30, 144, 255)) return "dodgerblue";
    if (colour == QColor(178, 34, 34)) return "firebrick";
    if (colour == QColor(255, 250, 240)) return "floralwhite";
    if (colour == QColor( 34, 139, 34)) return "forestgreen";
    if (colour == QColor(255, 0, 255)) return "fuchsia";
    if (colour == QColor(220, 220, 220)) return "gainsboro";
    if (colour == QColor(248, 248, 255)) return "ghostwhite";
    if (colour == QColor(255, 215, 0)) return "gold";
    if (colour == QColor(218, 165, 32)) return "goldenrod";
    if (colour == QColor(128, 128, 128)) return "grey";
    if (colour == QColor( 0, 128, 0)) return "green";
    if (colour == QColor(173, 255, 47)) return "greenyellow";
    if (colour == QColor(240, 255, 240)) return "honeydew";
    if (colour == QColor(255, 105, 180)) return "hotpink";
    if (colour == QColor(205, 92, 92)) return "indianred";
    if (colour == QColor( 75, 0, 130)) return "indigo";
    if (colour == QColor(255, 255, 240)) return "ivory";
    if (colour == QColor(240, 230, 140)) return "khaki";
    if (colour == QColor(230, 230, 250)) return "lavender";
    if (colour == QColor(255, 240, 245)) return "lavenderblush";
    if (colour == QColor(124, 252, 0)) return "lawngreen";
    if (colour == QColor(255, 250, 205)) return "lemonchiffon";
    if (colour == QColor(173, 216, 230)) return "lightblue";
    if (colour == QColor(240, 128, 128)) return "lightcoral";
    if (colour == QColor(224, 255, 255)) return "lightcyan";
    if (colour == QColor(250, 250, 210)) return "lightgoldenrodyellow";
    if (colour == QColor(144, 238, 144)) return "lightgreen";
    if (colour == QColor(211, 211, 211)) return "lightgrey";
    if (colour == QColor(255, 182, 193)) return "lightpink";
    if (colour == QColor(255, 160, 122)) return "lightsalmon";
    if (colour == QColor( 32, 178, 170)) return "lightseagreen";
    if (colour == QColor(135, 206, 250)) return "lightskyblue";
    if (colour == QColor(119, 136, 153)) return "lightslategrey";
    if (colour == QColor(176, 196, 222)) return "lightsteelblue";
    if (colour == QColor(255, 255, 224)) return "lightyellow";
    if (colour == QColor( 0, 255, 0)) return "lime";
    if (colour == QColor( 50, 205, 50)) return "limegreen";
    if (colour == QColor(250, 240, 230)) return "linen";
    if (colour == QColor(255, 0, 255)) return "magenta";
    if (colour == QColor(128, 0, 0)) return "maroon";
    if (colour == QColor(102, 205, 170)) return "mediumaquamarine";
    if (colour == QColor( 0, 0, 205)) return "mediumblue";
    if (colour == QColor(186, 85, 211)) return "mediumorchid";
    if (colour == QColor(147, 112, 219)) return "mediumpurple";
    if (colour == QColor( 60, 179, 113)) return "mediumseagreen";
    if (colour == QColor(123, 104, 238)) return "mediumslateblue";
    if (colour == QColor( 0, 250, 154)) return "mediumspringgreen";
    if (colour == QColor( 72, 209, 204)) return "mediumturquoise";
    if (colour == QColor(199, 21, 133)) return "mediumvioletred";
    if (colour == QColor( 25, 25, 112)) return "midnightblue";
    if (colour == QColor(245, 255, 250)) return "mintcream";
    if (colour == QColor(255, 228, 225)) return "mistyrose";
    if (colour == QColor(255, 228, 181)) return "moccasin";
    if (colour == QColor(255, 222, 173)) return "navajowhite";
    if (colour == QColor( 0, 0, 128)) return "navy";
    if (colour == QColor(253, 245, 230)) return "oldlace";
    if (colour == QColor(128, 128, 0)) return "olive";
    if (colour == QColor(107, 142, 35)) return "olivedrab";
    if (colour == QColor(255, 165, 0)) return "orange";
    if (colour == QColor(255, 69, 0)) return "orangered";
    if (colour == QColor(218, 112, 214)) return "orchid";
    if (colour == QColor(238, 232, 170)) return "palegoldenrod";
    if (colour == QColor(152, 251, 152)) return "palegreen";
    if (colour == QColor(175, 238, 238)) return "paleturquoise";
    if (colour == QColor(219, 112, 147)) return "palevioletred";
    if (colour == QColor(255, 239, 213)) return "papayawhip";
    if (colour == QColor(255, 218, 185)) return "peachpuff";
    if (colour == QColor(205, 133, 63)) return "peru";
    if (colour == QColor(255, 192, 203)) return "pink";
    if (colour == QColor(221, 160, 221)) return "plum";
    if (colour == QColor(176, 224, 230)) return "powderblue";
    if (colour == QColor(128, 0, 128)) return "purple";
    if (colour == QColor(255, 0, 0)) return "red";
    if (colour == QColor(188, 143, 143)) return "rosybrown";
    if (colour == QColor( 65, 105, 225)) return "royalblue";
    if (colour == QColor(139, 69, 19)) return "saddlebrown";
    if (colour == QColor(250, 128, 114)) return "salmon";
    if (colour == QColor(244, 164, 96)) return "sandybrown";
    if (colour == QColor( 46, 139, 87)) return "seagreen";
    if (colour == QColor(255, 245, 238)) return "seashell";
    if (colour == QColor(160, 82, 45)) return "sienna";
    if (colour == QColor(192, 192, 192)) return "silver";
    if (colour == QColor(135, 206, 235)) return "skyblue";
    if (colour == QColor(106, 90, 205)) return "slateblue";
    if (colour == QColor(112, 128, 144)) return "slategrey";
    if (colour == QColor(255, 250, 250)) return "snow";
    if (colour == QColor( 0, 255, 127)) return "springgreen";
    if (colour == QColor( 70, 130, 180)) return "steelblue";
    if (colour == QColor(210, 180, 140)) return "tan";
    if (colour == QColor( 0, 128, 128)) return "teal";
    if (colour == QColor(216, 191, 216)) return "thistle";
    if (colour == QColor(255, 99, 71)) return "tomato";
    if (colour == QColor( 64, 224, 208)) return "turquoise";
    if (colour == QColor(238, 130, 238)) return "violet";
    if (colour == QColor(245, 222, 179)) return "wheat";
    if (colour == QColor(255, 255, 255)) return "white";
    if (colour == QColor(245, 245, 245)) return "whitesmoke";
    if (colour == QColor(255, 255, 0)) return "yellow";
    if (colour == QColor(154, 205, 50)) return "yellowgreen";

    return colour.name();
}
