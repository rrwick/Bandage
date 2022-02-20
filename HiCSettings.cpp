#include "HiCSettings.h"
#include "../graph/debruijnnode.h"
#include <stdlib.h>

HiCSettings::HiCSettings() : minWeight(0), minLength(1), minDist(1) {}
HiCSettings::~HiCSettings() {}

bool HiCSettings::isDrawn(DeBruijnEdge* edge) {
    edge->determineIfDrawn();
    bool drawEdge = edge->isDrawn();
    if (!drawEdge)
        return false;
    bool res(edge->getWeight() >= minWeight &&
        edge->getStartingNode()->getLength() >= minLength &&
        edge->getEndingNode()->getLength() >= minLength &&
        (filterHiC == 0 ||
            (filterHiC == 1 && edge->getStartingNode()->getComponentId() != edge->getEndingNode()->getComponentId()) ||
            (filterHiC == 2 && contains(edge)))
    );
    return res;

}

bool HiCSettings::addEdgeIfNeeded(DeBruijnEdge* edge) {
    if (edge->getStartingNode()->getComponentId() != edge->getEndingNode()->getComponentId() && edge->isPositiveEdge()) {
        QPair<int, int> key = qMakePair(std::min(edge->getStartingNode()->getComponentId(), edge->getEndingNode()->getComponentId()),
            std::max(edge->getStartingNode()->getComponentId(), edge->getEndingNode()->getComponentId()));
        if (!componentEdgeMap.contains(key)) {
            componentEdgeMap[key] = edge;
            return true;
        }
        if (componentEdgeMap[key]->getStartingNode() == edge->getStartingNode()->getReverseComplement() &&
            componentEdgeMap[key]->getEndingNode() == edge->getEndingNode()->getReverseComplement() ||
            componentEdgeMap[key]->getStartingNode() == edge->getEndingNode()->getReverseComplement() &&
            componentEdgeMap[key]->getEndingNode() == edge->getStartingNode()->getReverseComplement()) {
            QPair<int, int> reverseKey = qMakePair(std::max(edge->getStartingNode()->getComponentId(), edge->getEndingNode()->getComponentId()),
                std::min(edge->getStartingNode()->getComponentId(), edge->getEndingNode()->getComponentId()));
            componentEdgeMap[reverseKey] = edge;
        }
    }
    return false;
}

bool HiCSettings::contains(DeBruijnEdge* edge) {
    QPair<int, int> key = qMakePair(std::min(edge->getStartingNode()->getComponentId(), edge->getEndingNode()->getComponentId()),
        std::max(edge->getStartingNode()->getComponentId(), edge->getEndingNode()->getComponentId()));
    return componentEdgeMap.contains(key) &&
        componentEdgeMap[key] == edge;
}

