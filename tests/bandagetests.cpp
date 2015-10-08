//Copyright 2015 Ryan Wick

//This file is part of Bandage.

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


#include <QtTest/QtTest>

#include "ogdf/basic/Graph.h"
#include "ogdf/basic/GraphAttributes.h"
#include "../graph/assemblygraph.h"
#include "../program/settings.h"
#include "../blast/blastsearch.h"
#include "../ui/mygraphicsview.h"
#include "../program/memory.h"
#include "../graph/debruijnnode.h"
#include "../graph/debruijnedge.h"
#include "../program/globals.h"
#include "../command_line/commoncommandlinefunctions.h"

class BandageTests : public QObject
{
    Q_OBJECT

private slots:
    void loadFastg();
    void loadLastGraph();
    void loadTrinity();
    void pathFunctionsOnLastGraph();
    void pathFunctionsOnFastg();
    void graphLocationFunctions();
    void loadCsvData();
    void loadCsvDataTrinity();
    void blastSearch();
    void blastSearchFilters();
    void graphScope();
    void commandLineSettings();
    void sciNotComparisons();
    void graphEdits();
    void velvetToGfa();


private:
    void createGlobals();
    bool createBlastTempDirectory();
    void deleteBlastTempDirectory();
    QString getTestDirectory();
    DeBruijnEdge * getEdgeFromNodeNames(QString startingNodeName,
                                        QString endingNodeName);
};




void BandageTests::loadFastg()
{
    createGlobals();
    bool fastgGraphLoaded = g_assemblyGraph->loadGraphFromFile(getTestDirectory() + "test.fastg");

    //Check that the graph loaded properly.
    QCOMPARE(fastgGraphLoaded, true);

    //Check that the appropriate number of nodes/edges are present.
    QCOMPARE(g_assemblyGraph->m_deBruijnGraphNodes.size(), 88);
    QCOMPARE(int(g_assemblyGraph->m_deBruijnGraphEdges.size()), 118);

    //Check the length of a couple nodes.
    DeBruijnNode * node1 = g_assemblyGraph->m_deBruijnGraphNodes["1+"];
    DeBruijnNode * node28 = g_assemblyGraph->m_deBruijnGraphNodes["28-"];
    QCOMPARE(node1->getLength(), 6070);
    QCOMPARE(node28->getLength(), 79);
}


void BandageTests::loadLastGraph()
{
    createGlobals();
    bool lastGraphLoaded = g_assemblyGraph->loadGraphFromFile(getTestDirectory() + "test.LastGraph");

    //Check that the graph loaded properly.
    QCOMPARE(lastGraphLoaded, true);

    //Check that the appropriate number of nodes/edges are present.
    QCOMPARE(g_assemblyGraph->m_deBruijnGraphNodes.size(), 34);
    QCOMPARE(int(g_assemblyGraph->m_deBruijnGraphEdges.size()), 32);

    //Check the length of a couple nodes.
    DeBruijnNode * node1 = g_assemblyGraph->m_deBruijnGraphNodes["1+"];
    DeBruijnNode * node14 = g_assemblyGraph->m_deBruijnGraphNodes["14-"];
    QCOMPARE(node1->getLength(), 2000);
    QCOMPARE(node14->getLength(), 60);
}



void BandageTests::loadTrinity()
{
    createGlobals();
    bool trinityLoaded = g_assemblyGraph->loadGraphFromFile(getTestDirectory() + "test.Trinity.fasta");

    //Check that the graph loaded properly.
    QCOMPARE(trinityLoaded, true);

    //Check that the appropriate number of nodes/edges are present.
    QCOMPARE(g_assemblyGraph->m_deBruijnGraphNodes.size(), 1170);
    QCOMPARE(int(g_assemblyGraph->m_deBruijnGraphEdges.size()), 1056);

    //Check the length of a couple nodes.
    DeBruijnNode * node10241 = g_assemblyGraph->m_deBruijnGraphNodes["TR4|c4_10241+"];
    DeBruijnNode * node3901 = g_assemblyGraph->m_deBruijnGraphNodes["TR19|c0_3901-"];
    QCOMPARE(node10241->getLength(), 1186);
    QCOMPARE(node3901->getLength(), 1);
}


//LastGraph files have no overlap in the edges, so these tests look at paths
//where the connections are simple.
void BandageTests::pathFunctionsOnLastGraph()
{
    createGlobals();
    g_assemblyGraph->loadGraphFromFile(getTestDirectory() + "test.LastGraph");

    QString pathStringFailure;
    Path testPath1 = Path::makeFromString("(1996) 9+, 13+ (5)", false, &pathStringFailure);
    Path testPath2 = Path::makeFromString("(1996) 9+, 13+ (5)", false, &pathStringFailure);
    Path testPath3 = Path::makeFromString("(1996) 9+, 13+ (6)", false, &pathStringFailure);
    Path testPath4 = Path::makeFromString("9+, 13+, 14-", false, &pathStringFailure);

    DeBruijnNode * node4Minus = g_assemblyGraph->m_deBruijnGraphNodes["4-"];
    DeBruijnNode * node9Plus = g_assemblyGraph->m_deBruijnGraphNodes["9+"];
    DeBruijnNode * node13Plus = g_assemblyGraph->m_deBruijnGraphNodes["13+"];
    DeBruijnNode * node14Minus = g_assemblyGraph->m_deBruijnGraphNodes["14+"];
    DeBruijnNode * node7Plus = g_assemblyGraph->m_deBruijnGraphNodes["7+"];

    QCOMPARE(testPath1.getLength(), 10);
    QCOMPARE(testPath1.getPathSequence(), QByteArray("GACCTATAGA"));
    QCOMPARE(testPath1.isEmpty(), false);
    QCOMPARE(testPath1.isCircular(), false);
    QCOMPARE(testPath1 == testPath2, true);
    QCOMPARE(testPath1 == testPath3, false);
    QCOMPARE(testPath1.haveSameNodes(testPath3), true);
    QCOMPARE(testPath1.hasNodeSubset(testPath4), true);
    QCOMPARE(testPath4.hasNodeSubset(testPath1), false);
    QCOMPARE(testPath1.getString(true), QString("(1996) 9+, 13+ (5)"));
    QCOMPARE(testPath1.getString(false), QString("(1996)9+,13+(5)"));
    QCOMPARE(testPath4.getString(true), QString("9+, 13+, 14-"));
    QCOMPARE(testPath4.getString(false), QString("9+,13+,14-"));
    QCOMPARE(testPath1.containsEntireNode(node13Plus), false);
    QCOMPARE(testPath4.containsEntireNode(node13Plus), true);
    QCOMPARE(testPath4.isInMiddleOfPath(node13Plus), true);
    QCOMPARE(testPath4.isInMiddleOfPath(node14Minus), false);
    QCOMPARE(testPath4.isInMiddleOfPath(node9Plus), false);

    Path testPath4Extended;
    QCOMPARE(testPath4.canNodeFitOnEnd(node7Plus, &testPath4Extended), true);
    QCOMPARE(testPath4Extended.getString(true), QString("9+, 13+, 14-, 7+"));
    QCOMPARE(testPath4.canNodeFitAtStart(node4Minus, &testPath4Extended), true);
    QCOMPARE(testPath4Extended.getString(true), QString("4-, 9+, 13+, 14-"));
}



//FASTG files have overlaps in the edges, so these tests look at paths where
//the overlap has to be removed from the path sequence.
void BandageTests::pathFunctionsOnFastg()
{
    createGlobals();
    g_assemblyGraph->loadGraphFromFile(getTestDirectory() + "test.fastg");

    QString pathStringFailure;
    Path testPath1 = Path::makeFromString("(50234) 6+, 26+, 23+, 26+, 24+ (200)", false, &pathStringFailure);
    Path testPath2 = Path::makeFromString("26+, 23+", true, &pathStringFailure);
    QCOMPARE(testPath1.getLength(), 1764);
    QCOMPARE(testPath2.getLength(), 1387);
    QCOMPARE(testPath1.isCircular(), false);
    QCOMPARE(testPath2.isCircular(), true);
}



void BandageTests::graphLocationFunctions()
{
    //First do some tests with a FASTG, where the overlap results in a simpler
    //sitations: all positions have a reverse complement position in the
    //reverse complement node.
    createGlobals();
    g_assemblyGraph->loadGraphFromFile(getTestDirectory() + "test.fastg");
    DeBruijnNode * node12Plus = g_assemblyGraph->m_deBruijnGraphNodes["12+"];
    DeBruijnNode * node3Plus = g_assemblyGraph->m_deBruijnGraphNodes["3+"];

    GraphLocation location1(node12Plus, 1);
    GraphLocation revCompLocation1 = location1.reverseComplementLocation();

    QCOMPARE(location1.getBase(), 'C');
    QCOMPARE(revCompLocation1.getBase(), 'G');
    QCOMPARE(revCompLocation1.getPosition(), 394);

    GraphLocation location2 = GraphLocation::endOfNode(node3Plus);
    QCOMPARE(location2.getPosition(), 5869);

    location2.moveLocation(-1);
    QCOMPARE(location2.getPosition(), 5868);

    location2.moveLocation(2);
    QCOMPARE(location2.getNode()->getName(), QString("38-"));
    QCOMPARE(location2.getPosition(), 1);

    GraphLocation location3;
    QCOMPARE(location2.isNull(), false);
    QCOMPARE(location3.isNull(), true);

    //Now look at a LastGraph file which is more complex.  Because of the
    //offset, reverse complement positions can be in different nodes and may
    //not even exist.
    createGlobals();
    g_assemblyGraph->loadGraphFromFile(getTestDirectory() + "test.LastGraph");
    int kmer = g_assemblyGraph->m_kmer;
    DeBruijnNode * node13Plus = g_assemblyGraph->m_deBruijnGraphNodes["13+"];
    DeBruijnNode * node8Minus = g_assemblyGraph->m_deBruijnGraphNodes["8-"];

    GraphLocation location4 = GraphLocation::startOfNode(node13Plus);
    QCOMPARE(location4.getBase(), 'A');
    QCOMPARE(location4.getPosition(), 1);

    GraphLocation revCompLocation4 = location4.reverseComplementLocation();
    QCOMPARE(revCompLocation4.getBase(), 'T');
    QCOMPARE(revCompLocation4.getNode()->getName(), QString("13-"));
    QCOMPARE(revCompLocation4.getPosition(), node13Plus->getLength() - kmer + 1);

    GraphLocation location5 = GraphLocation::endOfNode(node8Minus);
    GraphLocation location6 = location5;
    location6.moveLocation(-60);
    GraphLocation revCompLocation5 = location5.reverseComplementLocation();
    GraphLocation revCompLocation6 = location6.reverseComplementLocation();
    QCOMPARE(revCompLocation5.isNull(), true);
    QCOMPARE(revCompLocation6.isNull(), false);
    QCOMPARE(revCompLocation6.getNode()->getName(), QString("8+"));
    QCOMPARE(revCompLocation6.getPosition(), 1);
}



void BandageTests::loadCsvData()
{
    createGlobals();
    g_assemblyGraph->loadGraphFromFile(getTestDirectory() + "test.fastg");

    QString errormsg;
    QStringList columns;
    g_assemblyGraph->loadCSV(getTestDirectory() + "test.csv", &columns, &errormsg);

    DeBruijnNode * node6Plus = g_assemblyGraph->m_deBruijnGraphNodes["6+"];
    DeBruijnNode * node6Minus = g_assemblyGraph->m_deBruijnGraphNodes["6-"];
    DeBruijnNode * node7Plus = g_assemblyGraph->m_deBruijnGraphNodes["7+"];
    DeBruijnNode * node4Plus = g_assemblyGraph->m_deBruijnGraphNodes["4+"];
    DeBruijnNode * node4Minus = g_assemblyGraph->m_deBruijnGraphNodes["4-"];
    DeBruijnNode * node3Plus = g_assemblyGraph->m_deBruijnGraphNodes["3+"];
    DeBruijnNode * node5Minus = g_assemblyGraph->m_deBruijnGraphNodes["5-"];
    DeBruijnNode * node8Plus = g_assemblyGraph->m_deBruijnGraphNodes["8+"];
    DeBruijnNode * node9Plus = g_assemblyGraph->m_deBruijnGraphNodes["9+"];

    QCOMPARE(columns.size(), 3);
    QCOMPARE(errormsg, QString("There were 2 unmatched entries in the CSV."));

    QCOMPARE(node6Plus->getCsvLine(0), QString("SIX_PLUS"));
    QCOMPARE(node6Plus->getCsvLine(1), QString("6plus"));
    QCOMPARE(node6Plus->getCsvLine(2), QString("plus6"));
    QCOMPARE(node9Plus->getCsvLine(3), QString(""));
    QCOMPARE(node9Plus->getCsvLine(25), QString(""));

    QCOMPARE(node6Minus->getCsvLine(0), QString("SIX_MINUS"));
    QCOMPARE(node6Minus->getCsvLine(1), QString("6minus"));
    QCOMPARE(node6Minus->getCsvLine(2), QString("minus6"));

    QCOMPARE(node7Plus->getCsvLine(0), QString("SEVEN_PLUS"));
    QCOMPARE(node7Plus->getCsvLine(1), QString("7plus"));
    QCOMPARE(node7Plus->getCsvLine(2), QString("plus7"));

    QCOMPARE(node4Plus->getCsvLine(0), QString("FOUR_PLUS"));
    QCOMPARE(node4Plus->getCsvLine(1), QString("4plus"));
    QCOMPARE(node4Plus->getCsvLine(2), QString("plus4"));

    QCOMPARE(node4Minus->getCsvLine(0), QString("FOUR_MINUS"));
    QCOMPARE(node4Minus->getCsvLine(1), QString("4minus"));
    QCOMPARE(node4Minus->getCsvLine(2), QString("minus4"));

    QCOMPARE(node3Plus->getCsvLine(0), QString("THREE_PLUS"));
    QCOMPARE(node3Plus->getCsvLine(1), QString("3plus"));
    QCOMPARE(node3Plus->getCsvLine(2), QString("plus3"));

    QCOMPARE(node5Minus->getCsvLine(0), QString("FIVE_MINUS"));
    QCOMPARE(node5Minus->getCsvLine(1), QString(""));
    QCOMPARE(node5Minus->getCsvLine(2), QString(""));

    QCOMPARE(node8Plus->getCsvLine(0), QString("EIGHT_PLUS"));
    QCOMPARE(node8Plus->getCsvLine(1), QString("8plus"));
    QCOMPARE(node8Plus->getCsvLine(2), QString("plus8"));

    QCOMPARE(node9Plus->getCsvLine(0), QString("NINE_PLUS"));
    QCOMPARE(node9Plus->getCsvLine(1), QString("9plus"));
    QCOMPARE(node9Plus->getCsvLine(2), QString("plus9"));
    QCOMPARE(node9Plus->getCsvLine(3), QString(""));
    QCOMPARE(node9Plus->getCsvLine(4), QString(""));
    QCOMPARE(node9Plus->getCsvLine(5), QString(""));
}


void BandageTests::loadCsvDataTrinity()
{
    createGlobals();
    g_assemblyGraph->loadGraphFromFile(getTestDirectory() + "test.Trinity.fasta");

    QString errormsg;
    QStringList columns;
    g_assemblyGraph->loadCSV(getTestDirectory() + "test.Trinity.csv", &columns, &errormsg);

    DeBruijnNode * node3912Plus = g_assemblyGraph->m_deBruijnGraphNodes["TR19|c0_3912+"];
    DeBruijnNode * node3912Minus = g_assemblyGraph->m_deBruijnGraphNodes["TR19|c0_3912-"];
    DeBruijnNode * node3914Plus = g_assemblyGraph->m_deBruijnGraphNodes["TR19|c0_3914+"];
    DeBruijnNode * node3915Plus = g_assemblyGraph->m_deBruijnGraphNodes["TR19|c0_3915+"];
    DeBruijnNode * node3923Plus = g_assemblyGraph->m_deBruijnGraphNodes["TR19|c0_3923+"];
    DeBruijnNode * node3924Plus = g_assemblyGraph->m_deBruijnGraphNodes["TR19|c0_3924+"];
    DeBruijnNode * node3940Plus = g_assemblyGraph->m_deBruijnGraphNodes["TR19|c0_3940+"];

    QCOMPARE(columns.size(), 1);

    QCOMPARE(node3912Plus->getCsvLine(0), QString("3912PLUS"));
    QCOMPARE(node3912Minus->getCsvLine(0), QString("3912MINUS"));
    QCOMPARE(node3914Plus->getCsvLine(0), QString("3914PLUS"));
    QCOMPARE(node3915Plus->getCsvLine(0), QString("3915PLUS"));
    QCOMPARE(node3923Plus->getCsvLine(0), QString("3923PLUS"));
    QCOMPARE(node3924Plus->getCsvLine(0), QString("3924PLUS"));
    QCOMPARE(node3940Plus->getCsvLine(0), QString("3940PLUS"));
}

void BandageTests::blastSearch()
{
    createGlobals();
    g_assemblyGraph->loadGraphFromFile(getTestDirectory() + "test.fastg");
    g_settings->blastQueryFilename = getTestDirectory() + "test_queries1.fasta";
    createBlastTempDirectory();

    g_blastSearch->doAutoBlastSearch();

    BlastQuery * exact = g_blastSearch->m_blastQueries.getQueryFromName("test_query_exact");
    BlastQuery * one_mismatch = g_blastSearch->m_blastQueries.getQueryFromName("test_query_one_mismatch");
    BlastQuery * one_insertion = g_blastSearch->m_blastQueries.getQueryFromName("test_query_one_insertion");
    BlastQuery * one_deletion = g_blastSearch->m_blastQueries.getQueryFromName("test_query_one_deletion");

    QCOMPARE(exact->getLength(), 100);
    QCOMPARE(one_mismatch->getLength(), 100);
    QCOMPARE(one_insertion->getLength(), 101);
    QCOMPARE(one_deletion->getLength(), 99);

    QSharedPointer<BlastHit> exactHit = exact->getHits().at(0);
    QSharedPointer<BlastHit> one_mismatchHit = one_mismatch->getHits().at(0);
    QSharedPointer<BlastHit> one_insertionHit = one_insertion->getHits().at(0);
    QSharedPointer<BlastHit> one_deletionHit = one_deletion->getHits().at(0);

    QCOMPARE(exactHit->m_numberMismatches, 0);
    QCOMPARE(exactHit->m_numberGapOpens, 0);
    QCOMPARE(one_mismatchHit->m_numberMismatches, 1);
    QCOMPARE(one_mismatchHit->m_numberGapOpens, 0);
    QCOMPARE(one_insertionHit->m_numberMismatches, 0);
    QCOMPARE(one_insertionHit->m_numberGapOpens, 1);
    QCOMPARE(one_deletionHit->m_numberMismatches, 0);
    QCOMPARE(one_deletionHit->m_numberGapOpens, 1);

    QCOMPARE(exactHit->m_percentIdentity < 100.0, false);
    QCOMPARE(one_mismatchHit->m_percentIdentity < 100.0, true);
    QCOMPARE(one_insertionHit->m_percentIdentity < 100.0, true);
    QCOMPARE(one_deletionHit->m_percentIdentity < 100.0, true);

    deleteBlastTempDirectory();
}



void BandageTests::blastSearchFilters()
{
    createGlobals();
    g_assemblyGraph->loadGraphFromFile(getTestDirectory() + "test.fastg");
    g_settings->blastQueryFilename = getTestDirectory() + "test_queries2.fasta";
    createBlastTempDirectory();

    //First do the search with no filters
    g_blastSearch->doAutoBlastSearch();
    int unfilteredHitCount = g_blastSearch->m_allHits.size();

    //Now filter by e-value.
    g_settings->blastEValueFilterOn = true;
    g_settings->blastEValueFilterValue = SciNot(1.0, -5);
    g_blastSearch->doAutoBlastSearch();
    QCOMPARE(g_blastSearch->m_allHits.size(), 14);
    QCOMPARE(g_blastSearch->m_allHits.size() < unfilteredHitCount, true);

    //Now add a bit score filter.
    g_settings->blastBitScoreFilterOn = true;
    g_settings->blastBitScoreFilterValue = 100.0;
    g_blastSearch->doAutoBlastSearch();
    QCOMPARE(g_blastSearch->m_allHits.size(), 9);

    //Now add an alignment length filter.
    g_settings->blastAlignmentLengthFilterOn = true;
    g_settings->blastAlignmentLengthFilterValue = 100;
    g_blastSearch->doAutoBlastSearch();
    QCOMPARE(g_blastSearch->m_allHits.size(), 8);

    //Now add an identity filter.
    g_settings->blastIdentityFilterOn = true;
    g_settings->blastIdentityFilterValue = 50.0;
    g_blastSearch->doAutoBlastSearch();
    QCOMPARE(g_blastSearch->m_allHits.size(), 7);

    //Now add a query coverage filter.
    g_settings->blastQueryCoverageFilterOn = true;
    g_settings->blastQueryCoverageFilterValue = 90.0;
    g_blastSearch->doAutoBlastSearch();
    QCOMPARE(g_blastSearch->m_allHits.size(), 5);

    deleteBlastTempDirectory();
}



void BandageTests::graphScope()
{
    createGlobals();
    g_assemblyGraph->loadGraphFromFile(getTestDirectory() + "test.fastg");

    QString errorTitle;
    QString errorMessage;
    int drawnNodes;
    std::vector<DeBruijnNode *> startingNodes;

    g_settings->graphScope = WHOLE_GRAPH;
    g_settings->nodeDistance = 0;
    g_settings->doubleMode = false;
    startingNodes = g_assemblyGraph->getStartingNodes(&errorTitle, &errorMessage, g_settings->doubleMode, g_settings->startingNodes, "");
    g_assemblyGraph->buildOgdfGraphFromNodesAndEdges(startingNodes, g_settings->nodeDistance);
    g_assemblyGraph->layoutGraph();
    drawnNodes = g_assemblyGraph->getDrawnNodeCount();
    QCOMPARE(drawnNodes, 44);

    g_settings->graphScope = WHOLE_GRAPH;
    g_settings->nodeDistance = 0;
    g_settings->doubleMode = true;
    startingNodes = g_assemblyGraph->getStartingNodes(&errorTitle, &errorMessage, g_settings->doubleMode, g_settings->startingNodes, "");
    g_assemblyGraph->buildOgdfGraphFromNodesAndEdges(startingNodes, g_settings->nodeDistance);
    g_assemblyGraph->layoutGraph();
    drawnNodes = g_assemblyGraph->getDrawnNodeCount();
    QCOMPARE(drawnNodes, 88);

    g_settings->graphScope = AROUND_NODE;
    g_settings->startingNodes = "1";
    g_settings->nodeDistance = 0;
    g_settings->doubleMode = false;
    startingNodes = g_assemblyGraph->getStartingNodes(&errorTitle, &errorMessage, g_settings->doubleMode, g_settings->startingNodes, "");
    g_assemblyGraph->buildOgdfGraphFromNodesAndEdges(startingNodes, g_settings->nodeDistance);
    g_assemblyGraph->layoutGraph();
    drawnNodes = g_assemblyGraph->getDrawnNodeCount();
    QCOMPARE(drawnNodes, 1);

    g_settings->graphScope = AROUND_NODE;
    g_settings->startingNodes = "1";
    g_settings->nodeDistance = 0;
    g_settings->doubleMode = true;
    startingNodes = g_assemblyGraph->getStartingNodes(&errorTitle, &errorMessage, g_settings->doubleMode, g_settings->startingNodes, "");
    g_assemblyGraph->buildOgdfGraphFromNodesAndEdges(startingNodes, g_settings->nodeDistance);
    g_assemblyGraph->layoutGraph();
    drawnNodes = g_assemblyGraph->getDrawnNodeCount();
    QCOMPARE(drawnNodes, 2);

    g_settings->graphScope = AROUND_NODE;
    g_settings->startingNodes = "1+";
    g_settings->nodeDistance = 0;
    g_settings->doubleMode = true;
    startingNodes = g_assemblyGraph->getStartingNodes(&errorTitle, &errorMessage, g_settings->doubleMode, g_settings->startingNodes, "");
    g_assemblyGraph->buildOgdfGraphFromNodesAndEdges(startingNodes, g_settings->nodeDistance);
    g_assemblyGraph->layoutGraph();
    drawnNodes = g_assemblyGraph->getDrawnNodeCount();
    QCOMPARE(drawnNodes, 1);

    g_settings->graphScope = AROUND_NODE;
    g_settings->startingNodes = "1";
    g_settings->nodeDistance = 1;
    g_settings->doubleMode = false;
    startingNodes = g_assemblyGraph->getStartingNodes(&errorTitle, &errorMessage, g_settings->doubleMode, g_settings->startingNodes, "");
    g_assemblyGraph->buildOgdfGraphFromNodesAndEdges(startingNodes, g_settings->nodeDistance);
    g_assemblyGraph->layoutGraph();
    drawnNodes = g_assemblyGraph->getDrawnNodeCount();
    QCOMPARE(drawnNodes, 3);

    g_settings->graphScope = AROUND_NODE;
    g_settings->startingNodes = "1";
    g_settings->nodeDistance = 2;
    g_settings->doubleMode = false;
    startingNodes = g_assemblyGraph->getStartingNodes(&errorTitle, &errorMessage, g_settings->doubleMode, g_settings->startingNodes, "");
    g_assemblyGraph->buildOgdfGraphFromNodesAndEdges(startingNodes, g_settings->nodeDistance);
    g_assemblyGraph->layoutGraph();
    drawnNodes = g_assemblyGraph->getDrawnNodeCount();
    QCOMPARE(drawnNodes, 10);

    g_settings->graphScope = READ_DEPTH_RANGE;
    g_settings->nodeDistance = 0;
    g_settings->doubleMode = false;
    g_settings->minReadDepthRange = 0.0;
    g_settings->maxReadDepthRange = 211.0;
    startingNodes = g_assemblyGraph->getStartingNodes(&errorTitle, &errorMessage, g_settings->doubleMode, g_settings->startingNodes, "");
    g_assemblyGraph->buildOgdfGraphFromNodesAndEdges(startingNodes, g_settings->nodeDistance);
    g_assemblyGraph->layoutGraph();
    drawnNodes = g_assemblyGraph->getDrawnNodeCount();
    QCOMPARE(drawnNodes, 43);

    g_settings->graphScope = READ_DEPTH_RANGE;
    g_settings->nodeDistance = 10;
    g_settings->doubleMode = false;
    g_settings->minReadDepthRange = 0.0;
    g_settings->maxReadDepthRange = 211.0;
    startingNodes = g_assemblyGraph->getStartingNodes(&errorTitle, &errorMessage, g_settings->doubleMode, g_settings->startingNodes, "");
    g_assemblyGraph->buildOgdfGraphFromNodesAndEdges(startingNodes, g_settings->nodeDistance);
    g_assemblyGraph->layoutGraph();
    drawnNodes = g_assemblyGraph->getDrawnNodeCount();
    QCOMPARE(drawnNodes, 43);

    g_settings->graphScope = READ_DEPTH_RANGE;
    g_settings->nodeDistance = 0;
    g_settings->doubleMode = false;
    g_settings->minReadDepthRange = 211.0;
    g_settings->maxReadDepthRange = 1000.0;
    startingNodes = g_assemblyGraph->getStartingNodes(&errorTitle, &errorMessage, g_settings->doubleMode, g_settings->startingNodes, "");
    g_assemblyGraph->buildOgdfGraphFromNodesAndEdges(startingNodes, g_settings->nodeDistance);
    g_assemblyGraph->layoutGraph();
    drawnNodes = g_assemblyGraph->getDrawnNodeCount();
    QCOMPARE(drawnNodes, 1);

    g_settings->graphScope = READ_DEPTH_RANGE;
    g_settings->nodeDistance = 0;
    g_settings->doubleMode = false;
    g_settings->minReadDepthRange = 40.0;
    g_settings->maxReadDepthRange = 211.0;
    startingNodes = g_assemblyGraph->getStartingNodes(&errorTitle, &errorMessage, g_settings->doubleMode, g_settings->startingNodes, "");
    g_assemblyGraph->buildOgdfGraphFromNodesAndEdges(startingNodes, g_settings->nodeDistance);
    g_assemblyGraph->layoutGraph();
    drawnNodes = g_assemblyGraph->getDrawnNodeCount();
    QCOMPARE(drawnNodes, 42);

    createBlastTempDirectory();

    g_settings->blastQueryFilename = getTestDirectory() + "test_queries1.fasta";
    g_blastSearch->doAutoBlastSearch();

    g_settings->graphScope = AROUND_BLAST_HITS;
    g_settings->nodeDistance = 0;
    g_settings->doubleMode = false;
    startingNodes = g_assemblyGraph->getStartingNodes(&errorTitle, &errorMessage, g_settings->doubleMode, g_settings->startingNodes, "all");
    g_assemblyGraph->buildOgdfGraphFromNodesAndEdges(startingNodes, g_settings->nodeDistance);
    g_assemblyGraph->layoutGraph();
    drawnNodes = g_assemblyGraph->getDrawnNodeCount();
    QCOMPARE(drawnNodes, 1);

    g_settings->graphScope = AROUND_BLAST_HITS;
    g_settings->nodeDistance = 1;
    g_settings->doubleMode = false;
    startingNodes = g_assemblyGraph->getStartingNodes(&errorTitle, &errorMessage, g_settings->doubleMode, g_settings->startingNodes, "all");
    g_assemblyGraph->buildOgdfGraphFromNodesAndEdges(startingNodes, g_settings->nodeDistance);
    g_assemblyGraph->layoutGraph();
    drawnNodes = g_assemblyGraph->getDrawnNodeCount();
    QCOMPARE(drawnNodes, 3);

    g_settings->graphScope = AROUND_BLAST_HITS;
    g_settings->nodeDistance = 2;
    g_settings->doubleMode = false;
    startingNodes = g_assemblyGraph->getStartingNodes(&errorTitle, &errorMessage, g_settings->doubleMode, g_settings->startingNodes, "all");
    g_assemblyGraph->buildOgdfGraphFromNodesAndEdges(startingNodes, g_settings->nodeDistance);
    g_assemblyGraph->layoutGraph();
    drawnNodes = g_assemblyGraph->getDrawnNodeCount();
    QCOMPARE(drawnNodes, 9);

    deleteBlastTempDirectory();
}

void BandageTests::commandLineSettings()
{
    createGlobals();
    QStringList commandLineSettings;

    commandLineSettings = QString("--scope entire").split(" ");
    parseSettings(commandLineSettings);
    QCOMPARE(g_settings->graphScope, WHOLE_GRAPH);

    commandLineSettings = QString("--scope aroundnodes").split(" ");
    parseSettings(commandLineSettings);
    QCOMPARE(g_settings->graphScope, AROUND_NODE);

    commandLineSettings = QString("--scope aroundblast").split(" ");
    parseSettings(commandLineSettings);
    QCOMPARE(g_settings->graphScope, AROUND_BLAST_HITS);

    commandLineSettings = QString("--scope depthrange").split(" ");
    parseSettings(commandLineSettings);
    QCOMPARE(g_settings->graphScope, READ_DEPTH_RANGE);

    commandLineSettings = QString("--nodes 5+").split(" ");
    parseSettings(commandLineSettings);
    QCOMPARE(g_settings->startingNodes, QString("5+"));

    commandLineSettings = QString("--nodes 1,2,3").split(" ");
    parseSettings(commandLineSettings);
    QCOMPARE(g_settings->startingNodes, QString("1,2,3"));

    QCOMPARE(g_settings->startingNodesExactMatch, true);
    commandLineSettings = QString("--partial").split(" ");
    parseSettings(commandLineSettings);
    QCOMPARE(g_settings->startingNodesExactMatch, false);

    commandLineSettings = QString("--distance 12").split(" ");
    parseSettings(commandLineSettings);
    QCOMPARE(g_settings->nodeDistance, 12);

    commandLineSettings = QString("--mindepth 1.2").split(" ");
    parseSettings(commandLineSettings);
    QCOMPARE(g_settings->minReadDepthRange, 1.2);

    commandLineSettings = QString("--maxdepth 2.1").split(" ");
    parseSettings(commandLineSettings);
    QCOMPARE(g_settings->maxReadDepthRange, 2.1);

    QCOMPARE(g_settings->doubleMode, false);
    commandLineSettings = QString("--double").split(" ");
    parseSettings(commandLineSettings);
    QCOMPARE(g_settings->doubleMode, true);

    QCOMPARE(g_settings->nodeLengthMode, AUTO_NODE_LENGTH);
    commandLineSettings = QString("--bases 1000").split(" ");
    parseSettings(commandLineSettings);
    QCOMPARE(g_settings->nodeLengthMode, MANUAL_NODE_LENGTH);
    QCOMPARE(g_settings->manualBasePairsPerSegment, 1000);

    commandLineSettings = QString("--quality 1").split(" ");
    parseSettings(commandLineSettings);
    QCOMPARE(g_settings->graphLayoutQuality, 1);

    commandLineSettings = QString("--nodewidth 4.2").split(" ");
    parseSettings(commandLineSettings);
    QCOMPARE(g_settings->averageNodeWidth, 4.2);

    commandLineSettings = QString("--depwidth 0.222").split(" ");
    parseSettings(commandLineSettings);
    QCOMPARE(g_settings->readDepthEffectOnWidth, 0.222);

    commandLineSettings = QString("--deppower 0.72").split(" ");
    parseSettings(commandLineSettings);
    QCOMPARE(g_settings->readDepthPower, 0.72);

    QCOMPARE(g_settings->displayNodeNames, false);
    commandLineSettings = QString("--names").split(" ");
    parseSettings(commandLineSettings);
    QCOMPARE(g_settings->displayNodeNames, true);

    QCOMPARE(g_settings->displayNodeLengths, false);
    commandLineSettings = QString("--lengths").split(" ");
    parseSettings(commandLineSettings);
    QCOMPARE(g_settings->displayNodeLengths, true);

    QCOMPARE(g_settings->displayNodeReadDepth, false);
    commandLineSettings = QString("--readdepth").split(" ");
    parseSettings(commandLineSettings);
    QCOMPARE(g_settings->displayNodeReadDepth, true);

    QCOMPARE(g_settings->displayBlastHits, false);
    commandLineSettings = QString("--blasthits").split(" ");
    parseSettings(commandLineSettings);
    QCOMPARE(g_settings->displayBlastHits, true);

    commandLineSettings = QString("--fontsize 5").split(" ");
    parseSettings(commandLineSettings);
    QCOMPARE(g_settings->labelFont.pointSize(), 5);

    commandLineSettings = QString("--edgecol #00ff00").split(" ");
    parseSettings(commandLineSettings);
    QCOMPARE(g_settings->edgeColour.name(), QString("#00ff00"));

    commandLineSettings = QString("--edgewidth 5.5").split(" ");
    parseSettings(commandLineSettings);
    QCOMPARE(g_settings->edgeWidth, 5.5);

    commandLineSettings = QString("--outcol #ff0000").split(" ");
    parseSettings(commandLineSettings);
    QCOMPARE(g_settings->outlineColour.name(), QString("#ff0000"));

    commandLineSettings = QString("--outline 0.123").split(" ");
    parseSettings(commandLineSettings);
    QCOMPARE(g_settings->outlineThickness, 0.123);

    commandLineSettings = QString("--selcol tomato").split(" ");
    parseSettings(commandLineSettings);
    QCOMPARE(g_settings->selectionColour.name(), QString("#ff6347"));

    QCOMPARE(g_settings->antialiasing, true);
    commandLineSettings = QString("--noaa").split(" ");
    parseSettings(commandLineSettings);
    QCOMPARE(g_settings->antialiasing, false);

    commandLineSettings = QString("--textcol #550000ff").split(" ");
    parseSettings(commandLineSettings);
    QCOMPARE(g_settings->textColour.name(), QString("#0000ff"));
    QCOMPARE(g_settings->textColour.alpha(), 85);

    commandLineSettings = QString("--toutcol steelblue").split(" ");
    parseSettings(commandLineSettings);
    QCOMPARE(getColourName(g_settings->textOutlineColour), QString("steelblue"));

    commandLineSettings = QString("--toutline 0.321").split(" ");
    parseSettings(commandLineSettings);
    QCOMPARE(g_settings->textOutlineThickness, 0.321);

    QCOMPARE(g_settings->positionTextNodeCentre, false);
    commandLineSettings = QString("--centre").split(" ");
    parseSettings(commandLineSettings);
    QCOMPARE(g_settings->positionTextNodeCentre, true);

    commandLineSettings = QString("--colour random").split(" ");
    parseSettings(commandLineSettings);
    QCOMPARE(g_settings->nodeColourScheme, RANDOM_COLOURS);

    commandLineSettings = QString("--colour uniform").split(" ");
    parseSettings(commandLineSettings);
    QCOMPARE(g_settings->nodeColourScheme, UNIFORM_COLOURS);

    commandLineSettings = QString("--colour readdepth").split(" ");
    parseSettings(commandLineSettings);
    QCOMPARE(g_settings->nodeColourScheme, READ_DEPTH_COLOUR);

    commandLineSettings = QString("--colour blastsolid").split(" ");
    parseSettings(commandLineSettings);
    QCOMPARE(g_settings->nodeColourScheme, BLAST_HITS_SOLID_COLOUR);

    commandLineSettings = QString("--colour blastrainbow").split(" ");
    parseSettings(commandLineSettings);
    QCOMPARE(g_settings->nodeColourScheme, BLAST_HITS_RAINBOW_COLOUR);

    commandLineSettings = QString("--ransatpos 12").split(" ");
    parseSettings(commandLineSettings);
    QCOMPARE(g_settings->randomColourPositiveSaturation, 12);

    commandLineSettings = QString("--ransatneg 23").split(" ");
    parseSettings(commandLineSettings);
    QCOMPARE(g_settings->randomColourNegativeSaturation, 23);

    commandLineSettings = QString("--ranligpos 34").split(" ");
    parseSettings(commandLineSettings);
    QCOMPARE(g_settings->randomColourPositiveLightness, 34);

    commandLineSettings = QString("--ranligneg 45").split(" ");
    parseSettings(commandLineSettings);
    QCOMPARE(g_settings->randomColourNegativeLightness, 45);

    commandLineSettings = QString("--ranopapos 56").split(" ");
    parseSettings(commandLineSettings);
    QCOMPARE(g_settings->randomColourPositiveOpacity, 56);

    commandLineSettings = QString("--ranopaneg 67").split(" ");
    parseSettings(commandLineSettings);
    QCOMPARE(g_settings->randomColourNegativeOpacity, 67);

    commandLineSettings = QString("--unicolpos springgreen").split(" ");
    parseSettings(commandLineSettings);
    QCOMPARE(getColourName(g_settings->uniformPositiveNodeColour), QString("springgreen"));

    commandLineSettings = QString("--unicolneg teal").split(" ");
    parseSettings(commandLineSettings);
    QCOMPARE(getColourName(g_settings->uniformNegativeNodeColour), QString("teal"));

    commandLineSettings = QString("--unicolspe papayawhip").split(" ");
    parseSettings(commandLineSettings);
    QCOMPARE(getColourName(g_settings->uniformNodeSpecialColour), QString("papayawhip"));

    commandLineSettings = QString("--depcollow mediumorchid").split(" ");
    parseSettings(commandLineSettings);
    QCOMPARE(getColourName(g_settings->lowReadDepthColour), QString("mediumorchid"));

    commandLineSettings = QString("--depcolhi linen").split(" ");
    parseSettings(commandLineSettings);
    QCOMPARE(getColourName(g_settings->highReadDepthColour), QString("linen"));

    QCOMPARE(g_settings->autoReadDepthValue, true);
    commandLineSettings = QString("--depvallow 56.7").split(" ");
    parseSettings(commandLineSettings);
    QCOMPARE(g_settings->lowReadDepthValue, 56.7);
    QCOMPARE(g_settings->autoReadDepthValue, false);

    commandLineSettings = QString("--depvalhi 67.8").split(" ");
    parseSettings(commandLineSettings);
    QCOMPARE(g_settings->highReadDepthValue, 67.8);

    commandLineSettings = QString("--depvalhi 67.8").split(" ");
    parseSettings(commandLineSettings);
    QCOMPARE(g_settings->highReadDepthValue, 67.8);

    commandLineSettings = QString("--query queries.fasta").split(" ");
    parseSettings(commandLineSettings);
    QCOMPARE(g_settings->blastQueryFilename, QString("queries.fasta"));

    commandLineSettings = QString("--blastp --abc").split(" ");
    parseSettings(commandLineSettings);
    QCOMPARE(g_settings->blastSearchParameters, QString("--abc"));

    QCOMPARE(g_settings->blastAlignmentLengthFilterOn, false);
    commandLineSettings = QString("--alfilter 543").split(" ");
    parseSettings(commandLineSettings);
    QCOMPARE(g_settings->blastAlignmentLengthFilterOn, true);
    QCOMPARE(g_settings->blastAlignmentLengthFilterValue, 543);

    QCOMPARE(g_settings->blastQueryCoverageFilterOn, false);
    commandLineSettings = QString("--qcfilter 67.8").split(" ");
    parseSettings(commandLineSettings);
    QCOMPARE(g_settings->blastQueryCoverageFilterOn, true);
    QCOMPARE(g_settings->blastQueryCoverageFilterValue, 67.8);

    QCOMPARE(g_settings->blastIdentityFilterOn, false);
    commandLineSettings = QString("--ifilter 12.3").split(" ");
    parseSettings(commandLineSettings);
    QCOMPARE(g_settings->blastIdentityFilterOn, true);
    QCOMPARE(g_settings->blastIdentityFilterValue, 12.3);

    QCOMPARE(g_settings->blastEValueFilterOn, false);
    commandLineSettings = QString("--evfilter 8.5e-14").split(" ");
    parseSettings(commandLineSettings);
    QCOMPARE(g_settings->blastEValueFilterOn, true);
    QCOMPARE(g_settings->blastEValueFilterValue.getCoefficient(), 8.5);
    QCOMPARE(g_settings->blastEValueFilterValue.getExponent(), -14);

    QCOMPARE(g_settings->blastBitScoreFilterOn, false);
    commandLineSettings = QString("--bsfilter 1234.5").split(" ");
    parseSettings(commandLineSettings);
    QCOMPARE(g_settings->blastBitScoreFilterOn, true);
    QCOMPARE(g_settings->blastBitScoreFilterValue, 1234.5);

    commandLineSettings = QString("--pathnodes 3").split(" ");
    parseSettings(commandLineSettings);
    QCOMPARE(g_settings->maxQueryPathNodes, 3);

    commandLineSettings = QString("--minpatcov 0.543").split(" ");
    parseSettings(commandLineSettings);
    QCOMPARE(g_settings->minQueryCoveredByPath, 0.543);

    commandLineSettings = QString("--minhitcov 0.654").split(" ");
    parseSettings(commandLineSettings);
    QCOMPARE(g_settings->minQueryCoveredByHits, 0.654);

    commandLineSettings = QString("--minmeanid 0.765").split(" ");
    parseSettings(commandLineSettings);
    QCOMPARE(g_settings->minMeanHitIdentity, 0.765);

    commandLineSettings = QString("--maxlendis 0.03").split(" ");
    parseSettings(commandLineSettings);
    QCOMPARE(g_settings->maxLengthDiscrepancy, 0.03);

    commandLineSettings = QString("--maxevprod 4e-500").split(" ");
    parseSettings(commandLineSettings);
    QCOMPARE(g_settings->maxEValueProduct.getCoefficient(), 4.0);
    QCOMPARE(g_settings->maxEValueProduct.getExponent(), -500);
}


void BandageTests::sciNotComparisons()
{
    SciNot sn01(1.0, 10);
    SciNot sn02(10.0, 9);
    SciNot sn03(0.1, 11);
    SciNot sn04(5.0, 10);
    SciNot sn05(-5.0, 15);
    SciNot sn06(-6.0, 15);
    SciNot sn07(-3.0, 3);
    SciNot sn08(-0.3, 4);
    SciNot sn09(-3.0, -3);
    SciNot sn10(-0.3, -2);
    SciNot sn11(1.4, 2);
    SciNot sn12("1.4e2");
    SciNot sn13("140");

    QCOMPARE(sn01 == sn02, true);
    QCOMPARE(sn01 == sn03, true);
    QCOMPARE(sn01 == sn03, true);
    QCOMPARE(sn01 >= sn03, true);
    QCOMPARE(sn01 <= sn03, true);
    QCOMPARE(sn01 != sn03, false);
    QCOMPARE(sn01 < sn04, true);
    QCOMPARE(sn01 <= sn04, true);
    QCOMPARE(sn01 > sn04, false);
    QCOMPARE(sn01 >= sn04, false);
    QCOMPARE(sn04 > sn05, true);
    QCOMPARE(sn04 < sn05, false);
    QCOMPARE(sn06 < sn05, true);
    QCOMPARE(sn06 <= sn05, true);
    QCOMPARE(sn06 > sn05, false);
    QCOMPARE(sn06 >= sn05, false);
    QCOMPARE(sn07 == sn08, true);
    QCOMPARE(sn07 != sn08, false);
    QCOMPARE(sn09 == sn10, true);
    QCOMPARE(sn09 != sn10, false);
    QCOMPARE(sn11 == sn12, true);
    QCOMPARE(sn11 == sn13, true);
    QCOMPARE(sn01.asString(true), QString("1e10"));
    QCOMPARE(sn01.asString(false), QString("1e10"));
    QCOMPARE(sn11.asString(true), QString("1.4e2"));
    QCOMPARE(sn11.asString(false), QString("140"));
}


void BandageTests::graphEdits()
{
    createGlobals();
    g_assemblyGraph->loadGraphFromFile(getTestDirectory() + "test.fastg");

    QCOMPARE(g_assemblyGraph->m_deBruijnGraphNodes.size(), 88);
    QCOMPARE(int(g_assemblyGraph->m_deBruijnGraphEdges.size()), 118);

    //Get the path sequence now to compare to the merged node sequence at the
    //end.
    QString pathStringFailure;
    Path path = Path::makeFromString("6+, 26+, 23+, 26+, 24+", false, &pathStringFailure);
    QByteArray pathSequence = path.getPathSequence();

    g_assemblyGraph->duplicateNodePair(g_assemblyGraph->m_deBruijnGraphNodes["26+"], 0);

    QCOMPARE(g_assemblyGraph->m_deBruijnGraphNodes.size(), 90);
    QCOMPARE(int(g_assemblyGraph->m_deBruijnGraphEdges.size()), 126);

    std::vector<DeBruijnEdge *> edgesToRemove;
    edgesToRemove.push_back(getEdgeFromNodeNames("26_copy+", "24+"));
    edgesToRemove.push_back(getEdgeFromNodeNames("6+", "26+"));
    edgesToRemove.push_back(getEdgeFromNodeNames("26+", "23+"));
    edgesToRemove.push_back(getEdgeFromNodeNames("23+", "26_copy+"));
    g_assemblyGraph->deleteEdges(&edgesToRemove);

    QCOMPARE(g_assemblyGraph->m_deBruijnGraphNodes.size(), 90);
    QCOMPARE(int(g_assemblyGraph->m_deBruijnGraphEdges.size()), 118);

    g_assemblyGraph->mergeAllPossible();

    QCOMPARE(g_assemblyGraph->m_deBruijnGraphNodes.size(), 82);
    QCOMPARE(int(g_assemblyGraph->m_deBruijnGraphEdges.size()), 110);

    DeBruijnNode * mergedNode = g_assemblyGraph->m_deBruijnGraphNodes["6_26_copy_23_26_24+"];
    QCOMPARE(pathSequence, mergedNode->getSequence());
}


//Saving a Velvet graph to GFA is a bit complex because the node sequence offset
//must be filled in.  This function tests aspects of that process.
void BandageTests::velvetToGfa()
{
    //First load the graph as a LastGraph and pull out some information and a
    //circular path sequence.
    createGlobals();
    g_assemblyGraph->loadGraphFromFile(getTestDirectory() + "big_test.LastGraph");

    int lastGraphNodeCount = g_assemblyGraph->m_nodeCount;
    int lastGraphEdgeCount= g_assemblyGraph->m_edgeCount;
    long long lastGraphTotalLength = g_assemblyGraph->m_totalLength;
    long long lastGraphShortestContig = g_assemblyGraph->m_shortestContig;
    long long lastGraphLongestContig = g_assemblyGraph->m_longestContig;

    QString pathStringFailure;
    Path lastGraphTestPath1 = Path::makeFromString("3176-, 3176+, 4125+, 3178-, 3283+, 3180+, 3177-", true, &pathStringFailure);
    Path lastGraphTestPath2 = Path::makeFromString("3368+, 3369+, 3230+, 3231+, 3370-, 3143+, 3144+, 3145+, 3240+, 3241+, 3788-, 3787-, 3231+, 3252+, 3241-, 3240-, 3145-, 4164-, 4220+, 3368-, 3367+", true, &pathStringFailure);
    QByteArray lastGraphTestPath1Sequence = lastGraphTestPath1.getPathSequence();
    QByteArray lastGraphTestPath2Sequence = lastGraphTestPath2.getPathSequence();

    //Now save the graph as a GFA and reload it and grab the same information.
    g_assemblyGraph->saveEntireGraphToGfa(getTestDirectory() + "big_test.gfa");
    createGlobals();
    g_assemblyGraph->loadGraphFromFile(getTestDirectory() + "big_test.gfa");

    int gfaNodeCount = g_assemblyGraph->m_nodeCount;
    int gfaEdgeCount= g_assemblyGraph->m_edgeCount;
    long long gfaTotalLength = g_assemblyGraph->m_totalLength;
    long long gfaShortestContig = g_assemblyGraph->m_shortestContig;
    long long gfaLongestContig = g_assemblyGraph->m_longestContig;

    Path gfaTestPath1 = Path::makeFromString("3176-, 3176+, 4125+, 3178-, 3283+, 3180+, 3177-", true, &pathStringFailure);
    Path gfaTestPath2 = Path::makeFromString("3368+, 3369+, 3230+, 3231+, 3370-, 3143+, 3144+, 3145+, 3240+, 3241+, 3788-, 3787-, 3231+, 3252+, 3241-, 3240-, 3145-, 4164-, 4220+, 3368-, 3367+", true, &pathStringFailure);
    QByteArray gfaTestPath1Sequence = gfaTestPath1.getPathSequence();
    QByteArray gfaTestPath2Sequence = gfaTestPath2.getPathSequence();

    //Now we compare the LastGraph info to the GFA info to make sure they are
    //the same (or appropriately different).  The k-mer size for this graph is
    //51, so we expect each node to get 50 base pairs longer.
    QCOMPARE(lastGraphNodeCount, gfaNodeCount);
    QCOMPARE(lastGraphEdgeCount, gfaEdgeCount);
    QCOMPARE(lastGraphTotalLength + 50 * lastGraphNodeCount, gfaTotalLength);
    QCOMPARE(lastGraphShortestContig + 50, gfaShortestContig);
    QCOMPARE(lastGraphLongestContig + 50, gfaLongestContig);
    QCOMPARE(lastGraphTestPath1Sequence, gfaTestPath1Sequence);
    QCOMPARE(lastGraphTestPath2Sequence, gfaTestPath2Sequence);

    //Finally, delete the gfa file.
    QFile::remove(getTestDirectory() + "big_test.gfa");
}
















void BandageTests::createGlobals()
{
    g_settings.reset(new Settings());
    g_memory.reset(new Memory());
    g_blastSearch.reset(new BlastSearch());
    g_assemblyGraph.reset(new AssemblyGraph());
    g_graphicsView = new MyGraphicsView();
}

bool BandageTests::createBlastTempDirectory()
{
    //Running from the command line, it makes more sense to put the temp
    //directory in the current directory.
    g_blastSearch->m_tempDirectory = "bandage_temp-" + QString::number(QCoreApplication::applicationPid()) + "/";

    if (!QDir().mkdir(g_blastSearch->m_tempDirectory))
        return false;

    g_blastSearch->m_blastQueries.createTempQueryFiles();
    return true;
}

void BandageTests::deleteBlastTempDirectory()
{
    if (g_blastSearch->m_tempDirectory != "" &&
            QDir(g_blastSearch->m_tempDirectory).exists() &&
            QDir(g_blastSearch->m_tempDirectory).dirName().contains("bandage_temp"))
        QDir(g_blastSearch->m_tempDirectory).removeRecursively();
}

QString BandageTests::getTestDirectory()
{
    QDir directory = QDir::current();

    //We want to find a directory "Bandage/tests/".  Keep backing up in the
    //directory structure until we find it.
    QString path;
    while (true)
    {
        path = directory.path() + "/Bandage/tests/";
        if (QDir(path).exists())
            return path;
        if (!directory.cdUp())
            return "";
    }

    return "";
}

DeBruijnEdge * BandageTests::getEdgeFromNodeNames(QString startingNodeName,
                                                  QString endingNodeName)
{
    DeBruijnNode * startingNode = g_assemblyGraph->m_deBruijnGraphNodes[startingNodeName];
    DeBruijnNode * endingNode = g_assemblyGraph->m_deBruijnGraphNodes[endingNodeName];

    QPair<DeBruijnNode*, DeBruijnNode*> nodePair(startingNode, endingNode);

    if (g_assemblyGraph->m_deBruijnGraphEdges.contains(nodePair))
        return g_assemblyGraph->m_deBruijnGraphEdges[nodePair];
    else
        return 0;
}



QTEST_MAIN(BandageTests)
#include "bandagetests.moc"
