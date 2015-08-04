#include "memory.h"

#include <QDir>

Memory::Memory()
{

    rememberedPath = QDir::homePath();

    commandLineCommand = NO_COMMAND;

}

