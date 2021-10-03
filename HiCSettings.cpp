#include "HiCSettings.h"
#include "../graph/debruijnnode.h"
#include "../graph/debruijnedge.h"

HiCSettings::HiCSettings():numOfNodes(0), minWeight(0), minLength(1), minDist(1) {}
HiCSettings::~HiCSettings() {}

bool HiCSettings::isDrawn(DeBruijnEdge* edge) {
    /*bool drawEdge = (edge->getStartingNode()->isDrawn() || edge->getStartingNode()->getReverseComplement()->isDrawn())
        && (edge->getEndingNode()->isDrawn() || edge->getEndingNode()->getReverseComplement()->isDrawn());*/
    edge->determineIfDrawn();
    bool drawEdge = edge->isDrawn();
    if (!drawEdge)
        return false;
    return (getEdgeWeight(edge) >= minWeight && 
        edge -> getStartingNode() -> getLength() >= minLength &&
        edge->getEndingNode()->getLength() >= minLength);
}

int HiCSettings::getEdgeWeight(DeBruijnEdge* edge) {
    QString startingNodeName = edge->getStartingNode()->getName();
    QString endingNodeName = edge->getEndingNode()->getName();
    int startingNodeInd = -1;
    int endingNodeInd = -1;
    for (int i = 0; i < numOfNodes; i++) {
        if (kontigNames.at(i) == startingNodeName) {
            startingNodeInd = i;
        }
        if (kontigNames.at(i) == endingNodeName) {
            endingNodeInd = i;
        }
    }
    QFile logFile("C\:\\Users\\anastasia\\study\\maga\\Bandage\\my_test_data\\hic_1_1.log");
    logFile.open(QIODevice::WriteOnly);
    logFile.write("getEdgeWeight: " + startingNodeName.toUtf8() + " " + endingNodeName.toUtf8() + "\n");
    logFile.write("getEdgeWeight: " + startingNodeInd + ' ' + endingNodeInd + '\n');
    logFile.close();
    if (startingNodeInd != -1 && endingNodeInd != -1) {
        return weightMatrix.at(startingNodeInd).at(endingNodeInd);
    }
    return 0;
}

