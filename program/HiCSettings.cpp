#include "HiCSettings.h"
#include "../graph/debruijnnode.h"
#include <stdlib.h>

HiCSettings::HiCSettings() : minWeight(0), minLength(1), minDist(1) {}
HiCSettings::~HiCSettings() {}

bool HiCSettings::isDrawnWithNode(DeBruijnEdge* edge) {
    return isDrawnWithNode(edge, this->inclusionFilter);

}

bool HiCSettings::isDrawnWithNode(DeBruijnEdge* edge, HiCInclusionFilter filterHiC) {
    edge->determineIfDrawn();
    bool drawEdge = edge->isDrawn();
    if (!drawEdge)
        return false;
    bool res = (edge->getWeight() >= minWeight &&
        edge->getStartingNode()->getLength() >= minLength &&
        edge->getEndingNode()->getLength() >= minLength &&
        isValidContigLength(edge) &&
        (filterHiC == ALL ||
            filterHiC == ALL_BETWEEN_GRAPH_COMPONENTS && edge->getStartingNode()->getComponentId() != edge->getEndingNode()->getComponentId() ||
            filterHiC == ONE_BETWEEN_GRAPH_COMPONENT && contains(edge) ||
            filterHiC == ONE_FROM_TARGET_COMPONENT && contains(edge) && 
                (isTargetComponent(edge->getStartingNode()->getComponentId()) || isTargetComponent(edge->getEndingNode()->getComponentId()))));
    return res;

}

bool HiCSettings::isDrawn(DeBruijnEdge* edge) {
    return isDrawn(edge, inclusionFilter);

}

bool HiCSettings::isDrawn(DeBruijnEdge* edge, HiCInclusionFilter filterHiC) {
    bool res(edge->getWeight() >= minWeight &&
        edge->getStartingNode()->getLength() >= minLength &&
        edge->getEndingNode()->getLength() >= minLength &&
        isValidContigLength(edge) &&
        (filterHiC == ALL ||
            filterHiC == ALL_BETWEEN_GRAPH_COMPONENTS && edge->getStartingNode()->getComponentId() != edge->getEndingNode()->getComponentId() ||
            filterHiC == ONE_BETWEEN_GRAPH_COMPONENT && contains(edge) ||
            filterHiC == ONE_FROM_TARGET_COMPONENT && contains(edge) &&
            (isTargetComponent(edge->getStartingNode()->getComponentId()) || isTargetComponent(edge->getEndingNode()->getComponentId()))));
    return res;

}

bool HiCSettings::addEdgeIfNeeded(DeBruijnEdge* edge) {
    if (!edge->isHiC()) {
        return false;
    }
    QPair<int, int> key = getComponentKey(edge);
    int startingComponentId = edge->getStartingNode()->getComponentId();
    int endingComponentId = edge->getEndingNode()->getComponentId();
    if (startingComponentId != endingComponentId &&
        isBigComponent(startingComponentId) &&
        isBigComponent(endingComponentId) &&
        isValidContigLength(edge)) {
        sumWeightBetweenComponent += edge->getWeight();
        countOfEdgesBetweenComponent += 1;
        if (!componentEdgeMap.contains(key)) {
            componentEdgeMap[key] = edge;
            return true;
        }
        else if (componentEdgeMap[key]->getWeight() < edge -> getWeight()) {
            componentEdgeMap[key] = edge;
            return true;
        }
    } 
    return false;
}

bool HiCSettings::isValidContigLength(DeBruijnEdge* edge) {
    return !checkContigLength || max(1000, averageSize[edge->getStartingNode()->getComponentId() - 1]) <= edge->getStartingNode()->getLength() &&
        max(1000, averageSize[edge->getEndingNode()->getComponentId() - 1]) <= edge->getEndingNode()->getLength();
}

bool HiCSettings::contains(DeBruijnEdge* edge) {
    if (!edge->isHiC())
        return false;
    QPair<int, int> key = getComponentKey(edge);
    return componentEdgeMap.contains(key) &&
        componentEdgeMap[key] == edge;
}

void HiCSettings::addTargetComponentIfNeeded(int id) {
    if (!targetComponents.contains(id)) {
        targetComponents.push_back(id);
    }
}
bool HiCSettings::isTargetComponent(int componentId) {
    return targetComponents.contains(componentId);
}

bool HiCSettings::isConnectedWithTargetComponent(int componentId) {
    if (isTargetComponent(componentId)) {
        return true;
    }
    for (int targetComponentId : g_hicSettings->targetComponents) {
        QPair<int, int> key = qMakePair(componentId, targetComponentId);
        QPair<int, int> reverseKey = qMakePair(targetComponentId, componentId);
        if ((g_hicSettings->componentEdgeMap.contains(key) /*&& g_hicSettings->componentEdgeMap[key]->getWeight() >= g_hicSettings->minWeight*/ ) ||
            (g_hicSettings->componentEdgeMap.contains(reverseKey) /*&& g_hicSettings->componentEdgeMap[reverseKey]->getWeight() >= g_hicSettings->minWeight*/)) {
            return true;
        }
    }
    return false;
}

bool HiCSettings::isBigComponent(int componentId) {
    return componentSize[componentId - 1] >= 500;
}

QPair<int, int> HiCSettings::getComponentKey(DeBruijnEdge* edge) {
    if (edge->getStartingNode()->isPositiveNode() && edge->getEndingNode()->isPositiveNode())
        return qMakePair(std::min(edge->getStartingNode()->getComponentId(), edge->getEndingNode()->getComponentId()), 
            std::max(edge->getStartingNode()->getComponentId(), edge->getEndingNode()->getComponentId()));
    else 
        return qMakePair(std::max(edge->getStartingNode()->getComponentId(), edge->getEndingNode()->getComponentId()),
            std::min(edge->getStartingNode()->getComponentId(), edge->getEndingNode()->getComponentId()));
}
