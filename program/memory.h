#ifndef MEMORY_H
#define MEMORY_H

#include "../program/globals.h"
#include <QList>
#include <QStringList>
#include "../graph/querydistance.h"

class Memory
{
public:
    Memory();
    void clearGraphSpecificMemory();

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
    QList<QueryDistance> distanceSearchResults;

    //These store the last used distance path search queries/paths.
    QString distancePathSearchQuery1;
    QString distancePathSearchQuery2;
    QString distancePathSearchQuery1Path;
    QString distancePathSearchQuery2Path;
};

#endif // MEMORY_H
