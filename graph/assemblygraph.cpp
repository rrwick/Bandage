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
#include "../program/memory.h"


AssemblyGraph::AssemblyGraph() :
    m_kmer(0), m_contiguitySearchDone(false)
{
    m_ogdfGraph = new ogdf::Graph();
    m_graphAttributes = new ogdf::GraphAttributes(*m_ogdfGraph, ogdf::GraphAttributes::nodeGraphics |
                                                  ogdf::GraphAttributes::edgeGraphics);
    clearGraphInfo();
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
void AssemblyGraph::createDeBruijnEdge(QString node1Name, QString node2Name, int overlap)
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
    const std::vector<DeBruijnEdge *> * edges = node1->getEdgesPointer();
    for (size_t i = 0; i < edges->size(); ++i)
    {
        if ((*edges)[i]->getStartingNode() == node1 &&
                (*edges)[i]->getEndingNode() == node2)
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

    forwardEdge->setReverseComplement(backwardEdge);
    backwardEdge->setReverseComplement(forwardEdge);

    forwardEdge->setOverlap(overlap);
    backwardEdge->setOverlap(overlap);

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


double AssemblyGraph::getMeanReadDepth(bool drawnNodesOnly)
{
    long double readDepthSum = 0.0;
    long long totalLength = 0;

    QMapIterator<QString, DeBruijnNode*> i(m_deBruijnGraphNodes);
    while (i.hasNext())
    {
        i.next();
        DeBruijnNode * node = i.value();

        if (drawnNodesOnly && node->isNotDrawn())
            continue;

        totalLength += node->getLength();
        readDepthSum += node->getLength() * node->getReadDepth();
    }

    if (totalLength == 0)
        return 0.0;
    else
        return readDepthSum / totalLength;
}


double AssemblyGraph::getMeanReadDepth(std::vector<DeBruijnNode *> nodes)
{
    int nodeCount = 0;
    long double readDepthSum = 0.0;
    long long totalLength = 0;

    for (size_t i = 0; i < nodes.size(); ++i)
    {
        DeBruijnNode * node = nodes[i];

        ++nodeCount;
        totalLength += node->getLength();
        readDepthSum += node->getLength() * node->getReadDepth();
    }

    if (totalLength == 0)
        return 0.0;
    else
        return readDepthSum / totalLength;
}


void AssemblyGraph::resetNodeContiguityStatus()
{
    QMapIterator<QString, DeBruijnNode*> i(m_deBruijnGraphNodes);
    while (i.hasNext())
    {
        i.next();
        i.value()->resetContiguityStatus();
    }
    m_contiguitySearchDone = false;
}

void AssemblyGraph::resetAllNodeColours()
{
    QMapIterator<QString, DeBruijnNode*> i(m_deBruijnGraphNodes);
    while (i.hasNext())
    {
        i.next();
        if (i.value()->getGraphicsItemNode() != 0)
            i.value()->getGraphicsItemNode()->setNodeColour();
    }
}

void AssemblyGraph::clearAllBlastHitPointers()
{
    QMapIterator<QString, DeBruijnNode*> i(m_deBruijnGraphNodes);
    while (i.hasNext())
    {
        i.next();
        i.value()->clearBlastHits();
    }
}

void AssemblyGraph::determineGraphInfo()
{
    m_shortestContig = std::numeric_limits<long long>::max();
    m_longestContig = 0;
    int nodeCount = 0;
    long long totalLength = 0;
    std::vector<double> nodeReadDepths;

    QMapIterator<QString, DeBruijnNode*> i(m_deBruijnGraphNodes);
    while (i.hasNext())
    {
        i.next();
        long long nodeLength = i.value()->getLength();

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

        nodeReadDepths.push_back(i.value()->getReadDepth());
    }

    //Count up the edges.  Edges that are their own pairs will
    //not be counted, as these won't show up in single mode.
    int edgeCount = 0;
    for (size_t i = 0; i < m_deBruijnGraphEdges.size(); ++i)
    {
        DeBruijnEdge * edge = m_deBruijnGraphEdges[i];
        if (edge != edge->getReverseComplement())
            ++edgeCount;
    }
    edgeCount /= 2;

    m_nodeCount = nodeCount;
    m_edgeCount = edgeCount;
    m_totalLength = totalLength;
    m_meanReadDepth = getMeanReadDepth();

    std::sort(nodeReadDepths.begin(), nodeReadDepths.end());

    double firstQuartileIndex = nodeReadDepths.size() / 4.0;
    double medianIndex = nodeReadDepths.size() / 2.0;
    double thirdQuartileIndex = nodeReadDepths.size() * 3.0 / 4.0;

    m_firstQuartileReadDepth = getValueUsingFractionalIndex(&nodeReadDepths, firstQuartileIndex);
    m_medianReadDepth = getValueUsingFractionalIndex(&nodeReadDepths, medianIndex);
    m_thirdQuartileReadDepth = getValueUsingFractionalIndex(&nodeReadDepths, thirdQuartileIndex);

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

    m_meanReadDepth = 0.0;
    m_firstQuartileReadDepth = 0.0;
    m_medianReadDepth = 0.0;
    m_thirdQuartileReadDepth = 0.0;
}






void AssemblyGraph::buildDeBruijnGraphFromLastGraph(QString fullFileName)
{
    m_graphFileType = LAST_GRAPH;

    bool firstLine = true;
    QFile inputFile(fullFileName);
    if (inputFile.open(QIODevice::ReadOnly))
    {
        QTextStream in(&inputFile);
        while (!in.atEnd())
        {
            QApplication::processEvents();
            QString line = in.readLine();

            if (firstLine)
            {
                QStringList firstLineParts = line.split(QRegExp("\\s+"));
                if (firstLineParts.size() > 2)
                m_kmer = firstLineParts[2].toInt();
                firstLine = false;
            }

            if (line.startsWith("NODE"))
            {
                QStringList nodeDetails = line.split(QRegExp("\\s+"));

                if (nodeDetails.size() < 4)
                    throw "load error";

                QString nodeName = nodeDetails.at(1);
                QString posNodeName = nodeName + "+";
                QString negNodeName = nodeName + "-";

                int nodeLength = nodeDetails.at(2).toInt();

                double nodeReadDepth;
                if (nodeLength > 0)
                    nodeReadDepth = double(nodeDetails.at(3).toInt()) / nodeLength; //IS THIS COLUMN ($COV_SHORT1) THE BEST ONE TO USE?
                else
                    nodeReadDepth = double(nodeDetails.at(3).toInt());

                QByteArray sequence = in.readLine().toLocal8Bit();
                QByteArray revCompSequence = in.readLine().toLocal8Bit();

                DeBruijnNode * node = new DeBruijnNode(posNodeName, nodeReadDepth, sequence);
                DeBruijnNode * reverseComplementNode = new DeBruijnNode(negNodeName, nodeReadDepth, revCompSequence);
                node->setReverseComplement(reverseComplementNode);
                reverseComplementNode->setReverseComplement(node);
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
        std::vector<int> edgeOverlaps;

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

                //If there is an attribute holding the read depth, we'll use that.
                //If there isn't, then we'll use zero.
                double nodeReadDepth = 0.0;

                for (int i = 3; i < lineParts.size(); ++i)
                {
                    QString part = lineParts.at(i);
                    if (part.size() < 6)
                        continue;
                    else if (part.left(5) == "KC:f:")
                        nodeReadDepth = part.right(part.length() - 5).toDouble();
                }

                DeBruijnNode * node = new DeBruijnNode(posNodeName, nodeReadDepth, sequence);
                DeBruijnNode * reverseComplementNode = new DeBruijnNode(negNodeName, nodeReadDepth, revCompSequence);
                node->setReverseComplement(reverseComplementNode);
                reverseComplementNode->setReverseComplement(node);
                m_deBruijnGraphNodes.insert(posNodeName, node);
                m_deBruijnGraphNodes.insert(negNodeName, reverseComplementNode);
            }

            //Lines beginning with "L" are link (edge) lines
            else if (lineParts.at(0) == "L")
            {
                //Edges aren't made now, in case their sequence hasn't yet been specified.
                //Instead, we save the starting and ending nodes and make the edges after
                //we're done looking at the file.

                if (lineParts.size() < 6)
                    throw "load error";

                //Parts 1 and 3 hold the node names and parts 2 and 4 hold the corresponding +/-.
                QString startingNode = lineParts.at(1) + lineParts.at(2);
                QString endingNode = lineParts.at(3) + lineParts.at(4);
                edgeStartingNodeNames.push_back(startingNode);
                edgeEndingNodeNames.push_back(endingNode);

                //Part 5 holds the node overlap cigar string
                QString cigar = lineParts.at(5);
                edgeOverlaps.push_back(getLengthFromCigar(cigar));
            }
        }

        //Create all of the edges
        for (size_t i = 0; i < edgeStartingNodeNames.size(); ++i)
        {
            QString node1Name = edgeStartingNodeNames[i];
            QString node2Name = edgeEndingNodeNames[i];
            int overlap = edgeOverlaps[i];
            createDeBruijnEdge(node1Name, node2Name, overlap);
        }
    }

    if (m_deBruijnGraphNodes.size() == 0)
        throw "load error";
}




//This function converts a CIGAR string to a length.  It is
//currently incomplete, only looking at matches and mismatches.
//I'm waiting for the GFA format to be pinned down before
//putting more work into this.
int AssemblyGraph::getLengthFromCigar(QString cigar)
{
    int matchCount = getCigarCount("M", cigar);
    int mismatchCount = getCigarCount("X", cigar);

    return matchCount + mismatchCount;
}


//This function totals up the numbers for any given CIGAR code.
int AssemblyGraph::getCigarCount(QString cigarCode, QString cigar)
{
    QRegExp rx("(\\d+)" + cigarCode);
    QStringList list;
    int pos = 0;
    while ((pos = rx.indexIn(cigar, pos)) != -1)
    {
        list << rx.cap(1);
        pos += rx.matchedLength();
    }

    int sum = 0;
    for (int i = 0; i < list.size(); ++i)
        sum += list.at(i).toInt();

    return sum;
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
            double nodeReadDepth;

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

                QString nodeReadDepthString = thisNodeDetails.at(5);
                if (negativeNode)
                {
                    //It may be necessary to remove a single quote from the end of the read depth
                    if (nodeReadDepthString.at(nodeReadDepthString.size() - 1) == '\'')
                        nodeReadDepthString.chop(1);
                }
                nodeReadDepth = nodeReadDepthString.toDouble();

                //Make the node
                node = new DeBruijnNode(nodeName, nodeReadDepth, ""); //Sequence string is currently empty - will be added to on subsequent lines of the fastg file
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
                    node->appendToSequence(sequenceLine);
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
    QString reverseComplementName = getOppositeNodeName(node->getName());

    DeBruijnNode * reverseComplementNode = m_deBruijnGraphNodes[reverseComplementName];
    if (reverseComplementNode == 0)
    {
        DeBruijnNode * newNode = new DeBruijnNode(reverseComplementName, node->getReadDepth(),
                                                  getReverseComplement(node->getSequence()));
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
            DeBruijnNode * negativeNode = m_deBruijnGraphNodes[getOppositeNodeName(positiveNode->getName())];
            if (negativeNode != 0)
            {
                positiveNode->setReverseComplement(negativeNode);
                negativeNode->setReverseComplement(positiveNode);
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
                DeBruijnNode * node = new DeBruijnNode(nodeName, 0.0, nodeSequence);
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
    if (checkFileIsFastG(fullFileName))
        return FASTG;
    if (checkFileIsGfa(fullFileName))
        return GFA;
    if (checkFileIsTrinityFasta(fullFileName))
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

/* Split a QString according to CSV rules
 *
 * @param line  line of a csv
 * @param sep   field separator to use
 * @result      list of fields with escaping removed
 *
 * Known Bugs: CSV (as per RFC4180) allows multi-line fields (\r\n between "..."), which
 *             can't be parsed line-by line an hence isn't supported.
 */
QStringList AssemblyGraph::splitCsv(QString line, QString sep)
{
    QStringList list;
    QRegExp rx("(\"(?:[^\"]|\"\")*\"|[^"+sep+"]*)");
    int pos = 0;

    while (rx.indexIn(line, pos) != -1)
    {
        QString field = rx.cap().replace("\"\"","\"");
        if (field[0] == '"' && field[field.length()-1] == '"')
            field=field.mid(1,field.length()-2);
        list << field;
        pos += rx.matchedLength() +1;
    }
    return list;
}

/* Load data from CSV and add to deBruijnGraphNodes
 *
 * @param filename  the full path of the file to be loaded
 * @param *columns  will contain the names of each column after loading data
 *                  (to add these to the GUI)
 * @param *errormsg if not empty, message to be displayed to user containing warning
 *                  or other information
 * @returns         true/false if loading data worked
 */
bool AssemblyGraph::loadCSV(QString filename, QStringList * columns, QString * errormsg)
{
    clearAllCsvData();

    QFile inputFile(filename);
    if (!inputFile.open(QIODevice::ReadOnly))
    {
        *errormsg = "Unable to read from specified file.";
        return false;
    }
    QTextStream in(&inputFile);
    QString line = in.readLine();

    // guess at separator; this assumes that any tab in the first line means
    // we have a tab separated file
    QString sep = "\t";
    if (line.split(sep).length() == 1)
    {
        sep = ",";
        if (line.split(sep).length() == 1)
        {
            *errormsg = "Neither tab nor comma in first line. Please check file format.";
            return false;
        }
    }

    int unmatched_nodes = 0; // keep a counter for lines in file that can't be matched to nodes

    QStringList headers = splitCsv(line, sep);
    if (headers.size() < 2)
    {
        *errormsg = "Not enough CSV headers: at least two required.";
        return false;
    }

    headers.pop_front();
    *columns = headers;
    int columnCount = headers.size();

    while (!in.atEnd())
    {
        QApplication::processEvents();

        QStringList cols = splitCsv(in.readLine(), sep);
        QString nodeName = getNodeNameFromString(cols[0]);

        //Get rid of the node name - no need to save that.
        cols.pop_front();

        //Get rid of any extra data that doesn't have a header.
        while (cols.size() > columnCount)
            cols.pop_back();

        if (nodeName != "" && m_deBruijnGraphNodes.contains(nodeName))
            m_deBruijnGraphNodes[nodeName]->setCsvData(cols);
        else
            unmatched_nodes++;
    }

    if (unmatched_nodes)
        *errormsg = "There were " + QString::number(unmatched_nodes) + " unmatched entries in the CSV.";

    return true;
}


//This function extracts a node name from a string.
//It first assumes the string has this format: NODE_6+_length_50434_cov_42.3615
//If that doesn't seem to work, it assumes the string is just the node name.
//If the node name it finds does not end in a '+' or '-', it will add '+'.
QString AssemblyGraph::getNodeNameFromString(QString string)
{
    QString nodeName;
    QStringList parts = string.split("_");
    if (parts.size() >= 2)
        nodeName = parts[1];
    else
        nodeName = parts[0];

    int nameLength = nodeName.length();
    if (nameLength == 0)
        return "";

    QChar lastChar = nodeName.at(nameLength - 1);
    if (lastChar == '+' || lastChar == '-')
        return nodeName;
    else
        return nodeName + "+";
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
    g_memory->clearGraphSpecificMemory();
    return true;
}



//The startingNodes and nodeDistance parameters are only used if the graph scope
//is not WHOLE_GRAPH.
void AssemblyGraph::buildOgdfGraphFromNodesAndEdges(std::vector<DeBruijnNode *> startingNodes, int nodeDistance)
{
    if (g_settings->graphScope == WHOLE_GRAPH)
    {
        QMapIterator<QString, DeBruijnNode*> i(m_deBruijnGraphNodes);
        while (i.hasNext())
        {
            i.next();

            //If double mode is off, only positive nodes are drawn.  If it's
            //on, all nodes are drawn.
            if (i.value()->isPositiveNode() || g_settings->doubleMode)
                i.value()->setAsDrawn();
        }
    }
    else //The scope is either around specified nodes, around nodes with BLAST hits or a read depth range.
    {
        //Distance is only used for around nodes and around blast scopes, not
        //for the read depth range scope.
        if (g_settings->graphScope == READ_DEPTH_RANGE)
            nodeDistance = 0;

        for (size_t i = 0; i < startingNodes.size(); ++i)
        {
            DeBruijnNode * node = startingNodes[i];

            //If we are in single mode, make sure that each node is positive.
            if (!g_settings->doubleMode && node->isNegativeNode())
                node = node->getReverseComplement();

            node->setAsDrawn();
            node->setAsSpecial();
            node->labelNeighbouringNodesAsDrawn(nodeDistance, 0);
        }
    }

    //First loop through each node, adding it to OGDF if it is drawn.
    QMapIterator<QString, DeBruijnNode*> i(m_deBruijnGraphNodes);
    while (i.hasNext())
    {
        i.next();
        if (i.value()->isDrawn())
            i.value()->addToOgdfGraph(m_ogdfGraph);
    }

    //Then loop through each determining its drawn status and adding it
    //to OGDF if it is drawn.
    for (size_t i = 0; i < m_deBruijnGraphEdges.size(); ++i)
    {
        m_deBruijnGraphEdges[i]->determineIfDrawn();
        if (m_deBruijnGraphEdges[i]->isDrawn())
            m_deBruijnGraphEdges[i]->addToOgdfGraph(m_ogdfGraph);
    }
}



void AssemblyGraph::addGraphicsItemsToScene(MyGraphicsScene * scene)
{
    scene->clear();

    double meanDrawnReadDepth = getMeanReadDepth(true);

    //First make the GraphicsItemNode objects
    QMapIterator<QString, DeBruijnNode*> i(m_deBruijnGraphNodes);
    while (i.hasNext())
    {
        i.next();
        DeBruijnNode * node = i.value();

        if (node->isDrawn())
        {
            if (meanDrawnReadDepth == 0)
                node->setReadDepthRelativeToMeanDrawnReadDepth(1.0);
            else
                node->setReadDepthRelativeToMeanDrawnReadDepth(node->getReadDepth() / meanDrawnReadDepth);
            GraphicsItemNode * graphicsItemNode = new GraphicsItemNode(node, m_graphAttributes);
            node->setGraphicsItemNode(graphicsItemNode);
            graphicsItemNode->setFlag(QGraphicsItem::ItemIsSelectable);
            graphicsItemNode->setFlag(QGraphicsItem::ItemIsMovable);
        }
    }

    resetAllNodeColours();

    //Then make the GraphicsItemEdge objects and add them to the scene first
    //so they are drawn underneath
    for (size_t i = 0; i < m_deBruijnGraphEdges.size(); ++i)
    {
        if (m_deBruijnGraphEdges[i]->isDrawn())
        {
            GraphicsItemEdge * graphicsItemEdge = new GraphicsItemEdge(m_deBruijnGraphEdges[i]);
            m_deBruijnGraphEdges[i]->setGraphicsItemEdge(graphicsItemEdge);
            graphicsItemEdge->setFlag(QGraphicsItem::ItemIsSelectable);
            scene->addItem(graphicsItemEdge);
        }
    }

    //Now add the GraphicsItemNode objects to the scene so they are drawn
    //on top
    QMapIterator<QString, DeBruijnNode*> j(m_deBruijnGraphNodes);
    while (j.hasNext())
    {
        j.next();
        DeBruijnNode * node = j.value();
        if (node->hasGraphicsItem())
            scene->addItem(node->getGraphicsItemNode());
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
        std::vector<DeBruijnNode *> startingNodes = getNodesFromBlastHits(blastQueryName);

        if (startingNodes.size() == 0)
        {
            *errorTitle = "No BLAST hits";
            *errorMessage = "To draw the graph around BLAST hits, you must first conduct a BLAST search.";
            return startingNodes;
        }
    }

    else if (g_settings->graphScope == READ_DEPTH_RANGE)
    {
        if (g_settings->minReadDepthRange > g_settings->maxReadDepthRange)
        {
            *errorTitle = "Invalid read depth range";
            *errorMessage = "The maximum read depth must be greater than or equal to the minimum read depth.";
            return startingNodes;
        }

        std::vector<DeBruijnNode *> startingNodes = getNodesInReadDepthRange(g_settings->minReadDepthRange,
                                                                             g_settings->maxReadDepthRange);

        if (startingNodes.size() == 0)
        {
            *errorTitle = "No nodes in range";
            *errorMessage = "There are no nodes with read depths in the specified range.";
            return startingNodes;
        }
    }

    g_settings->doubleMode = doubleMode;
    clearOgdfGraphAndResetNodes();

    if (g_settings->graphScope == AROUND_NODE)
        startingNodes = getNodesFromString(nodesList, g_settings->startingNodesExactMatch);
    else if (g_settings->graphScope == AROUND_BLAST_HITS)
        startingNodes = getNodesFromBlastHits(blastQueryName);
    else if (g_settings->graphScope == READ_DEPTH_RANGE)
        startingNodes = getNodesInReadDepthRange(g_settings->minReadDepthRange,
                                                 g_settings->maxReadDepthRange);

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
std::vector<DeBruijnNode *> AssemblyGraph::getNodesFromListExact(QStringList nodesList,
                                                                 std::vector<QString> * nodesNotInGraph)
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
            if (m_deBruijnGraphNodes.contains(nodeName))
                returnVector.push_back(m_deBruijnGraphNodes[nodeName]);
            else if (nodesNotInGraph != 0)
                nodesNotInGraph->push_back(nodesList.at(i).trimmed());
        }
        else
        {
            QString posNodeName = nodeName + "+";
            QString negNodeName = nodeName + "-";

            bool posNodeFound = false;
            if (m_deBruijnGraphNodes.contains(posNodeName))
            {
                returnVector.push_back(m_deBruijnGraphNodes[posNodeName]);
                posNodeFound = true;
            }

            bool negNodeFound = false;
            if (m_deBruijnGraphNodes.contains(negNodeName))
            {
                returnVector.push_back(m_deBruijnGraphNodes[negNodeName]);
                negNodeFound = true;
            }

            if (!posNodeFound && !negNodeFound && nodesNotInGraph != 0)
                nodesNotInGraph->push_back(nodesList.at(i).trimmed());
        }
    }

    return returnVector;
}

std::vector<DeBruijnNode *> AssemblyGraph::getNodesFromListPartial(QStringList nodesList,
                                                                   std::vector<QString> * nodesNotInGraph)
{
    std::vector<DeBruijnNode *> returnVector;

    for (int i = 0; i < nodesList.size(); ++i)
    {
        QString queryName = nodesList.at(i).simplified();
        if (queryName == "")
            continue;

        bool found = false;
        QMapIterator<QString, DeBruijnNode*> j(m_deBruijnGraphNodes);
        while (j.hasNext())
        {
            j.next();
            QString nodeName = j.value()->getName();

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

    //Add pointers to nodes that have a hit for the selected target(s).
    for (size_t i = 0; i < queries.size(); ++i)
    {
        BlastQuery * currentQuery = queries[i];
        for (int j = 0; j < g_blastSearch->m_allHits.size(); ++j)
        {
            if (g_blastSearch->m_allHits[j]->m_query == currentQuery)
                returnVector.push_back(g_blastSearch->m_allHits[j]->m_node);
        }
    }

    return returnVector;
}

std::vector<DeBruijnNode *> AssemblyGraph::getNodesInReadDepthRange(double min,
                                                                    double max)
{
    std::vector<DeBruijnNode *> returnVector;

    QMapIterator<QString, DeBruijnNode*> i(m_deBruijnGraphNodes);
    while (i.hasNext())
    {
        i.next();
        DeBruijnNode * node = i.value();

        if (node->isInReadDepthRange(min, max))
            returnVector.push_back(node);
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
    GraphLayoutWorker * graphLayoutWorker = new GraphLayoutWorker(&fmmm, m_graphAttributes,
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
    int edgeCount = int(m_deBruijnGraphEdges.size());
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
                mostCommonOverlap = int(i);
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
            if (edge->getOverlap() == sortedOverlaps[j])
                break;
            else if (edge->testExactOverlap(sortedOverlaps[j]))
            {
                edge->setOverlap(sortedOverlaps[j]);
                break;
            }
        }
    }
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
        int overlap = m_deBruijnGraphEdges[i]->getOverlap();

        //Add the overlap to the count vector
        if (int(overlapCounts.size()) < overlap + 1)
            overlapCounts.resize(overlap + 1, 0);
        ++overlapCounts[overlap];
    }

    return overlapCounts;
}


//The function returns a node name, replacing "+" at the end with "-" or
//vice-versa.
QString AssemblyGraph::getOppositeNodeName(QString nodeName)
{
    QChar lastChar = nodeName.at(nodeName.size() - 1);
    nodeName.chop(1);

    if (lastChar == '-')
        return nodeName + "+";
    else
        return nodeName + "-";
}


void AssemblyGraph::readFastaFile(QString filename, std::vector<QString> * names, std::vector<QString> * sequences)
{
    QFile inputFile(filename);
    if (inputFile.open(QIODevice::ReadOnly))
    {
        QString name = "";
        QString sequence = "";

        QTextStream in(&inputFile);
        while (!in.atEnd())
        {
            QApplication::processEvents();

            QString line = in.readLine();

            if (line.length() == 0)
                continue;

            if (line.at(0) == '>')
            {
                //If there is a current sequence, add it to the vectors now.
                if (name.length() > 0)
                {
                    names->push_back(name);
                    sequences->push_back(sequence);
                }

                line.remove(0, 1); //Remove '>' from start
                name = line;
                sequence = "";
            }

            else //It's a sequence line
                sequence += line.simplified();
        }

        //Add the last target to the results now.
        if (name.length() > 0)
        {
            names->push_back(name);
            sequences->push_back(sequence);
        }

        inputFile.close();
    }
}



void AssemblyGraph::recalculateAllNodeWidths()
{
    QMapIterator<QString, DeBruijnNode*> i(m_deBruijnGraphNodes);
    while (i.hasNext())
    {
        i.next();
        GraphicsItemNode * graphicsItemNode = i.value()->getGraphicsItemNode();
        if (graphicsItemNode != 0)
            graphicsItemNode->setWidth();
    }
}



void AssemblyGraph::clearAllCsvData()
{
    QMapIterator<QString, DeBruijnNode*> i(m_deBruijnGraphNodes);
    while (i.hasNext())
    {
        i.next();
        i.value()->clearCsvData();
    }
}


int AssemblyGraph::getDrawnNodeCount() const
{
    int nodeCount = 0;

    QMapIterator<QString, DeBruijnNode*> i(m_deBruijnGraphNodes);
    while (i.hasNext())
    {
        i.next();
        DeBruijnNode * node = i.value();

        if (node->isDrawn())
            ++nodeCount;
    }

    return nodeCount;
}


void AssemblyGraph::deleteNodes(std::vector<DeBruijnNode *> * nodes)
{
    //Build a list of nodes to delete.
    QList<DeBruijnNode *> nodesToDelete;
    for (size_t i = 0; i < nodes->size(); ++i)
    {
        DeBruijnNode * node = (*nodes)[i];
        DeBruijnNode * rcNode = node->getReverseComplement();

        if (!nodesToDelete.contains(node))
            nodesToDelete.push_back(node);
        if (!nodesToDelete.contains(rcNode))
            nodesToDelete.push_back(rcNode);
    }

    //Build a list of edges to delete.
    std::vector<DeBruijnEdge *> edgesToDelete;
    for (int i = 0; i < nodesToDelete.size(); ++i)
    {
        DeBruijnNode * node = nodesToDelete[i];
        const std::vector<DeBruijnEdge *> * nodeEdges = node->getEdgesPointer();
        for (size_t j = 0; j < nodeEdges->size(); ++j)
        {
            DeBruijnEdge * edge = (*nodeEdges)[j];
            bool alreadyAdded = std::find(edgesToDelete.begin(), edgesToDelete.end(), edge) != edgesToDelete.end();
            if (!alreadyAdded)
                edgesToDelete.push_back(edge);
        }
    }

    //Build a list of node names to delete.
    QStringList nodesNamesToDelete;
    for (int i = 0; i < nodesToDelete.size(); ++i)
    {
        DeBruijnNode * node = nodesToDelete[i];
        nodesNamesToDelete.push_back(node->getName());
    }

    //Remove the edges from the graph,
    deleteEdges(&edgesToDelete);

    //Remove the nodes from the graph.
    for (int i = 0; i < nodesNamesToDelete.size(); ++i)
    {
        QString nodeName = nodesNamesToDelete[i];
        m_deBruijnGraphNodes.remove(nodeName);
    }
    for (int i = 0; i < nodesToDelete.size(); ++i)
    {
        DeBruijnNode * node = nodesToDelete[i];
        delete node;
    }

    determineGraphInfo();
}

void AssemblyGraph::deleteEdges(std::vector<DeBruijnEdge *> * edges)
{
    //Build a list of edges to delete.
    QList<DeBruijnEdge *> edgesToDelete;
    for (size_t i = 0; i < edges->size(); ++i)
    {
        DeBruijnEdge * edge = (*edges)[i];
        DeBruijnEdge * rcEdge = edge->getReverseComplement();

        if (!edgesToDelete.contains(edge))
            edgesToDelete.push_back(edge);
        if (!edgesToDelete.contains(rcEdge))
            edgesToDelete.push_back(rcEdge);
    }

    //Remove the edges from the graph,
    for (int i = 0; i < edgesToDelete.size(); ++i)
    {
        DeBruijnEdge * edge = edgesToDelete[i];
        m_deBruijnGraphEdges.erase(std::remove(m_deBruijnGraphEdges.begin(), m_deBruijnGraphEdges.end(), edge), m_deBruijnGraphEdges.end());

        edge->getStartingNode()->removeEdge(edge);
        edge->getEndingNode()->removeEdge(edge);

        delete edge;
    }

    determineGraphInfo();
}



//This function assumes it is receiving a positive node.  It will duplicate both
//the positive and negative node in the pair.  It divided their read depth in
//two, giving half to each node.
void AssemblyGraph::duplicateNodePair(DeBruijnNode * node, MyGraphicsScene * scene)
{
    DeBruijnNode * originalPosNode = node;
    DeBruijnNode * originalNegNode = node->getReverseComplement();

    QString newNodeBaseName = getNewNodeName(originalPosNode->getName());
    QString newPosNodeName = newNodeBaseName + "+";
    QString newNegNodeName = newNodeBaseName + "-";

    double newReadDepth = node->getReadDepth() / 2.0;

    DeBruijnNode * newPosNode = new DeBruijnNode(newPosNodeName, newReadDepth, originalPosNode->getSequence());
    DeBruijnNode * newNegNode = new DeBruijnNode(newNegNodeName, newReadDepth, originalNegNode->getSequence());

    newPosNode->setReverseComplement(newNegNode);
    newNegNode->setReverseComplement(newPosNode);

    m_deBruijnGraphNodes.insert(newPosNodeName, newPosNode);
    m_deBruijnGraphNodes.insert(newNegNodeName, newNegNode);

    std::vector<DeBruijnNode *> downstreamNodes = originalPosNode->getDownstreamNodes();
    for (size_t i = 0; i < downstreamNodes.size(); ++i)
        createDeBruijnEdge(newPosNodeName, downstreamNodes[i]->getName());

    std::vector<DeBruijnNode *> upstreamNodes = originalPosNode->getUpstreamNodes();
    for (size_t i = 0; i < upstreamNodes.size(); ++i)
        createDeBruijnEdge(upstreamNodes[i]->getName(), newPosNodeName);


    originalPosNode->setReadDepth(newReadDepth);
    originalNegNode->setReadDepth(newReadDepth);

    double meanDrawnReadDepth = getMeanReadDepth(true);
    if (meanDrawnReadDepth == 0)
    {
        originalPosNode->setReadDepthRelativeToMeanDrawnReadDepth(1.0);
        originalNegNode->setReadDepthRelativeToMeanDrawnReadDepth(1.0);
    }
    else
    {
        originalPosNode->setReadDepthRelativeToMeanDrawnReadDepth(node->getReadDepth() / meanDrawnReadDepth);
        originalNegNode->setReadDepthRelativeToMeanDrawnReadDepth(node->getReadDepth() / meanDrawnReadDepth);
    }

    duplicateGraphicsNode(originalPosNode, newPosNode, scene);
    duplicateGraphicsNode(originalNegNode, newNegNode, scene);

    determineGraphInfo();
}

QString AssemblyGraph::getNewNodeName(QString oldNodeName)
{
    oldNodeName.chop(1); //Remove trailing +/-

    QString newNodeNameBase = oldNodeName + "_copy";
    QString newNodeName = newNodeNameBase;

    int suffix = 1;
    while (m_deBruijnGraphNodes.contains(newNodeName + "+"))
    {
        ++suffix;
        newNodeName = newNodeNameBase + QString::number(suffix);
    }

    return newNodeName;
}


void AssemblyGraph::duplicateGraphicsNode(DeBruijnNode * originalNode, DeBruijnNode * newNode, MyGraphicsScene * scene)
{
    GraphicsItemNode * originalGraphicsItemNode = originalNode->getGraphicsItemNode();
    if (originalGraphicsItemNode == 0)
        return;

    originalGraphicsItemNode->setWidth();

    GraphicsItemNode * newGraphicsItemNode = new GraphicsItemNode(newNode, originalGraphicsItemNode);

    newNode->setGraphicsItemNode(newGraphicsItemNode);
    newGraphicsItemNode->setFlag(QGraphicsItem::ItemIsSelectable);
    newGraphicsItemNode->setFlag(QGraphicsItem::ItemIsMovable);

    originalGraphicsItemNode->shiftPointsLeft();
    newGraphicsItemNode->shiftPointsRight();
    originalGraphicsItemNode->fixEdgePaths();

    originalGraphicsItemNode->setNodeColour();
    newGraphicsItemNode->setNodeColour();

    scene->addItem(newGraphicsItemNode);

    const std::vector<DeBruijnEdge *> * newEdges = newNode->getEdgesPointer();
    for (size_t i = 0; i < newEdges->size(); ++i)
    {
        DeBruijnEdge * newEdge = (*newEdges)[i];
        GraphicsItemEdge * graphicsItemEdge = new GraphicsItemEdge(newEdge);
        graphicsItemEdge->setZValue(-1.0);
        newEdge->setGraphicsItemEdge(graphicsItemEdge);
        graphicsItemEdge->setFlag(QGraphicsItem::ItemIsSelectable);
        scene->addItem(graphicsItemEdge);
    }
}
