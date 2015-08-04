#ifndef MEMORY_H
#define MEMORY_H

#include "../program/globals.h"
#include "../graph/path.h"

class Memory
{
public:
    Memory();

    QString rememberedPath;

    CommandLineCommand commandLineCommand;

    QString blastSearchParameters;

    //This flag is true when the 'Specify exact path...' dialog is visible.
    bool pathDialogIsVisible;

    //These store the user input in the 'Specify exact path...' dialog so it is
    //retained between uses.
    Path userSpecifiedPath;
    QString userSpecifiedPathString;
    bool userSpecifiedPathCircular;
};

#endif // MEMORY_H
