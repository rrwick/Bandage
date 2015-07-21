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


#include "assemblygraph.h"
#include <QMapIterator>
#include "../program/globals.h"
#include "../program/settings.h"
#include <limits>
#include <algorithm>
#include "../graph/debruijnnode.h"
#include "../graph/debruijnedge.h"
#include "../graph/graphicsitemnode.h"
#include <QFile>
#include <QTextStream>
#include <QApplication>
#include "../graph/graphicsitemedge.h"
#include "../blast/blastsearch.h"
#include <ogdf/energybased/FMMMLayout.h>
#include "../program/graphlayoutworker.h"


AssemblyGraph::AssemblyGraph() :
    m_contiguitySearchDone(false)
{
    m_ogdfGraph = new ogdf::Graph();
    m_graphAttributes = new ogdf::GraphAttributes(*m_ogdfGraph, ogdf::GraphAttributes::nodeGraphics |
                                                  ogdf::GraphAttributes::edgeGraphics);
}

AssemblyGraph::~AssemblyGraph()
{
    delete m_graphAttributes;
    delete m_ogdfGraph;
}


void AssemblyGraph::cleanUp()
{
    QMapIterator<QString, DeBruijnNode*> i(m_deBruijnGraphNodes);
    while (i.hasNext())
    {
        i.next();
        delete i.value();
    }
    m_deBruijnGraphNodes.clear();

    for (size_t i = 0; i < m_deBruijnGraphEdges.size(); ++i)
        delete m_deBruijnGraphEdges[i];
    m_deBruijnGraphEdges.clear();

    m_contiguitySearchDone = false;

    clearGraphInfo();
}





//This function makes a double edge: in one direction for the given nodes
//and the opposite direction for their reverse complements.  It adds the
//new edges to the vector here and to the nodes themselves.
void AssemblyGraph::createDeBruijnEdge(QString node1Name, QString node2Name)
{
    QString node1Opposite = getOppositeNodeName(node1Name);
    QString node2Opposite = getOppositeNodeName(node2Name);

    //Quit if any of the nodes don't exist.
    if (!m_deBruijnGraphNodes.contains(node1Name) ||
            !m_deBruijnGraphNodes.contains(node2Name) ||
            !m_deBruijnGraphNodes.contains(node1Opposite) ||
            !m_deBruijnGraphNodes.contains(node2Opposite))
        return;

    DeBruijnNode * node1 = m_deBruijnGraphNodes[node1Name];
    DeBruijnNode * node2 = m_deBruijnGraphNodes[node2Name];
    DeBruijnNode * negNode1 = m_deBruijnGraphNodes[node1Opposite];
    DeBruijnNode * negNode2 = m_deBruijnGraphNodes[node2Opposite];

    //Quit if the edge already exists
    for (size_t i = 0; i < node1->m_edges.size(); ++i)
    {
        if (node1->m_edges[i]->m_startingNode == node1 &&
                node1->m_edges[i]->m_endingNode == node2)
            return;
    }

    //Usually, an edge has a different pair, but it is possible
    //for an edge to be its own pair.
    bool isOwnPair = (node1 == negNode2 && node2 == negNode1);

    DeBruijnEdge * forwardEdge = new DeBruijnEdge(node1, node2);
    DeBruijnEdge * backwardEdge;

    if (isOwnPair)
        backwardEdge = forwardEdge;
    else
        backwardEdge = new DeBruijnEdge(negNode2, negNode1);

    forwardEdge->m_reverseComplement = backwardEdge;
    backwardEdge->m_reverseComplement = forwardEdge;

    m_deBruijnGraphEdges.push_back(forwardEdge);
    if (!isOwnPair)
        m_deBruijnGraphEdges.push_back(backwardEdge);

    node1->addEdge(forwardEdge);
    node2->addEdge(forwardEdge);
    negNode1->addEdge(backwardEdge);
    negNode2->addEdge(backwardEdge);
}




void AssemblyGraph::clearOgdfGraphAndResetNodes()
{
    QMapIterator<QString, DeBruijnNode*> i(m_deBruijnGraphNodes);
    while (i.hasNext())
    {
        i.next();
        i.value()->resetNode();
    }

    m_ogdfGraph->clear();
}






//http://www.code10.info/index.php?option=com_content&view=article&id=62:articledna-reverse-complement&catid=49:cat_coding_algorithms_bioinformatics&Itemid=74
QByteArray AssemblyGraph::getReverseComplement(QByteArray forwardSequence)
{
    QByteArray reverseComplement;

    for (int i = forwardSequence.length() - 1; i >= 0; --i)
    {
        char letter = forwardSequence.at(i);

        switch (letter)
        {
        case 'A': reverseComplement.append('T'); break;
        case 'T': reverseComplement.append('A'); break;
        case 'G': reverseComplement.append('C'); break;
        case 'C': reverseComplement.append('G'); break;
        case 'a': reverseComplement.append('t'); break;
        case 't': reverseComplement.append('a'); break;
        case 'g': reverseComplement.append('c'); break;
        case 'c': reverseComplement.append('g'); break;
        case 'R': reverseComplement.append('Y'); break;
        case 'Y': reverseComplement.append('R'); break;
        case 'S': reverseComplement.append('S'); break;
        case 'W': reverseComplement.append('W'); break;
        case 'K': reverseComplement.append('M'); break;
        case 'M': reverseComplement.append('K'); break;
        case 'r': reverseComplement.append('y'); break;
        case 'y': reverseComplement.append('r'); break;
        case 's': reverseComplement.append('s'); break;
        case 'w': reverseComplement.append('w'); break;
        case 'k': reverseComplement.append('m'); break;
        case 'm': reverseComplement.append('k'); break;
        case 'B': reverseComplement.append('V'); break;
        case 'D': reverseComplement.append('H'); break;
        case 'H': reverseComplement.append('D'); break;
        case 'V': reverseComplement.append('B'); break;
        case 'b': reverseComplement.append('v'); break;
        case 'd': reverseComplement.append('h'); break;
        case 'h': reverseComplement.append('d'); break;
        case 'v': reverseComplement.append('b'); break;
        case 'N': reverseComplement.append('N'); break;
        case 'n': reverseComplement.append('n'); break;
        case '.': reverseComplement.append('.'); break;
        case '-': reverseComplement.append('-'); break;
        case '?': reverseComplement.append('?'); break;
        }
    }

    return reverseComplement;
}




void AssemblyGraph::resetEdges()
{
    for (size_t i = 0; i < m_deBruijnGraphEdges.size(); ++i)
        m_deBruijnGraphEdges[i]->reset();
}


double AssemblyGraph::getMeanDeBruijnGraphCoverage(bool drawnNodesOnly)
{
    int nodeCount = 0;
    long double coverageSum = 0.0;
    long long totalLength = 0;

    QMapIterator<QString, DeBruijnNode*> i(m_deBruijnGraphNodes);
    while (i.hasNext())
    {
        i.next();
        DeBruijnNode * node = i.value();

        if (drawnNodesOnly && !node->m_drawn)
            continue;

        ++nodeCount;
        totalLength += node->m_length;
        coverageSum += node->m_length * node->m_coverage;

    }

    if (totalLength == 0)
        return 0.0;
    else
        return coverageSum / totalLength;
}

double AssemblyGraph::getMaxDeBruijnGraphCoverageOfDrawnNodes()
{
    double maxCoverage = 1.0;

    QMapIterator<QString, DeBruijnNode*> i(m_deBruijnGraphNodes);
    while (i.hasNext())
    {
        i.next();

        if (i.value()->m_graphicsItemNode != 0 && i.value()->m_coverage > maxCoverage)
            maxCoverage = i.value()->m_coverage;
    }

    return maxCoverage;
}


void AssemblyGraph::resetNodeContiguityStatus()
{
    QMapIterator<QString, DeBruijnNode*> i(m_deBruijnGraphNodes);
    while (i.hasNext())
    {
        i.next();
        i.value()->m_contiguityStatus = NOT_CONTIGUOUS;
    }
    m_contiguitySearchDone = false;
}

void AssemblyGraph::resetAllNodeColours()
{
    QMapIterator<QString, DeBruijnNode*> i(m_deBruijnGraphNodes);
    while (i.hasNext())
    {
        i.next();
        if (i.value()->m_graphicsItemNode != 0)
            i.value()->m_graphicsItemNode->setNodeColour();
    }
}

void AssemblyGraph::clearAllBlastHitPointers()
{
    QMapIterator<QString, DeBruijnNode*> i(m_deBruijnGraphNodes);
    while (i.hasNext())
    {
        i.next();
        DeBruijnNode * node = i.value();
        node->m_blastHits.clear();
    }
}



void AssemblyGraph::determineGraphInfo()
{
    m_shortestContig = std::numeric_limits<long long>::max();
    m_longestContig = 0;
    int nodeCount = 0;
    long long totalLength = 0;
    std::vector<double> nodeCoverages;

    QMapIterator<QString, DeBruijnNode*> i(m_deBruijnGraphNodes);
    while (i.hasNext())
    {
        i.next();
        long long nodeLength = i.value()->m_length;

        if (nodeLength < m_shortestContig)
            m_shortestContig = nodeLength;
        if (nodeLength > m_longestContig)
            m_longestContig = nodeLength;

        //Only add up the length for positive nodes
        if (i.value()->isPositiveNode())
        {
            totalLength += nodeLength;
            ++nodeCount;
        }

        nodeCoverages.push_back(i.value()->m_coverage);
    }

    //Count up the edges.  Edges that are their own pairs will
    //not be counted, as these won't show up in single mode.
    int edgeCount = 0;
    for (size_t i = 0; i < m_deBruijnGraphEdges.size(); ++i)
    {
        DeBruijnEdge * edge = m_deBruijnGraphEdges[i];
        if (edge != edge->m_reverseComplement)
            ++edgeCount;
    }
    edgeCount /= 2;

    m_nodeCount = nodeCount;
    m_edgeCount = edgeCount;
    m_totalLength = totalLength;
    m_meanCoverage = getMeanDeBruijnGraphCoverage();

    std::sort(nodeCoverages.begin(), nodeCoverages.end());

    double firstQuartileIndex = nodeCoverages.size() / 4.0;
    double medianIndex = nodeCoverages.size() / 2.0;
    double thirdQuartileIndex = nodeCoverages.size() * 3.0 / 4.0;

    m_firstQuartileCoverage = getValueUsingFractionalIndex(&nodeCoverages, firstQuartileIndex);
    m_medianCoverage = getValueUsingFractionalIndex(&nodeCoverages, medianIndex);
    m_thirdQuartileCoverage = getValueUsingFractionalIndex(&nodeCoverages, thirdQuartileIndex);

    //Set the auto base pairs per segment
    int totalSegments = m_nodeCount * g_settings->meanSegmentsPerNode;
    g_settings->autoBasePairsPerSegment = m_totalLength / totalSegments;
}

double AssemblyGraph::getValueUsingFractionalIndex(std::vector<double> * doubleVector, double index)
{
    if (doubleVector->size() == 0)
        return 0.0;
    if (doubleVector->size() == 1)
        return (*doubleVector)[0];

    int wholePart = floor(index);

    if (wholePart < 0)
        return (*doubleVector)[0];
    if (wholePart >= int(doubleVector->size()) - 1)
        return (*doubleVector)[doubleVector->size() - 1];

    double fractionalPart = index - wholePart;

    double piece1 = (*doubleVector)[wholePart];
    double piece2 = (*doubleVector)[wholePart+1];

    return piece1 * (1.0 - fractionalPart) + piece2 * fractionalPart;
}

void AssemblyGraph::clearGraphInfo()
{
    m_totalLength = 0;
    m_shortestContig = 0;
    m_longestContig = 0;

    m_meanCoverage = 0.0;
    m_firstQuartileCoverage = 0.0;
    m_medianCoverage = 0.0;
    m_thirdQuartileCoverage = 0.0;
}






void AssemblyGraph::buildDeBruijnGraphFromLastGraph(QString fullFileName)
{
    m_graphFileType = LAST_GRAPH;

    QFile inputFile(fullFileName);
    if (inputFile.open(QIODevice::ReadOnly))
    {
        QTextStream in(&inputFile);
        while (!in.atEnd())
        {
            QApplication::processEvents();
            QString line = in.readLine();
            if (line.startsWith("NODE"))
            {
                QStringList nodeDetails = line.split(QRegExp("\\s+"));

                if (nodeDetails.size() < 4)
                    throw "load error";

                QString nodeName = nodeDetails.at(1);
                QString posNodeName = nodeName + "+";
                QString negNodeName = nodeName + "-";

                int nodeLength = nodeDetails.at(2).toInt();

                double nodeCoverage;
                if (nodeLength > 0)
                    nodeCoverage = double(nodeDetails.at(3).toInt()) / nodeLength; //IS THIS COLUMN ($COV_SHORT1) THE BEST ONE TO USE?
                else
                    nodeCoverage = double(nodeDetails.at(3).toInt());

                QByteArray sequence = in.readLine().toLocal8Bit();
                QByteArray revCompSequence = in.readLine().toLocal8Bit();

                DeBruijnNode * node = new DeBruijnNode(posNodeName, nodeLength, nodeCoverage, sequence);
                DeBruijnNode * reverseComplementNode = new DeBruijnNode(negNodeName, nodeLength, nodeCoverage, revCompSequence);
                node->m_reverseComplement = reverseComplementNode;
                reverseComplementNode->m_reverseComplement = node;
                m_deBruijnGraphNodes.insert(posNodeName, node);
                m_deBruijnGraphNodes.insert(negNodeName, reverseComplementNode);
            }
            else if (line.startsWith("ARC"))
            {
                QStringList arcDetails = line.split(QRegExp("\\s+"));

                if (arcDetails.size() < 3)
                    throw "load error";

                QString node1Name = convertNormalNumberStringToBandageNodeName(arcDetails.at(1));
                QString node2Name = convertNormalNumberStringToBandageNodeName(arcDetails.at(2));

                createDeBruijnEdge(node1Name, node2Name);
            }
        }
        inputFile.close();

        setAllEdgesExactOverlap(0);
    }

    if (m_deBruijnGraphNodes.size() == 0)
        throw "load error";
}


//This function takes a normal number string like "5" or "-6" and changes
//it to "5+" or "6-" - the format of Bandage node names.
QString AssemblyGraph::convertNormalNumberStringToBandageNodeName(QString number)
{
    if (number.at(0) == '-')
    {
        number.remove(0, 1);
        return number + "-";
    }
    else
        return number + "+";
}


void AssemblyGraph::buildDeBruijnGraphFromGfa(QString fullFileName)
{
    m_graphFileType = GFA;

    QFile inputFile(fullFileName);
    if (inputFile.open(QIODevice::ReadOnly))
    {
        std::vector<QString> edgeStartingNodeNames;
        std::vector<QString> edgeEndingNodeNames;

        QTextStream in(&inputFile);
        while (!in.atEnd())
        {
            QApplication::processEvents();
            QString line = in.readLine();

            QStringList lineParts = line.split(QRegExp("\t"));

            if (lineParts.size() < 1)
                continue;

            //Lines beginning with "S" are sequence (node) lines
            if (lineParts.at(0) == "S")
            {
                if (lineParts.size() < 3)
                    throw "load error";

                QString nodeName = lineParts.at(1);
                QString posNodeName = nodeName + "+";
                QString negNodeName = nodeName + "-";

                QByteArray sequence = lineParts.at(2).toLocal8Bit();
                QByteArray revCompSequence = getReverseComplement(sequence);

                //If there is an attribute holding the sequence length, we'll use
                //that.  If there isn't, then we'll just look at the length of the
                //sequence string.
                int nodeLength = sequence.length();

                //If there is an attribute holding the coverage, we'll use that.
                //If there isn't, then we'll use zero.
                double nodeCoverage = 0.0;

                for (int i = 3; i < lineParts.size(); ++i)
                {
                    QString part = lineParts.at(i);
                    if (part.size() < 6)
                        continue;
                    else if (part.left(5) == "LN:i:")
                        nodeLength = part.right(part.length() - 5).toInt();
                    else if (part.left(5) == "KC:f:")
                        nodeCoverage = part.right(part.length() - 5).toDouble();
                }

                DeBruijnNode * node = new DeBruijnNode(posNodeName, nodeLength, nodeCoverage, sequence);
                DeBruijnNode * reverseComplementNode = new DeBruijnNode(negNodeName, nodeLength, nodeCoverage, revCompSequence);
                node->m_reverseComplement = reverseComplementNode;
                reverseComplementNode->m_reverseComplement = node;
                m_deBruijnGraphNodes.insert(posNodeName, node);
                m_deBruijnGraphNodes.insert(negNodeName, reverseComplementNode);
            }

            //Lines beginning with "L" are link (edge) lines
            else if (lineParts.at(0) == "L")
            {
                //Edges aren't made now, in case their sequence hasn't yet been specified.
                //Instead, we save the starting and ending nodes and make the edges after
                //we're done looking at the file.

                if (lineParts.size() < 5)
                    throw "load error";

                //Parts 1 and 3 hold the node names and parts 2 and 4 hold the corresponding +/-.
                QString startingNode = lineParts.at(1) + lineParts.at(2);
                QString endingNode = lineParts.at(3) + lineParts.at(4);
                edgeStartingNodeNames.push_back(startingNode);
                edgeEndingNodeNames.push_back(endingNode);
            }
        }

        //Create all of the edges
        for (size_t i = 0; i < edgeStartingNodeNames.size(); ++i)
        {
            QString node1Name = edgeStartingNodeNames[i];
            QString node2Name = edgeEndingNodeNames[i];
            createDeBruijnEdge(node1Name, node2Name);
        }
    }

    if (m_deBruijnGraphNodes.size() == 0)
        throw "load error";
}



void AssemblyGraph::buildDeBruijnGraphFromFastg(QString fullFileName)
{
    m_graphFileType = FASTG;

    QFile inputFile(fullFileName);
    if (inputFile.open(QIODevice::ReadOnly))
    {
        std::vector<QString> edgeStartingNodeNames;
        std::vector<QString> edgeEndingNodeNames;
        DeBruijnNode * node = 0;

        QTextStream in(&inputFile);
        while (!in.atEnd())
        {
            QApplication::processEvents();

            QString nodeName;
            int nodeLength;
            double nodeCoverage;

            QString line = in.readLine();

            //If the line starts with a '>', then we are beginning a new node.
            if (line.startsWith(">"))
            {
                line.remove(0, 1); //Remove '>' from start
                line.chop(1); //Remove ';' from end
                QStringList nodeDetails = line.split(":");

                QString thisNode = nodeDetails.at(0);

                //A single quote as the last character indicates a negative node.
                bool negativeNode = thisNode.at(thisNode.size() - 1) == '\'';

                QStringList thisNodeDetails = thisNode.split("_");

                if (thisNodeDetails.size() < 6)
                    throw "load error";

                nodeName = thisNodeDetails.at(1);
                if (negativeNode)
                    nodeName += "-";
                else
                    nodeName += "+";

                nodeLength = thisNodeDetails.at(3).toInt();

                QString nodeCoverageString = thisNodeDetails.at(5);
                if (negativeNode)
                {
                    //It may be necessary to remove a single quote from the end of the coverage
                    if (nodeCoverageString.at(nodeCoverageString.size() - 1) == '\'')
                        nodeCoverageString.chop(1);
                }
                nodeCoverage = nodeCoverageString.toDouble();

                //Make the node
                node = new DeBruijnNode(nodeName, nodeLength, nodeCoverage, ""); //Sequence string is currently empty - will be added to on subsequent lines of the fastg file
                m_deBruijnGraphNodes.insert(nodeName, node);

                //The second part of nodeDetails is a comma-delimited list of edge nodes.
                //Edges aren't made right now (because the other node might not yet exist),
                //so they are saved into vectors and made after all the nodes have been made.
                if (nodeDetails.size() == 1)
                    continue;
                QStringList edgeNodes = nodeDetails.at(1).split(",");
                for (int i = 0; i < edgeNodes.size(); ++i)
                {
                    QString edgeNode = edgeNodes.at(i);

                    QChar lastChar = edgeNode.at(edgeNode.size() - 1);
                    bool negativeNode = false;
                    if (lastChar == '\'')
                    {
                        negativeNode = true;
                        edgeNode.chop(1);
                    }
                    QStringList edgeNodeDetails = edgeNode.split("_");

                    if (edgeNodeDetails.size() < 2)
                        throw "load error";

                    QString edgeNodeName = edgeNodeDetails.at(1);
                    if (negativeNode)
                        edgeNodeName += "-";
                    else
                        edgeNodeName += "+";

                    edgeStartingNodeNames.push_back(nodeName);
                    edgeEndingNodeNames.push_back(edgeNodeName);
                }
            }

            //If the line does not start with a '>', then this line is part of the
            //sequence for the last node.
            else
            {
                QByteArray sequenceLine = line.simplified().toLocal8Bit();
                if (node != 0)
                    node->m_sequence.append(sequenceLine);
            }
        }

        inputFile.close();

        //If all went well, each node will have a reverse complement and the code
        //will never get here.  However, I have noticed that some SPAdes fastg files
        //have, for some reason, negative nodes with no positive counterpart.  For
        //that reason, we will now make any reverse complement nodes for nodes that
        //lack them.
        QMapIterator<QString, DeBruijnNode*> i(m_deBruijnGraphNodes);
        while (i.hasNext())
        {
            i.next();
            DeBruijnNode * node = i.value();
            makeReverseComplementNodeIfNecessary(node);
        }
        pointEachNodeToItsReverseComplement();


        //Create all of the edges
        for (size_t i = 0; i < edgeStartingNodeNames.size(); ++i)
        {
            QString node1Name = edgeStartingNodeNames[i];
            QString node2Name = edgeEndingNodeNames[i];
            createDeBruijnEdge(node1Name, node2Name);
        }
    }

    autoDetermineAllEdgesExactOverlap();

    if (m_deBruijnGraphNodes.size() == 0)
        throw "load error";
}


void AssemblyGraph::makeReverseComplementNodeIfNecessary(DeBruijnNode * node)
{
    QString reverseComplementName = getOppositeNodeName(node->m_name);

    DeBruijnNode * reverseComplementNode = m_deBruijnGraphNodes[reverseComplementName];
    if (reverseComplementNode == 0)
    {
        DeBruijnNode * newNode = new DeBruijnNode(reverseComplementName, node->m_length, node->m_coverage,
                                                  getReverseComplement(node->m_sequence));
        m_deBruijnGraphNodes.insert(reverseComplementName, newNode);
    }
}


void AssemblyGraph::pointEachNodeToItsReverseComplement()
{
    QMapIterator<QString, DeBruijnNode*> i(m_deBruijnGraphNodes);
    while (i.hasNext())
    {
        i.next();
        DeBruijnNode * positiveNode = i.value();

        if (positiveNode->isPositiveNode())
        {
            DeBruijnNode * negativeNode = m_deBruijnGraphNodes[getOppositeNodeName(positiveNode->m_name)];
            if (negativeNode != 0)
            {
                positiveNode->m_reverseComplement = negativeNode;
                negativeNode->m_reverseComplement = positiveNode;
            }
        }
    }
}




void AssemblyGraph::buildDeBruijnGraphFromTrinityFasta(QString fullFileName)
{
    m_graphFileType = TRINITY;

    std::vector<QString> names;
    std::vector<QString> sequences;
    readFastaFile(fullFileName, &names, &sequences);

    std::vector<QString> edgeStartingNodeNames;
    std::vector<QString> edgeEndingNodeNames;

    for (size_t i = 0; i < names.size(); ++i)
    {
        QString name = names[i];
        QString sequence = sequences[i];

        //The header can come in a couple of different formats:
        // TR1|c0_g1_i1 len=280 path=[274:0-228 275:229-279] [-1, 274, 275, -2]
        // GG1|c0_g1_i1 len=302 path=[1:0-301]
        // comp0_c0_seq1 len=286 path=[6:0-285]
        // c0_g1_i1 len=363 path=[119:0-185 43:186-244 43:245-303 43:304-362]

        //The node names will begin with a string that contains everything
        //up to the component number (e.g. "c0"), in the same format as it is
        //in the Trinity.fasta file.

        if (name.length() < 4)
            throw "load error";

        int componentStartIndex = name.indexOf(QRegExp("c\\d+_"));
        int componentEndIndex = name.indexOf("_", componentStartIndex);

        if (componentStartIndex < 0 || componentEndIndex < 0)
            throw "load error";

        QString component = name.left(componentEndIndex);

        if (component.length() < 2)
            throw "load error";

        int pathStartIndex = name.indexOf("path=[") + 6;
        int pathEndIndex = name.indexOf("]", pathStartIndex);
        if (pathStartIndex < 0 || pathEndIndex < 0)
            throw "load error";
        int pathLength = pathEndIndex - pathStartIndex;
        QString path = name.mid(pathStartIndex, pathLength);
        if (path.size() == 0)
            throw "load error";

        QStringList pathParts = path.split(" ");

        //Each path part is a node
        QString previousNodeName;
        for (int i = 0; i < pathParts.length(); ++i)
        {
            QString pathPart = pathParts.at(i);
            QStringList nodeParts = pathPart.split(":");
            if (nodeParts.size() < 2)
                throw "load error";

            //Most node numbers will be formatted simply as the number, but some
            //(I don't know why) have '@' and the start and '@!' at the end.  In
            //these cases, we must strip those extra characters off.
            QString nodeNumberString = nodeParts.at(0);
            if (nodeNumberString.at(0) == '@')
                nodeNumberString = nodeNumberString.mid(1, nodeNumberString.length() - 3);

            QString nodeName = component + "_" + nodeNumberString + "+";

            //If the node doesn't yet exist, make it now.
            if (!m_deBruijnGraphNodes.contains(nodeName))
            {
                QString nodeRange = nodeParts.at(1);
                QStringList nodeRangeParts = nodeRange.split("-");

                if (nodeRangeParts.size() < 2)
                    throw "load error";

                int nodeRangeStart = nodeRangeParts.at(0).toInt();
                int nodeRangeEnd = nodeRangeParts.at(1).toInt();
                int nodeLength = nodeRangeEnd - nodeRangeStart + 1;

                QByteArray nodeSequence = sequence.mid(nodeRangeStart, nodeLength).toLocal8Bit();
                DeBruijnNode * node = new DeBruijnNode(nodeName, nodeLength, 0.0, nodeSequence);
                m_deBruijnGraphNodes.insert(nodeName, node);
            }

            //Remember to make an edge for the previous node to this one.
            if (i > 0)
            {
                edgeStartingNodeNames.push_back(previousNodeName);
                edgeEndingNodeNames.push_back(nodeName);
            }
            previousNodeName = nodeName;
        }
    }

    //Even though the Trinity.fasta file only contains positive nodes, Bandage
    //expects negative reverse complements nodes, so make them now.
    QMapIterator<QString, DeBruijnNode*> i(m_deBruijnGraphNodes);
    while (i.hasNext())
    {
        i.next();
        DeBruijnNode * node = i.value();
        makeReverseComplementNodeIfNecessary(node);
    }
    pointEachNodeToItsReverseComplement();

    //Create all of the edges.  The createDeBruijnEdge function checks for
    //duplicates, so it's okay if we try to add the same edge multiple times.
    for (size_t i = 0; i < edgeStartingNodeNames.size(); ++i)
    {
        QString node1Name = edgeStartingNodeNames[i];
        QString node2Name = edgeEndingNodeNames[i];
        createDeBruijnEdge(node1Name, node2Name);
    }

    setAllEdgesExactOverlap(0);

    if (m_deBruijnGraphNodes.size() == 0)
        throw "load error";
}


GraphFileType AssemblyGraph::getGraphFileTypeFromFile(QString fullFileName)
{
    if (checkFileIsLastGraph(fullFileName))
        return LAST_GRAPH;
    if (g_assemblyGraph->checkFileIsFastG(fullFileName))
        return FASTG;
    if (g_assemblyGraph->checkFileIsGfa(fullFileName))
        return GFA;
    if (g_assemblyGraph->checkFileIsTrinityFasta(fullFileName))
        return TRINITY;
    return UNKNOWN_FILE_TYPE;
}


//Cursory look to see if file appears to be a LastGraph file.
bool AssemblyGraph::checkFileIsLastGraph(QString fullFileName)
{
    return checkFirstLineOfFile(fullFileName, "\\d+\\s+\\d+\\s+\\d+\\s+\\d+");
}

//Cursory look to see if file appears to be a FASTG file.
bool AssemblyGraph::checkFileIsFastG(QString fullFileName)
{
    return checkFirstLineOfFile(fullFileName, ">NODE");
}

//Cursory look to see if file appears to be a GFA file.
bool AssemblyGraph::checkFileIsGfa(QString fullFileName)
{
    return checkFirstLineOfFile(fullFileName, "[SLH]\t");
}

//Cursory look to see if file appears to be a Trinity.fasta file.
bool AssemblyGraph::checkFileIsTrinityFasta(QString fullFileName)
{
    return checkFirstLineOfFile(fullFileName, "path=\\[");
}


bool AssemblyGraph::checkFirstLineOfFile(QString fullFileName, QString regExp)
{
    QFile inputFile(fullFileName);
    if (inputFile.open(QIODevice::ReadOnly))
    {
        QTextStream in(&inputFile);
        if (in.atEnd())
            return false;
        QRegExp rx(regExp);
        QString line = in.readLine();
        if (rx.indexIn(line) != -1)
            return true;
    }
    return false;
}


//Returns true if successful, false if not.
bool AssemblyGraph::loadGraphFromFile(QString filename)
{
    GraphFileType graphFileType = getGraphFileTypeFromFile(filename);

    if (graphFileType == UNKNOWN_FILE_TYPE)
        return false;

    try
    {
        if (graphFileType == LAST_GRAPH)
            buildDeBruijnGraphFromLastGraph(filename);
        if (graphFileType == FASTG)
            buildDeBruijnGraphFromFastg(filename);
        if (graphFileType == GFA)
            buildDeBruijnGraphFromGfa(filename);
        if (graphFileType == TRINITY)
            buildDeBruijnGraphFromTrinityFasta(filename);
    }

    catch (...)
    {
        return false;
    }

    determineGraphInfo();
    return true;
}



//The startingNodes and nodeDistance parameters are only used if the graph scope
//is not WHOLE_GRAPH.
void AssemblyGraph::buildOgdfGraphFromNodesAndEdges(std::vector<DeBruijnNode *> startingNodes, int nodeDistance)
{
    if (g_settings->graphScope == WHOLE_GRAPH)
    {
        QMapIterator<QString, DeBruijnNode*> i(g_assemblyGraph->m_deBruijnGraphNodes);
        while (i.hasNext())
        {
            i.next();

            //If double mode is off, only positive nodes are drawn.  If it's
            //on, all nodes are drawn.
            if (i.value()->isPositiveNode() || g_settings->doubleMode)
                i.value()->m_drawn = true;
        }
    }
    else //The scope is either around specified nodes or around nodes with BLAST hits
    {
        for (size_t i = 0; i < startingNodes.size(); ++i)
        {
            DeBruijnNode * node = startingNodes[i];

            //If we are in single mode, make sure that each node is positive.
            if (!g_settings->doubleMode && node->isNegativeNode())
                node = node->m_reverseComplement;

            node->m_drawn = true;
            node->m_startingNode = true;
            node->labelNeighbouringNodesAsDrawn(nodeDistance, 0);
        }
    }

    //First loop through each node, adding it to OGDF if it is drawn.
    QMapIterator<QString, DeBruijnNode*> i(g_assemblyGraph->m_deBruijnGraphNodes);
    while (i.hasNext())
    {
        i.next();
        if (i.value()->m_drawn)
            i.value()->addToOgdfGraph(g_assemblyGraph->m_ogdfGraph);
    }

    //Then loop through each determining its drawn status and adding it
    //to OGDF if it is drawn.
    for (size_t i = 0; i < g_assemblyGraph->m_deBruijnGraphEdges.size(); ++i)
    {
        g_assemblyGraph->m_deBruijnGraphEdges[i]->determineIfDrawn();
        if (g_assemblyGraph->m_deBruijnGraphEdges[i]->m_drawn)
            g_assemblyGraph->m_deBruijnGraphEdges[i]->addToOgdfGraph(g_assemblyGraph->m_ogdfGraph);
    }
}



void AssemblyGraph::addGraphicsItemsToScene(MyGraphicsScene * scene)
{
    scene->clear();

    double meanDrawnCoverage = g_assemblyGraph->getMeanDeBruijnGraphCoverage(true);

    //First make the GraphicsItemNode objects
    QMapIterator<QString, DeBruijnNode*> i(g_assemblyGraph->m_deBruijnGraphNodes);
    while (i.hasNext())
    {
        i.next();
        DeBruijnNode * node = i.value();

        if (node->m_drawn)
        {
            if (meanDrawnCoverage == 0)
                node->m_coverageRelativeToMeanDrawnCoverage = 1.0;
            else
                node->m_coverageRelativeToMeanDrawnCoverage = node->m_coverage / meanDrawnCoverage;
            GraphicsItemNode * graphicsItemNode = new GraphicsItemNode(node, g_assemblyGraph->m_graphAttributes);
            node->m_graphicsItemNode = graphicsItemNode;
            graphicsItemNode->setFlag(QGraphicsItem::ItemIsSelectable);
            graphicsItemNode->setFlag(QGraphicsItem::ItemIsMovable);
        }
    }

    g_assemblyGraph->resetAllNodeColours();

    //Then make the GraphicsItemEdge objects and add them to the scene first
    //so they are drawn underneath
    for (size_t i = 0; i < g_assemblyGraph->m_deBruijnGraphEdges.size(); ++i)
    {
        if (g_assemblyGraph->m_deBruijnGraphEdges[i]->m_drawn)
        {
            GraphicsItemEdge * graphicsItemEdge = new GraphicsItemEdge(g_assemblyGraph->m_deBruijnGraphEdges[i]);
            g_assemblyGraph->m_deBruijnGraphEdges[i]->m_graphicsItemEdge = graphicsItemEdge;
            graphicsItemEdge->setFlag(QGraphicsItem::ItemIsSelectable);
            scene->addItem(graphicsItemEdge);
        }
    }

    //Now add the GraphicsItemNode objects to the scene so they are drawn
    //on top
    QMapIterator<QString, DeBruijnNode*> j(g_assemblyGraph->m_deBruijnGraphNodes);
    while (j.hasNext())
    {
        j.next();
        DeBruijnNode * node = j.value();
        if (node->hasGraphicsItem())
            scene->addItem(node->m_graphicsItemNode);
    }
}




std::vector<DeBruijnNode *> AssemblyGraph::getStartingNodes(QString * errorTitle, QString * errorMessage, bool doubleMode,
                                                            QString nodesList, QString blastQueryName)
{
    std::vector<DeBruijnNode *> startingNodes;

    if (g_settings->graphScope == AROUND_NODE)
    {
        if (checkIfStringHasNodes(nodesList))
        {
            *errorTitle = "No starting nodes";
            *errorMessage = "Please enter at least one node when drawing the graph using the 'Around node(s)' scope. "
                            "Separate multiple nodes with commas.";
            return startingNodes;
        }

        //Make sure the nodes the user typed in are actually in the graph.
        std::vector<QString> nodesNotInGraph;
        std::vector<DeBruijnNode *> nodesInGraph = getNodesFromString(nodesList,
                                                                      g_settings->startingNodesExactMatch,
                                                                      &nodesNotInGraph);
        if (nodesNotInGraph.size() > 0)
        {
            *errorTitle = "Nodes not found";
            *errorMessage = generateNodesNotFoundErrorMessage(nodesNotInGraph, g_settings->startingNodesExactMatch);
            if (nodesInGraph.size() == 0)
                return startingNodes;
        }
    }

    else if (g_settings->graphScope == AROUND_BLAST_HITS)
    {
        std::vector<DeBruijnNode *> startingNodes = g_assemblyGraph->getNodesFromBlastHits(blastQueryName);

        if (startingNodes.size() == 0)
        {
            *errorTitle = "No BLAST hits";
            *errorMessage = "To draw the graph around BLAST hits, you must first conduct a BLAST search.";
            return startingNodes;
        }
    }

    g_settings->doubleMode = doubleMode;
    g_assemblyGraph->clearOgdfGraphAndResetNodes();

    if (g_settings->graphScope == AROUND_NODE)
        startingNodes = getNodesFromString(nodesList, g_settings->startingNodesExactMatch);
    else if (g_settings->graphScope == AROUND_BLAST_HITS)
        startingNodes = g_assemblyGraph->getNodesFromBlastHits(blastQueryName);

    return startingNodes;
}


bool AssemblyGraph::checkIfStringHasNodes(QString nodesString)
{
    nodesString = nodesString.simplified();
    QStringList nodesList = nodesString.split(",");
    nodesList = removeNullStringsFromList(nodesList);
    return (nodesList.size() == 0);
}


QString AssemblyGraph::generateNodesNotFoundErrorMessage(std::vector<QString> nodesNotInGraph, bool exact)
{
    QString errorMessage;
    if (exact)
        errorMessage += "The following nodes are not in the graph:\n";
    else
        errorMessage += "The following queries do not match any nodes in the graph:\n";

    for (size_t i = 0; i < nodesNotInGraph.size(); ++i)
    {
        errorMessage += nodesNotInGraph[i];
        if (i != nodesNotInGraph.size() - 1)
            errorMessage += ", ";
    }
    errorMessage += "\n";

    return errorMessage;
}


std::vector<DeBruijnNode *> AssemblyGraph::getNodesFromString(QString nodeNamesString, bool exactMatch, std::vector<QString> * nodesNotInGraph)
{
    nodeNamesString = nodeNamesString.simplified();
    QStringList nodesList = nodeNamesString.split(",");

    if (exactMatch)
        return getNodesFromListExact(nodesList, nodesNotInGraph);
    else
        return getNodesFromListPartial(nodesList, nodesNotInGraph);
}


//Given a list of node names (as strings), this function will return all nodes which match
//those names exactly.  The last +/- on the end of the node name is optional - if missing
//both + and - nodes will be returned.
std::vector<DeBruijnNode *> AssemblyGraph::getNodesFromListExact(QStringList nodesList, std::vector<QString> * nodesNotInGraph)
{
    std::vector<DeBruijnNode *> returnVector;

    for (int i = 0; i < nodesList.size(); ++i)
    {
        QString nodeName = nodesList.at(i).simplified();
        if (nodeName == "")
            continue;

        //If the node name ends in +/-, then we assume the user was specifying the exact
        //node in the pair.  If the node name does not end in +/-, then we assume the
        //user is asking for either node in the pair.
        QChar lastChar = nodeName.at(nodeName.length() - 1);
        if (lastChar == '+' || lastChar == '-')
        {
            if (g_assemblyGraph->m_deBruijnGraphNodes.contains(nodeName))
                returnVector.push_back(g_assemblyGraph->m_deBruijnGraphNodes[nodeName]);
            else if (nodesNotInGraph != 0)
                nodesNotInGraph->push_back(nodesList.at(i).trimmed());
        }
        else
        {
            QString posNodeName = nodeName + "+";
            QString negNodeName = nodeName + "-";

            bool posNodeFound = false;
            if (g_assemblyGraph->m_deBruijnGraphNodes.contains(posNodeName))
            {
                returnVector.push_back(g_assemblyGraph->m_deBruijnGraphNodes[posNodeName]);
                posNodeFound = true;
            }

            bool negNodeFound = false;
            if (g_assemblyGraph->m_deBruijnGraphNodes.contains(negNodeName))
            {
                returnVector.push_back(g_assemblyGraph->m_deBruijnGraphNodes[negNodeName]);
                negNodeFound = true;
            }

            if (!posNodeFound && !negNodeFound && nodesNotInGraph != 0)
                nodesNotInGraph->push_back(nodesList.at(i).trimmed());
        }
    }

    return returnVector;
}

std::vector<DeBruijnNode *> AssemblyGraph::getNodesFromListPartial(QStringList nodesList, std::vector<QString> * nodesNotInGraph)
{
    std::vector<DeBruijnNode *> returnVector;

    for (int i = 0; i < nodesList.size(); ++i)
    {
        QString queryName = nodesList.at(i).simplified();
        if (queryName == "")
            continue;

        bool found = false;
        QMapIterator<QString, DeBruijnNode*> j(g_assemblyGraph->m_deBruijnGraphNodes);
        while (j.hasNext())
        {
            j.next();
            QString nodeName = j.value()->m_name;

            if (nodeName.contains(queryName))
            {
                found = true;
                returnVector.push_back(j.value());
            }
        }

        if (!found && nodesNotInGraph != 0)
            nodesNotInGraph->push_back(queryName.trimmed());
    }

    return returnVector;
}

std::vector<DeBruijnNode *> AssemblyGraph::getNodesFromBlastHits(QString queryName)
{
    std::vector<DeBruijnNode *> returnVector;

    if (g_blastSearch->m_blastQueries.m_queries.size() == 0)
        return returnVector;

    std::vector<BlastQuery *> queries;

    //If "all" is selected, then we'll display nodes with hits from any query
    if (queryName == "all")
        queries = g_blastSearch->m_blastQueries.m_queries;

    //If only one query is selected, then we just display nodes with hits from that query
    else
        queries.push_back(g_blastSearch->m_blastQueries.getQueryFromName(queryName));

    //Add the blast hit pointers to nodes that have a hit for
    //the selected target(s).
    for (size_t i = 0; i < queries.size(); ++i)
    {
        BlastQuery * currentQuery = queries[i];
        for (size_t j = 0; j < g_blastSearch->m_hits.size(); ++j)
        {
            if (g_blastSearch->m_hits[j].m_query == currentQuery)
                returnVector.push_back(g_blastSearch->m_hits[j].m_node);
        }
    }

    return returnVector;
}



QStringList AssemblyGraph::removeNullStringsFromList(QStringList in)
{
    QStringList out;

    for (int i = 0; i < in.size(); ++i)
    {
        QString string = in.at(i);
        if (string.length() > 0)
            out.push_back(string);
    }
    return out;
}






//Unlike the equivalent function in MainWindow, this does the graph layout in the main thread.
void AssemblyGraph::layoutGraph()
{
    ogdf::FMMMLayout fmmm;
    GraphLayoutWorker * graphLayoutWorker = new GraphLayoutWorker(&fmmm, g_assemblyGraph->m_graphAttributes,
                                                                  g_settings->graphLayoutQuality, g_settings->segmentLength);
    graphLayoutWorker->layoutGraph();
}



void AssemblyGraph::setAllEdgesExactOverlap(int overlap)
{
    for (size_t i = 0; i < m_deBruijnGraphEdges.size(); ++i)
        m_deBruijnGraphEdges[i]->setExactOverlap(overlap);
}



void AssemblyGraph::autoDetermineAllEdgesExactOverlap()
{
    int edgeCount = m_deBruijnGraphEdges.size();
    if (edgeCount == 0)
        return;

    //Determine the overlap for each edge and produce a vector
    //that
    for (size_t i = 0; i < m_deBruijnGraphEdges.size(); ++i)
        m_deBruijnGraphEdges[i]->autoDetermineExactOverlap();

    //The expectation here is that most overlaps will be
    //the same or from a small subset of possible sizes.
    //Edges with an overlap that do not match the most common
    //overlap(s) are suspected of having their overlap
    //misidentified.  They are therefore rechecked using the
    //common ones.
    std::vector<int> overlapCounts = makeOverlapCountVector();

    //Sort the overlaps in order of decreasing numbers of edges.
    //I.e. the first overlap size in the vector will be the most
    //common overlap, the second will be the second most common,
    //etc.
    std::vector<int> sortedOverlaps;
    int overlapsSoFar = 0;
    double fractionOverlapsFound = 0.0;
    while (fractionOverlapsFound < 1.0)
    {
        int mostCommonOverlap = 0;
        int mostCommonOverlapCount = 0;

        //Find the overlap size with the most instances.
        for (size_t i = 0; i < overlapCounts.size(); ++i)
        {
            if (overlapCounts[i] > mostCommonOverlapCount)
            {
                mostCommonOverlap = i;
                mostCommonOverlapCount = overlapCounts[i];
            }
        }

        //Add that overlap to the common collection and remove it from the counts.
        sortedOverlaps.push_back(mostCommonOverlap);
        overlapsSoFar += mostCommonOverlapCount;
        fractionOverlapsFound = double(overlapsSoFar) / edgeCount;
        overlapCounts[mostCommonOverlap] = 0;
    }

    //For each edge, see if one of the more common overlaps also works.
    //If so, use that instead.
    for (size_t i = 0; i < m_deBruijnGraphEdges.size(); ++i)
    {
        DeBruijnEdge * edge = m_deBruijnGraphEdges[i];
        for (size_t j = 0; j < sortedOverlaps.size(); ++j)
        {
            if (edge->m_overlap == sortedOverlaps[j])
                break;
            else if (edge->testExactOverlap(sortedOverlaps[j]))
            {
                edge->m_overlap = sortedOverlaps[j];
                break;
            }
        }

    }
    overlapCounts = makeOverlapCountVector(); //TEMP
    int test = 5; //TEMP
}


//This function produces a vector for which the values are the number
//of edges that have an overlap of the index length.
//E.g. if overlapVector[61] = 123, that means that 123 edges have an
//overlap of 61.
std::vector<int> AssemblyGraph::makeOverlapCountVector()
{
    std::vector<int> overlapCounts;

    for (size_t i = 0; i < m_deBruijnGraphEdges.size(); ++i)
    {
        int overlap = m_deBruijnGraphEdges[i]->m_overlap;

        //Add the overlap to the count vector
        if (int(overlapCounts.size()) < overlap + 1)
            overlapCounts.resize(overlap + 1, 0);
        ++overlapCounts[overlap];
    }

    return overlapCounts;
}
