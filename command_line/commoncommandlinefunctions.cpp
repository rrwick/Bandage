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
#include <QDir>
#include "../blast/blastsearch.h"
#include <QApplication>
#include "../program/memory.h"
#include <limits>


QStringList getArgumentList(int argc, char *argv[])
{
    QStringList arguments;
    for (int i = 1; i < argc; ++i)
    {
        QString argument = argv[i];
        arguments.push_back(argument);
    }
    return arguments;
}



void getSettingsUsage(QStringList * text)
{
    QString dashes = "";
    for (int i = 0; i < g_memory->terminalWidth - 10; ++i)
        dashes += '-';

    *text << "Settings: The following options configure the Bandage settings that are available in the Bandage GUI.";
    *text << "";
    *text << "Colours can be specified using hex values, with or without an alpha channel, (e.g. #FFB6C1 or #7FD2B48C) or using standard colour names (e.g. red, yellowgreen or skyblue).  Note that hex colours will either need to be enclosed in quotes (e.g. \"#FFB6C1\") or have the hash symbol escaped (e.g. \\#FFB6C1).";
    *text << "";
    *text << "Graph scope";
    *text << dashes;
    *text << "These settings control the graph scope.  If the aroundnodes scope is used, then the --nodes option must also be used.  If the aroundblast scope is used, a BLAST query must be given with the --query option.";
    getGraphScopeOptions(text);
    *text << "--double            Draw graph in double mode (default: off)";
    *text << "";
    *text << "Graph size";
    *text << dashes;
    *text << "--nodelen <float>   Node length per megabase " + getRangeAndDefault(g_settings->manualNodeLengthPerMegabase, "auto");
    *text << "--minnodlen <float> Minimum node length " + getRangeAndDefault(g_settings->minimumNodeLength);
    *text << "--edgelen <float>   Edge length " + getRangeAndDefault(g_settings->edgeLength);
    *text << "--edgewidth <float> Edge width " + getRangeAndDefault(g_settings->edgeWidth);
    *text << "--doubsep <float>   Double mode separation " + getRangeAndDefault(g_settings->doubleModeNodeSeparation);
    *text << "";
    *text << "Graph layout quality";
    *text << dashes;
    *text << "--nodseglen <float> Node segment length " + getRangeAndDefault(g_settings->nodeSegmentLength);
    *text << "--iter <int>        Graph layout iterations " + getRangeAndDefault(g_settings->graphLayoutQuality);
    *text << "";
    *text << "Graph appearance";
    *text << dashes;
    *text << "--edgecol <col>     Colour for edges " + getDefaultColour(g_settings->edgeColour);
    *text << "--outcol <col>      Colour for node outlines " + getDefaultColour(g_settings->outlineColour);
    *text << "--outline <float>   Node outline thickness " + getRangeAndDefault(g_settings->outlineThickness);
    *text << "--selcol <col>      Colour for selections " + getDefaultColour(g_settings->selectionColour);
    *text << "--noaa              Disable antialiasing (default: antialiasing on)";
    *text << "";
    *text << "Text appearance";
    *text << dashes;
    *text << "--textcol <col>     Colour for label text " + getDefaultColour(g_settings->textColour);
    *text << "--toutcol <col>     Colour for text outline " + getDefaultColour(g_settings->textOutlineColour);
    *text << "--toutline <float>  Surround text with an outline with this thickness " + getRangeAndDefault(g_settings->textOutlineThickness);
    *text << "--centre            Node labels appear at the centre of the node (default: off, node labels appear over visible parts of nodes)";
    *text << "";
    *text << "Node width";
    *text << dashes;
    *text << "Node widths are determined using the following formula:";
    *text << "a*b*((c/d)^e-1)+1";
    *text << "  a = average node width";
    *text << "  b = read depth effect on width";
    *text << "  c = node read depth";
    *text << "  d = mean read depth";
    *text << "  e = power of read depth effect on width";
    *text << "--nodewidth <float> Average node width " + getRangeAndDefault(g_settings->averageNodeWidth);
    *text << "--depwidth <float>  Read depth effect on width " + getRangeAndDefault(g_settings->readDepthEffectOnWidth);
    *text << "--deppower <float>  Power of read depth effect on width " + getRangeAndDefault(g_settings->readDepthPower);
    *text << "";
    *text << "Node labels";
    *text << dashes;
    *text << "--names             Label nodes with name (default: off)";
    *text << "--lengths           Label nodes with length (default: off)";
    *text << "--readdepth         Label nodes with read depth (default: off)";
    *text << "--blasthits         Label BLAST hits (default: off)";
    *text << "--fontsize <int>    Font size for node labels " + getRangeAndDefault(g_settings->labelFont.pointSize(), 1, 100);
    *text << "";
    *text << "Node colours";
    *text << dashes;
    *text << "--colour <scheme>   Node colouring scheme, from one of the following options: random, uniform, readdepth, blastsolid, blastrainbow (default: random if --query option not used, blastsolid if --query option used)";
    *text << "";
    *text << "Random colour scheme";
    *text << dashes;
    *text << "These settings only apply when the random colour scheme is used.";
    *text << "--ransatpos <int>   Positive node saturation " + getRangeAndDefault(g_settings->randomColourPositiveSaturation);
    *text << "--ransatneg <int>   Negative node saturation " + getRangeAndDefault(g_settings->randomColourNegativeSaturation);
    *text << "--ranligpos <int>   Positive node lightness " + getRangeAndDefault(g_settings->randomColourPositiveLightness);
    *text << "--ranligneg <int>   Negative node lightness " + getRangeAndDefault(g_settings->randomColourNegativeLightness);
    *text << "--ranopapos <int>   Positive node opacity " + getRangeAndDefault(g_settings->randomColourPositiveOpacity);
    *text << "--ranopaneg <int>   Negative node opacity " + getRangeAndDefault(g_settings->randomColourNegativeOpacity);
    *text << "";
    *text << "Uniform colour scheme";
    *text << dashes;
    *text << "These settings only apply when the uniform colour scheme is used.";
    *text << "--unicolpos <col>   Positive node colour " + getDefaultColour(g_settings->uniformPositiveNodeColour);
    *text << "--unicolneg <col>   Negative node colour " + getDefaultColour(g_settings->uniformNegativeNodeColour);
    *text << "--unicolspe <col>   Special node colour " + getDefaultColour(g_settings->uniformNodeSpecialColour);
    *text << "";
    *text << "Read depth colour scheme";
    *text << dashes;
    *text << "These settings only apply when the read depth colour scheme is used.";
    *text << "--depcollow <col>   Colour for nodes with read depth below the low read depth value " + getDefaultColour(g_settings->lowReadDepthColour);
    *text << "--depcolhi <col>    Colour for nodes with read depth above the high read depth value " + getDefaultColour(g_settings->highReadDepthColour);
    *text << "--depvallow <float> Low read depth value " + getRangeAndDefault(g_settings->lowReadDepthValue, "auto");
    *text << "--depvalhi <float>  High read depth value " + getRangeAndDefault(g_settings->highReadDepthValue, "auto");
    *text << "";
    *text << "BLAST search";
    *text << dashes;
    *text << "--query <fastafile> A FASTA file of either nucleotide or protein sequences to be used as BLAST queries (default: none)";
    *text << "--blastp <param>    Parameters to be used by blastn and tblastn when conducting a BLAST search in Bandage (default: none). Format BLAST parameters exactly as they would be used for blastn/tblastn on the command line, and enclose them in quotes.";
    *text << "--alfilter <int>    Alignment length filter for BLAST hits. Hits with shorter alignments will be excluded " + getRangeAndDefault(g_settings->blastAlignmentLengthFilter);
    *text << "--qcfilter <float>  Query coverage filter for BLAST hits. Hits with less coverage will be excluded " + getRangeAndDefault(g_settings->blastQueryCoverageFilter);
    *text << "--ifilter <float>   Identity filter for BLAST hits. Hits with less identity will be excluded " + getRangeAndDefault(g_settings->blastIdentityFilter);
    *text << "--evfilter <sci>    E-value filter for BLAST hits. Hits with larger e-values will be excluded " + getRangeAndDefault(g_settings->blastEValueFilter);
    *text << "--bsfilter <float>  Bit score filter for BLAST hits. Hits with lower bit scores will be excluded " + getRangeAndDefault(g_settings->blastBitScoreFilter);
    *text << "";
    *text << "BLAST query paths";
    *text << dashes;
    *text << "These settings control how Bandage searches for query paths after conducting a BLAST search.";
    *text << "--pathnodes <int>   The number of allowed nodes in a BLAST query path " + getRangeAndDefault(g_settings->maxQueryPathNodes);
    *text << "--minpatcov <float> Minimum fraction of a BLAST query which must be covered by a query path " + getRangeAndDefault(g_settings->minQueryCoveredByPath);
    *text << "--minhitcov <float> Minimum fraction of a BLAST query which must be covered by BLAST hits in a query path " + getRangeAndDefault(g_settings->minQueryCoveredByHits);
    *text << "--minmeanid <float> Minimum mean identity of BLAST hits in a query path " + getRangeAndDefault(g_settings->minMeanHitIdentity);
    *text << "--maxevprod <sci>   Maximum e-value product for all BLAST hits in a query path " + getRangeAndDefault(g_settings->maxEValueProduct);
    *text << "--minpatlen <float> Minimum allowed relative path length as compared to the query " + getRangeAndDefault(g_settings->minLengthPercentage);
    *text << "--maxpatlen <float> Maximum allowed relative path length as compared to the query " + getRangeAndDefault(g_settings->maxLengthPercentage);
    *text << "--minlendis <int>   Minimum allowed length discrepancy (in bases) between a BLAST query and its path in the graph " + getRangeAndDefault(g_settings->minLengthBaseDiscrepancy);
    *text << "--maxlendis <int>   Maximum allowed length discrepancy (in bases) between a BLAST query and its path in the graph " + getRangeAndDefault(g_settings->maxLengthBaseDiscrepancy);
    *text << "";
}



void outputText(QString text, QTextStream * out)
{
    QStringList list;
    list << text;
    outputText(list, out);
}

void outputText(QStringList text, QTextStream * out)
{
    QStringList wrapped;

    bool seenHeaderOrList = false;
    for (int i = 0; i < text.size(); ++i)
    {
        QString line = text[i];

        if (isSectionHeader(line) || isListItem(line))
            seenHeaderOrList = true;

        if (isError(line))
            wrapped << wrapText(line, g_memory->terminalWidth, 0, 0);
        else if (!seenHeaderOrList)
            wrapped << wrapText(line, g_memory->terminalWidth, 0, 0);
        else if (isSectionHeader(line) && line.contains("--"))
            wrapped << wrapText(line, g_memory->terminalWidth, 0, 30);
        else if (isSectionHeader(line))
            wrapped << wrapText(line, g_memory->terminalWidth, 0, 10);
        else if (isListItem(line))
            wrapped << wrapText(line, g_memory->terminalWidth, 2, 4);
        else if (isCommand(line))
            wrapped << wrapText(line, g_memory->terminalWidth, 10, 23);
        else if (isOption(line))
            wrapped << wrapText(line, g_memory->terminalWidth, 10, 30);
        else
            wrapped << wrapText(line, g_memory->terminalWidth, 10, 10);
    }

    *out << endl;
    for (int i = 0; i < wrapped.size(); ++i)
    {
        *out << wrapped[i];
        *out << endl;
    }
}


//This is in a separate function because the command line tool Bandage reduce
//also displays these.
void getGraphScopeOptions(QStringList * text)
{
    *text << "--scope <scope>     Graph scope, from one of the following options: entire, aroundnodes, aroundblast, depthrange (default: entire)";
    *text << "--nodes <list>      A comma-separated list of starting nodes for the aroundnodes scope (default: none)";
    *text << "--partial           Use partial node name matching (default: exact node name matching)";
    *text << "--distance <int>    The number of node steps away to draw for the aroundnodes and aroundblast scopes " + getRangeAndDefault(g_settings->nodeDistance);
    *text << "--mindepth <float>  The minimum allowed read depth for the depthrange scope " + getRangeAndDefault(g_settings->minReadDepthRange);
    *text << "--maxdepth <float>  The maximum allowed read depth for the depthrange scope "  + getRangeAndDefault(g_settings->maxReadDepthRange);
}


//This function checks the values of the Bandage settings.
//If everything is fine, it removes the good arguments/values and returns
//a null string.  If there's a problem, it returns an error message.
QString checkForInvalidOrExcessSettings(QStringList * arguments)
{
    QStringList argumentsCopy = *arguments;

    QStringList validScopeOptions;
    validScopeOptions << "entire" << "aroundnodes" << "aroundblast" << "depthrange";
    QString error;

    error = checkOptionForString("--scope", arguments, validScopeOptions); if (error.length() > 0) return error;
    error = checkOptionForString("--nodes", arguments, QStringList(), "a list of node names"); if (error.length() > 0) return error;
    checkOptionWithoutValue("--partial", arguments);
    error = checkOptionForInt("--distance", arguments, g_settings->nodeDistance, false); if (error.length() > 0) return error;
    error = checkOptionForFloat("--mindepth", arguments, g_settings->minReadDepthRange, false); if (error.length() > 0) return error;
    error = checkOptionForFloat("--maxdepth", arguments, g_settings->maxReadDepthRange, false); if (error.length() > 0) return error;
    if (isOptionPresent("--query", arguments) && g_memory->commandLineCommand == NO_COMMAND) return "The --query option can only be used with Bandage load and Bandage image";
    error = checkOptionForFile("--query", arguments); if (error.length() > 0) return error;
    error = checkOptionForString("--blastp", arguments, QStringList(), "blastn/tblastn parameters"); if (error.length() > 0) return error;
    checkOptionWithoutValue("--double", arguments);
    error = checkOptionForFloat("--nodelen", arguments, g_settings->manualNodeLengthPerMegabase, false); if (error.length() > 0) return error;
    error = checkOptionForFloat("--minnodlen", arguments, g_settings->minimumNodeLength, false); if (error.length() > 0) return error;
    error = checkOptionForFloat("--edgelen", arguments, g_settings->edgeLength, false); if (error.length() > 0) return error;
    error = checkOptionForFloat("--doubsep", arguments, g_settings->doubleModeNodeSeparation, false); if (error.length() > 0) return error;
    error = checkOptionForInt("--iter", arguments, g_settings->graphLayoutQuality, false); if (error.length() > 0) return error;
    error = checkOptionForFloat("--nodseglen", arguments, g_settings->nodeSegmentLength, false); if (error.length() > 0) return error;
    error = checkOptionForFloat("--nodewidth", arguments, g_settings->averageNodeWidth, false); if (error.length() > 0) return error;
    error = checkOptionForFloat("--depwidth", arguments, g_settings->readDepthEffectOnWidth, false); if (error.length() > 0) return error;
    error = checkOptionForFloat("--deppower", arguments, g_settings->readDepthPower, false); if (error.length() > 0) return error;
    error = checkOptionForFloat("--edgewidth", arguments, g_settings->edgeWidth, false); if (error.length() > 0) return error;
    error = checkOptionForFloat("--outline", arguments, g_settings->outlineThickness, false); if (error.length() > 0) return error;
    checkOptionWithoutValue("--names", arguments);
    checkOptionWithoutValue("--lengths", arguments);
    checkOptionWithoutValue("--readdepth", arguments);
    checkOptionWithoutValue("--blasthits", arguments);
    error = checkOptionForInt("--fontsize", arguments, IntSetting(0, 1, 100), false); if (error.length() > 0) return error;
    error = checkOptionForFloat("--toutline", arguments, g_settings->textOutlineThickness, false); if (error.length() > 0) return error;
    checkOptionWithoutValue("--centre", arguments);
    error = checkOptionForColour("--edgecol", arguments); if (error.length() > 0) return error;
    error = checkOptionForColour("--outcol", arguments); if (error.length() > 0) return error;
    error = checkOptionForColour("--selcol", arguments); if (error.length() > 0) return error;
    error = checkOptionForColour("--textcol", arguments); if (error.length() > 0) return error;
    error = checkOptionForColour("--toutcol", arguments); if (error.length() > 0) return error;
    checkOptionWithoutValue("--noaa", arguments);
    QStringList validColourOptions;
    validColourOptions << "random" << "uniform" << "readdepth" << "blastsolid" << "blastrainbow";
    error = checkOptionForString("--colour", arguments, validColourOptions); if (error.length() > 0) return error;
    error = checkOptionForInt("--ransatpos", arguments, g_settings->randomColourPositiveSaturation, false); if (error.length() > 0) return error;
    error = checkOptionForInt("--ransatneg", arguments, g_settings->randomColourNegativeSaturation, false); if (error.length() > 0) return error;
    error = checkOptionForInt("--ranligpos", arguments, g_settings->randomColourPositiveLightness, false); if (error.length() > 0) return error;
    error = checkOptionForInt("--ranligneg", arguments, g_settings->randomColourNegativeLightness, false); if (error.length() > 0) return error;
    error = checkOptionForInt("--ranopapos", arguments, g_settings->randomColourPositiveOpacity, false); if (error.length() > 0) return error;
    error = checkOptionForInt("--ranopaneg", arguments, g_settings->randomColourNegativeOpacity, false); if (error.length() > 0) return error;
    error = checkOptionForColour("--unicolpos", arguments); if (error.length() > 0) return error;
    error = checkOptionForColour("--unicolneg", arguments); if (error.length() > 0) return error;
    error = checkOptionForColour("--unicolspe", arguments); if (error.length() > 0) return error;
    error = checkOptionForColour("--depcollow", arguments); if (error.length() > 0) return error;
    error = checkOptionForColour("--depcolhi", arguments); if (error.length() > 0) return error;
    error = checkTwoOptionsForFloats("--depvallow", "--depvalhi", arguments, g_settings->lowReadDepthValue, g_settings->highReadDepthValue, true); if (error.length() > 0) return error;
    error = checkOptionForInt("--pathnodes", arguments, g_settings->maxQueryPathNodes, false); if (error.length() > 0) return error;
    error = checkOptionForFloat("--minpatcov", arguments, g_settings->minQueryCoveredByPath, false); if (error.length() > 0) return error;
    error = checkOptionForFloat("--minhitcov", arguments, g_settings->minQueryCoveredByHits, true); if (error.length() > 0) return error;
    error = checkOptionForFloat("--minmeanid", arguments, g_settings->minMeanHitIdentity, true); if (error.length() > 0) return error;
    error = checkOptionForSciNot("--maxevprod", arguments, g_settings->maxEValueProduct, true); if (error.length() > 0) return error;
    error = checkOptionForFloat("--minpatlen", arguments, g_settings->minLengthPercentage, true); if (error.length() > 0) return error;
    error = checkOptionForFloat("--maxpatlen", arguments, g_settings->maxLengthPercentage, true); if (error.length() > 0) return error;
    error = checkOptionForInt("--minlendis", arguments, g_settings->minLengthBaseDiscrepancy, true); if (error.length() > 0) return error;
    error = checkOptionForInt("--maxlendis", arguments, g_settings->maxLengthBaseDiscrepancy, true); if (error.length() > 0) return error;
    error = checkOptionForInt("--alfilter", arguments, g_settings->blastAlignmentLengthFilter, false); if (error.length() > 0) return error;
    error = checkOptionForFloat("--qcfilter", arguments, g_settings->blastQueryCoverageFilter, false); if (error.length() > 0) return error;
    error = checkOptionForFloat("--ifilter", arguments, g_settings->blastIdentityFilter, false); if (error.length() > 0) return error;
    error = checkOptionForSciNot("--evfilter", arguments, g_settings->blastEValueFilter, false); if (error.length() > 0) return error;
    error = checkOptionForFloat("--bsfilter", arguments, g_settings->blastBitScoreFilter, false); if (error.length() > 0) return error;

    //Make sure that the min read depth is less than or equal to the max read
    //depth.
    double minReadDepth = g_settings->minReadDepthRange;
    double maxReadDepth = g_settings->maxReadDepthRange;
    if (isOptionPresent("--mindepth", &argumentsCopy))
        minReadDepth = getFloatOption("--mindepth", &argumentsCopy);
    if (isOptionPresent("--maxdepth", &argumentsCopy))
        maxReadDepth = getFloatOption("--maxdepth", &argumentsCopy);
    if (minReadDepth > maxReadDepth)
        return "the maximum read depth must be greater than or equal to the minimum read depth.";

    //Make sure that the min path length is less than or equal to the max path
    //length.
    bool minLengthPercentageOn = g_settings->minLengthPercentage.on;
    bool maxLengthPercentageOn = g_settings->maxLengthPercentage.on;
    double minLengthPercentage = g_settings->minLengthPercentage;
    double maxLengthPercentage = g_settings->maxLengthPercentage;
    if (isOptionPresent("--minpatlen", &argumentsCopy))
    {
        QString optionString = getStringOption("--minpatlen", &argumentsCopy);
        if (optionString.toLower() == "off")
            minLengthPercentageOn = false;
        else
        {
            minLengthPercentageOn = true;
            minLengthPercentage = getFloatOption("--minpatlen", &argumentsCopy);
        }
    }
    if (isOptionPresent("--maxpatlen", &argumentsCopy))
    {
        QString optionString = getStringOption("--maxpatlen", &argumentsCopy);
        if (optionString.toLower() == "off")
            maxLengthPercentageOn = false;
        else
        {
            maxLengthPercentageOn = true;
            maxLengthPercentage = getFloatOption("--maxpatlen", &argumentsCopy);
        }
    }
    if (minLengthPercentageOn && maxLengthPercentageOn &&
            minLengthPercentage > maxLengthPercentage)
        return "the maximum BLAST query path length discrepancy must be greater than or equal to the minimum length discrepancy.";

    //Make sure that the min length discrepancy is less than or equal to the max
    //length discrepancy.
    bool minLengthBaseDiscrepancyOn = g_settings->minLengthBaseDiscrepancy.on;
    bool maxLengthBaseDiscrepancyOn = g_settings->maxLengthBaseDiscrepancy.on;
    int minLengthBaseDiscrepancy = g_settings->minLengthBaseDiscrepancy;
    int maxLengthBaseDiscrepancy = g_settings->maxLengthBaseDiscrepancy;
    if (isOptionPresent("--minlendis", &argumentsCopy))
    {
        QString optionString = getStringOption("--minlendis", &argumentsCopy);
        if (optionString.toLower() == "off")
            minLengthBaseDiscrepancyOn = false;
        else
        {
            minLengthBaseDiscrepancyOn = true;
            minLengthBaseDiscrepancy = getIntOption("--minlendis", &argumentsCopy);
        }
    }
    if (isOptionPresent("--maxlendis", &argumentsCopy))
    {
        QString optionString = getStringOption("--maxlendis", &argumentsCopy);
        if (optionString.toLower() == "off")
            g_settings->maxLengthBaseDiscrepancy.on = false;
        else
        {
            maxLengthBaseDiscrepancyOn = true;
            maxLengthBaseDiscrepancy = getIntOption("--maxlendis", &argumentsCopy);
        }
    }
    if (minLengthBaseDiscrepancyOn && maxLengthBaseDiscrepancyOn &&
            minLengthBaseDiscrepancy > maxLengthBaseDiscrepancy)
        return "the maximum BLAST query path length discrepancy must be greater than or equal to the minimum length discrepancy.";

    bool blastScope = isOptionAndValuePresent("--scope", "aroundblast", &argumentsCopy);
    bool queryFile = isOptionPresent("--query", &argumentsCopy);
    if (blastScope && !queryFile)
        return "A BLAST query must be given with the --query option when the\naroundblast scope is used.";

    bool nodesScope = isOptionAndValuePresent("--scope", "aroundnodes", &argumentsCopy);
    bool nodesList = isOptionPresent("--nodes", &argumentsCopy);
    if (nodesScope && !nodesList)
        return "A list of starting nodes must be given with the --nodes option\nwhen the aroundnodes scope is used.";

    bool readDepthScope = isOptionAndValuePresent("--scope", "depthrange", &argumentsCopy);
    bool minDepth = isOptionPresent("--mindepth", &argumentsCopy);
    bool maxDepth = isOptionPresent("--maxdepth", &argumentsCopy);
    if (readDepthScope && !(minDepth && maxDepth))
        return "A read depth range must be given with the --mindepth and\n--maxdepth options when the aroundnodes scope is used.";

    return checkForExcessArguments(*arguments);
}



void parseSettings(QStringList arguments)
{
    if (isOptionPresent("--scope", &arguments))
        g_settings->graphScope = getGraphScopeOption("--scope", &arguments);

    if (isOptionPresent("--distance", &arguments))
        g_settings->nodeDistance = getIntOption("--distance", &arguments);

    if (isOptionPresent("--mindepth", &arguments))
        g_settings->minReadDepthRange = getFloatOption("--mindepth", &arguments);
    if (isOptionPresent("--maxdepth", &arguments))
        g_settings->maxReadDepthRange = getFloatOption("--maxdepth", &arguments);

    if (isOptionPresent("--nodes", &arguments))
        g_settings->startingNodes = getStringOption("--nodes", &arguments);
    g_settings->startingNodesExactMatch = !isOptionPresent("--partial", &arguments);

    if (isOptionPresent("--query", &arguments))
        g_settings->blastQueryFilename = getStringOption("--query", &arguments);
    if (isOptionPresent("--blastp", &arguments))
        g_settings->blastSearchParameters = getStringOption("--blastp", &arguments);

    g_settings->doubleMode = isOptionPresent("--double", &arguments);

    if (isOptionPresent("--nodelen", &arguments))
    {
        g_settings->manualNodeLengthPerMegabase = getIntOption("--nodelen", &arguments);
        g_settings->nodeLengthMode = MANUAL_NODE_LENGTH;
    }

    if (isOptionPresent("--iter", &arguments))
    {
        int quality = getIntOption("--iter", &arguments);
        if (quality < 0)
            quality = 0;
        if (quality > 4)
            quality = 4;
        g_settings->graphLayoutQuality = quality;
    }

    if (isOptionPresent("--nodewidth", &arguments))
        g_settings->averageNodeWidth = getFloatOption("--nodewidth", &arguments);
    if (isOptionPresent("--depwidth", &arguments))
        g_settings->readDepthEffectOnWidth = getFloatOption("--depwidth", &arguments);
    if (isOptionPresent("--deppower", &arguments))
        g_settings->readDepthPower = getFloatOption("--deppower", &arguments);

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
    g_settings->displayNodeReadDepth = isOptionPresent("--readdepth", &arguments);
    g_settings->displayBlastHits = isOptionPresent("--blasthits", &arguments);

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

    if (isOptionPresent("--depcollow", &arguments))
        g_settings->lowReadDepthColour = getColourOption("--depcollow", &arguments);
    if (isOptionPresent("--depcolhi", &arguments))
        g_settings->highReadDepthColour = getColourOption("--depcolhi", &arguments);
    if (isOptionPresent("--depvallow", &arguments))
    {
        g_settings->lowReadDepthValue = getFloatOption("--depvallow", &arguments);
        g_settings->autoReadDepthValue = false;
    }
    if (isOptionPresent("--depvalhi", &arguments))
    {
        g_settings->highReadDepthValue = getFloatOption("--depvalhi", &arguments);
        g_settings->autoReadDepthValue = false;
    }

    if (isOptionPresent("--pathnodes", &arguments))
        g_settings->maxQueryPathNodes = getIntOption("--pathnodes", &arguments);
    if (isOptionPresent("--minpatcov", &arguments))
        g_settings->minQueryCoveredByPath = getFloatOption("--minpatcov", &arguments);
    if (isOptionPresent("--minhitcov", &arguments))
    {
        QString optionString = getStringOption("--minhitcov", &arguments);
        if (optionString.toLower() == "off")
            g_settings->minQueryCoveredByHits.on = false;
        else
        {
            g_settings->minQueryCoveredByHits.on = true;
            g_settings->minQueryCoveredByHits = getFloatOption("--minhitcov", &arguments);
        }
    }
    if (isOptionPresent("--minmeanid", &arguments))
    {
        QString optionString = getStringOption("--minmeanid", &arguments);
        if (optionString.toLower() == "off")
            g_settings->minMeanHitIdentity.on = false;
        else
        {
            g_settings->minMeanHitIdentity.on = true;
            g_settings->minMeanHitIdentity = getFloatOption("--minmeanid", &arguments);
        }
    }
    if (isOptionPresent("--maxevprod", &arguments))
    {
        QString optionString = getStringOption("--maxevprod", &arguments);
        if (optionString.toLower() == "off")
            g_settings->maxEValueProduct.on = false;
        else
        {
            g_settings->maxEValueProduct.on = true;
            g_settings->maxEValueProduct = getSciNotOption("--maxevprod", &arguments);
        }
    }
    if (isOptionPresent("--minpatlen", &arguments))
    {
        QString optionString = getStringOption("--minpatlen", &arguments);
        if (optionString.toLower() == "off")
            g_settings->minLengthPercentage.on = false;
        else
        {
            g_settings->minLengthPercentage.on = true;
            g_settings->minLengthPercentage = getFloatOption("--minpatlen", &arguments);
        }
    }
    if (isOptionPresent("--maxpatlen", &arguments))
    {
        QString optionString = getStringOption("--maxpatlen", &arguments);
        if (optionString.toLower() == "off")
            g_settings->maxLengthPercentage.on = false;
        else
        {
            g_settings->maxLengthPercentage.on = true;
            g_settings->maxLengthPercentage = getFloatOption("--maxpatlen", &arguments);
        }
    }
    if (isOptionPresent("--minlendis", &arguments))
    {
        QString optionString = getStringOption("--minlendis", &arguments);
        if (optionString.toLower() == "off")
            g_settings->minLengthBaseDiscrepancy.on = false;
        else
        {
            g_settings->minLengthBaseDiscrepancy.on = true;
            g_settings->minLengthBaseDiscrepancy = getIntOption("--minlendis", &arguments);
        }
    }
    if (isOptionPresent("--maxlendis", &arguments))
    {
        QString optionString = getStringOption("--maxlendis", &arguments);
        if (optionString.toLower() == "off")
            g_settings->maxLengthBaseDiscrepancy.on = false;
        else
        {
            g_settings->maxLengthBaseDiscrepancy.on = true;
            g_settings->maxLengthBaseDiscrepancy = getIntOption("--maxlendis", &arguments);
        }
    }

    if (isOptionPresent("--alfilter", &arguments))
    {
        g_settings->blastAlignmentLengthFilter.on = true;
        g_settings->blastAlignmentLengthFilter = getIntOption("--alfilter", &arguments);
    }
    if (isOptionPresent("--qcfilter", &arguments))
    {
        g_settings->blastQueryCoverageFilter.on = true;
        g_settings->blastQueryCoverageFilter = getFloatOption("--qcfilter", &arguments);
    }
    if (isOptionPresent("--ifilter", &arguments))
    {
        g_settings->blastIdentityFilter.on = true;
        g_settings->blastIdentityFilter = getFloatOption("--ifilter", &arguments);
    }
    if (isOptionPresent("--evfilter", &arguments))
    {
        g_settings->blastEValueFilter.on = true;
        g_settings->blastEValueFilter = getSciNotOption("--evfilter", &arguments);
    }
    if (isOptionPresent("--bsfilter", &arguments))
    {
        g_settings->blastBitScoreFilter.on = true;
        g_settings->blastBitScoreFilter = getFloatOption("--bsfilter", &arguments);
    }
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



//This function checks the value for an integer-accepting command line option.
//If offOkay is true, then it will also accept "off" as a valid argument.
//Returns empty string if everything is okay and an error message if there's a
//problem.  If everything is okay, it also removes the option and its value from
//arguments.
QString checkOptionForInt(QString option, QStringList * arguments, IntSetting setting, bool offOkay)
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
    bool optionIsOff = arguments->at(integerIndex).toLower() == "off";
    if (offOkay && !(optionIsInt || optionIsOff))
        return option + " must be followed by an integer or \"off\"";
    if (!offOkay && !optionIsInt)
        return option + " must be followed by an integer";

    //Check the range of the option.
    if (optionIsInt)
    {
        if (optionInt < setting.min || optionInt > setting.max)
            return "Value of " + option + " must be between "
                    + QString::number(setting.min) + " and " + QString::number(setting.max) +
                    " (inclusive)";
    }

    //If the code got here, the option and its integer are okay.
    //Remove them from the arguments.
    arguments->removeAt(integerIndex);
    arguments->removeAt(optionIndex);

    return "";
}


//This function checks the value for a float-accepting command line option.
//If offOkay is true, then it will also accept "off" as a valid argument.
//Returns empty string if everything is okay and an error message if there's a
//problem.  If everything is okay, it also removes the option and its value from
//arguments.
QString checkOptionForFloat(QString option, QStringList * arguments, FloatSetting setting, bool offOkay)
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
    bool optionIsOff = arguments->at(floatIndex).toLower() == "off";
    if (offOkay && !(optionIsFloat || optionIsOff))
        return option + " must be followed by a number or \"off\"";
    if (!offOkay && !optionIsFloat)
        return option + " must be followed by a number";

    //Check the range of the option.
    if (optionIsFloat)
    {
        if (optionFloat < setting.min || optionFloat > setting.max)
            return "Value of " + option + " must be between "
                    + QString::number(setting.min) + " and " + QString::number(setting.max) +
                    " (inclusive)";
    }

    //If the code got here, the option and its number are okay.
    //Remove them from the arguments.
    arguments->removeAt(floatIndex);
    arguments->removeAt(optionIndex);

    return "";
}


//This function checks the value for a scientific notation-accepting command
//line option.  If offOkay is true, then it will also accept "off" as a valid
//argument.
//Returns empty string if everything is okay and an error message if there's a
//problem.  If everything is okay, it also removes the option and its value from
//arguments.
QString checkOptionForSciNot(QString option, QStringList * arguments,
                             SciNotSetting setting, bool offOkay)
{
    int optionIndex = arguments->indexOf(option);

    //If the option isn't found, that's fine.
    if (optionIndex == -1)
        return "";

    int sciNotIndex = optionIndex + 1;

    //If nothing follows the option, that's a problem.
    if (sciNotIndex >= arguments->size())
        return option + " must be followed by a number in scientific notation";

    //If the thing following the option isn't a number in scientific notation or
    //"off", that's a problem.
    bool optionIsSciNot = SciNot::isValidSciNotString(arguments->at(sciNotIndex));
    bool optionIsOff = arguments->at(sciNotIndex).toLower() == "off";
    if (offOkay && !(optionIsSciNot || optionIsOff))
        return option + " must be followed by a number in scientific notation or \"off\"";
    if (!offOkay && !optionIsSciNot)
        return option + " must be followed by a number in scientific notation";

    SciNot optionSciNot = SciNot(arguments->at(sciNotIndex));

    //Check the range of the option.
    if (optionIsSciNot)
    {
        if (optionSciNot < setting.min || optionSciNot > setting.max)
            return "Value of " + option + " must be between "
                    + setting.min.asString(true) + " and " + setting.max.asString(true) +
                    " (inclusive)";
    }

    //If the code got here, the option and its number are okay.
    //Remove them from the arguments.
    arguments->removeAt(sciNotIndex);
    arguments->removeAt(optionIndex);

    return "";
}


//Returns empty string if everything is okay and an error
//message if there's a problem.  If everything is okay, it
//also removes the option and its value from arguments.
QString checkOptionForString(QString option, QStringList * arguments, QStringList validOptionsList, QString validDescription)
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
    if (validOptions == "")
        validOptions = validDescription;

    //If nothing follows the option, that's a problem.
    if (stringIndex >= arguments->size())
        return option + " must be followed by " + validOptions;

    //If the thing following the option isn't a valid choice, that's a problem.
    if (validOptionsList.size() > 0)
    {
        QString value = arguments->at(stringIndex);
        if (!validOptionsList.contains(value, Qt::CaseInsensitive))
            return option + " must be followed by " + validOptions;
    }

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
        return option + " must be followed by a 6-digit hex colour (e.g. #FFB6C1), an 8-digit hex colour (e.g. #7FD2B48C) or a standard colour name (e.g. skyblue)";

    //If the thing following the option isn't a colour, that's a problem.
    QColor colour(arguments->at(colIndex));
    if (!colour.isValid())
        return option + " must be followed by a 6-digit hex colour (e.g. #FFB6C1), an 8-digit hex colour (e.g. #7FD2B48C) or a standard colour name (e.g. skyblue)";

    //If the code got here, the option and its colour are okay.
    //Remove them from the arguments.
    arguments->removeAt(colIndex);
    arguments->removeAt(optionIndex);

    return "";
}


QString checkOptionForFile(QString option, QStringList * arguments)
{
    int optionIndex = arguments->indexOf(option);

    //If the option isn't found, that's fine.
    if (optionIndex == -1)
        return "";

    int fileIndex = optionIndex + 1;

    //If nothing follows the option, that's a problem.
    if (fileIndex >= arguments->size())
        return option + " must be followed by a filename";

    //If the thing that follows the option isn't a file that's a problem
    if (!checkIfFileExists(arguments->at(fileIndex)))
        return option + " must be followed by a valid filename";

    //If the code got here, the option and its file are okay.
    //Remove them from the arguments.
    arguments->removeAt(fileIndex);
    arguments->removeAt(optionIndex);

    return "";
}


bool checkIfFileExists(QString filename)
{
    QFileInfo checkFile(filename);
    return (checkFile.exists() && checkFile.isFile());
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
                                 FloatSetting setting1, FloatSetting setting2,
                                 bool secondMustBeEqualOrLarger)
{
    //First check each option independently
    QStringList argumentsCopy = *arguments;
    QString option1Error = checkOptionForFloat(option1, &argumentsCopy, setting1, false);
    if (option1Error != "")
        return option1Error;
    QString option2Error = checkOptionForFloat(option2, &argumentsCopy, setting2, false);
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
    checkOptionForFloat(option1, arguments, setting1, false);
    checkOptionForFloat(option2, arguments, setting2, false);
    return "";
}



bool isOptionPresent(QString option, QStringList * arguments)
{
    return (arguments->indexOf(option) > -1);
}

bool isOptionAndValuePresent(QString option, QString value, QStringList * arguments)
{
    int optionIndex = arguments->indexOf(option);
    if (optionIndex == -1)
        return false;

    int valueIndex = optionIndex + 1;
    if (valueIndex >= arguments->size())
        return false;

    QString optionValue = arguments->at(valueIndex);
    return (optionValue == value);
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


SciNot getSciNotOption(QString option, QStringList * arguments)
{
     int optionIndex = arguments->indexOf(option);
     if (optionIndex == -1)
         return 0;

     int sciNotIndex = optionIndex + 1;
     if (sciNotIndex >= arguments->size())
         return SciNot();

     return SciNot(arguments->at(sciNotIndex));
}

NodeColourScheme getColourSchemeOption(QString option, QStringList * arguments)
{
    NodeColourScheme defaultScheme = RANDOM_COLOURS;
    if (isOptionPresent("--query", arguments))
        defaultScheme = BLAST_HITS_SOLID_COLOUR;

    int optionIndex = arguments->indexOf(option);
    if (optionIndex == -1)
        return defaultScheme;

    int colourIndex = optionIndex + 1;
    if (colourIndex >= arguments->size())
        return defaultScheme;

    QString colourString = arguments->at(colourIndex).toLower();
    if (colourString == "random")
        return RANDOM_COLOURS;
    else if (colourString == "uniform")
        return UNIFORM_COLOURS;
    else if (colourString == "readdepth")
        return READ_DEPTH_COLOUR;
    else if (colourString == "blastsolid")
        return BLAST_HITS_SOLID_COLOUR;
    else if (colourString == "blastrainbow")
        return BLAST_HITS_RAINBOW_COLOUR;

    //Random colours is the default
    return defaultScheme;
}


GraphScope getGraphScopeOption(QString option, QStringList * arguments)
{
    int optionIndex = arguments->indexOf(option);
    if (optionIndex == -1)
        return WHOLE_GRAPH;

    int scopeIndex = optionIndex + 1;
    if (scopeIndex >= arguments->size())
        return WHOLE_GRAPH;

    QString scopeString = arguments->at(scopeIndex).toLower();
    if (scopeString == "entire")
        return WHOLE_GRAPH;
    else if (scopeString == "aroundnodes")
        return AROUND_NODE;
    else if (scopeString == "aroundblast")
        return AROUND_BLAST_HITS;
    else if (scopeString == "depthrange")
        return READ_DEPTH_RANGE;

    //Entire graph scope is the default.
    return WHOLE_GRAPH;
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


QString getStringOption(QString option, QStringList * arguments)
{
    int optionIndex = arguments->indexOf(option);
    if (optionIndex == -1)
        return "";

    int stringIndex = optionIndex + 1;
    if (stringIndex >= arguments->size())
        return "";

    return arguments->at(stringIndex);
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


void getCommonHelp(QStringList * text)
{
    *text << "--help              View this help message";
    *text << "--helpall           View all command line settings";
    *text << "--version           View Bandage version number";
    *text << "";
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


bool createBlastTempDirectory()
{
    //Running from the command line, it makes more sense to put the temp
    //directory in the current directory.
    g_blastSearch->m_tempDirectory = "bandage_temp-" + QString::number(QApplication::applicationPid()) + "/";

    if (!QDir().mkdir(g_blastSearch->m_tempDirectory))
        return false;

    g_blastSearch->m_blastQueries.createTempQueryFiles();
    return true;
}

void deleteBlastTempDirectory()
{
    if (g_blastSearch->m_tempDirectory != "" &&
            QDir(g_blastSearch->m_tempDirectory).exists() &&
            QDir(g_blastSearch->m_tempDirectory).dirName().contains("bandage_temp"))
        QDir(g_blastSearch->m_tempDirectory).removeRecursively();
}



QString getElapsedTime(QDateTime start, QDateTime end)
{
    int msecElapsed = start.msecsTo(end);
    int secElapsed = msecElapsed / 1000;
    msecElapsed = msecElapsed % 1000;
    int minElapsed = secElapsed / 60;
    secElapsed = secElapsed % 60;
    int hoursElapsed = minElapsed / 60;
    minElapsed = minElapsed % 60;

    QString msecString = QString("%1").arg(msecElapsed, 2, 10, QChar('0'));
    QString secString = QString("%1").arg(secElapsed, 2, 10, QChar('0'));
    QString minString = QString("%1").arg(minElapsed, 2, 10, QChar('0'));
    QString hourString = QString("%1").arg(hoursElapsed, 2, 10, QChar('0'));

    return hourString + ":" + minString + ":" + secString + "." + msecString;
}


QStringList wrapText(QString text, int width, int firstLineIndent, int laterLineIndent)
{
    QStringList returnList;

    QString firstLineSpaces = "";
    for (int i = 0; i < firstLineIndent; ++i)
        firstLineSpaces += ' ';
    QString laterLineSpaces = "";
    for (int i = 0; i < laterLineIndent; ++i)
        laterLineSpaces += ' ';

    text = firstLineSpaces + text;

    //If the terminal width is at the minimum, don't bother wrapping.
    if (g_memory->terminalWidth <= 50)
    {
        returnList << text;
        return returnList;
    }

    while (text.length() > width)
    {
        QString leftString = text.left(width);
        int spaceIndex = leftString.lastIndexOf(' ');
        if (spaceIndex < width / 2)
            spaceIndex = width;

        leftString = text.left(spaceIndex);
        returnList << rstrip(leftString);
        text = laterLineSpaces + text.mid(spaceIndex).trimmed();
    }

    returnList << text;
    return returnList;
}

//http://stackoverflow.com/questions/8215303/how-do-i-remove-trailing-whitespace-from-a-qstring
QString rstrip(const QString& str)
{
    int n = str.size() - 1;
    for (; n >= 0; --n)
    {
        if (!str.at(n).isSpace())
            return str.left(n + 1);
    }
    return "";
}

QString getRangeAndDefault(IntSetting setting) {return (setting.on) ? getRangeAndDefault(setting.min, setting.max, setting.val) : getRangeAndDefault(setting.min, setting.max, "off");}
QString getRangeAndDefault(IntSetting setting, QString defaultVal) {return getRangeAndDefault(setting.min, setting.max, defaultVal);}
QString getRangeAndDefault(FloatSetting setting) {return (setting.on) ? getRangeAndDefault(setting.min, setting.max, setting.val) : getRangeAndDefault(setting.min, setting.max, "off");}
QString getRangeAndDefault(FloatSetting setting, QString defaultVal) {return getRangeAndDefault(setting.min, setting.max, defaultVal);}
QString getRangeAndDefault(SciNotSetting setting) {return (setting.on) ? getRangeAndDefault(setting.min.asString(true), setting.max.asString(true), setting.val.asString(true)) : getRangeAndDefault(setting.min.asString(true), setting.max.asString(true), "off");}
QString getRangeAndDefault(int min, int max, int defaultVal) { return getRangeAndDefault(double(min), double(max), QString::number(defaultVal));}
QString getRangeAndDefault(int min, int max, QString defaultVal) {return getRangeAndDefault(double(min), double(max), defaultVal);}
QString getRangeAndDefault(double min, double max, double defaultVal) {return getRangeAndDefault(min, max, QString::number(defaultVal));}
QString getRangeAndDefault(double min, double max, QString defaultVal) {return getRangeAndDefault(QString::number(min), QString::number(max), defaultVal);}

QString getRangeAndDefault(QString min, QString max, QString defaultVal)
{
    return "(" + min + " to " + max + ", default: " + defaultVal + ")";
}

QString getDefaultColour(QColor colour)
{
    return "(default: " + getColourName(colour.name()) + ")";
}

QString getBandageTitleAsciiArt()
{
    return "  ____                  _                  \n |  _ \\                | |                 \n | |_) | __ _ _ __   __| | __ _  __ _  ___ \n |  _ < / _` | '_ \\ / _` |/ _` |/ _` |/ _ \\\n | |_) | (_| | | | | (_| | (_| | (_| |  __/\n |____/ \\__,_|_| |_|\\__,_|\\__,_|\\__, |\\___|\n                                 __/ |     \n                                |___/      ";
}

bool isOption(QString text)
{
    bool option = (text.length() > 2 && text[0] == '-' && text[1] == '-' && text[2] != '-');

    QRegExp rx("^<[\\w_]+>");
    bool positional = (rx.indexIn(text) != -1);

    return option || positional;
}

bool isSectionHeader(QString text)
{
    //Make an exception:
    if (text.startsWith("Node widths are determined"))
        return false;

    QRegExp rx("^[\\w ]+:");
    return (rx.indexIn(text) != -1);
}


bool isListItem(QString text)
{
    return (text.length() > 1 && text[0] == '*' && text[1] == ' ');
}

bool isCommand(QString text)
{
    return text.startsWith("load   ") ||
            text.startsWith("info   ") ||
            text.startsWith("image   ") ||
            text.startsWith("querypaths   ") ||
            text.startsWith("reduce   ");
}


bool isError(QString text)
{
    return text.startsWith("Bandage error");
}

void getOnlineHelpMessage(QStringList * text)
{
    *text << "Online Bandage help: https://github.com/rrwick/Bandage/wiki";
    *text << "";
}

