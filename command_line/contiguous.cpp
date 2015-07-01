#include "contiguous.h"
#include "command_line/commoncommandlinefunctions.h"
#include "../program/settings.h"

int bandageContiguous(QStringList arguments)
{
    if (checkForHelp(arguments))
    {
        printContiguousUsage();
        return 0;
    }

    if (arguments.size() < 2)
    {
        printContiguousUsage();
        return 1;
    }

    QString graphFile = arguments.at(0);
    arguments.pop_front();

    QString targetFile = arguments.at(0);
    arguments.pop_front();

    QString error = checkForInvalidContiguousOptions(arguments);
    if (error.length() > 0)
    {
        QTextStream(stdout) << "" << endl << "Error: " << error << endl;
        printContiguousUsage();
        return 1;
    }

    parseContiguousOptions(arguments);

    g_settings = new Settings();

    bool loadSuccess = loadAssemblyGraph(graphFile);
    if (!loadSuccess)
        return 1;









    return 0;
}



void printContiguousUsage()
{
    QTextStream(stdout) << "" << endl;
    QTextStream(stdout) << "Usage:   Bandage contiguous <graph> <target> [options]" << endl << endl;
    QTextStream(stdout) << "Options: " << endl << endl;
}



QString checkForInvalidContiguousOptions(QStringList arguments)
{
    return checkForExcessArguments(arguments);
}



void parseContiguousOptions(QStringList arguments)
{


}

