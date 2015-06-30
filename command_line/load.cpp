#include "load.h"

int launchBandageAndLoadFile(QApplication * a, QStringList arguments)
{
    if (arguments.size() == 0)
    {
        printLoadUsage();
        return 1;
    }

    QString filename = arguments.at(0);
    arguments.pop_front();
    arguments.removeDuplicates();

    bool drawGraph = false;
    QStringList invalidOptions;

    for (int i = 0; i < arguments.size(); ++i)
    {
        if (arguments.at(i) == "-d")
            drawGraph = true;
        else
            invalidOptions.push_back(arguments.at(i));
    }

    if (invalidOptions.size() > 0)
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
        printLoadUsage();
        QTextStream(stdout) << "" << endl;
        return 1;
    }

    MainWindow w(filename, drawGraph);
    w.show();
    return a->exec();
}


void printLoadUsage()
{
    QTextStream(stdout) << "" << endl;
    QTextStream(stdout) << "Usage:   Bandage load <filename>" << endl << endl;
    QTextStream(stdout) << "Options: -d    draw graph after loading" << endl << endl;
}
