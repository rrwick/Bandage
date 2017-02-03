//Copyright 2017 Ryan Wick

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


#include "image.h"
#include "commoncommandlinefunctions.h"
#include "../program/globals.h"
#include "../ui/mygraphicsscene.h"
#include "../ui/mygraphicsview.h"
#include "../graph/assemblygraph.h"
#include <vector>
#include "../program/settings.h"
#include <QPainter>
#include <QSvgGenerator>
#include <QDir>
#include "../blast/blastsearch.h"

int bandageImage(QStringList arguments)
{
    QTextStream out(stdout);
    QTextStream err(stderr);

    if (checkForHelp(arguments))
    {
        printImageUsage(&out, false);
        return 0;
    }

    if (checkForHelpAll(arguments))
    {
        printImageUsage(&out, true);
        return 0;
    }

    if (arguments.size() < 2)
    {
        printImageUsage(&err, false);
        return 1;
    }

    QString graphFilename = arguments.at(0);
    arguments.pop_front();

    if (!checkIfFileExists(graphFilename))
    {
        outputText("Bandage error: " + graphFilename + " does not exist", &err);
        return 1;
    }

    QString imageSaveFilename = arguments.at(0);
    arguments.pop_front();

    QString imageFileExtension = imageSaveFilename.right(4);
    bool pixelImage;
    if (imageFileExtension == ".png" || imageFileExtension == ".jpg")
        pixelImage = true;
    else if (imageFileExtension == ".svg")
        pixelImage = false;
    else
    {
        outputText("Bandage error: the output filename must end in .png, .jpg or .svg", &err);
        return 1;
    }

    QString error = checkForInvalidImageOptions(arguments);
    if (error.length() > 0)
    {
        outputText("Bandage error: " + error, &err);
        return 1;
    }

    bool loadSuccess = g_assemblyGraph->loadGraphFromFile(graphFilename);
    if (!loadSuccess)
    {
        outputText("Bandage error: could not load " + graphFilename, &err);
        return 1;
    }

    int width = 0;
    int height = 0;

    //Since frame rate performance doesn't matter for a fixed image, set the
    //default node outline to a nonzero value.
    g_settings->outlineThickness = 0.3;

    parseImageOptions(arguments, &width, &height);

    //For Bandage image, it is necessary to position node labels at the
    //centre of the node, not the visible centre(s).  This is because there
    //is no viewport.
    g_settings->positionTextNodeCentre = true;

    bool blastUsed = isOptionPresent("--query", &arguments);

    if (blastUsed)
    {
        if (!createBlastTempDirectory())
        {
            err << "Error creating temporary directory for BLAST files" << endl;
            return 1;
        }

        QString blastError = g_blastSearch->doAutoBlastSearch();

        if (blastError != "")
        {
            err << blastError << endl;
            return 1;
        }
    }

    QString errorTitle;
    QString errorMessage;
    std::vector<DeBruijnNode *> startingNodes = g_assemblyGraph->getStartingNodes(&errorTitle, &errorMessage,
                                                                                  g_settings->doubleMode,
                                                                                  g_settings->startingNodes,
                                                                                  "all");

    QString errormsg;
    QStringList columns;
    bool coloursLoaded = false;
    QString csvPath = parseColorsOption(arguments);
    if (csvPath != "")
    {
        if(!g_assemblyGraph->loadCSV(csvPath, &columns, &errormsg, &coloursLoaded))
        {
            err << errormsg << endl;
            return 1;
        }

        if(coloursLoaded == false)
        {
            err << csvPath << " didn't contains color" << endl;
            return 1;
        }
         g_settings->nodeColourScheme = CUSTOM_COLOURS;
    }

    if (errorMessage != "")
    {
        err << errorMessage << endl;
        return 1;
    }

    g_assemblyGraph->buildOgdfGraphFromNodesAndEdges(startingNodes, g_settings->nodeDistance);
    g_assemblyGraph->layoutGraph();

    MyGraphicsScene scene;
    g_assemblyGraph->addGraphicsItemsToScene(&scene);
    scene.setSceneRectangle();
    double sceneRectAspectRatio = scene.sceneRect().width() / scene.sceneRect().height();

    //Determine image size
    //If neither height nor width set, use a default of height = 1000.
    if (height == 0 && width == 0)
        height = 1000;

    //If only height or width is set, scale the other to fit.
    if (height > 0 && width == 0)
        width = height * sceneRectAspectRatio;
    else if (height == 0 && width > 0)
        height = width / sceneRectAspectRatio;

    bool success = true;
    QPainter painter;
    if (pixelImage)
    {
        QImage image(width, height, QImage::Format_ARGB32);
        image.fill(Qt::white);
        painter.begin(&image);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setRenderHint(QPainter::TextAntialiasing);
        scene.render(&painter);
        success = image.save(imageSaveFilename);
        painter.end();
    }
    else //SVG
    {
        QSvgGenerator generator;
        generator.setFileName(imageSaveFilename);
        generator.setSize(QSize(width, height));
        generator.setViewBox(QRect(0, 0, width, height));
        painter.begin(&generator);
        painter.fillRect(0, 0, width, height, Qt::white);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setRenderHint(QPainter::TextAntialiasing);
        scene.render(&painter);
        painter.end();
    }

    int returnCode;
    if (!success)
    {
        out << "There was an error writing the image to file." << endl;
        returnCode = 1;
    }
    else
        returnCode = 0;

    if (blastUsed)
        deleteBlastTempDirectory();

    return returnCode;
}


void printImageUsage(QTextStream * out, bool all)
{
    QStringList text;

    text << "Bandage image will generate an image file of the graph visualisation without opening the GUI.";
    text << "";
    text << "Usage:    Bandage image <graph> <outputfile> [options]";
    text << "";
    text << "Positional parameters:";
    text << "<graph>             A graph file of any type supported by Bandage";
    text << "<outputfile>        The image file to be created (must end in '.jpg', '.png' or '.svg')";
    text << "";
    text << "Options:  --height <int>      Image height (default: 1000)";
    text << "--width <int>       Image width (default: not set)";
    text << "--color <file>       csv file with 2 column first the node name second the node color";
    text << "";
    text << "If only height or width is set, the other will be determined automatically. If both are set, the image will be exactly that size.";
    text << "";

    getCommonHelp(&text);
    if (all)
        getSettingsUsage(&text);
    getOnlineHelpMessage(&text);

    outputText(text, out);
}

QString checkForInvalidImageOptions(QStringList arguments)
{
    QString error = checkOptionForInt("--height", &arguments, IntSetting(0, 1, 32767), false);
    if (error.length() > 0) return error;

    error = checkOptionForInt("--width", &arguments, IntSetting(0, 1, 32767), false);
    if (error.length() > 0) return error;

    error = checkOptionForString("--colors", &arguments, QStringList(), "a path of csv file");
    if (error.length() > 0) return error;

    return checkForInvalidOrExcessSettings(&arguments);
}




//This function parses the command line options.  It assumes that the options
//have already been checked for correctness.
void parseImageOptions(QStringList arguments, int * width, int * height)
{
    if (isOptionPresent("--height", &arguments))
        *height = getIntOption("--height", &arguments);

    if (isOptionPresent("--width", &arguments))
        *width = getIntOption("--width", &arguments);

    parseSettings(arguments);
}

//This function parses the command line options. It assumes that the options
//have already been checked for correctness.
QString parseColorsOption(QStringList arguments)
{
    QString path = "";
    if (isOptionPresent("--colors", &arguments))
        path = getStringOption("--colors", &arguments);

    parseSettings(arguments);

    return path;
}
