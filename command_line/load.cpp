#include "load.h"
#include "commoncommandlinefunctions.h"

int bandageLoad(QApplication * a, QStringList arguments)
{
    if (arguments.size() == 0)
    {
        printLoadUsage();
        return 1;
    }

    QString filename = arguments.at(0);
    arguments.pop_front();


    QString error = checkForInvalidLoadOptions(arguments);
    if (error.length() > 0)
    {
        QTextStream(stdout) << "" << endl << "Error: " << error << endl;
        printLoadUsage();
        return 1;
    }

    bool drawGraph;
    parseLoadOptions(arguments, &drawGraph);

    MainWindow w(filename, drawGraph);
    w.show();
    return a->exec();
}


void printLoadUsage()
{
    QTextStream(stdout) << "" << endl;
    QTextStream(stdout) << "Usage:   Bandage load <graph>" << endl << endl;
    QTextStream(stdout) << "Options: -d    draw graph after loading" << endl << endl;
}



QString checkForInvalidLoadOptions(QStringList arguments)
{
    checkOptionWithoutValue("-d", &arguments);

    return checkForExcessArguments(arguments);
}



void parseLoadOptions(QStringList arguments, bool * drawGraph)
{
    int drawIndex = arguments.indexOf("-d");
    *drawGraph = (drawIndex > -1);
}
