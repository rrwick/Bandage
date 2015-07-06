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

int bandageImage(QStringList arguments)
{
    //Since frame rate performance doesn't matter for a fixed image, set the
    //default node outline to a nonzero value.
    g_settings->outlineThickness = 0.3;

    QTextStream out(stdout);
    QTextStream err(stdout);

    if (checkForHelp(arguments))
    {
        printImageUsage(&out, false);
        return 0;
    }

    if (isOptionPresent("--helpall", &arguments))
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
        err << "Bandage error: the output filename must end in .png, .jpg or .svg" << endl;
        return 1;
    }

    QString error = checkForInvalidImageOptions(arguments);
    if (error.length() > 0)
    {
        err << "Bandage error: " << error << endl;
        return 1;
    }

    bool loadSuccess = loadAssemblyGraph(graphFilename);
    if (!loadSuccess)
        return 1;

    MyGraphicsScene scene;

    int width = 0;
    int height = 0;
    parseImageOptions(arguments, &width, &height);

    //CURRENTLY FIXED AS WHOLE_GRAPH, THOUGH I WOULD LIKE TO ADD
    //SUPPORT FOR AROUND_BLAST_HITS
    g_settings->graphScope = WHOLE_GRAPH;
    int nodeDistance = 0;
    std::vector<DeBruijnNode *> startingNodes;

    g_assemblyGraph->buildOgdfGraphFromNodesAndEdges(startingNodes, nodeDistance);
    layoutGraph();

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

    if (!success)
    {
        out << "There was an error writing the image to file." << endl;
        return 1;
    }
    else
        return 0;
}


void printImageUsage(QTextStream * out, bool all)
{
    *out << endl;
    *out << "Usage:    Bandage image <graphfile> <outputfile> [options]" << endl;
    *out << endl;
    *out << "Options:  --height <int>      image height (default: 1000)" << endl;
    *out << "          --width <int>       image width (default: not set)" << endl;
    *out << "                              If only height or width is set, the other will be" << endl;
    *out << "                              determined automatically. If both are set, the" << endl;
    *out << "                              image will be exactly that size." << endl;
    *out << endl;
//           ------------------------------|------------------------------------------------|  //80 character guide
    printCommonHelp(out);
    if (all)
        printSettingsUsage(out);
}

QString checkForInvalidImageOptions(QStringList arguments)
{
    QString  error = checkOptionForInt("--height", &arguments, 1, 32767);
    if (error.length() > 0) return error;

    error = checkOptionForInt("--width", &arguments, 1, 32767);
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

