#ifndef MEMORY_H
#define MEMORY_H

#include "../program/globals.h"
#include "../graph/path.h"
#include <QList>
#include <QStringList>

class Memory
{
public:
    Memory();
    void clearGraphSpecificMemory();
    void clearDistancePathSearchMemory();

    QString rememberedPath;

    CommandLineCommand commandLineCommand;

    //This flag is true when the 'Specify exact path...' dialog is visible.
    bool pathDialogIsVisible;

    //These store the user input in the 'Specify exact path...' dialog so it is
    //retained between uses.
    Path userSpecifiedPath;
    QString userSpecifiedPathString;
    bool userSpecifiedPathCircular;

    //These store the results of a distance search between two queries.
    QStringList distanceSearchOrientations;
    QList<int> distanceSearchDistances;
    QList<Path> distanceSearchPaths;

    //These store the indices of the distance path search queries.
    int distancePathSearchQuery1;
    int distancePathSearchQuery2;



};

#endif // MEMORY_H
