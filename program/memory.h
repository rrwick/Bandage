#ifndef MEMORY_H
#define MEMORY_H

#include "../program/globals.h"
#include "../graph/path.h"
#include <QList>
#include <QStringList>

class BlastQuery;

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
    QStringList distanceSearchOrientations;
    QList<int> distanceSearchDistances;
    QList<Path> distanceSearchPaths;

    //These store the last used distance path search queries/paths.
    QString distancePathSearchQuery1;
    QString distancePathSearchQuery2;
    QString distancePathSearchQuery1Path;
    QString distancePathSearchQuery2Path;

    //This stores the BLAST query selected by the user when the BLAST search
    //dialog is opened/shown.  It is remembered so the same query can be
    //selected (if possible) when the dialog is closed/hidden.
    BlastQuery * queryBeforeBlastDialog;

};

#endif // MEMORY_H
