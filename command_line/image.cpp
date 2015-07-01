#include "image.h"
#include "command_line/commoncommandlinefunctions.h"
#include "../program/globals.h"
#include "../ui/mygraphicsscene.h"
#include "../graph/assemblygraph.h"
#include <vector>
#include "../program/settings.h"
#include <QPainter>

int bandageImage(QStringList arguments)
{
    if (arguments.size() < 2)
    {
        printImageUsage();
        return 1;
    }

    QString graphFilename = arguments.at(0);
    arguments.pop_front();

    QString imageSaveFilename = arguments.at(0);
    arguments.pop_front();

    QStringList invalidOptions;
    arguments.removeDuplicates();
    for (int i = 0; i < arguments.size(); ++i)
    {
//        if (arguments.at(i) == "-d")
//            drawGraph = true;
//        else
//            invalidOptions.push_back(arguments.at(i));
    }

    if (invalidOptions.size() > 0)
    {
        voidPrintInvalidImageOptions(invalidOptions);
        return 1;
    }

    g_settings = new Settings();

    bool loadSuccess = loadAssemblyGraph(graphFilename);
    if (!loadSuccess)
        return 1;

    AssemblyGraph * test = g_assemblyGraph;

    MyGraphicsScene scene;

    //SET SCOPE HERE, BASED ON OPTIONS
    g_settings->graphScope = WHOLE_GRAPH;

    //SET DOUBLE MODE HERE, BASED ON OPTIONS
    g_settings->doubleMode = false;

    //SET IMAGE SIZE HERE, BASED ON OPTIONS
    int width = 0;
    int height = 0;

    //SET BASE PAIRS PER SEGMENT HERE, BASED ON OPTIONS


    //IF WE HAVE A REDUCED SCOPE, WE'LL NEED TO SET THE NODE DISTANCE HERE
    int nodeDistance = 0;

    //IF WE HAVE A REDUCED SCOPE, PREPARE STARTING NODES HERE
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

    //If only height or width is set, set that one dimension and scale the
    //other to fit.
    if (height > 0 && width == 0)
        width = height * sceneRectAspectRatio;
    else if (height == 0 && width > 0)
        height = width / sceneRectAspectRatio;

    //If both are set, scale the scene to fit in an image of exactly that
    //size.
    else
    {
        double widthFromHeight = height * sceneRectAspectRatio;
        if (widthFromHeight > width)
            width = widthFromHeight;
        else
        {
            double heightFromWidth = width / sceneRectAspectRatio;
            height = heightFromWidth;
        }
    }

    //Quit if width or height are bad values.
    if (width <= 0 || height <= 0 || width > 32767 || height > 32767)
        return 1;

    QImage image(width, height, QImage::Format_ARGB32);
    image.fill(Qt::white);
    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing);
    scene.render(&painter);
    image.save(imageSaveFilename);

    return 0;
}


void printImageUsage()
{
    QTextStream(stdout) << "" << endl;
    QTextStream(stdout) << "Usage:   Bandage image <graphfile> <outputfile> [options]" << endl << endl;
    QTextStream(stdout) << "Options: " << endl << endl;
}


void voidPrintInvalidImageOptions(QStringList invalidOptions)
{
    QString invalidOptionText = "Invalid option";
    if (invalidOptions.size() > 1)
        invalidOptionText += "s";
    invalidOptionText += ": ";
    for (int i = 0; i < invalidOptions.size(); ++i)
    {
        invalidOptionText += invalidOptions.at(i);
        if (i < invalidOptions.size() - 1)
            invalidOptionText += ", ";
    }

    QTextStream(stdout) << "" << endl;
    QTextStream(stdout) << invalidOptionText << endl;
    printImageUsage();
    QTextStream(stdout) << "" << endl;
}


