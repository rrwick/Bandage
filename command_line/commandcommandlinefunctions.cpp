#include "commandcommandlinefunctions.h"
#include "../graph/assemblygraph.h"

bool loadAssemblyGraph(QString filename)
{
    g_assemblyGraph = new AssemblyGraph();
    return g_assemblyGraph->loadGraphFromFile(filename);
}
