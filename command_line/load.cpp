#include "load.h"

int launchBandageAndLoadFile(QApplication * a, QStringList arguments)
{
    if (arguments.size() == 0)
    {
        QTextStream(stdout) << "" << endl;
        QTextStream(stdout) << "Usage:   Bandage load <filename>" << endl << endl;
        QTextStream(stdout) << "Options: -d    draw graph after loading" << endl << endl;
        return 1;
    }

    QString filename = arguments.at(0);

    MainWindow w(filename);
    w.show();
    return a->exec();
}

