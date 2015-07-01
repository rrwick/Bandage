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

    QString imageFileExtension = imageSaveFilename.right(4);
    if (imageFileExtension != ".png" && imageFileExtension != ".jpg")
    {
        QTextStream(stdout) << "Error: the image filename must end in .png or .jpg" << endl;
        return 1;
    }

    QString error = checkForInvalidOptions(arguments);
    if (error.length() > 0)
    {
        QTextStream(stdout) << "" << endl;
        QTextStream(stdout) << "Error: " << error << endl;
        printImageUsage();
        QTextStream(stdout) << "" << endl;
        return 1;
    }

    g_settings = new Settings();

    bool loadSuccess = loadAssemblyGraph(graphFilename);
    if (!loadSuccess)
        return 1;

    MyGraphicsScene scene;

    int width = 0;
    int height = 0;
    parseOptions(arguments, &width, &height);

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
        QTextStream(stdout) << "There was an error writing the image to file." << endl;
        return 1;
    }
    return 0;
}


void printImageUsage()
{
    QTextStream(stdout) << "" << endl;
    QTextStream(stdout) << "Usage:   Bandage image <graphfile> <outputfile> [options]" << endl << endl;
    QTextStream(stdout) << "Options: -h <int> image height (default: 1000)" << endl;
    QTextStream(stdout) << "         -w <int> image width (default: not set)" << endl;
    QTextStream(stdout) << "                  If only height or width is set, the other will be determined" << endl;
    QTextStream(stdout) << "                  automatically. If both are set, the image will be exactly that" << endl;
    QTextStream(stdout) << "                  size." << endl;
    QTextStream(stdout) << "         -d       draw graph in double mode (default: off)" << endl;
    QTextStream(stdout) << "         -b <int> base pairs per segment (default: auto)" << endl;
    QTextStream(stdout) << "                  High values result in longer nodes, small values in shorter" << endl;
    QTextStream(stdout) << "                  nodes." << endl;
    QTextStream(stdout) << "         -q <int> graph layout quality, 1 (low) to 5 (high) (default: 3)" << endl;
    QTextStream(stdout) << "--------------------------------------------------------------------------------" << endl;
    QTextStream(stdout) << "" << endl;
}

QString checkForInvalidOptions(QStringList arguments)
{
    QString error = "";

    error = checkOptionForInt("-h", &arguments, 1, 32767);
    if (error.length() > 0) return error;

    error = checkOptionForInt("-w", &arguments, 1, 32767);
    if (error.length() > 0) return error;

    error = checkOptionForInt("-b", &arguments, 1, std::numeric_limits<int>::max());
    if (error.length() > 0) return error;

    error = checkOptionForInt("-q", &arguments, 1, 5);
    if (error.length() > 0) return error;

    checkOptionWithoutValue("-d", &arguments);

    return checkForExcessArguments(arguments);
}



void parseOptions(QStringList arguments, int * width, int * height)
{
    int heightIndex = arguments.indexOf("-h") + 1;
    if (heightIndex != 0 && heightIndex < arguments.size())
        *height = arguments.at(heightIndex).toInt();
    int widthIndex = arguments.indexOf("-w") + 1;
    if (widthIndex != 0 && widthIndex < arguments.size())
        *width = arguments.at(widthIndex).toInt();

    int basePairsPerSegmentIndex = arguments.indexOf("-b") + 1;
    if (basePairsPerSegmentIndex != 0 && basePairsPerSegmentIndex < arguments.size())
    {
        g_settings->manualBasePairsPerSegment = arguments.at(basePairsPerSegmentIndex).toInt();
        g_settings->nodeLengthMode = MANUAL_NODE_LENGTH;
    }

    int qualityIndex = arguments.indexOf("-q") + 1;
    if (qualityIndex != 0 && qualityIndex < arguments.size())
    {
        int quality = arguments.at(qualityIndex).toInt() - 1;
        if (quality < 0)
            quality = 0;
        if (quality > 4)
            quality = 4;
        g_settings->graphLayoutQuality = quality;
    }

    int doubleIndex = arguments.indexOf("-d");
    g_settings->doubleMode = (doubleIndex > -1);
}

