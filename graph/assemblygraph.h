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

#include <QObject>
#include <vector>
#include "ogdf/basic/Graph.h"
#include "ogdf/basic/GraphAttributes.h"
#include <QString>
#include <QMap>
#include "../program/globals.h"
#include "../ui/mygraphicsscene.h"
#include "path.h"

class DeBruijnNode;
class DeBruijnEdge;
class MyProgressDialog;

class AssemblyGraph : public QObject
{
    Q_OBJECT

public:
    AssemblyGraph();
    ~AssemblyGraph();

    //Nodes are stored in a map with a key of the node's name.
    QMap<QString, DeBruijnNode*> m_deBruijnGraphNodes;

    //Edges are stored in a map with a key of the starting and ending node
    //pointers.
    QMap<QPair<DeBruijnNode*, DeBruijnNode*>, DeBruijnEdge*> m_deBruijnGraphEdges;

    ogdf::Graph * m_ogdfGraph;
    ogdf::GraphAttributes * m_graphAttributes;

    int m_kmer;
    int m_nodeCount;
    int m_edgeCount;
    long long m_totalLength;
    long long m_shortestContig;
    long long m_longestContig;
    double m_meanReadDepth;
    double m_firstQuartileReadDepth;
    double m_medianReadDepth;
    double m_thirdQuartileReadDepth;
    GraphFileType m_graphFileType;
    bool m_contiguitySearchDone;

    void cleanUp();
    void createDeBruijnEdge(QString node1Name, QString node2Name,
                            int overlap = 0,
                            EdgeOverlapType overlapType = UNKNOWN_OVERLAP);
    void clearOgdfGraphAndResetNodes();
    static QByteArray getReverseComplement(QByteArray forwardSequence);
    void resetEdges();
    double getMeanReadDepth(bool drawnNodesOnly = false);
    double getMeanReadDepth(std::vector<DeBruijnNode *> nodes);
    double getMeanReadDepth(QList<DeBruijnNode *> nodes);
    void resetNodeContiguityStatus();
    void resetAllNodeColours();
    void clearAllBlastHitPointers();
    void determineGraphInfo();
    void clearGraphInfo();
    void buildDeBruijnGraphFromLastGraph(QString fullFileName);
    void buildDeBruijnGraphFromGfa(QString fullFileName);
    void buildDeBruijnGraphFromFastg(QString fullFileName);
    void buildDeBruijnGraphFromTrinityFasta(QString fullFileName);
    void recalculateAllNodeWidths();

    GraphFileType getGraphFileTypeFromFile(QString fullFileName);
    bool checkFileIsLastGraph(QString fullFileName);
    bool checkFileIsFastG(QString fullFileName);
    bool checkFileIsGfa(QString fullFileName);
    bool checkFileIsTrinityFasta(QString fullFileName);
    bool checkFirstLineOfFile(QString fullFileName, QString regExp);

    bool loadGraphFromFile(QString filename);
    void buildOgdfGraphFromNodesAndEdges(std::vector<DeBruijnNode *> startingNodes,
                                         int nodeDistance);
    void addGraphicsItemsToScene(MyGraphicsScene * scene);

    QStringList splitCsv(QString line, QString sep=",");
    bool loadCSV(QString filename, QStringList * columns,QString * errormsg);
    std::vector<DeBruijnNode *> getStartingNodes(QString * errorTitle,
                                                 QString * errorMessage,
                                                 bool doubleMode,
                                                 QString nodesList,
                                                 QString blastQueryName);

    bool checkIfStringHasNodes(QString nodesString);
    QString generateNodesNotFoundErrorMessage(std::vector<QString> nodesNotInGraph,
                                              bool exact);
    std::vector<DeBruijnNode *> getNodesFromString(QString nodeNamesString,
                                                   bool exactMatch,
                                                   std::vector<QString> * nodesNotInGraph = 0);
    void layoutGraph();

    void setAllEdgesExactOverlap(int overlap);
    void autoDetermineAllEdgesExactOverlap();

    static void readFastaFile(QString filename, std::vector<QString> * names,
                              std::vector<QString> * sequences);

    int getDrawnNodeCount() const;
    void deleteNodes(std::vector<DeBruijnNode *> * nodes);
    void deleteEdges(std::vector<DeBruijnEdge *> * edges);
    void duplicateNodePair(DeBruijnNode * node, MyGraphicsScene * scene);
    bool mergeNodes(QList<DeBruijnNode *> nodes, MyGraphicsScene * scene,
                    bool recalulateReadDepth);
    void removeGraphicsItemEdges(const std::vector<DeBruijnEdge *> * edges,
                                 bool reverseComplement,
                                 MyGraphicsScene * scene);
    void removeGraphicsItemNodes(const std::vector<DeBruijnNode *> * nodes,
                                 bool reverseComplement,
                                 MyGraphicsScene * scene);
    int mergeAllPossible(MyGraphicsScene * scene = 0,
                         MyProgressDialog * progressDialog = 0);

    void saveEntireGraphToFasta(QString filename);
    void saveEntireGraphToFastaOnlyPositiveNodes(QString filename);
    void saveEntireGraphToGfa(QString filename);
    void saveVisibleGraphToGfa(QString filename);

private:
    double getValueUsingFractionalIndex(std::vector<double> * doubleVector, double index);
    QString convertNormalNumberStringToBandageNodeName(QString number);
    void makeReverseComplementNodeIfNecessary(DeBruijnNode * node);
    void pointEachNodeToItsReverseComplement();
    QStringList removeNullStringsFromList(QStringList in);
    std::vector<DeBruijnNode *> getNodesFromListExact(QStringList nodesList, std::vector<QString> * nodesNotInGraph);
    std::vector<DeBruijnNode *> getNodesFromListPartial(QStringList nodesList, std::vector<QString> * nodesNotInGraph);
    std::vector<DeBruijnNode *> getNodesFromBlastHits(QString queryName);
    std::vector<DeBruijnNode *> getNodesInReadDepthRange(double min, double max);
    std::vector<int> makeOverlapCountVector();
    int getLengthFromCigar(QString cigar);
    int getCigarCount(QString cigarCode, QString cigar);
    QString getOppositeNodeName(QString nodeName);
    void clearAllCsvData();
    QString getNodeNameFromString(QString string);
    QString getNewNodeName(QString oldNodeName);
    void duplicateGraphicsNode(DeBruijnNode * originalNode, DeBruijnNode * newNode, MyGraphicsScene * scene);
    bool canAddNodeToStartOfMergeList(QList<DeBruijnNode *> * mergeList,
                                      DeBruijnNode * potentialNode);
    bool canAddNodeToEndOfMergeList(QList<DeBruijnNode *> * mergeList,
                                    DeBruijnNode * potentialNode);
    QString getUniqueNodeName(QString baseName);
    void mergeGraphicsNodes(QList<DeBruijnNode *> * originalNodes,
                            QList<DeBruijnNode *> * revCompOriginalNodes,
                            DeBruijnNode * newNode, MyGraphicsScene * scene);
    void mergeGraphicsNodes2(QList<DeBruijnNode *> * originalNodes,
                             DeBruijnNode * newNode, MyGraphicsScene * scene);
    void removeAllGraphicsEdgesFromNode(DeBruijnNode * node,
                                        bool reverseComplement,
                                        MyGraphicsScene * scene);

signals:
    void setMergeTotalCount(int totalCount);
    void setMergeCompletedCount(int completedCount);
};

#endif // ASSEMBLYGRAPH_H
