#include "memory.h"

#include <QDir>

Memory::Memory()
{

    rememberedPath = QDir::homePath();

    commandLineCommand = NO_COMMAND;

    pathDialogIsVisible = false;

    userSpecifiedPath = Path();
    userSpecifiedPathString = "";
    userSpecifiedPathCircular = false;

    distancePathSearchQuery1 = "";
    distancePathSearchQuery2 = "";
    distancePathSearchQuery1Path = "";
    distancePathSearchQuery2Path = "";
}



//This function clears all memory that is particular to a graph.  It should be
//called whenever a new graph is loaded.
void Memory::clearGraphSpecificMemory()
{
    userSpecifiedPath = Path();
    userSpecifiedPathString = "";
    userSpecifiedPathCircular = false;

    distanceSearchOrientations.clear();
    distanceSearchDistances.clear();
    distanceSearchPaths.clear();

    distancePathSearchQuery1 = "";
    distancePathSearchQuery2 = "";
    distancePathSearchQuery1Path = "";
    distancePathSearchQuery2Path = "";
}

