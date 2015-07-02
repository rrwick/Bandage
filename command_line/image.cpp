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
#include "../graph/assemblygraph.h"
#include <vector>
#include "../program/settings.h"
#include <QPainter>

int bandageImage(QStringList arguments)
{
    QTextStream out(stdout);
    QTextStream err(stdout);

    if (checkForHelp(arguments))
    {
        printImageUsage(&out);
        return 0;
    }

    if (arguments.size() < 2)
    {
        printImageUsage(&err);
        return 1;
    }

    QString graphFilename = arguments.at(0);
    arguments.pop_front();

    QString imageSaveFilename = arguments.at(0);
    arguments.pop_front();

    QString imageFileExtension = imageSaveFilename.right(4);
    if (imageFileExtension != ".png" && imageFileExtension != ".jpg")
    {
        err << "" << endl << "Error: the output filename must end in .png or .jpg" << endl;
        printImageUsage(&err);
        return 1;
    }

    QString error = checkForInvalidImageOptions(arguments);
    if (error.length() > 0)
    {
        err << "" << endl << "Error: " << error << endl;
        printImageUsage(&err);
        return 1;
    }

    g_settings = new Settings();

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

    QImage image(width, height, QImage::Format_ARGB32);
    image.fill(Qt::white);
    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing);
    scene.render(&painter);

    bool success = image.save(imageSaveFilename);
    if (!success)
    {
        out << "There was an error writing the image to file." << endl;
        return 1;
    }
    return 0;
}


void printImageUsage(QTextStream * out)
{
    *out << "" << endl;
    *out << "Usage:   Bandage image <graphfile> <outputfile> [options]" << endl << endl;
    *out << "Options: --height <int> image height (default: 1000)" << endl;
    *out << "         --width <int>  image width (default: not set)" << endl;
    *out << "                        If only height or width is set, the other will be" << endl;
    *out << "                        determined automatically. If both are set, the image" << endl;
    *out << "                        will be exactly that size." << endl;
//           --------------------------------------------------------------------------------  //80 character guide
    *out << "         --double       draw graph in double mode (default: off)" << endl;
    *out << "         --bases <int>  base pairs per segment (default: auto)" << endl;
    *out << "                        High values result in longer nodes, small values in" << endl;
    *out << "                        shorter nodes." << endl;
    *out << "         --qual <int>   graph layout quality, 1 (low) to 5 (high) (default: 3)" << endl << endl;
//           --------------------------------------------------------------------------------  //80 character guide
}

QString checkForInvalidImageOptions(QStringList arguments)
{
    QString error = "";

    error = checkOptionForInt("--height", &arguments, 1, 32767);
    if (error.length() > 0) return error;

    error = checkOptionForInt("--width", &arguments, 1, 32767);
    if (error.length() > 0) return error;

    error = checkOptionForInt("--bases", &arguments, 1, std::numeric_limits<int>::max());
    if (error.length() > 0) return error;

    error = checkOptionForInt("--qual", &arguments, 1, 5);
    if (error.length() > 0) return error;

    checkOptionWithoutValue("--double", &arguments);

    return checkForExcessArguments(arguments);
}



void parseImageOptions(QStringList arguments, int * width, int * height)
{
    int heightIndex = arguments.indexOf("--height") + 1;
    if (heightIndex != 0 && heightIndex < arguments.size())
        *height = arguments.at(heightIndex).toInt();
    int widthIndex = arguments.indexOf("--width") + 1;
    if (widthIndex != 0 && widthIndex < arguments.size())
        *width = arguments.at(widthIndex).toInt();

    int basePairsPerSegmentIndex = arguments.indexOf("--bases") + 1;
    if (basePairsPerSegmentIndex != 0 && basePairsPerSegmentIndex < arguments.size())
    {
        g_settings->manualBasePairsPerSegment = arguments.at(basePairsPerSegmentIndex).toInt();
        g_settings->nodeLengthMode = MANUAL_NODE_LENGTH;
    }

    int qualityIndex = arguments.indexOf("--qual") + 1;
    if (qualityIndex != 0 && qualityIndex < arguments.size())
    {
        int quality = arguments.at(qualityIndex).toInt() - 1;
        if (quality < 0)
            quality = 0;
        if (quality > 4)
            quality = 4;
        g_settings->graphLayoutQuality = quality;
    }

    int doubleIndex = arguments.indexOf("--double");
    g_settings->doubleMode = (doubleIndex > -1);
}

