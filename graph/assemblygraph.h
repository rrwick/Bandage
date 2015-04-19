//Copyright 2015 Ryan Wick

//This file is part of Bandage

//Bandage is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//Bandage is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

//You should have received a copy of the GNU General Public License
//along with Bandage.  If not, see <http://www.gnu.org/licenses/>.


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
    bool m_trinityGraph;
    bool m_contiguitySearchDone;

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
