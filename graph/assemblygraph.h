#ifndef ASSEMBLYGRAPH_H
#define ASSEMBLYGRAPH_H

#include <vector>
#include "ogdf/basic/Graph.h"
#include "ogdf/basic/GraphAttributes.h"
#include "../graph/debruijnnode.h"
#include "../graph/debruijnedge.h"
#include "../graph/graphicsitemnode.h"
#include "../graph/graphicsitemedge.h"
#include "../graph/ogdfnode.h"
#include <QString>

class AssemblyGraph
{
public:
    AssemblyGraph();
    ~AssemblyGraph();

    QMap<int, DeBruijnNode*> m_deBruijnGraphNodes;
    std::vector<DeBruijnEdge*> m_deBruijnGraphEdges;

    ogdf::Graph * m_ogdfGraph;
    ogdf::GraphAttributes * m_graphAttributes;


    void cleanUp();
    void createDeBruijnEdge(int node1Number, int node2Number);
    void clearOgdfGraphAndResetNodes();
    QByteArray getReverseComplement(QByteArray forwardSequence);
    void resetEdges();
    double getMeanDeBruijnGraphCoverage(bool drawnNodesOnly = false);
    double getMaxDeBruijnGraphCoverageOfDrawnNodes();
    void resetNodeContiguityStatus();
    void resetAllNodeColours();
    void clearAllBlastHitPointers();
};

#endif // ASSEMBLYGRAPH_H
