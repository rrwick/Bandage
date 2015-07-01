#include "contiguous.h"
#include "command_line/commoncommandlinefunctions.h"

int bandageContiguous(QStringList arguments)
{
    if (arguments.size() < 2)
    {
        printContiguousUsage();
        return 1;
    }

    QString graphFile = arguments.at(0);
    arguments.pop_front();

    QString targetFile = arguments.at(0);
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
        voidPrintInvalidContiguousOptions(invalidOptions);
        return 1;
    }

    bool loadSuccess = loadAssemblyGraph(graphFile);
    if (!loadSuccess)
        return 1;









    return 0;
}



void voidPrintInvalidContiguousOptions(QStringList invalidOptions)
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
    printContiguousUsage();
    QTextStream(stdout) << "" << endl;
}

void printContiguousUsage()
{
    QTextStream(stdout) << "" << endl;
    QTextStream(stdout) << "Usage:   Bandage contiguous <graph> <target> [options]" << endl << endl;
    QTextStream(stdout) << "Options: " << endl << endl;
}

