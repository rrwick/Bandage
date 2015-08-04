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

    distancePathSearchQuery1 = 0;
    distancePathSearchQuery2 = 1;
}



//This function clears all memory that is particular to a graph.  It should be
//called whenever a new graph is loaded.
void Memory::clearGraphSpecificMemory()
{
    userSpecifiedPath = Path();
    userSpecifiedPathString = "";
    userSpecifiedPathCircular = false;

    clearDistancePathSearchMemory();
}



//This function clears all memory related to a distance path search.  It should
//be called whenever anything changes that would affect what the user would see
//in the distance search dialog.
void Memory::clearDistancePathSearchMemory()
{
    distanceSearchOrientations.clear();
    distanceSearchDistances.clear();
    distanceSearchPaths.clear();

    distancePathSearchQuery1 = 0;
    distancePathSearchQuery2 = 1;
}
