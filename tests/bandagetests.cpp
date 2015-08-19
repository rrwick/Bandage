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

class BandageTests : public QObject
{
    Q_OBJECT

private slots:
    void loadFastg();
    void loadLastGraph();
    void pathFunctionsOnLastGraph();
    void pathFunctionsOnFastg();
    void graphLocationFunctions();

private:
    void createGlobals();
};




void BandageTests::loadFastg()
{
    createGlobals();
    bool fastgGraphLoaded = g_assemblyGraph->loadGraphFromFile("/Users/Ryan/Programs/Bandage/tests/test1.fastg");

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
    bool lastGraphLoaded = g_assemblyGraph->loadGraphFromFile("/Users/Ryan/Programs/Bandage/tests/test1.LastGraph");

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



//LastGraph files have no overlap in the edges, so these tests look at paths
//where the connections are simple.
void BandageTests::pathFunctionsOnLastGraph()
{
    createGlobals();
    g_assemblyGraph->loadGraphFromFile("/Users/Ryan/Programs/Bandage/tests/test1.LastGraph");

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
    g_assemblyGraph->loadGraphFromFile("/Users/Ryan/Programs/Bandage/tests/test1.fastg");

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
    g_assemblyGraph->loadGraphFromFile("/Users/Ryan/Programs/Bandage/tests/test1.fastg");
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
    g_assemblyGraph->loadGraphFromFile("/Users/Ryan/Programs/Bandage/tests/test1.LastGraph");
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

















void BandageTests::createGlobals()
{
    g_settings.reset(new Settings());
    g_memory.reset(new Memory());
    g_blastSearch.reset(new BlastSearch());
    g_assemblyGraph.reset(new AssemblyGraph());
    g_graphicsView = new MyGraphicsView();
}


QTEST_MAIN(BandageTests)
#include "bandagetests.moc"
