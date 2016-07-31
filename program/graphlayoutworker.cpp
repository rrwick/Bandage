//Copyright 2016 Ryan Wick

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


#include "graphlayoutworker.h"
#include <time.h>
#include "ogdf/basic/geometry.h"
#include <QLineF>

GraphLayoutWorker::GraphLayoutWorker(ogdf::FMMMLayout * fmmm, ogdf::GraphAttributes * graphAttributes,
                                     ogdf::EdgeArray<double> * edgeArray,
                                     int graphLayoutQuality) :
    m_fmmm(fmmm), m_graphAttributes(graphAttributes), m_edgeArray(edgeArray),
    m_graphLayoutQuality(graphLayoutQuality)
{
}


void GraphLayoutWorker::layoutGraph()
{
    m_fmmm->randSeed(clock());
    m_fmmm->useHighLevelOptions(false);
    m_fmmm->initialPlacementForces(ogdf::FMMMLayout::ipfRandomRandIterNr);
    m_fmmm->unitEdgeLength(1.0);
    m_fmmm->allowedPositions(ogdf::FMMMLayout::apAll);

    switch (m_graphLayoutQuality)
    {
    case 0:
        m_fmmm->fixedIterations(3);
        m_fmmm->fineTuningIterations(1);
        m_fmmm->nmPrecision(2);
        break;
    case 1:
        m_fmmm->fixedIterations(12);
        m_fmmm->fineTuningIterations(8);
        m_fmmm->nmPrecision(2);
        break;
    case 2:
        m_fmmm->fixedIterations(30);
        m_fmmm->fineTuningIterations(20);
        m_fmmm->nmPrecision(4);
        break;
    case 3:
        m_fmmm->fixedIterations(60);
        m_fmmm->fineTuningIterations(40);
        m_fmmm->nmPrecision(6);
        break;
    case 4:
        m_fmmm->fixedIterations(120);
        m_fmmm->fineTuningIterations(80);
        m_fmmm->nmPrecision(8);
        break;
    }

    m_fmmm->call(*m_graphAttributes, *m_edgeArray);

    // Now that the actual OGDF layout is done, we check for a few common quirks that we can manually fix up.
    fixTwistedSplits(m_graphAttributes);

    emit finishedLayout();
}


void GraphLayoutWorker::fixTwistedSplits(ogdf::GraphAttributes * graphAttributes) {
    const ogdf::Graph &G = graphAttributes->constGraph();
    ogdf::node v;
    forall_nodes(v, G) {

        // In order to be a simple two-way split, this node must lead to three others, two of which merge back
        // together in the same number of steps.
        std::vector<ogdf::node> adjacentNodes = getAdjacentNodes(v);
        if (adjacentNodes.size() == 3) {
            ogdf::node direction1Finish;
            std::vector<ogdf::node> direction1Path;
            int direction1Steps;
            followNodesUntilBranch(v, adjacentNodes[0], &direction1Finish, &direction1Path, &direction1Steps);
            ogdf::node direction2Finish;
            std::vector<ogdf::node> direction2Path;
            int direction2Steps;
            followNodesUntilBranch(v, adjacentNodes[1], &direction2Finish, &direction2Path, &direction2Steps);
            ogdf::node direction3Finish;
            std::vector<ogdf::node> direction3Path;
            int direction3Steps;
            followNodesUntilBranch(v, adjacentNodes[2], &direction3Finish, &direction3Path, &direction3Steps);

            std::vector<ogdf::node> * path1 = 0;
            std::vector<ogdf::node> * path2 = 0;
            if (direction1Finish == direction2Finish && direction1Steps == direction2Steps && direction1Finish != direction3Finish) {
                path1 = &direction1Path;
                path2 = &direction1Path;
            }
            else if (direction1Finish == direction3Finish && direction1Steps == direction3Steps && direction1Finish != direction2Finish) {
                path1 = &direction1Path;
                path2 = &direction3Path;
            }
            else if (direction2Finish == direction3Finish && direction2Steps == direction3Steps && direction2Finish != direction1Finish) {
                path1 = &direction2Path;
                path2 = &direction3Path;
            }
            if (path1 != 0 && path2 != 0 && path1->size() > 1 && path2->size() > 1) {
                // If we got here, that means we've found a simple split! path1 and path2 store the nodes in order, so
                // we check if any of them cross, and if so, we swap their positions to uncross them.
                for (int i = 0; i < path1->size() - 1; ++i) {
                    ogdf::node path1Node1 = (*path1)[i];
                    ogdf::node path1Node2 = (*path1)[i+1];
                    ogdf::node path2Node1 = (*path2)[i];
                    ogdf::node path2Node2 = (*path2)[i+1];
                    QPointF path1Node1Location(graphAttributes->x(path1Node1), graphAttributes->y(path1Node1));
                    QPointF path1Node2Location(graphAttributes->x(path1Node2), graphAttributes->y(path1Node2));
                    QPointF path2Node1Location(graphAttributes->x(path2Node1), graphAttributes->y(path2Node1));
                    QPointF path2Node2Location(graphAttributes->x(path2Node2), graphAttributes->y(path2Node2));
                    QLineF line1(path1Node1Location, path1Node2Location);
                    QLineF line2(path2Node1Location, path2Node2Location);
                    QPointF intersectionPoint;
                    if (line1.intersect(line2, &intersectionPoint) == QLineF::BoundedIntersection) {
                        graphAttributes->x(path1Node2) = path2Node2Location.x();
                        graphAttributes->y(path1Node2) = path2Node2Location.y();
                        graphAttributes->x(path2Node2) = path1Node2Location.x();
                        graphAttributes->y(path2Node2) = path1Node2Location.y();
                    }
                }
            }
        }
    }
}


std::vector<ogdf::node> GraphLayoutWorker::getAdjacentNodes(ogdf::node v) {
    std::vector<ogdf::node> adjacentNodes;
    ogdf::edge e;
    forall_adj_edges(e, v) {
        if (e->source() != v)
            adjacentNodes.push_back(e->source());
        if (e->target() != v)
            adjacentNodes.push_back(e->target());
    }
    return adjacentNodes;
}


std::vector<ogdf::node> GraphLayoutWorker::getAdjacentNodesExcluding(ogdf::node v, ogdf::node ex) {
    std::vector<ogdf::node> adjacentNodes;
    ogdf::edge e;
    forall_adj_edges(e, v) {
        if (e->source() != v && e->source() != ex)
            adjacentNodes.push_back(e->source());
        if (e->target() != v && e->source() != ex)
            adjacentNodes.push_back(e->target());
    }
    return adjacentNodes;
}

void GraphLayoutWorker::followNodesUntilBranch(ogdf::node start, ogdf::node first,
                                               ogdf::node * finish, std::vector<ogdf::node> * path, int * steps) {
    ogdf::node prev = start;
    ogdf::node current = first;
    *steps = 0;
    while (true) {
        std::vector<ogdf::node> adjacentNodes = getAdjacentNodesExcluding(current, prev);
        if (adjacentNodes.size() != 1)
            break;
        prev = current;
        current = adjacentNodes[0];
        *steps += 1;
        path->push_back(prev);
    }
    *finish = current;
}
