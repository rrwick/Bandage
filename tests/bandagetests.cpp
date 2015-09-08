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
    void loadCsvData();
    void blastSearchFilters();

private:
    void createGlobals();
    bool createBlastTempDirectory();
    void deleteBlastTempDirectory();
};




void BandageTests::loadFastg()
{
    createGlobals();
    bool fastgGraphLoaded = g_assemblyGraph->loadGraphFromFile("/Users/Ryan/Programs/Bandage/tests/test.fastg");

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
    bool lastGraphLoaded = g_assemblyGraph->loadGraphFromFile("/Users/Ryan/Programs/Bandage/tests/test.LastGraph");

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
    g_assemblyGraph->loadGraphFromFile("/Users/Ryan/Programs/Bandage/tests/test.LastGraph");

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
    g_assemblyGraph->loadGraphFromFile("/Users/Ryan/Programs/Bandage/tests/test.fastg");

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
    g_assemblyGraph->loadGraphFromFile("/Users/Ryan/Programs/Bandage/tests/test.fastg");
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
    g_assemblyGraph->loadGraphFromFile("/Users/Ryan/Programs/Bandage/tests/test.LastGraph");
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
    g_assemblyGraph->loadGraphFromFile("/Users/Ryan/Programs/Bandage/tests/test.fastg");

    QString errormsg;
    QStringList columns;
    g_assemblyGraph->loadCSV("/Users/Ryan/Programs/Bandage/tests/test.csv", &columns, &errormsg);

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



void BandageTests::blastSearchFilters()
{
    createGlobals();
    g_assemblyGraph->loadGraphFromFile("/Users/Ryan/Programs/Bandage/tests/test.fastg");
    g_settings->blastQueryFilename = "/Users/Ryan/Programs/Bandage/tests/test_queries.fasta";
    createBlastTempDirectory();

    //First do the search with no filters
    g_blastSearch->doAutoBlastSearch();
    int unfilteredHitCount = g_blastSearch->m_allHits.size();

    //Now filter by e-value.
    g_settings->blastEValueFilterOn = true;
    g_settings->blastEValueFilterCoefficientValue = 1.0;
    g_settings->blastEValueFilterExponentValue = -5;
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



QTEST_MAIN(BandageTests)
#include "bandagetests.moc"
