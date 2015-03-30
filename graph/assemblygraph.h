#ifndef ASSEMBLYGRAPH_H
#define ASSEMBLYGRAPH_H

#include <vector>
#include "ogdf/basic/Graph.h"
#include "ogdf/basic/GraphAttributes.h"
#include <QString>
#include <QMap>

class DeBruijnNode;
class DeBruijnEdge;

class AssemblyGraph
{
public:
    AssemblyGraph();
    ~AssemblyGraph();

    QMap<long long, DeBruijnNode*> m_deBruijnGraphNodes;
    std::vector<DeBruijnEdge*> m_deBruijnGraphEdges;

    ogdf::Graph * m_ogdfGraph;
    ogdf::GraphAttributes * m_graphAttributes;

    int m_nodeCount;
    int m_edgeCount;
    long long m_totalLength;
    long long m_shortestContig;
    long long m_longestContig;
    double m_meanCoverage;
    double m_firstQuartileCoverage;
    double m_medianCoverage;
    double m_thirdQuartileCoverage;

    void cleanUp();
    void createDeBruijnEdge(long long node1Number, long long node2Number);
    void clearOgdfGraphAndResetNodes();
    QByteArray getReverseComplement(QByteArray forwardSequence);
    void resetEdges();
    double getMeanDeBruijnGraphCoverage(bool drawnNodesOnly = false);
    double getMaxDeBruijnGraphCoverageOfDrawnNodes();
    void resetNodeContiguityStatus();
    void resetAllNodeColours();
    void clearAllBlastHitPointers();
    void determineGraphInfo();
    void clearGraphInfo();

private:
    double getValueUsingFractionalIndex(std::vector<double> * doubleVector, double index);
};

#endif // ASSEMBLYGRAPH_H
