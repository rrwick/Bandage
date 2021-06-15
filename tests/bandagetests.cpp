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
#include <QDebug>
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
#include "../program/dotplot.h"

class BandageTests : public QObject
{
    Q_OBJECT

private slots:
    void loadFastg();
    void loadLastGraph();
    void loadTrinity();
    void pathFunctionsOnLastGraph();
    void pathFunctionsOnFastg();
    void pathFunctionsOnGfaSequencesInGraph();
    void pathFunctionsOnGfaSequencesInFasta();
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
    void spadesToGfa();
    void mergeNodesOnGfa();
    void changeNodeNames();
    void changeNodeDepths();
    void blastQueryPaths();
    void bandageInfo();
    void testReverseComplement();
    void testHashKmers();
    void testFindHits();
    void testFindKmerMatches();

private:
    void createGlobals();
    bool createBlastTempDirectory();
    void deleteBlastTempDirectory();
    QString getTestDirectory();
    DeBruijnEdge * getEdgeFromNodeNames(QString startingNodeName,
                                        QString endingNodeName);
    bool doCircularSequencesMatch(QByteArray s1, QByteArray s2);
};




void BandageTests::loadFastg()
{
    createGlobals();
    bool fastgGraphLoaded = g_assemblyGraph->loadGraphFromFile(getTestDirectory() + "test.fastg");

    //Check that the graph loaded properly.
    QCOMPARE(fastgGraphLoaded, true);

    //Check that the appropriate number of nodes/edges are present.
    QCOMPARE(g_assemblyGraph->m_deBruijnGraphNodes.size(), 88);
    QCOMPARE(g_assemblyGraph->m_deBruijnGraphEdges.size(), 118);

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
    QCOMPARE(g_assemblyGraph->m_deBruijnGraphEdges.size(), 32);

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
    QCOMPARE(g_assemblyGraph->m_deBruijnGraphEdges.size(), 1056);

    //Check the length of a couple nodes.
    DeBruijnNode * node10241 = g_assemblyGraph->m_deBruijnGraphNodes["4|c4_10241+"];
    DeBruijnNode * node3901 = g_assemblyGraph->m_deBruijnGraphNodes["19|c0_3901-"];
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


//This function tests paths on a GFA file which keeps its sequences in the GFA
//file.
void BandageTests::pathFunctionsOnGfaSequencesInGraph()
{
    createGlobals();
    bool gfaLoaded = g_assemblyGraph->loadGraphFromFile(getTestDirectory() + "test_plasmids.gfa");
    QCOMPARE(gfaLoaded, true);

    //Check that the number of nodes/edges.
    QCOMPARE(g_assemblyGraph->m_deBruijnGraphNodes.size(), 18);
    QCOMPARE(g_assemblyGraph->m_deBruijnGraphEdges.size(), 24);

    //Check a couple short paths.
    QString pathStringFailure;

    Path testPath1 = Path::makeFromString("232+, 277+", false, &pathStringFailure);
    QByteArray testPath1Sequence = "CCTTATACGAAGCCCAGGTTAATCCTGGGCTTTTTGTTGAATCTGATCATTGGTAGCAAACAGATCAGGATTGGTAATTTTGATGTTTTCCTGACAACTCCTGCAAAGCATCAGCCCAGCAAAAAGTTGTACATGTTCCGTTGATTCACAGAAGGCACATGGCTTAGGAAAAGAGATGATATTGGCTGAATTGACTAATTCTGTTGACATAAGAAGTAACCTTGGATTGTACATATTTATTTTTAATAAATTTCAATACTTTATACCTAATTTAGCCACTAAAATTTGTACATTATTGTATAATCCATGTGTACATATTATTTTTTATAATTTTCATTCACTTAGACCCAAAATAGTATTTATTTTTGTACAACACCATGTACAGAACAATAATCATATAAATCAAATTTTTAGCATAAAAAAGTCCATTAATTTTGTACACAATTCTGAAACTTAAAAATCTAAACTTTCATCAATTTTTTTCATAATTTCAATAAATTAACCTTAATTTTAAGATATATTCTGAAATTTGGTTTGAAAGCTGTTTTTACATTATATTTCAATACTTTAAATCAAAAAATTGGATATTTTTTGAAAAACTCAATGAAAGTTTATTTTTTATTTAAAAACAACTAGTTATATTAGTTTTTATCCATTTTTTTGAAACAGTTTCTATATGATAAAAAAACCTATAAAAACCATATCTAGCAAAGGTTTGAGGGTTATCATCTTTAGATGCGTGGTGTGTGACAAAAAAATCCCGGCATGTGCCGGATTCTGGATTAGAAAATTGGCTAAAGTGACGTAGGACTGGTGCTTGGTTTTACATGGAAAAAAGTATTTATTTTCTGGTTTATAAAAACGTAAAAAGATCAGTTTTTGTTCATTCATCCAGGTTAAAAATTTCAACCTAAAACTTTAATTATGAAAAGCTTCACAGAAAGCATTCAAATGCGATTTAAGAGCCTTTATCTAAAAAACATAGATCTTATAGCGAAAAACAGAAAACAGCTCAAAAAACGCAAAAGAGAGTGAAGTAAAGAGATGTTTTGACTTTAGATAGCTGCATAGAGCGAGTGTCTACGAGCGAACTATCAAAATTTGCGTCTAGACTCTCTGAAAAACATTTTTTTGCCCTCTTTAGCCTAAGAAAGCTTAATTTTCATGCAGAAATTTGCTCCTGGACCGAGCGTAGCGAGAAAAAAAGCTCATGAGCGAAGCGAATTCCGAGTTGCTTTTGCTTTTTCTTAAAGTCACGCAAGTATTAACCAAAAAATTGCCCCGACGAACTGAGCGAAAGCGAAGTTCAATAGAGTTTGAGCGAAGCGAAAACCAAGGGC";
    Path testPath2 = Path::makeFromString("277-, 232-", false, &pathStringFailure);
    QByteArray testPath2Sequence = "GCCCTTGGTTTTCGCTTCGCTCAAACTCTATTGAACTTCGCTTTCGCTCAGTTCGTCGGGGCAATTTTTTGGTTAATACTTGCGTGACTTTAAGAAAAAGCAAAAGCAACTCGGAATTCGCTTCGCTCATGAGCTTTTTTTCTCGCTACGCTCGGTCCAGGAGCAAATTTCTGCATGAAAATTAAGCTTTCTTAGGCTAAAGAGGGCAAAAAAATGTTTTTCAGAGAGTCTAGACGCAAATTTTGATAGTTCGCTCGTAGACACTCGCTCTATGCAGCTATCTAAAGTCAAAACATCTCTTTACTTCACTCTCTTTTGCGTTTTTTGAGCTGTTTTCTGTTTTTCGCTATAAGATCTATGTTTTTTAGATAAAGGCTCTTAAATCGCATTTGAATGCTTTCTGTGAAGCTTTTCATAATTAAAGTTTTAGGTTGAAATTTTTAACCTGGATGAATGAACAAAAACTGATCTTTTTACGTTTTTATAAACCAGAAAATAAATACTTTTTTCCATGTAAAACCAAGCACCAGTCCTACGTCACTTTAGCCAATTTTCTAATCCAGAATCCGGCACATGCCGGGATTTTTTTGTCACACACCACGCATCTAAAGATGATAACCCTCAAACCTTTGCTAGATATGGTTTTTATAGGTTTTTTTATCATATAGAAACTGTTTCAAAAAAATGGATAAAAACTAATATAACTAGTTGTTTTTAAATAAAAAATAAACTTTCATTGAGTTTTTCAAAAAATATCCAATTTTTTGATTTAAAGTATTGAAATATAATGTAAAAACAGCTTTCAAACCAAATTTCAGAATATATCTTAAAATTAAGGTTAATTTATTGAAATTATGAAAAAAATTGATGAAAGTTTAGATTTTTAAGTTTCAGAATTGTGTACAAAATTAATGGACTTTTTTATGCTAAAAATTTGATTTATATGATTATTGTTCTGTACATGGTGTTGTACAAAAATAAATACTATTTTGGGTCTAAGTGAATGAAAATTATAAAAAATAATATGTACACATGGATTATACAATAATGTACAAATTTTAGTGGCTAAATTAGGTATAAAGTATTGAAATTTATTAAAAATAAATATGTACAATCCAAGGTTACTTCTTATGTCAACAGAATTAGTCAATTCAGCCAATATCATCTCTTTTCCTAAGCCATGTGCCTTCTGTGAATCAACGGAACATGTACAACTTTTTGCTGGGCTGATGCTTTGCAGGAGTTGTCAGGAAAACATCAAAATTACCAATCCTGATCTGTTTGCTACCAATGATCAGATTCAACAAAAAGCCCAGGATTAACCTGGGCTTCGTATAAGG";
    QCOMPARE(testPath1.getPathSequence(), testPath1Sequence);
    QCOMPARE(testPath2.getPathSequence(), testPath2Sequence);
}


//This function tests paths on a GFA file which keeps its sequences in a
//separate FASTA file.
void BandageTests::pathFunctionsOnGfaSequencesInFasta()
{
    createGlobals();
    bool gfaLoaded = g_assemblyGraph->loadGraphFromFile(getTestDirectory() + "test_plasmids_separate_sequences.gfa");
    QCOMPARE(gfaLoaded, true);

    //Check that the number of nodes/edges.
    QCOMPARE(g_assemblyGraph->m_deBruijnGraphNodes.size(), 18);
    QCOMPARE(g_assemblyGraph->m_deBruijnGraphEdges.size(), 24);

    //Since a node sequence hasn't be required yet, the nodes should still have
    //just '*' for their sequence. But they should still report the correct
    //length.
    DeBruijnNode * node282Plus = g_assemblyGraph->m_deBruijnGraphNodes["282+"];
    DeBruijnNode * node282Minus = g_assemblyGraph->m_deBruijnGraphNodes["282-"];
    QCOMPARE(node282Plus->sequenceIsMissing(), true);
    QCOMPARE(node282Minus->sequenceIsMissing(), true);
    QCOMPARE(node282Plus->getLength(), 1819);
    QCOMPARE(node282Minus->getLength(), 1819);

    //Check a couple short paths.
    QString pathStringFailure;
    Path testPath1 = Path::makeFromString("232+, 277+", false, &pathStringFailure);
    QByteArray testPath1Sequence = "CCTTATACGAAGCCCAGGTTAATCCTGGGCTTTTTGTTGAATCTGATCATTGGTAGCAAACAGATCAGGATTGGTAATTTTGATGTTTTCCTGACAACTCCTGCAAAGCATCAGCCCAGCAAAAAGTTGTACATGTTCCGTTGATTCACAGAAGGCACATGGCTTAGGAAAAGAGATGATATTGGCTGAATTGACTAATTCTGTTGACATAAGAAGTAACCTTGGATTGTACATATTTATTTTTAATAAATTTCAATACTTTATACCTAATTTAGCCACTAAAATTTGTACATTATTGTATAATCCATGTGTACATATTATTTTTTATAATTTTCATTCACTTAGACCCAAAATAGTATTTATTTTTGTACAACACCATGTACAGAACAATAATCATATAAATCAAATTTTTAGCATAAAAAAGTCCATTAATTTTGTACACAATTCTGAAACTTAAAAATCTAAACTTTCATCAATTTTTTTCATAATTTCAATAAATTAACCTTAATTTTAAGATATATTCTGAAATTTGGTTTGAAAGCTGTTTTTACATTATATTTCAATACTTTAAATCAAAAAATTGGATATTTTTTGAAAAACTCAATGAAAGTTTATTTTTTATTTAAAAACAACTAGTTATATTAGTTTTTATCCATTTTTTTGAAACAGTTTCTATATGATAAAAAAACCTATAAAAACCATATCTAGCAAAGGTTTGAGGGTTATCATCTTTAGATGCGTGGTGTGTGACAAAAAAATCCCGGCATGTGCCGGATTCTGGATTAGAAAATTGGCTAAAGTGACGTAGGACTGGTGCTTGGTTTTACATGGAAAAAAGTATTTATTTTCTGGTTTATAAAAACGTAAAAAGATCAGTTTTTGTTCATTCATCCAGGTTAAAAATTTCAACCTAAAACTTTAATTATGAAAAGCTTCACAGAAAGCATTCAAATGCGATTTAAGAGCCTTTATCTAAAAAACATAGATCTTATAGCGAAAAACAGAAAACAGCTCAAAAAACGCAAAAGAGAGTGAAGTAAAGAGATGTTTTGACTTTAGATAGCTGCATAGAGCGAGTGTCTACGAGCGAACTATCAAAATTTGCGTCTAGACTCTCTGAAAAACATTTTTTTGCCCTCTTTAGCCTAAGAAAGCTTAATTTTCATGCAGAAATTTGCTCCTGGACCGAGCGTAGCGAGAAAAAAAGCTCATGAGCGAAGCGAATTCCGAGTTGCTTTTGCTTTTTCTTAAAGTCACGCAAGTATTAACCAAAAAATTGCCCCGACGAACTGAGCGAAAGCGAAGTTCAATAGAGTTTGAGCGAAGCGAAAACCAAGGGC";
    Path testPath2 = Path::makeFromString("277-, 232-", false, &pathStringFailure);
    QByteArray testPath2Sequence = "GCCCTTGGTTTTCGCTTCGCTCAAACTCTATTGAACTTCGCTTTCGCTCAGTTCGTCGGGGCAATTTTTTGGTTAATACTTGCGTGACTTTAAGAAAAAGCAAAAGCAACTCGGAATTCGCTTCGCTCATGAGCTTTTTTTCTCGCTACGCTCGGTCCAGGAGCAAATTTCTGCATGAAAATTAAGCTTTCTTAGGCTAAAGAGGGCAAAAAAATGTTTTTCAGAGAGTCTAGACGCAAATTTTGATAGTTCGCTCGTAGACACTCGCTCTATGCAGCTATCTAAAGTCAAAACATCTCTTTACTTCACTCTCTTTTGCGTTTTTTGAGCTGTTTTCTGTTTTTCGCTATAAGATCTATGTTTTTTAGATAAAGGCTCTTAAATCGCATTTGAATGCTTTCTGTGAAGCTTTTCATAATTAAAGTTTTAGGTTGAAATTTTTAACCTGGATGAATGAACAAAAACTGATCTTTTTACGTTTTTATAAACCAGAAAATAAATACTTTTTTCCATGTAAAACCAAGCACCAGTCCTACGTCACTTTAGCCAATTTTCTAATCCAGAATCCGGCACATGCCGGGATTTTTTTGTCACACACCACGCATCTAAAGATGATAACCCTCAAACCTTTGCTAGATATGGTTTTTATAGGTTTTTTTATCATATAGAAACTGTTTCAAAAAAATGGATAAAAACTAATATAACTAGTTGTTTTTAAATAAAAAATAAACTTTCATTGAGTTTTTCAAAAAATATCCAATTTTTTGATTTAAAGTATTGAAATATAATGTAAAAACAGCTTTCAAACCAAATTTCAGAATATATCTTAAAATTAAGGTTAATTTATTGAAATTATGAAAAAAATTGATGAAAGTTTAGATTTTTAAGTTTCAGAATTGTGTACAAAATTAATGGACTTTTTTATGCTAAAAATTTGATTTATATGATTATTGTTCTGTACATGGTGTTGTACAAAAATAAATACTATTTTGGGTCTAAGTGAATGAAAATTATAAAAAATAATATGTACACATGGATTATACAATAATGTACAAATTTTAGTGGCTAAATTAGGTATAAAGTATTGAAATTTATTAAAAATAAATATGTACAATCCAAGGTTACTTCTTATGTCAACAGAATTAGTCAATTCAGCCAATATCATCTCTTTTCCTAAGCCATGTGCCTTCTGTGAATCAACGGAACATGTACAACTTTTTGCTGGGCTGATGCTTTGCAGGAGTTGTCAGGAAAACATCAAAATTACCAATCCTGATCTGTTTGCTACCAATGATCAGATTCAACAAAAAGCCCAGGATTAACCTGGGCTTCGTATAAGG";

    //Check the paths sequences. Doing so will trigger the loading of node
    //sequences from the FASTA file.
    QCOMPARE(testPath1.getPathSequence(), testPath1Sequence);
    QCOMPARE(testPath2.getPathSequence(), testPath2Sequence);

    //Now that the paths have been accessed, the node sequences should be
    //loaded (even the ones not in the path).
    QCOMPARE(node282Plus->sequenceIsMissing(), false);
    QCOMPARE(node282Minus->sequenceIsMissing(), false);
    QCOMPARE(node282Plus->getLength(), 1819);
    QCOMPARE(node282Minus->getLength(), 1819);
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
    bool coloursLoaded = false;
    g_assemblyGraph->loadCSV(getTestDirectory() + "test.csv", &columns, &errormsg, &coloursLoaded);

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
    bool coloursLoaded = false;
    g_assemblyGraph->loadCSV(getTestDirectory() + "test.Trinity.csv", &columns, &errormsg, &coloursLoaded);

    DeBruijnNode * node3912Plus = g_assemblyGraph->m_deBruijnGraphNodes["19|c0_3912+"];
    DeBruijnNode * node3912Minus = g_assemblyGraph->m_deBruijnGraphNodes["19|c0_3912-"];
    DeBruijnNode * node3914Plus = g_assemblyGraph->m_deBruijnGraphNodes["19|c0_3914+"];
    DeBruijnNode * node3915Plus = g_assemblyGraph->m_deBruijnGraphNodes["19|c0_3915+"];
    DeBruijnNode * node3923Plus = g_assemblyGraph->m_deBruijnGraphNodes["19|c0_3923+"];
    DeBruijnNode * node3924Plus = g_assemblyGraph->m_deBruijnGraphNodes["19|c0_3924+"];
    DeBruijnNode * node3940Plus = g_assemblyGraph->m_deBruijnGraphNodes["19|c0_3940+"];

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
    g_settings->blastEValueFilter.on = true;
    g_settings->blastEValueFilter = SciNot(1.0, -5);
    g_blastSearch->doAutoBlastSearch();
    QCOMPARE(g_blastSearch->m_allHits.size(), 14);
    QCOMPARE(g_blastSearch->m_allHits.size() < unfilteredHitCount, true);

    //Now add a bit score filter.
    g_settings->blastBitScoreFilter.on = true;
    g_settings->blastBitScoreFilter = 100.0;
    g_blastSearch->doAutoBlastSearch();
    QCOMPARE(g_blastSearch->m_allHits.size(), 9);

    //Now add an alignment length filter.
    g_settings->blastAlignmentLengthFilter.on = true;
    g_settings->blastAlignmentLengthFilter = 100;
    g_blastSearch->doAutoBlastSearch();
    QCOMPARE(g_blastSearch->m_allHits.size(), 8);

    //Now add an identity filter.
    g_settings->blastIdentityFilter.on = true;
    g_settings->blastIdentityFilter = 50.0;
    g_blastSearch->doAutoBlastSearch();
    QCOMPARE(g_blastSearch->m_allHits.size(), 7);

    //Now add a query coverage filter.
    g_settings->blastQueryCoverageFilter.on = true;
    g_settings->blastQueryCoverageFilter = 90.0;
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

    g_settings->graphScope = DEPTH_RANGE;
    g_settings->nodeDistance = 0;
    g_settings->doubleMode = false;
    g_settings->minDepthRange = 0.0;
    g_settings->maxDepthRange = 211.0;
    startingNodes = g_assemblyGraph->getStartingNodes(&errorTitle, &errorMessage, g_settings->doubleMode, g_settings->startingNodes, "");
    g_assemblyGraph->buildOgdfGraphFromNodesAndEdges(startingNodes, g_settings->nodeDistance);
    g_assemblyGraph->layoutGraph();
    drawnNodes = g_assemblyGraph->getDrawnNodeCount();
    QCOMPARE(drawnNodes, 43);

    g_settings->graphScope = DEPTH_RANGE;
    g_settings->nodeDistance = 10;
    g_settings->doubleMode = false;
    g_settings->minDepthRange = 0.0;
    g_settings->maxDepthRange = 211.0;
    startingNodes = g_assemblyGraph->getStartingNodes(&errorTitle, &errorMessage, g_settings->doubleMode, g_settings->startingNodes, "");
    g_assemblyGraph->buildOgdfGraphFromNodesAndEdges(startingNodes, g_settings->nodeDistance);
    g_assemblyGraph->layoutGraph();
    drawnNodes = g_assemblyGraph->getDrawnNodeCount();
    QCOMPARE(drawnNodes, 43);

    g_settings->graphScope = DEPTH_RANGE;
    g_settings->nodeDistance = 0;
    g_settings->doubleMode = false;
    g_settings->minDepthRange = 211.0;
    g_settings->maxDepthRange = 1000.0;
    startingNodes = g_assemblyGraph->getStartingNodes(&errorTitle, &errorMessage, g_settings->doubleMode, g_settings->startingNodes, "");
    g_assemblyGraph->buildOgdfGraphFromNodesAndEdges(startingNodes, g_settings->nodeDistance);
    g_assemblyGraph->layoutGraph();
    drawnNodes = g_assemblyGraph->getDrawnNodeCount();
    QCOMPARE(drawnNodes, 1);

    g_settings->graphScope = DEPTH_RANGE;
    g_settings->nodeDistance = 0;
    g_settings->doubleMode = false;
    g_settings->minDepthRange = 40.0;
    g_settings->maxDepthRange = 211.0;
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
    QCOMPARE(g_settings->graphScope, DEPTH_RANGE);

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
    QCOMPARE(g_settings->nodeDistance.val, 12);

    commandLineSettings = QString("--mindepth 1.2").split(" ");
    parseSettings(commandLineSettings);
    QCOMPARE(g_settings->minDepthRange.val, 1.2);

    commandLineSettings = QString("--maxdepth 2.1").split(" ");
    parseSettings(commandLineSettings);
    QCOMPARE(g_settings->maxDepthRange.val, 2.1);

    QCOMPARE(g_settings->doubleMode, false);
    commandLineSettings = QString("--double").split(" ");
    parseSettings(commandLineSettings);
    QCOMPARE(g_settings->doubleMode, true);

    QCOMPARE(g_settings->nodeLengthMode, AUTO_NODE_LENGTH);
    commandLineSettings = QString("--nodelen 10000").split(" ");
    parseSettings(commandLineSettings);
    QCOMPARE(g_settings->nodeLengthMode, MANUAL_NODE_LENGTH);
    QCOMPARE(g_settings->manualNodeLengthPerMegabase.val, 10000.0);

    commandLineSettings = QString("--iter 1").split(" ");
    parseSettings(commandLineSettings);
    QCOMPARE(g_settings->graphLayoutQuality.val, 1);

    commandLineSettings = QString("--nodewidth 4.2").split(" ");
    parseSettings(commandLineSettings);
    QCOMPARE(g_settings->averageNodeWidth.val, 4.2);

    commandLineSettings = QString("--depwidth 0.222").split(" ");
    parseSettings(commandLineSettings);
    QCOMPARE(g_settings->depthEffectOnWidth.val, 0.222);

    commandLineSettings = QString("--deppower 0.72").split(" ");
    parseSettings(commandLineSettings);
    QCOMPARE(g_settings->depthPower.val, 0.72);

    QCOMPARE(g_settings->displayNodeNames, false);
    commandLineSettings = QString("--names").split(" ");
    parseSettings(commandLineSettings);
    QCOMPARE(g_settings->displayNodeNames, true);

    QCOMPARE(g_settings->displayNodeLengths, false);
    commandLineSettings = QString("--lengths").split(" ");
    parseSettings(commandLineSettings);
    QCOMPARE(g_settings->displayNodeLengths, true);

    QCOMPARE(g_settings->displayNodeDepth, false);
    commandLineSettings = QString("--depth").split(" ");
    parseSettings(commandLineSettings);
    QCOMPARE(g_settings->displayNodeDepth, true);

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
    QCOMPARE(g_settings->edgeWidth.val, 5.5);

    commandLineSettings = QString("--outcol #ff0000").split(" ");
    parseSettings(commandLineSettings);
    QCOMPARE(g_settings->outlineColour.name(), QString("#ff0000"));

    commandLineSettings = QString("--outline 0.123").split(" ");
    parseSettings(commandLineSettings);
    QCOMPARE(g_settings->outlineThickness.val, 0.123);

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
    QCOMPARE(g_settings->textOutlineThickness.val, 0.321);

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

    commandLineSettings = QString("--colour depth").split(" ");
    parseSettings(commandLineSettings);
    QCOMPARE(g_settings->nodeColourScheme, DEPTH_COLOUR);

    commandLineSettings = QString("--colour blastsolid").split(" ");
    parseSettings(commandLineSettings);
    QCOMPARE(g_settings->nodeColourScheme, BLAST_HITS_SOLID_COLOUR);

    commandLineSettings = QString("--colour blastrainbow").split(" ");
    parseSettings(commandLineSettings);
    QCOMPARE(g_settings->nodeColourScheme, BLAST_HITS_RAINBOW_COLOUR);

    commandLineSettings = QString("--ransatpos 12").split(" ");
    parseSettings(commandLineSettings);
    QCOMPARE(g_settings->randomColourPositiveSaturation.val, 12);

    commandLineSettings = QString("--ransatneg 23").split(" ");
    parseSettings(commandLineSettings);
    QCOMPARE(g_settings->randomColourNegativeSaturation.val, 23);

    commandLineSettings = QString("--ranligpos 34").split(" ");
    parseSettings(commandLineSettings);
    QCOMPARE(g_settings->randomColourPositiveLightness.val, 34);

    commandLineSettings = QString("--ranligneg 45").split(" ");
    parseSettings(commandLineSettings);
    QCOMPARE(g_settings->randomColourNegativeLightness.val, 45);

    commandLineSettings = QString("--ranopapos 56").split(" ");
    parseSettings(commandLineSettings);
    QCOMPARE(g_settings->randomColourPositiveOpacity.val, 56);

    commandLineSettings = QString("--ranopaneg 67").split(" ");
    parseSettings(commandLineSettings);
    QCOMPARE(g_settings->randomColourNegativeOpacity.val, 67);

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
    QCOMPARE(getColourName(g_settings->lowDepthColour), QString("mediumorchid"));

    commandLineSettings = QString("--depcolhi linen").split(" ");
    parseSettings(commandLineSettings);
    QCOMPARE(getColourName(g_settings->highDepthColour), QString("linen"));

    QCOMPARE(g_settings->autoDepthValue, true);
    commandLineSettings = QString("--depvallow 56.7").split(" ");
    parseSettings(commandLineSettings);
    QCOMPARE(g_settings->lowDepthValue.val, 56.7);
    QCOMPARE(g_settings->autoDepthValue, false);

    commandLineSettings = QString("--depvalhi 67.8").split(" ");
    parseSettings(commandLineSettings);
    QCOMPARE(g_settings->highDepthValue.val, 67.8);

    commandLineSettings = QString("--depvalhi 67.8").split(" ");
    parseSettings(commandLineSettings);
    QCOMPARE(g_settings->highDepthValue.val, 67.8);

    commandLineSettings = QString("--query queries.fasta").split(" ");
    parseSettings(commandLineSettings);
    QCOMPARE(g_settings->blastQueryFilename, QString("queries.fasta"));

    commandLineSettings = QString("--blastp --abc").split(" ");
    parseSettings(commandLineSettings);
    QCOMPARE(g_settings->blastSearchParameters, QString("--abc"));

    QCOMPARE(g_settings->blastAlignmentLengthFilter.on, false);
    commandLineSettings = QString("--alfilter 543").split(" ");
    parseSettings(commandLineSettings);
    QCOMPARE(g_settings->blastAlignmentLengthFilter.on, true);
    QCOMPARE(g_settings->blastAlignmentLengthFilter.val, 543);

    QCOMPARE(g_settings->blastQueryCoverageFilter.on, false);
    commandLineSettings = QString("--qcfilter 67.8").split(" ");
    parseSettings(commandLineSettings);
    QCOMPARE(g_settings->blastQueryCoverageFilter.on, true);
    QCOMPARE(g_settings->blastQueryCoverageFilter.val, 67.8);

    QCOMPARE(g_settings->blastIdentityFilter.on, false);
    commandLineSettings = QString("--ifilter 12.3").split(" ");
    parseSettings(commandLineSettings);
    QCOMPARE(g_settings->blastIdentityFilter.on, true);
    QCOMPARE(g_settings->blastIdentityFilter.val, 12.3);

    QCOMPARE(g_settings->blastEValueFilter.on, false);
    commandLineSettings = QString("--evfilter 8.5e-14").split(" ");
    parseSettings(commandLineSettings);
    QCOMPARE(g_settings->blastEValueFilter.on, true);
    QCOMPARE(g_settings->blastEValueFilter.val, SciNot(8.5, -14));
    QCOMPARE(g_settings->blastEValueFilter.val.getCoefficient(), 8.5);
    QCOMPARE(g_settings->blastEValueFilter.val.getExponent(), -14);

    QCOMPARE(g_settings->blastBitScoreFilter.on, false);
    commandLineSettings = QString("--bsfilter 1234.5").split(" ");
    parseSettings(commandLineSettings);
    QCOMPARE(g_settings->blastBitScoreFilter.on, true);
    QCOMPARE(g_settings->blastBitScoreFilter.val, 1234.5);

    commandLineSettings = QString("--pathnodes 3").split(" ");
    parseSettings(commandLineSettings);
    QCOMPARE(g_settings->maxQueryPathNodes.val, 3);

    commandLineSettings = QString("--minpatcov 0.543").split(" ");
    parseSettings(commandLineSettings);
    QCOMPARE(g_settings->minQueryCoveredByPath.val, 0.543);

    commandLineSettings = QString("--minhitcov 0.654").split(" ");
    parseSettings(commandLineSettings);
    QCOMPARE(g_settings->minQueryCoveredByHits.val, 0.654);
    QCOMPARE(g_settings->minQueryCoveredByHits.on, true);

    commandLineSettings = QString("--minhitcov off").split(" ");
    parseSettings(commandLineSettings);
    QCOMPARE(g_settings->minQueryCoveredByHits.on, false);

    commandLineSettings = QString("--minmeanid 0.765").split(" ");
    parseSettings(commandLineSettings);
    QCOMPARE(g_settings->minMeanHitIdentity.val, 0.765);
    QCOMPARE(g_settings->minMeanHitIdentity.on, true);

    commandLineSettings = QString("--minmeanid off").split(" ");
    parseSettings(commandLineSettings);
    QCOMPARE(g_settings->minMeanHitIdentity.on, false);

    commandLineSettings = QString("--minpatlen 0.97").split(" ");
    parseSettings(commandLineSettings);
    QCOMPARE(g_settings->minLengthPercentage.val, 0.97);
    QCOMPARE(g_settings->minLengthPercentage.on, true);

    commandLineSettings = QString("--minpatlen off").split(" ");
    parseSettings(commandLineSettings);
    QCOMPARE(g_settings->minLengthPercentage.on, false);

    commandLineSettings = QString("--maxpatlen 1.03").split(" ");
    parseSettings(commandLineSettings);
    QCOMPARE(g_settings->maxLengthPercentage.val, 1.03);
    QCOMPARE(g_settings->maxLengthPercentage.on, true);

    commandLineSettings = QString("--maxpatlen off").split(" ");
    parseSettings(commandLineSettings);
    QCOMPARE(g_settings->maxLengthPercentage.on, false);

    commandLineSettings = QString("--minlendis -1234").split(" ");
    parseSettings(commandLineSettings);
    QCOMPARE(g_settings->minLengthBaseDiscrepancy.val, -1234);
    QCOMPARE(g_settings->minLengthBaseDiscrepancy.on, true);

    commandLineSettings = QString("--minlendis off").split(" ");
    parseSettings(commandLineSettings);
    QCOMPARE(g_settings->minLengthBaseDiscrepancy.on, false);

    commandLineSettings = QString("--maxlendis 4321").split(" ");
    parseSettings(commandLineSettings);
    QCOMPARE(g_settings->maxLengthBaseDiscrepancy.val, 4321);
    QCOMPARE(g_settings->maxLengthBaseDiscrepancy.on, true);

    commandLineSettings = QString("--maxlendis off").split(" ");
    parseSettings(commandLineSettings);
    QCOMPARE(g_settings->maxLengthBaseDiscrepancy.on, false);

    commandLineSettings = QString("--maxevprod 4e-500").split(" ");
    parseSettings(commandLineSettings);
    QCOMPARE(g_settings->maxEValueProduct.val.getCoefficient(), 4.0);
    QCOMPARE(g_settings->maxEValueProduct.val.getExponent(), -500);
    QCOMPARE(g_settings->maxEValueProduct.on, true);

    commandLineSettings = QString("--maxevprod off").split(" ");
    parseSettings(commandLineSettings);
    QCOMPARE(g_settings->maxEValueProduct.on, false);
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
    g_assemblyGraph->saveEntireGraphToGfa(getTestDirectory() + "big_test_temp.gfa");
    createGlobals();
    g_assemblyGraph->loadGraphFromFile(getTestDirectory() + "big_test_temp.gfa");

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
    QFile::remove(getTestDirectory() + "big_test_temp.gfa");
}


void BandageTests::spadesToGfa()
{
    //First load the graph as a FASTG and pull out some information and a
    //path sequence.
    createGlobals();
    g_assemblyGraph->loadGraphFromFile(getTestDirectory() + "test.fastg");

    int fastgNodeCount = g_assemblyGraph->m_nodeCount;
    int fastgEdgeCount= g_assemblyGraph->m_edgeCount;
    long long fastgTotalLength = g_assemblyGraph->m_totalLength;
    long long fastgShortestContig = g_assemblyGraph->m_shortestContig;
    long long fastgLongestContig = g_assemblyGraph->m_longestContig;

    QString pathStringFailure;
    Path fastgTestPath1 = Path::makeFromString("24+, 14+, 39-, 43-, 42-, 2-, 4+, 33+, 35-, 31-, 44-, 27+, 9-, 28-, 44+, 31+, 36+", false, &pathStringFailure);
    Path fastgTestPath2 = Path::makeFromString("16+, 7+, 13+, 37+, 41-, 40-, 42+, 43+, 39+, 14-, 22-, 20+, 7-, 25-, 32-, 38+, 3-", false, &pathStringFailure);
    QByteArray fastgTestPath1Sequence = fastgTestPath1.getPathSequence();
    QByteArray fastgTestPath2Sequence = fastgTestPath2.getPathSequence();

    //Now save the graph as a GFA and reload it and grab the same information.
    g_assemblyGraph->saveEntireGraphToGfa(getTestDirectory() + "test_temp.gfa");
    createGlobals();
    g_assemblyGraph->loadGraphFromFile(getTestDirectory() + "test_temp.gfa");

    int gfaNodeCount = g_assemblyGraph->m_nodeCount;
    int gfaEdgeCount= g_assemblyGraph->m_edgeCount;
    long long gfaTotalLength = g_assemblyGraph->m_totalLength;
    long long gfaShortestContig = g_assemblyGraph->m_shortestContig;
    long long gfaLongestContig = g_assemblyGraph->m_longestContig;

    Path gfaTestPath1 = Path::makeFromString("24+, 14+, 39-, 43-, 42-, 2-, 4+, 33+, 35-, 31-, 44-, 27+, 9-, 28-, 44+, 31+, 36+", false, &pathStringFailure);
    Path gfaTestPath2 = Path::makeFromString("16+, 7+, 13+, 37+, 41-, 40-, 42+, 43+, 39+, 14-, 22-, 20+, 7-, 25-, 32-, 38+, 3-", false, &pathStringFailure);
    QByteArray gfaTestPath1Sequence = gfaTestPath1.getPathSequence();
    QByteArray gfaTestPath2Sequence = gfaTestPath2.getPathSequence();

    //Now we compare the LastGraph info to the GFA info to make sure they are
    //the same (or appropriately different).  The k-mer size for this graph is
    //51, so we expect each node to get 50 base pairs longer.
    QCOMPARE(fastgNodeCount, gfaNodeCount);
    QCOMPARE(fastgEdgeCount, gfaEdgeCount);
    QCOMPARE(fastgTotalLength, gfaTotalLength);
    QCOMPARE(fastgShortestContig, gfaShortestContig);
    QCOMPARE(fastgLongestContig, gfaLongestContig);
    QCOMPARE(fastgTestPath1Sequence, gfaTestPath1Sequence);
    QCOMPARE(fastgTestPath2Sequence, gfaTestPath2Sequence);

    //Finally, delete the gfa file.
    QFile::remove(getTestDirectory() + "test_temp.gfa");
}


void BandageTests::mergeNodesOnGfa()
{
    createGlobals();
    g_assemblyGraph->loadGraphFromFile(getTestDirectory() + "test_plasmids.gfa");

    DeBruijnNode * node6Plus = g_assemblyGraph->m_deBruijnGraphNodes["6+"];
    DeBruijnNode * node280Plus = g_assemblyGraph->m_deBruijnGraphNodes["280+"];
    DeBruijnNode * node232Minus = g_assemblyGraph->m_deBruijnGraphNodes["232-"];
    DeBruijnNode * node333Plus = g_assemblyGraph->m_deBruijnGraphNodes["333+"];
    DeBruijnNode * node289Plus = g_assemblyGraph->m_deBruijnGraphNodes["289+"];
    DeBruijnNode * node283Plus = g_assemblyGraph->m_deBruijnGraphNodes["283+"];
    DeBruijnNode * node277Plus = g_assemblyGraph->m_deBruijnGraphNodes["277+"];
    DeBruijnNode * node297Plus = g_assemblyGraph->m_deBruijnGraphNodes["297+"];
    DeBruijnNode * node282Plus = g_assemblyGraph->m_deBruijnGraphNodes["282+"];

    //Create a path before merging the nodes.
    QString pathStringFailure;
    Path testPath1 = Path::makeFromString("6+, 280+, 232-, 333+, 289+, 283+", true, &pathStringFailure);
    QByteArray path1Sequence = testPath1.getPathSequence();

    int path1Length = testPath1.getLength();
    int nodeTotalLength = node6Plus->getLength() + node280Plus->getLength() + node232Minus->getLength() +
            node333Plus->getLength() + node289Plus->getLength() + node283Plus->getLength();

    //The k-mer size in this test is 81, so the path should remove all of those overlaps.
    QCOMPARE(path1Length, nodeTotalLength - 6 * 81);

    //Now remove excess nodes.
    std::vector<DeBruijnNode*> nodesToDelete;
    nodesToDelete.push_back(node277Plus);
    nodesToDelete.push_back(node297Plus);
    nodesToDelete.push_back(node282Plus);
    g_assemblyGraph->deleteNodes(&nodesToDelete);

    //There should now be six nodes in the graph (plus complements).
    QCOMPARE(12, g_assemblyGraph->m_deBruijnGraphNodes.size());

    //After a merge, there should be only one node (and its complement).
    g_assemblyGraph->mergeAllPossible();
    QCOMPARE(2, g_assemblyGraph->m_deBruijnGraphNodes.size());

    //That last node should have a length of its six constituent nodes, minus
    //the overlaps.
    DeBruijnNode * lastNode = g_assemblyGraph->m_deBruijnGraphNodes.first();
    QCOMPARE(lastNode->getLength(), nodeTotalLength - 5 * 81);

    //If we make a circular path with this node, its length should be equal to
    //the length of the path made before.
    Path testPath2 = Path::makeFromString(lastNode->getName(), true, &pathStringFailure);
    QCOMPARE(path1Length, testPath2.getLength());

    //The sequence of this second path should also match the sequence of the
    //first path.
    QByteArray path2Sequence = testPath2.getPathSequence();
    QCOMPARE(doCircularSequencesMatch(path1Sequence, path2Sequence), true);
}



void BandageTests::changeNodeNames()
{
    createGlobals();
    g_assemblyGraph->loadGraphFromFile(getTestDirectory() + "test.fastg");

    DeBruijnNode * node6Plus = g_assemblyGraph->m_deBruijnGraphNodes["6+"];
    DeBruijnNode * node6Minus = g_assemblyGraph->m_deBruijnGraphNodes["6-"];
    int nodeCountBefore = g_assemblyGraph->m_deBruijnGraphNodes.size();

    g_assemblyGraph->changeNodeName("6", "12345");

    DeBruijnNode * node12345Plus = g_assemblyGraph->m_deBruijnGraphNodes["12345+"];
    DeBruijnNode * node12345Minus = g_assemblyGraph->m_deBruijnGraphNodes["12345-"];
    int nodeCountAfter = g_assemblyGraph->m_deBruijnGraphNodes.size();

    QCOMPARE(node6Plus, node12345Plus);
    QCOMPARE(node6Minus, node12345Minus);
    QCOMPARE(nodeCountBefore, nodeCountAfter);
}

void BandageTests::changeNodeDepths()
{
    createGlobals();
    g_assemblyGraph->loadGraphFromFile(getTestDirectory() + "test.fastg");

    DeBruijnNode * node6Plus = g_assemblyGraph->m_deBruijnGraphNodes["6+"];
    DeBruijnNode * node6Minus = g_assemblyGraph->m_deBruijnGraphNodes["6-"];
    DeBruijnNode * node7Plus = g_assemblyGraph->m_deBruijnGraphNodes["7+"];
    DeBruijnNode * node7Minus = g_assemblyGraph->m_deBruijnGraphNodes["7-"];

    std::vector<DeBruijnNode *> nodes;
    nodes.push_back(node6Plus);
    nodes.push_back(node7Plus);

    //Complementary pairs should have the same depth.
    QCOMPARE(node6Plus->getDepth(), node6Minus->getDepth());
    QCOMPARE(node7Plus->getDepth(), node7Minus->getDepth());

    g_assemblyGraph->changeNodeDepth(&nodes, 0.5);

    //Check to make sure the change worked.
    QCOMPARE(0.5, node6Plus->getDepth());
    QCOMPARE(0.5, node6Minus->getDepth());
    QCOMPARE(0.5, node7Plus->getDepth());
    QCOMPARE(0.5, node7Minus->getDepth());
}

void BandageTests::blastQueryPaths()
{
    createGlobals();
    g_assemblyGraph->loadGraphFromFile(getTestDirectory() + "test_query_paths.gfa");

    Settings defaultSettings;
    g_settings->blastQueryFilename = getTestDirectory() + "test_query_paths.fasta";
    defaultSettings.blastQueryFilename = getTestDirectory() + "test_query_paths.fasta";

    createBlastTempDirectory();

    //Now filter by e-value to get only strong hits and do the BLAST search.
    g_settings->blastEValueFilter.on = true;
    g_settings->blastEValueFilter = SciNot(1.0, -5);

    QList<BlastQueryPath> query1Paths;
    QList<BlastQueryPath> query2Paths;
    QList<BlastQueryPath> query3Paths;
    QList<BlastQueryPath> query4Paths;
    QList<BlastQueryPath> query5Paths;
    QList<BlastQueryPath> query6Paths;
    QList<BlastQueryPath> query7Paths;

    //With the default settings, queries 1 to 5 should each have one path and
    //queries 6 and 7 should have 0 (because of their large inserts).
    g_blastSearch->doAutoBlastSearch();
    query1Paths = g_blastSearch->m_blastQueries.m_queries[0]->getPaths();
    query2Paths = g_blastSearch->m_blastQueries.m_queries[1]->getPaths();
    query3Paths = g_blastSearch->m_blastQueries.m_queries[2]->getPaths();
    query4Paths = g_blastSearch->m_blastQueries.m_queries[3]->getPaths();
    query5Paths = g_blastSearch->m_blastQueries.m_queries[4]->getPaths();
    query6Paths = g_blastSearch->m_blastQueries.m_queries[5]->getPaths();
    query7Paths = g_blastSearch->m_blastQueries.m_queries[6]->getPaths();
    QCOMPARE(query1Paths.size(), 1);
    QCOMPARE(query2Paths.size(), 1);
    QCOMPARE(query3Paths.size(), 1);
    QCOMPARE(query4Paths.size(), 1);
    QCOMPARE(query5Paths.size(), 1);
    QCOMPARE(query6Paths.size(), 0);
    QCOMPARE(query7Paths.size(), 0);

    //query2 has a mean hit identity of 0.98.
    g_settings->minMeanHitIdentity.on = true;
    g_settings->minMeanHitIdentity = 0.979;
    g_blastSearch->doAutoBlastSearch();
    query2Paths = g_blastSearch->m_blastQueries.m_queries[1]->getPaths();
    QCOMPARE(query2Paths.size(), 1);
    g_settings->minMeanHitIdentity = 0.981;
    g_blastSearch->doAutoBlastSearch();
    query2Paths = g_blastSearch->m_blastQueries.m_queries[1]->getPaths();
    QCOMPARE(query2Paths.size(), 0);

    //Turning the filter off should make the path return.
    g_settings->minMeanHitIdentity.on = false;
    g_blastSearch->doAutoBlastSearch();
    query2Paths = g_blastSearch->m_blastQueries.m_queries[1]->getPaths();
    QCOMPARE(query2Paths.size(), 1);
    *g_settings = defaultSettings;

    //query3 has a length discrepancy of -20.
    g_settings->minLengthBaseDiscrepancy.on = true;
    g_settings->minLengthBaseDiscrepancy = -20;
    g_blastSearch->doAutoBlastSearch();
    query3Paths = g_blastSearch->m_blastQueries.m_queries[2]->getPaths();
    QCOMPARE(query3Paths.size(), 1);
    g_settings->minLengthBaseDiscrepancy = -19;
    g_blastSearch->doAutoBlastSearch();
    query3Paths = g_blastSearch->m_blastQueries.m_queries[2]->getPaths();
    QCOMPARE(query3Paths.size(), 0);

    //Turning the filter off should make the path return.
    g_settings->minLengthBaseDiscrepancy.on = false;
    g_blastSearch->doAutoBlastSearch();
    query3Paths = g_blastSearch->m_blastQueries.m_queries[2]->getPaths();
    QCOMPARE(query3Paths.size(), 1);
    *g_settings = defaultSettings;

    //query4 has a length discrepancy of +20.
    g_settings->maxLengthBaseDiscrepancy.on = true;
    g_settings->maxLengthBaseDiscrepancy = 20;
    g_blastSearch->doAutoBlastSearch();
    query4Paths = g_blastSearch->m_blastQueries.m_queries[3]->getPaths();
    QCOMPARE(query4Paths.size(), 1);
    g_settings->maxLengthBaseDiscrepancy = 19;
    g_blastSearch->doAutoBlastSearch();
    query4Paths = g_blastSearch->m_blastQueries.m_queries[3]->getPaths();
    QCOMPARE(query4Paths.size(), 0);

    //Turning the filter off should make the path return.
    g_settings->maxLengthBaseDiscrepancy.on = false;
    g_blastSearch->doAutoBlastSearch();
    query4Paths = g_blastSearch->m_blastQueries.m_queries[3]->getPaths();
    QCOMPARE(query4Paths.size(), 1);
    *g_settings = defaultSettings;

    //query5 has a path through 4 nodes.
    g_settings->maxQueryPathNodes = 4;
    g_blastSearch->doAutoBlastSearch();
    query5Paths = g_blastSearch->m_blastQueries.m_queries[4]->getPaths();
    QCOMPARE(query5Paths.size(), 1);
    g_settings->maxQueryPathNodes = 3;
    g_blastSearch->doAutoBlastSearch();
    query5Paths = g_blastSearch->m_blastQueries.m_queries[4]->getPaths();
    QCOMPARE(query5Paths.size(), 0);
    *g_settings = defaultSettings;

    //By turning off length restrictions, queries 6 and 7 should get path with
    //a large insert in the middle.
    g_settings->minLengthPercentage.on = false;
    g_settings->maxLengthPercentage.on = false;
    g_settings->minLengthBaseDiscrepancy.on = false;
    g_settings->maxLengthBaseDiscrepancy.on = false;
    g_blastSearch->doAutoBlastSearch();
    query6Paths = g_blastSearch->m_blastQueries.m_queries[5]->getPaths();
    query7Paths = g_blastSearch->m_blastQueries.m_queries[6]->getPaths();
    QCOMPARE(query6Paths.size(), 1);
    QCOMPARE(query7Paths.size(), 1);

    //Adjusting on the max length restriction can allow query 6 to get a path
    //and then query 7.
    g_settings->maxLengthBaseDiscrepancy.on = true;
    g_settings->maxLengthBaseDiscrepancy = 1999;
    g_blastSearch->doAutoBlastSearch();
    query6Paths = g_blastSearch->m_blastQueries.m_queries[5]->getPaths();
    query7Paths = g_blastSearch->m_blastQueries.m_queries[6]->getPaths();
    QCOMPARE(query6Paths.size(), 1);
    QCOMPARE(query7Paths.size(), 0);
    g_settings->maxLengthBaseDiscrepancy = 2000;
    g_blastSearch->doAutoBlastSearch();
    query6Paths = g_blastSearch->m_blastQueries.m_queries[5]->getPaths();
    query7Paths = g_blastSearch->m_blastQueries.m_queries[6]->getPaths();
    QCOMPARE(query6Paths.size(), 1);
    QCOMPARE(query7Paths.size(), 1);
}

void BandageTests::bandageInfo()
{
    int n50 = 0;
    int shortestNode = 0;
    int firstQuartile = 0;
    int median = 0;
    int thirdQuartile = 0;
    int longestNode = 0;
    int componentCount = 0;
    int largestComponentLength = 0;


    createGlobals();
    g_assemblyGraph->loadGraphFromFile(getTestDirectory() + "test.LastGraph");
    g_assemblyGraph->getNodeStats(&n50, &shortestNode, &firstQuartile, &median, &thirdQuartile, &longestNode);
    g_assemblyGraph->getGraphComponentCountAndLargestComponentSize(&componentCount, &largestComponentLength);
    QCOMPARE(17, g_assemblyGraph->m_nodeCount);
    QCOMPARE(16, g_assemblyGraph->m_edgeCount);
    QCOMPARE(29939, g_assemblyGraph->m_totalLength);
    QCOMPARE(10, g_assemblyGraph->getDeadEndCount());
    QCOMPARE(2000, n50);
    QCOMPARE(59, shortestNode);
    QCOMPARE(2000, longestNode);
    QCOMPARE(1, componentCount);
    QCOMPARE(29939, largestComponentLength);

    createGlobals();
    g_assemblyGraph->loadGraphFromFile(getTestDirectory() + "test.fastg");
    g_assemblyGraph->getNodeStats(&n50, &shortestNode, &firstQuartile, &median, &thirdQuartile, &longestNode);
    g_assemblyGraph->getGraphComponentCountAndLargestComponentSize(&componentCount, &largestComponentLength);
    QCOMPARE(44, g_assemblyGraph->m_nodeCount);
    QCOMPARE(59, g_assemblyGraph->m_edgeCount);
    QCOMPARE(214441, g_assemblyGraph->m_totalLength);
    QCOMPARE(0, g_assemblyGraph->getDeadEndCount());
    QCOMPARE(35628, n50);
    QCOMPARE(78, shortestNode);
    QCOMPARE(52213, longestNode);
    QCOMPARE(1, componentCount);
    QCOMPARE(214441, largestComponentLength);

    createGlobals();
    g_assemblyGraph->loadGraphFromFile(getTestDirectory() + "test.Trinity.fasta");
    g_assemblyGraph->getNodeStats(&n50, &shortestNode, &firstQuartile, &median, &thirdQuartile, &longestNode);
    g_assemblyGraph->getGraphComponentCountAndLargestComponentSize(&componentCount, &largestComponentLength);
    QCOMPARE(149, g_assemblyGraph->getDeadEndCount());
    QCOMPARE(66, componentCount);
    QCOMPARE(9398, largestComponentLength);
}

void BandageTests::testReverseComplement()
{
    QCOMPARE( reverseComplement("A"), std::string("T"));
    QCOMPARE( reverseComplement("C"), std::string("G"));
    QCOMPARE( reverseComplement("T"), std::string("A"));
    QCOMPARE( reverseComplement("G"), std::string("C"));
    QCOMPARE( reverseComplement(""), std::string(""));
    QCOMPARE( reverseComplement("ACTG"), std::string("CAGT"));
}

void BandageTests::testHashKmers()
{
    {   // Test for an empty string.
        std::string seq = "";
        int32_t k = 10;
        bool seq_is_rev = false;
        std::vector<KmerPos> expected_kmers = { };
        std::vector<KmerPos> kmers = hashKmers(seq, k, seq_is_rev);
        QCOMPARE((int64_t) kmers.size(), (int64_t) 0);
    }

    {   // Test the behaviour when a non [ACTG] character is given.
        std::string seq = "AAAAANAAAAA";
        int32_t k = 10;
        bool seq_is_rev = false;
        std::vector<KmerPos> expected_kmers = { };
        std::vector<KmerPos> kmers = hashKmers(seq, k, seq_is_rev);
        QCOMPARE((int64_t) kmers.size(), (int64_t) 0);
    }

    {   // A simple normal test case. Entire seq should be only one kmer.
        std::string seq = "AAAAAAAAAA";
        int32_t k = 10;
        bool seq_is_rev = false;
        std::vector<KmerPos> expected_kmers = { KmerPos(0x0, 0) };
        std::vector<KmerPos> kmers = hashKmers(seq, k, seq_is_rev);
        QCOMPARE(kmers, expected_kmers);
    }

    {   // Similar to before, but 6 kmers.
        std::string seq = "AAAAAAAAAAAAAAA";
        int32_t k = 10;
        bool seq_is_rev = false;
        std::vector<KmerPos> expected_kmers = { KmerPos(0x0, 0), KmerPos(0x0, 1),
                                                KmerPos(0x0, 2), KmerPos(0x0, 3),
                                                KmerPos(0x0, 4), KmerPos(0x0, 5) };
        std::vector<KmerPos> kmers = hashKmers(seq, k, seq_is_rev);
        QCOMPARE(kmers, expected_kmers);
    }

    {   // Test the reverse complement.
        std::string seq = "AAAAAAAAAA";
        int32_t k = 10;
        bool seq_is_rev = true;
        std::vector<KmerPos> expected_kmers = { KmerPos(0x0, 9) };
        std::vector<KmerPos> kmers = hashKmers(seq, k, seq_is_rev);
        QCOMPARE(kmers, expected_kmers);
    }

    {   // Test what happens if an invalid kmer size is given.
        std::string seq = "AAAAAAAAAA";
        int32_t k = 0;
        bool seq_is_rev = false;
        std::vector<KmerPos> kmers = hashKmers(seq, k, seq_is_rev);
        QCOMPARE((int64_t) kmers.size(), (int64_t) 0);
    }

    {   // Test for a normal sequence, but a more complex variation.
        std::string seq = "ACTGAAAGACT";
        int32_t k = 10;
        bool seq_is_rev = false;
        std::vector<KmerPos> expected_kmers = { KmerPos(0x1E021, 0), KmerPos(0x78087, 1) };
        std::vector<KmerPos> kmers = hashKmers(seq, k, seq_is_rev);
        QCOMPARE(kmers, expected_kmers);
    }

    {   // Similar as before, but in reverse. Hash keys should be same, but positions different.
        std::string seq = "ACTGAAAGACT";
        int32_t k = 10;
        bool seq_is_rev = true;
        std::vector<KmerPos> expected_kmers = { KmerPos(0x1E021, 10), KmerPos(0x78087, 9) };
        std::vector<KmerPos> kmers = hashKmers(seq, k, seq_is_rev);
        QCOMPARE(kmers, expected_kmers);
    }

    {   // Test a normal sequence and a normal (smaller) kmer size.
        std::string seq = "ACTGAAAGACT";
        int32_t k = 4;
        bool seq_is_rev = false;
        std::vector<KmerPos> expected_kmers = { KmerPos(0x1E, 0), KmerPos(0x78, 1), KmerPos(0xE0, 2),
                                                KmerPos(0x80, 3), KmerPos(0x2, 4), KmerPos(0x8, 5),
                                                KmerPos(0x21, 6), KmerPos(0x87, 7) };
        std::vector<KmerPos> kmers = hashKmers(seq, k, seq_is_rev);
        QCOMPARE(kmers, expected_kmers);
    }
}

void BandageTests::testFindHits()
{
    {   // Test on empty inputs.
        std::vector<KmerPos> sorted_kmers_seq1 = { };
        std::vector<KmerPos> sorted_kmers_seq2 = { };
        std::vector<KmerHit> result = findHits(sorted_kmers_seq1, sorted_kmers_seq2);
        QCOMPARE((int64_t) result.size(), (int64_t) 0);
    }

    {   // No hits are output if any of the input arrays are not sorted.
        std::vector<KmerPos> sorted_kmers_seq1 = {KmerPos(1, 0), KmerPos(0, 1)};
        std::vector<KmerPos> sorted_kmers_seq2 = {KmerPos(0, 0), KmerPos(1, 1)};
        std::vector<KmerHit> result = findHits(sorted_kmers_seq1, sorted_kmers_seq2);
        QCOMPARE((int64_t) result.size(), (int64_t) 0);
    }

    {   /// Sorted but no hits.
        std::vector<KmerPos> sorted_kmers_seq1 = {KmerPos(2, 0), KmerPos(3, 1)};
        std::vector<KmerPos> sorted_kmers_seq2 = {KmerPos(0, 0), KmerPos(1, 1)};
        std::vector<KmerHit> result = findHits(sorted_kmers_seq1, sorted_kmers_seq2);
        QCOMPARE((int64_t) result.size(), (int64_t) 0);
    }

    {   // There should be two hits.
        std::vector<KmerPos> sorted_kmers_seq1 = {KmerPos(0, 0), KmerPos(1, 1)};
        std::vector<KmerPos> sorted_kmers_seq2 = {KmerPos(0, 0), KmerPos(1, 1)};
        std::vector<KmerHit> result = findHits(sorted_kmers_seq1, sorted_kmers_seq2);
        std::vector<KmerHit> expected = {KmerHit(0, 0), KmerHit(1, 1)};
        QCOMPARE(result, expected);
    }

    {   // One is empty, the other is not.
        std::vector<KmerPos> sorted_kmers_seq1 = { };
        std::vector<KmerPos> sorted_kmers_seq2 = {KmerPos(0, 0), KmerPos(1, 1)};
        std::vector<KmerHit> result = findHits(sorted_kmers_seq1, sorted_kmers_seq2);
        QCOMPARE((int64_t) result.size(), (int64_t) 0);
    }

    {   // One is empty, the other is not.
        std::vector<KmerPos> sorted_kmers_seq1 = {KmerPos(0, 0), KmerPos(1, 1)};
        std::vector<KmerPos> sorted_kmers_seq2 = { };
        std::vector<KmerHit> result = findHits(sorted_kmers_seq1, sorted_kmers_seq2);
        QCOMPARE((int64_t) result.size(), (int64_t) 0);
    }

    {   // Only a subset matches.
        std::vector<KmerPos> sorted_kmers_seq1 = {KmerPos(3, 1), KmerPos(4, 2)};
        std::vector<KmerPos> sorted_kmers_seq2 = {KmerPos(0, 0), KmerPos(1, 1),
                                                  KmerPos(3, 3), KmerPos(4, 7),
                                                  KmerPos(5, 8) };
        std::vector<KmerHit> result = findHits(sorted_kmers_seq1, sorted_kmers_seq2);
        std::vector<KmerHit> expected = {KmerHit(1, 3), KmerHit(2, 7)};
        QCOMPARE(result, expected);
    }

    {   // Test multiple successive hits.
        std::vector<KmerPos> sorted_kmers_seq1 = {  KmerPos(2, 0), KmerPos(3, 1), KmerPos(4, 2) };
        std::vector<KmerPos> sorted_kmers_seq2 = {  KmerPos(0, 0), KmerPos(1, 1),
                                                    KmerPos(3, 3), KmerPos(3, 7),
                                                    KmerPos(3, 8), KmerPos(5, 9) };
        std::vector<KmerHit> result = findHits(sorted_kmers_seq1, sorted_kmers_seq2);
        std::vector<KmerHit> expected = { KmerHit(1, 3), KmerHit(1, 7), KmerHit(1, 8)};
        QCOMPARE(result, expected);
    }
}

void BandageTests::testFindKmerMatches()
{
    {   // Test a simple basic match case.
        std::string seq1 = "CT";
        std::string seq2 = "CT";
        int32_t k = 2;
        std::vector<KmerHit> result = findKmerMatches(seq1, seq2, k);
        std::vector<KmerHit> expected = {KmerHit(0, 0)};
        QCOMPARE(result, expected);
    }

    {   // Test a case with no hits.
        std::string seq1 = "AAAAA";
        std::string seq2 = "CT";
        int32_t k = 2;
        std::vector<KmerHit> result = findKmerMatches(seq1, seq2, k);
        std::vector<KmerHit> expected = { };
        QCOMPARE((int64_t) result.size(), (int64_t) expected.size());
    }

    {   // Test a normal one-hit case.
        std::string seq1 = "ACTTGGGA";
        std::string seq2 =  "CT";
        int32_t k = 2;
        std::vector<KmerHit> result = findKmerMatches(seq1, seq2, k);
        std::vector<KmerHit> expected = {KmerHit(1, 0)};
        QCOMPARE((int64_t) result.size(), (int64_t) expected.size());
        QCOMPARE(result, expected);
    }

    {   // Test matching empty sequences.
        std::string seq1 = "";
        std::string seq2 = "";
        int32_t k = 10;
        std::vector<KmerHit> result = findKmerMatches(seq1, seq2, k);
        std::vector<KmerHit> expected = { };
        QCOMPARE((int64_t) result.size(), (int64_t) expected.size());
        QCOMPARE(result, expected);
    }

    {   // Test finding of hits in a larger sequence.
        std::string seq1 = "CTCGCACTTGGGGAATCGCGCAGACCTCACCCGGTTTGCAGGCTTGCGCCGGGCGGTAGATGCGCCGCCAGGCGAAAAACAGCGCGACCAGCGCTGCGCC";
        std::string seq2 = "CTCGCACTTG"         "AGACCTCACC"                       "TAGATGCGCCG";
        // Reverse complement:
        // CGGCGCATCTA GGTGAGGTCT CAAGTGCGAG
        int32_t k = 10;
        std::vector<KmerHit> result = findKmerMatches(seq1, seq2, k);
        std::vector<KmerHit> expected = {KmerHit(0, 0),  KmerHit(21, 10),
                                         KmerHit(56, 20), KmerHit(57, 21) };
        std::sort(result.begin(), result.end());
        std::sort(expected.begin(), expected.end());
        QCOMPARE((int64_t) result.size(), (int64_t) expected.size());
        QCOMPARE(result, expected);
    }

    {   // Test the reverse complement matching.
        std::string seq1 = "CTCGCACTTG";
        std::string seq2 = "CAAGTGCGAG";
        int32_t k = 10;
        std::vector<KmerHit> result = findKmerMatches(seq1, seq2, k);
        std::vector<KmerHit> expected = {KmerHit(0, 9) };
        std::sort(result.begin(), result.end());
        std::sort(expected.begin(), expected.end());
        QCOMPARE((int64_t) result.size(), (int64_t) expected.size());
        QCOMPARE(result, expected);
    }
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


//This function checks to see if two circular sequences match.  It needs to
//check each possible rotation, as well as reverse complements.
bool BandageTests::doCircularSequencesMatch(QByteArray s1, QByteArray s2)
{
    for (int i = 0; i < s1.length() - 1; ++i)
    {
        QByteArray rotatedS1 = s1.right(s1.length() - i) + s1.left(i);
        if (rotatedS1 == s2)
            return true;
    }

    //If the code got here, then all possible rotations of s1 failed to match
    //s2.  Now we try the reverse complement.
    QByteArray s1Rc = AssemblyGraph::getReverseComplement(s1);
    for (int i = 0; i < s1Rc.length() - 1; ++i)
    {
        QByteArray rotatedS1Rc = s1Rc.right(s1Rc.length() - i) + s1Rc.left(i);
        if (rotatedS1Rc == s2)
            return true;
    }

    return false;
}



QTEST_MAIN(BandageTests)
#include "bandagetests.moc"
