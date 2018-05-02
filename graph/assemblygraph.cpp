//Copyright 2017 Ryan Wick

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
#include "../ogdf/energybased/FMMMLayout.h"
#include "../program/graphlayoutworker.h"
#include "../program/memory.h"
#include "path.h"
#include "../ui/myprogressdialog.h"
#include <limits>
#include <QSet>
#include <QQueue>
#include <QList>
#include <math.h>
#include <QFileInfo>
#include <QDir>
#include "ogdfnode.h"
#include "../command_line/commoncommandlinefunctions.h"

AssemblyGraph::AssemblyGraph() :
    m_kmer(0), m_contiguitySearchDone(false),
    m_sequencesLoadedFromFasta(NOT_READY)
{
    m_ogdfGraph = new ogdf::Graph();
    m_edgeArray = new ogdf::EdgeArray<double>(*m_ogdfGraph);
    m_graphAttributes = new ogdf::GraphAttributes(*m_ogdfGraph, ogdf::GraphAttributes::nodeGraphics |
                                                  ogdf::GraphAttributes::edgeGraphics);
    clearGraphInfo();
}

AssemblyGraph::~AssemblyGraph()
{
    delete m_graphAttributes;
    delete m_edgeArray;
    delete m_ogdfGraph;
}


void AssemblyGraph::cleanUp()
{
    {
        QMapIterator<QString, Path*> i(m_deBruijnGraphPaths);
        while (i.hasNext())
        {
            i.next();
            delete i.value();
        }
        m_deBruijnGraphPaths.clear();
    }


    {
        QMapIterator<QString, DeBruijnNode*> i(m_deBruijnGraphNodes);
        while (i.hasNext())
        {
            i.next();
            delete i.value();
        }
        m_deBruijnGraphNodes.clear();
    }

    QMapIterator<QPair<DeBruijnNode*, DeBruijnNode*>, DeBruijnEdge*> j(m_deBruijnGraphEdges);
    while (j.hasNext())
    {
        j.next();
        delete j.value();
    }
    m_deBruijnGraphEdges.clear();

    m_contiguitySearchDone = false;

    clearGraphInfo();
}





//This function makes a double edge: in one direction for the given nodes
//and the opposite direction for their reverse complements.  It adds the
//new edges to the vector here and to the nodes themselves.
void AssemblyGraph::createDeBruijnEdge(QString node1Name, QString node2Name,
                                       int overlap, EdgeOverlapType overlapType)
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
    forwardEdge->setOverlapType(overlapType);
    backwardEdge->setOverlapType(overlapType);

    m_deBruijnGraphEdges.insert(QPair<DeBruijnNode*, DeBruijnNode*>(forwardEdge->getStartingNode(), forwardEdge->getEndingNode()), forwardEdge);
    if (!isOwnPair)
        m_deBruijnGraphEdges.insert(QPair<DeBruijnNode*, DeBruijnNode*>(backwardEdge->getStartingNode(), backwardEdge->getEndingNode()), backwardEdge);

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
    m_edgeArray->init(*m_ogdfGraph);
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
        case '*': reverseComplement.append('*'); break;
        }
    }

    return reverseComplement;
}




void AssemblyGraph::resetEdges()
{
    QMapIterator<QPair<DeBruijnNode*, DeBruijnNode*>, DeBruijnEdge*> i(m_deBruijnGraphEdges);
    while (i.hasNext())
    {
        i.next();
        i.value()->reset();
    }
}


double AssemblyGraph::getMeanDepth(bool drawnNodesOnly)
{
    long double depthSum = 0.0;
    long long totalLength = 0;

    QMapIterator<QString, DeBruijnNode*> i(m_deBruijnGraphNodes);
    while (i.hasNext())
    {
        i.next();
        DeBruijnNode * node = i.value();

        if (drawnNodesOnly && node->isNotDrawn())
            continue;

        totalLength += node->getLength();
        depthSum += node->getLength() * node->getDepth();
    }

    if (totalLength == 0)
        return 0.0;
    else
        return depthSum / totalLength;
}


double AssemblyGraph::getMeanDepth(std::vector<DeBruijnNode *> nodes)
{
    if (nodes.size() == 0)
        return 0.0;

    if (nodes.size() == 1)
        return nodes[0]->getDepth();

    int nodeCount = 0;
    long double depthSum = 0.0;
    long long totalLength = 0;

    for (size_t i = 0; i < nodes.size(); ++i)
    {
        DeBruijnNode * node = nodes[i];
        ++nodeCount;
        totalLength += node->getLength();
        depthSum += node->getLength() * node->getDepth();
    }

    //If the total length is zero, that means all nodes have a length of zero.
    //In this case, just return the average node depth.
    if (totalLength == 0)
    {
        long double depthSum = 0.0;
        for (size_t i = 0; i < nodes.size(); ++i)
            depthSum += nodes[i]->getDepth();
        return depthSum / nodes.size();
    }

    return depthSum / totalLength;
}


double AssemblyGraph::getMeanDepth(QList<DeBruijnNode *> nodes)
{
    int nodeCount = 0;
    long double depthSum = 0.0;
    long long totalLength = 0;

    for (int i = 0; i < nodes.size(); ++i)
    {
        DeBruijnNode * node = nodes[i];
        ++nodeCount;
        totalLength += node->getLength();
        depthSum += node->getLength() * node->getDepth();
    }

    if (totalLength == 0)
        return 0.0;
    else
        return depthSum / totalLength;
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

    resetAllNodeColours();
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
    std::vector<double> nodeDepths;

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

        nodeDepths.push_back(i.value()->getDepth());
    }

    //Count up the edges that will be shown in single mode (i.e. positive
    //edges).
    int edgeCount = 0;
    QMapIterator<QPair<DeBruijnNode*, DeBruijnNode*>, DeBruijnEdge*> j(m_deBruijnGraphEdges);
    while (j.hasNext())
    {
        j.next();
        DeBruijnEdge * edge = j.value();
        if (edge->isPositiveEdge())
            ++edgeCount;
    }

    m_nodeCount = nodeCount;
    m_edgeCount = edgeCount;
    m_totalLength = totalLength;
    m_meanDepth = getMeanDepth();
    m_pathCount = m_deBruijnGraphPaths.size();

    std::sort(nodeDepths.begin(), nodeDepths.end());

    double firstQuartileIndex = (nodeDepths.size() - 1) / 4.0;
    double medianIndex = (nodeDepths.size() - 1) / 2.0;
    double thirdQuartileIndex = (nodeDepths.size() - 1) * 3.0 / 4.0;

    m_firstQuartileDepth = getValueUsingFractionalIndex(&nodeDepths, firstQuartileIndex);
    m_medianDepth = getValueUsingFractionalIndex(&nodeDepths, medianIndex);
    m_thirdQuartileDepth = getValueUsingFractionalIndex(&nodeDepths, thirdQuartileIndex);

    //Set the auto node length setting. This is determined by aiming for a
    //target average node length. But if the graph is small, the value will be
    //increased (to avoid having an overly small and simple graph layout).
    double targetDrawnGraphLength = std::max(m_nodeCount * g_settings->meanNodeLength,
                                             g_settings->minTotalGraphLength);
    double megabases = totalLength / 1000000.0;
    if (megabases > 0.0)
        g_settings->autoNodeLengthPerMegabase = targetDrawnGraphLength / megabases;
    else
        g_settings->autoNodeLengthPerMegabase = 10000.0;
}

template<typename T> double AssemblyGraph::getValueUsingFractionalIndex(std::vector<T> * v, double index) const
{
    if (v->size() == 0)
        return 0.0;
    if (v->size() == 1)
        return double((*v)[0]);

    int wholePart = floor(index);

    if (wholePart < 0)
        return double((*v)[0]);
    if (wholePart >= int(v->size()) - 1)
        return double((*v)[v->size() - 1]);

    double fractionalPart = index - wholePart;

    double piece1 = double((*v)[wholePart]);
    double piece2 = double((*v)[wholePart+1]);

    return piece1 * (1.0 - fractionalPart) + piece2 * fractionalPart;
}



void AssemblyGraph::clearGraphInfo()
{
    m_totalLength = 0;
    m_shortestContig = 0;
    m_longestContig = 0;

    m_meanDepth = 0.0;
    m_firstQuartileDepth = 0.0;
    m_medianDepth = 0.0;
    m_thirdQuartileDepth = 0.0;
}






void AssemblyGraph::buildDeBruijnGraphFromLastGraph(QString fullFileName)
{
    m_graphFileType = LAST_GRAPH;
    m_filename = fullFileName;
    m_depthTag = "KC";

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

                double nodeDepth;
                if (nodeLength > 0)
                    nodeDepth = double(nodeDetails.at(3).toInt()) / nodeLength; //IS THIS COLUMN ($COV_SHORT1) THE BEST ONE TO USE?
                else
                    nodeDepth = double(nodeDetails.at(3).toInt());

                QByteArray sequence = in.readLine().toLocal8Bit();
                QByteArray revCompSequence = in.readLine().toLocal8Bit();

                DeBruijnNode * node = new DeBruijnNode(posNodeName, nodeDepth, sequence);
                DeBruijnNode * reverseComplementNode = new DeBruijnNode(negNodeName, nodeDepth, revCompSequence);
                node->setReverseComplement(reverseComplementNode);
                reverseComplementNode->setReverseComplement(node);
                m_deBruijnGraphNodes.insert(posNodeName, node);
                m_deBruijnGraphNodes.insert(negNodeName, reverseComplementNode);
            }

            //ARC lines contain edges.
            else if (line.startsWith("ARC"))
            {
                QStringList arcDetails = line.split(QRegExp("\\s+"));

                if (arcDetails.size() < 3)
                    throw "load error";

                QString node1Name = convertNormalNumberStringToBandageNodeName(arcDetails.at(1));
                QString node2Name = convertNormalNumberStringToBandageNodeName(arcDetails.at(2));

                createDeBruijnEdge(node1Name, node2Name);
            }

            //NR lines occur after ARC lines, so we can quit looking when we see one.
            else if (line.startsWith("NR"))
                break;
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



//This function loads a graph from a GFA file.  It reports whether or not it
//encountered an unsupported CIGAR string, whether the GFA has custom labels
//and whether it has custom colours.
void AssemblyGraph::buildDeBruijnGraphFromGfa(QString fullFileName, bool *unsupportedCigar,
                                              bool *customLabels, bool *customColours, QString *bandageOptionsError)
{
    m_graphFileType = GFA;
    m_filename = fullFileName;

    *unsupportedCigar = false;
    *customLabels = false;
    *customColours = false;
    *bandageOptionsError = "";

    QFile inputFile(fullFileName);
    if (inputFile.open(QIODevice::ReadOnly)) {
        std::vector<QString> edgeStartingNodeNames;
        std::vector<QString> edgeEndingNodeNames;
        std::vector<int> edgeOverlaps;

        QMap<QString, QColor> colours;
        QMap<QString, QString> labels;
        QMap<QString, QString> paths;

        QTextStream in(&inputFile);
        while (!in.atEnd()) {
            QApplication::processEvents();
            QString line = in.readLine();

            QStringList lineParts = line.split(QRegExp("\t"));

            if (lineParts.size() < 1)
                continue;

            //Lines beginning with "H" are header lines.
            if (lineParts.at(0) == "H") {

                // Check for a tag containing Bandage options.
                for (int i = 1; i < lineParts.size(); ++i) {
                    QString part = lineParts.at(i);
                    if (part.size() < 6)
                        continue;
                    if (part.left(5) != "bn:Z:")
                        continue;
                    QString bandageOptionsString = part.right(part.length() - 5);
                    QStringList bandageOptions = bandageOptionsString.split(' ', QString::SkipEmptyParts);
                    QStringList bandageOptionsCopy = bandageOptions;
                    *bandageOptionsError = checkForInvalidOrExcessSettings(&bandageOptionsCopy);
                    if (bandageOptionsError->length() == 0)
                        parseSettings(bandageOptions);
                }
            }

            //Lines beginning with "S" are sequence (node) lines.
            if (lineParts.at(0) == "S") {
                if (lineParts.size() < 3)
                    throw "load error";

                QString nodeName = lineParts.at(1);
                if (nodeName.isEmpty())
                    nodeName = getUniqueNodeName("node");
                if (m_deBruijnGraphNodes.contains(nodeName + "+"))
                    throw "load error";

                QByteArray sequence = lineParts.at(2).toLocal8Bit();

                //Get the tags.
                bool kcFound = false, rcFound = false, fcFound = false, dpFound = false;
                double kc = 0.0, rc = 0.0, fc = 0.0, dp = 0.0;
                int ln = 0;
                QString lb, l2;
                QColor cl, c2;
                for (int i = 3; i < lineParts.size(); ++i) {
                    QString part = lineParts.at(i);
                    if (part.size() < 6)
                        continue;
                    if (part.at(2) != ':')
                        continue;
                    QString tag = part.left(2).toUpper();
                    QString valString = part.right(part.length() - 5);
                    if (tag == "KC") {
                        kcFound = true;
                        kc = valString.toDouble();
                    }
                    if (tag == "RC") {
                        rcFound = true;
                        rc = valString.toDouble();
                    }
                    if (tag == "FC") {
                        fcFound = true;
                        fc = valString.toDouble();
                    }
                    if (tag == "DP") {
                        dpFound = true;
                        dp = valString.toDouble();
                    }
                    if (tag == "LN")
                        ln = valString.toInt();
                    if (tag == "LB")
                        lb = valString;
                    if (tag == "CL")
                        cl = QColor(valString);
                    if (tag == "L2")
                        l2 = valString;
                    if (tag == "C2")
                        c2 = QColor(valString);
                }

                //GFA can use * to indicate that the sequence is not in the
                //file.  In this case, try to use the LN tag for length.  If
                //that's not available, use a length of 0.
                //If there is a sequence, then the LN tag will be ignored.
                int length;
                if (sequence == "*" || sequence == "") {
                    length = ln;
                    sequence = "";
                }
                else
                    length = sequence.length();

                //If there is an attribute holding the depth, we'll use that.
                //If there isn't, then we'll use 1.0.
                //We try to load 'DP' (depth), 'KC' (k-mer count), 'RC'
                //(read count) or 'FC'(fragment count) in that order of
                //preference.
                //If we use KC, RC or FC for the depth, then that is really a
                //count, so we need to divide by the sequence length to get the
                //depth.
                //We also remember which tag was used so if the graph is saved
                //we can use the same tag in the output.
                double nodeDepth = 1.0;
                if (dpFound) {
                    m_depthTag = "DP";
                    nodeDepth = dp;
                }
                else if (kcFound) {
                    m_depthTag = "KC";
                    if (length > 0)
                        nodeDepth = kc / length;
                }
                else if (rcFound) {
                    m_depthTag = "RC";
                    if (length > 0)
                        nodeDepth = rc / length;
                }
                else if (fcFound) {
                    m_depthTag = "FC";
                    if (length > 0)
                        nodeDepth = fc / length;
                }

                //We check to see if the node ended in a "+" or "-".
                //If so, we assume that is giving the orientation and leave it.
                //And if it doesn't end in a "+" or "-", we assume "+" and add
                //that to the node name.
                QString lastChar = nodeName.right(1);
                if (lastChar != "+" && lastChar != "-")
                    nodeName += "+";

                // Canu nodes start with "tig" which we can remove for simplicity.
                nodeName = simplifyCanuNodeName(nodeName);

                //Save custom colours and labels to be applied later, after
                //reverse complement nodes are built.
                if (cl.isValid()) {
                    *customColours = true;
                    colours.insert(nodeName, cl);
                }
                if (c2.isValid()) {
                    *customColours = true;
                    colours.insert(getOppositeNodeName(nodeName), c2);
                }
                if (!lb.isEmpty()) {
                    *customLabels = true;
                    labels.insert(nodeName, lb);
                }
                if (!l2.isEmpty()) {
                    *customLabels = true;
                    labels.insert(getOppositeNodeName(nodeName), l2);
                }

                DeBruijnNode * node = new DeBruijnNode(nodeName, nodeDepth, sequence, length);
                m_deBruijnGraphNodes.insert(nodeName, node);
            }

            //Lines beginning with "L" are link (edge) lines
            else if (lineParts.at(0) == "L") {
                //Edges aren't made now, in case their sequence hasn't yet been specified.
                //Instead, we save the starting and ending nodes and make the edges after
                //we're done looking at the file.

                if (lineParts.size() < 6)
                    throw "load error";

                //Parts 1 and 3 hold the node names and parts 2 and 4 hold the corresponding +/-.
                QString startingNode = lineParts.at(1) + lineParts.at(2);
                QString endingNode = lineParts.at(3) + lineParts.at(4);
                startingNode = simplifyCanuNodeName(startingNode);
                endingNode = simplifyCanuNodeName(endingNode);
                edgeStartingNodeNames.push_back(startingNode);
                edgeEndingNodeNames.push_back(endingNode);

                //Part 5 holds the node overlap cigar string.  A "*" means unspecified, so
                //we 0 for that.
                QString cigar = lineParts.at(5);
                if (cigar == "*")
                    edgeOverlaps.push_back(0);
                if (cigarContainsOnlyM(cigar))
                    edgeOverlaps.push_back(getLengthFromSimpleCigar(cigar));
                else {
                    edgeOverlaps.push_back(getLengthFromCigar(cigar));
                    *unsupportedCigar = true;
                }
            }

            // Load paths
            else if (lineParts.at(0) == "P") {
                if (lineParts.size() < 4)
                    continue;

                paths.insert(lineParts.at(1), lineParts.at(2));
            }
        }

        //Pair up reverse complements, creating them if necessary.
        QMapIterator<QString, DeBruijnNode*> i(m_deBruijnGraphNodes);
        while (i.hasNext()) {
            i.next();
            DeBruijnNode * node = i.value();
            makeReverseComplementNodeIfNecessary(node);
        }
        pointEachNodeToItsReverseComplement();

        //Add any custom colours or labels that were loaded.
        QMapIterator<QString, QColor> j(colours);
        while (j.hasNext()) {
            j.next();
            QString nodeName = j.key();
            if (m_deBruijnGraphNodes.contains(nodeName))
                m_deBruijnGraphNodes[nodeName]->setCustomColour(j.value());
        }
        QMapIterator<QString, QString> k(labels);
        while (k.hasNext()) {
            k.next();
            QString nodeName = k.key();
            if (m_deBruijnGraphNodes.contains(nodeName))
                m_deBruijnGraphNodes[nodeName]->setCustomLabel(k.value());
        }

        //Create all of the edges.
        for (size_t i = 0; i < edgeStartingNodeNames.size(); ++i) {
            QString node1Name = edgeStartingNodeNames[i];
            QString node2Name = edgeEndingNodeNames[i];
            int overlap = edgeOverlaps[i];
            createDeBruijnEdge(node1Name, node2Name, overlap, EXACT_OVERLAP);
        }

        // Create all the paths.
        QMapIterator<QString, QString> p(paths);
        while (p.hasNext()) {
            p.next();
            QString pathName = p.key();

            QString pathStringFailure;
            Path pp = Path::makeFromString(p.value(), false, &pathStringFailure);

            if (pp.isEmpty()) {
                std::cout << pathStringFailure.toUtf8().constData() << std::endl;
            } else {
                m_deBruijnGraphPaths.insert(pathName, new Path(pp));
            }
        }
    }

    if (m_deBruijnGraphNodes.size() == 0)
        throw "load error";

    // For Canu graphs, if there is a file called *.layout.readToTig, then we
    // can use that to get better read depth values.
    QFileInfo gfaFileInfo(m_filename);
    QString baseName = gfaFileInfo.completeBaseName();
    QString readToTigFilename = gfaFileInfo.dir().filePath(baseName + ".layout.readToTig");
    QFileInfo readToTigFileInfo(readToTigFilename);
    if (readToTigFileInfo.exists()) {
        QFile readToTigFile(readToTigFilename);
        if (readToTigFile.open(QIODevice::ReadOnly)) {
            // Keep track of how many bases are put into each node.
            QMap<QString, long long> baseCounts;
            QMapIterator<QString, DeBruijnNode*> i(m_deBruijnGraphNodes);
            while (i.hasNext()) {
                i.next();
                DeBruijnNode * node = i.value();
                if (node->isPositiveNode())
                    baseCounts[node->getNameWithoutSign()] = 0;
            }

            QTextStream in(&readToTigFile);
            while (!in.atEnd()) {
                QApplication::processEvents();
                QString line = in.readLine();
                QStringList lineParts = line.split(QRegExp("\t"));
                if (lineParts.length() >= 5) {
                    bool conversionOkay;
                    long long readStart = lineParts[3].toLongLong(&conversionOkay);
                    if (!conversionOkay)
                        continue;
                    long long readEnd = lineParts[4].toLongLong(&conversionOkay);
                    if (!conversionOkay)
                        continue;
                    long long readLength = (readEnd < readStart) ? (readStart - readEnd) : (readEnd - readStart);
                    QString nodeName = lineParts[1];
                    if (baseCounts.contains(nodeName))
                        baseCounts[nodeName] += readLength;
                }
            }

            // A node's depth is its total bases divided by its length.
            QMapIterator<QString, DeBruijnNode*> j(m_deBruijnGraphNodes);
            while (j.hasNext()) {
                j.next();
                DeBruijnNode * node = j.value();
                if (node->isPositiveNode()) {
                    QString nodeName = node->getNameWithoutSign();
                    double depth;
                    if (node->getLength() > 0)
                        depth = double(baseCounts[nodeName]) / double(node->getLength());
                    else
                        depth = 1.0;
                    node->setDepth(depth);
                    node->getReverseComplement()->setDepth(depth);
                }
            }
        }
    }

    m_sequencesLoadedFromFasta = NOT_TRIED;
}



bool AssemblyGraph::cigarContainsOnlyM(QString cigar)
{
    QRegExp rx("\\d+M");
    return (rx.indexIn(cigar) != -1);
}


//This function assumes that the cigar string is simple: just digits followed
//by "M".
int AssemblyGraph::getLengthFromSimpleCigar(QString cigar)
{
    cigar.chop(1);
    return cigar.toInt();
}

//This function returns the length defined by a cigar string, relative to the
//second sequence in the edge (the CIGAR reference).
//Bandage does not fully support non-M CIGAR strings, so this is fairly crude
//at the moment.
int AssemblyGraph::getLengthFromCigar(QString cigar)
{
    int length = 0;

    length = getCigarCount("M", cigar);
    length += getCigarCount("=", cigar);
    length += getCigarCount("X", cigar);
    length += getCigarCount("I", cigar);
    length -= getCigarCount("D", cigar);
    length -= getCigarCount("N", cigar);
    length += getCigarCount("S", cigar);
    length += getCigarCount("H", cigar);
    length += getCigarCount("P", cigar);

    return length;
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
    m_filename = fullFileName;
    m_depthTag = "KC";

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
            double nodeDepth;

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
                if (m_deBruijnGraphNodes.contains(nodeName))
                    throw "load error";

                QString nodeDepthString = thisNodeDetails.at(5);
                if (negativeNode)
                {
                    //It may be necessary to remove a single quote from the end of the depth
                    if (nodeDepthString.at(nodeDepthString.size() - 1) == '\'')
                        nodeDepthString.chop(1);
                }
                nodeDepth = nodeDepthString.toDouble();

                //Make the node
                node = new DeBruijnNode(nodeName, nodeDepth, ""); //Sequence string is currently empty - will be added to on subsequent lines of the fastg file
                m_deBruijnGraphNodes.insert(nodeName, node);

                //The second part of nodeDetails is a comma-delimited list of edge nodes.
                //Edges aren't made right now (because the other node might not yet exist),
                //so they are saved into vectors and made after all the nodes have been made.
                if (nodeDetails.size() == 1 || nodeDetails.at(1).isEmpty())
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


        //Create all of the edges.
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
        QByteArray nodeSequence;
        if (node->sequenceIsMissing())
            nodeSequence = "*";
        else
            nodeSequence = node->getSequence();
        DeBruijnNode * newNode = new DeBruijnNode(reverseComplementName, node->getDepth(),
                                                  getReverseComplement(nodeSequence),
                                                  node->getLength());
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
    m_filename = fullFileName;
    m_depthTag = "";

    std::vector<QString> names;
    std::vector<QByteArray> sequences;
    readFastaFile(fullFileName, &names, &sequences);

    std::vector<QString> edgeStartingNodeNames;
    std::vector<QString> edgeEndingNodeNames;

    for (size_t i = 0; i < names.size(); ++i)
    {
        QApplication::processEvents();

        QString name = names[i];
        QByteArray sequence = sequences[i];

        //The header can come in a few different formats:
        // TR1|c0_g1_i1 len=280 path=[274:0-228 275:229-279] [-1, 274, 275, -2]
        // TRINITY_DN31_c1_g1_i1 len=301 path=[279:0-300] [-1, 279, -2]
        // GG1|c0_g1_i1 len=302 path=[1:0-301]
        // comp0_c0_seq1 len=286 path=[6:0-285]
        // c0_g1_i1 len=363 path=[119:0-185 43:186-244 43:245-303 43:304-362]

        //The node names will begin with a string that contains everything
        //up to the component number (e.g. "c0"), in the same format as it is
        //in the Trinity.fasta file.  If the node name begins with "TRINITY_DN"
        //or "TRINITY_GG", "TR" or "GG", then that will be trimmed off.

        if (name.length() < 4)
            throw "load error";

        int componentStartIndex = name.indexOf(QRegExp("c\\d+_"));
        int componentEndIndex = name.indexOf("_", componentStartIndex);

        if (componentStartIndex < 0 || componentEndIndex < 0)
            throw "load error";

        QString component = name.left(componentEndIndex);
        if (component.startsWith("TRINITY_DN") || component.startsWith("TRINITY_GG"))
            component = component.remove(0, 10);
        else if (component.startsWith("TR") || component.startsWith("GG"))
            component = component.remove(0, 2);

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

                QByteArray nodeSequence = sequence.mid(nodeRangeStart, nodeLength);
                DeBruijnNode * node = new DeBruijnNode(nodeName, 1.0, nodeSequence);
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



//This function builds a graph from an ASQG file.  Bandage expects edges to
//conform to its expectation: overlaps are only at the ends of sequences and
//always have the same length in each of the two sequences.  It will not load
//edges which fail to meet this expectation.  The function's return value is
//the number of edges which could not be loaded.
int AssemblyGraph::buildDeBruijnGraphFromAsqg(QString fullFileName)
{
    m_graphFileType = ASQG;
    m_filename = fullFileName;
    m_depthTag = "";

    int badEdgeCount = 0;

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

            //Lines beginning with "VT" are sequence (node) lines
            if (lineParts.at(0) == "VT")
            {
                if (lineParts.size() < 3)
                    throw "load error";

                //We treat all nodes in this file as positive nodes and add "+" to the end of their names.
                QString nodeName = lineParts.at(1);
                if (nodeName.isEmpty())
                    nodeName = "node";
                nodeName += "+";

                QByteArray sequence = lineParts.at(2).toLocal8Bit();
                int length = sequence.length();

                //ASQG files don't seem to include depth, so just set this to one for every node.
                double nodeDepth = 1.0;

                DeBruijnNode * node = new DeBruijnNode(nodeName, nodeDepth, sequence, length);
                m_deBruijnGraphNodes.insert(nodeName, node);
            }

            //Lines beginning with "ED" are edge lines
            else if (lineParts.at(0) == "ED")
            {
                //Edges aren't made now, in case their sequence hasn't yet been specified.
                //Instead, we save the starting and ending nodes and make the edges after
                //we're done looking at the file.

                if (lineParts.size() < 2)
                    throw "load error";

                QStringList edgeParts = lineParts[1].split(" ");
                if (edgeParts.size() < 8)
                    throw "load error";

                QString s1Name = edgeParts.at(0);
                QString s2Name = edgeParts.at(1);
                int s1OverlapStart = edgeParts.at(2).toInt();
                int s1OverlapEnd = edgeParts.at(3).toInt();
                int s1Length = edgeParts.at(4).toInt();
                int s2OverlapStart = edgeParts.at(5).toInt();
                int s2OverlapEnd = edgeParts.at(6).toInt();
                int s2Length = edgeParts.at(7).toInt();

                //We want the overlap region of s1 to be at the end of the node sequence.  If it isn't, we use the
                //negative node and flip the overlap coordinates.
                if (s1OverlapEnd == s1Length - 1)
                    s1Name += "+";
                else
                {
                    s1Name += "-";
                    int newOverlapStart = s1Length - s1OverlapEnd - 1;
                    int newOverlapEnd = s1Length - s1OverlapStart - 1;
                    s1OverlapStart = newOverlapStart;
                    s1OverlapEnd = newOverlapEnd;
                }

                //We want the overlap region of s2 to be at the start of the node sequence.  If it isn't, we use the
                //negative node and flip the overlap coordinates.
                if (s2OverlapStart == 0)
                    s2Name += "+";
                else
                {
                    s2Name += "-";
                    int newOverlapStart = s2Length - s2OverlapEnd - 1;
                    int newOverlapEnd = s2Length - s2OverlapStart - 1;
                    s2OverlapStart = newOverlapStart;
                    s2OverlapEnd = newOverlapEnd;
                }

                int s1OverlapLength = s1OverlapEnd - s1OverlapStart + 1;
                int s2OverlapLength = s2OverlapEnd - s2OverlapStart + 1;

                //If the overlap between the two nodes is in agreement and the overlap regions extend to the ends of the
                //nodes, then we will make the edge.
                if (s1OverlapLength == s2OverlapLength && s1OverlapEnd == s1Length - 1 && s2OverlapStart == 0)
                {
                    edgeStartingNodeNames.push_back(s1Name);
                    edgeEndingNodeNames.push_back(s2Name);
                    edgeOverlaps.push_back(s1OverlapLength);
                }
                else
                    ++badEdgeCount;
            }
        }

        //Pair up reverse complements, creating them if necessary.
        QMapIterator<QString, DeBruijnNode*> i(m_deBruijnGraphNodes);
        while (i.hasNext())
        {
            i.next();
            DeBruijnNode * node = i.value();
            makeReverseComplementNodeIfNecessary(node);
        }
        pointEachNodeToItsReverseComplement();

        //Create all of the edges.
        for (size_t i = 0; i < edgeStartingNodeNames.size(); ++i)
        {
            QString node1Name = edgeStartingNodeNames[i];
            QString node2Name = edgeEndingNodeNames[i];
            int overlap = edgeOverlaps[i];
            createDeBruijnEdge(node1Name, node2Name, overlap, EXACT_OVERLAP);
        }
    }

    if (m_deBruijnGraphNodes.size() == 0)
        throw "load error";

    return badEdgeCount;
}


void AssemblyGraph::buildDeBruijnGraphFromPlainFasta(QString fullFileName)
{
    m_graphFileType = PLAIN_FASTA;
    m_filename = fullFileName;
    m_depthTag = "";

    std::vector<QString> names;
    std::vector<QByteArray> sequences;
    readFastaFile(fullFileName, &names, &sequences);

    std::vector<QString> circularNodeNames;
    for (size_t i = 0; i < names.size(); ++i)
    {
        QApplication::processEvents();

        QString name = names[i];
        QString lowerName = name.toLower();
        double depth = 1.0;
        QByteArray sequence = sequences[i];

        // Check to see if the node name matches the Velvet/SPAdes contig format.  If so, we can get the depth and node
        // number.
        QStringList thisNodeDetails = name.split("_");
        if (thisNodeDetails.size() >= 6 && thisNodeDetails[2] == "length" && thisNodeDetails[4] == "cov") {
            name = thisNodeDetails[1];
            depth = thisNodeDetails[5].toDouble();
            m_depthTag = "KC";
        }

        // If it doesn't match, then we will use the sequence name up to the first space.
        else {
            QStringList nameParts = name.split(" ");
            if (nameParts.size() > 0)
                name = nameParts[0];
        }

        name = cleanNodeName(name);
        name = getUniqueNodeName(name) + "+";

        //  Look for "depth=" and "circular=" in the full header and use them if possible.
        if (lowerName.contains("depth=")) {
            QString depthString = lowerName.split("depth=")[1];
            if (depthString.contains("x"))
                depthString = depthString.split("x")[0];
            else
                depthString = depthString.split(" ")[0];
            bool ok;
            double depthFromString = depthString.toFloat(&ok);
            if (ok)
                depth = depthFromString;
        }
        if (lowerName.contains("circular=true"))
            circularNodeNames.push_back(name);

        if (name.length() < 1)
            throw "load error";

        DeBruijnNode * node = new DeBruijnNode(name, depth, sequence);
        m_deBruijnGraphNodes.insert(name, node);
        makeReverseComplementNodeIfNecessary(node);
    }
    pointEachNodeToItsReverseComplement();

    // For any circular nodes, make an edge connecting them to themselves.
    for (auto circularNodeName : circularNodeNames) {
        createDeBruijnEdge(circularNodeName, circularNodeName, 0, EXACT_OVERLAP);
    }
}



//This function adjusts a node name to make sure it is valid for use in Bandage.
QString AssemblyGraph::cleanNodeName(QString name)
{
    //Replace whitespace with underscores
    name = name.replace(QRegExp("\\s"), "_");

    //Remove any commas.
    name = name.replace(",", "");

    //Remove any trailing + or -.
    if (name.endsWith('+') || name.endsWith('-'))
        name.chop(1);

    return name;
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
    if (checkFileIsAsqg(fullFileName))
        return ASQG;
    if (checkFileIsFasta(fullFileName))
        return PLAIN_FASTA;
    return UNKNOWN_FILE_TYPE;
}


//Cursory look to see if file appears to be a LastGraph file.
bool AssemblyGraph::checkFileIsLastGraph(QString fullFileName)
{
    return checkFirstLineOfFile(fullFileName, "^\\d+\\s+\\d+\\s+\\d+\\s+\\d+");
}

//Cursory look to see if file appears to be a FASTG file.
bool AssemblyGraph::checkFileIsFastG(QString fullFileName)
{
    return checkFirstLineOfFile(fullFileName, "^>(NODE|EDGE).*;");
}

//Cursory look to see if file appears to be a FASTA file.
bool AssemblyGraph::checkFileIsFasta(QString fullFileName)
{
    return checkFirstLineOfFile(fullFileName, "^>");
}

//Cursory look to see if file appears to be a GFA file.
bool AssemblyGraph::checkFileIsGfa(QString fullFileName)
{
    return checkFirstLineOfFile(fullFileName, "^[SLH]\t");
}

//Cursory look to see if file appears to be a Trinity.fasta file.
bool AssemblyGraph::checkFileIsTrinityFasta(QString fullFileName)
{
    return checkFirstLineOfFile(fullFileName, "path=\\[");
}

//Cursory look to see if file appears to be an ASQG file.
bool AssemblyGraph::checkFileIsAsqg(QString fullFileName)
{
    return checkFirstLineOfFile(fullFileName, "^HT\t");
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
bool AssemblyGraph::loadCSV(QString filename, QStringList * columns, QString * errormsg, bool * coloursLoaded)
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

    //Check to see if any of the columns holds colour data.
    int colourCol = -1;
    for (int i = 0; i < headers.size(); ++i)
    {
        QString header = headers[i].toLower();
        if (header == "colour" || header == "color")
        {
            colourCol = i;
            *coloursLoaded = true;
            break;
        }
    }

    *columns = headers;
    int columnCount = headers.size();
    QMap<QString, QColor> colourCategories;
    std::vector<QColor> presetColours = getPresetColours();

    while (!in.atEnd())
    {
        QApplication::processEvents();

        QStringList cols = splitCsv(in.readLine(), sep);
        QString nodeName = getNodeNameFromString(cols[0]);

        //Get rid of the node name - no need to save that.
        cols.pop_front();

        //If one of the columns holds colour data, get the colour from that one.
        //Acceptable colour formats: 6-digit hex colour (e.g. #FFB6C1), an 8-digit hex colour (e.g. #7FD2B48C) or a
        //standard colour name (e.g. skyblue).
        //If the colour value is something other than one of these, a colour will be assigned to the value.  That way
        //categorical names can be used and automatically given colours.
        QColor colour;
        if (colourCol != -1 && cols.size() > colourCol)
        {
            QString colourString = cols[colourCol];
            colour = QColor(colourString);
            if (!colour.isValid())
            {
                if (!colourCategories.contains(colourString))
                {
                    int nextColourIndex = colourCategories.size();
                    colourCategories[colourString] = presetColours[nextColourIndex];
                }
                colour = colourCategories[colourString];
            }
        }

        //Get rid of any extra data that doesn't have a header.
        while (cols.size() > columnCount)
            cols.pop_back();

        if (nodeName != "" && m_deBruijnGraphNodes.contains(nodeName))
        {
            m_deBruijnGraphNodes[nodeName]->setCsvData(cols);
            if (colour.isValid())
                m_deBruijnGraphNodes[nodeName]->setCustomColour(colour);
        }
        else
            ++unmatched_nodes;
    }

    if (unmatched_nodes)
        *errormsg = "There were " + QString::number(unmatched_nodes) + " unmatched entries in the CSV.";

    return true;
}


//This function extracts a node name from a string.
//The string may be in this Bandage format:
//        NODE_6+_length_50434_cov_42.3615
//Or in a number of variations of that format.
//If the node name it finds does not end in a '+' or '-', it will add '+'.
QString AssemblyGraph::getNodeNameFromString(QString string)
{
    // First check for the most obvious case, where the string is already a node name.
    if (m_deBruijnGraphNodes.contains(string))
        return string;
    if (m_deBruijnGraphNodes.contains(string + "+"))
        return string + "+";

    QStringList parts = string.split("_");
    if (parts.size() == 0)
        return "";

    if (parts[0] == "NODE")
        parts.pop_front();
    if (parts.size() == 0)
        return "";

    QString nodeName;

    //This checks for the standard Bandage format where the node name does
    //not have any underscores.
    if (parts.size() == 5 && parts[1] == "length")
        nodeName = parts[0];

    //This checks for the simple case of nothing but a node name.
    else if (parts.size() == 1)
        nodeName = parts[0];

    //If the code got here, then it is likely that the node name contains
    //underscores.  Grab everything in the string up until we encounter
    //"length".
    else
    {
        for (int i = 0; i < parts.size(); ++i)
        {
            if (parts[i] == "length")
                break;
            if (nodeName.length() > 0)
                nodeName += "_";
            nodeName += parts[i];
        }
    }

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
        {
            bool unsupportedCigar, customLabels, customColours;
            QString bandageOptionsError;
            buildDeBruijnGraphFromGfa(filename, &unsupportedCigar, &customLabels, &customColours, &bandageOptionsError);
        }
        if (graphFileType == TRINITY)
            buildDeBruijnGraphFromTrinityFasta(filename);
        if (graphFileType == ASQG)
            buildDeBruijnGraphFromAsqg(filename);
        if (graphFileType == PLAIN_FASTA)
            buildDeBruijnGraphFromPlainFasta(filename);
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
    else //The scope is either around specified nodes, around nodes with BLAST hits or a depth range.
    {
        //Distance is only used for around nodes and around blast scopes, not
        //for the depth range scope.
        if (g_settings->graphScope == DEPTH_RANGE)
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

    // If performing a linear layout, we first sort the drawn nodes and add them left-to-right.
    if (g_settings->linearLayout) {
        QList<DeBruijnNode *> sortedDrawnNodes;

        // We first try to sort the nodes numerically.
        QList<QPair<int, DeBruijnNode *>> numericallySortedDrawnNodes;
        QMapIterator<QString, DeBruijnNode*> i(m_deBruijnGraphNodes);
        bool successfulIntConversion = true;
        while (i.hasNext())
        {
            i.next();
            DeBruijnNode * node = i.value();
            if (node->isDrawn() && node->thisOrReverseComplementNotInOgdf()) {
                int nodeInt = node->getNameWithoutSign().toInt(&successfulIntConversion);
                if (!successfulIntConversion)
                    break;
                numericallySortedDrawnNodes.push_back(QPair<int, DeBruijnNode*>(nodeInt, node));
            }
        }
        if (successfulIntConversion) {
            std::sort(numericallySortedDrawnNodes.begin(), numericallySortedDrawnNodes.end(),
                [](const QPair<int, DeBruijnNode *> & a, const QPair<int, DeBruijnNode *> & b) {return a.first < b.first;});
            for (int i = 0; i < numericallySortedDrawnNodes.size(); ++i) {
                sortedDrawnNodes.reserve(numericallySortedDrawnNodes.size());
                sortedDrawnNodes.push_back(numericallySortedDrawnNodes[i].second);
            }
        }

        // If any of the conversions from node name to integer failed, then we instead sort the nodes alphabetically.
        else {
            i = QMapIterator<QString, DeBruijnNode*>(m_deBruijnGraphNodes);
            while (i.hasNext())
            {
                i.next();
                DeBruijnNode * node = i.value();
                if (node->isDrawn())
                sortedDrawnNodes.push_back(node);
            }
            std::sort(sortedDrawnNodes.begin(), sortedDrawnNodes.end(),
                [](DeBruijnNode * a, DeBruijnNode * b) {return QString::localeAwareCompare(a->getNameWithoutSign().toUpper(), b->getNameWithoutSign().toUpper()) < 0;});
        }

        // Now we add the drawn nodes to the OGDF graph, given them initial positions based on their sort order.
        QSet<QPair<long long, long long> > usedStartPositions;
        double lastXPos = 0.0;
        for (int i = 0; i < sortedDrawnNodes.size(); ++i) {
            DeBruijnNode * node = sortedDrawnNodes[i];
            if (node->thisOrReverseComplementInOgdf())
                continue;
            std::vector<DeBruijnNode *> upstreamNodes = node->getUpstreamNodes();
            for (size_t j = 0; j < upstreamNodes.size(); ++j) {
                DeBruijnNode * upstreamNode = upstreamNodes[j];
                if (!upstreamNode->inOgdf())
                    continue;
                ogdf::node upstreamEnd = upstreamNode->getOgdfNode()->getLast();
                double upstreamEndPos = m_graphAttributes->x(upstreamEnd);
                if (j == 0)
                    lastXPos = upstreamEndPos;
                else
                    lastXPos = std::max(lastXPos, upstreamEndPos);
            }
            double xPos = lastXPos + g_settings->edgeLength;
            double yPos = 0.0;
            long long intXPos = (long long)(xPos * 100.0);
            long long intYPos = (long long)(yPos * 100.0);
            while (usedStartPositions.contains(QPair<long long, long long>(intXPos, intYPos))) {
                yPos += g_settings->edgeLength;
                intYPos = (long long)(yPos * 100.0);
            }
            node->addToOgdfGraph(m_ogdfGraph, m_graphAttributes, m_edgeArray, xPos, yPos);
            usedStartPositions.insert(QPair<long long, long long>(intXPos, intYPos));
            lastXPos = m_graphAttributes->x(node->getOgdfNode()->getLast());
        }
    }

    // If the layout isn't linear, then we don't worry about the initial positions because they'll be randomised anyway.
    else {
        QMapIterator<QString, DeBruijnNode*> i(m_deBruijnGraphNodes);
        while (i.hasNext())
        {
            i.next();
            DeBruijnNode * node = i.value();
            if (node->isDrawn() && node->thisOrReverseComplementNotInOgdf())
                node->addToOgdfGraph(m_ogdfGraph, m_graphAttributes, m_edgeArray, 0.0, 0.0);
        }
    }

    //Then loop through each edge determining its drawn status and adding it to OGDF if it is drawn.
    QMapIterator<QPair<DeBruijnNode*, DeBruijnNode*>, DeBruijnEdge*> j(m_deBruijnGraphEdges);
    while (j.hasNext())
    {
        j.next();
        DeBruijnEdge * edge = j.value();
        edge->determineIfDrawn();
        if (edge->isDrawn())
            edge->addToOgdfGraph(m_ogdfGraph, m_edgeArray);
    }
}



void AssemblyGraph::addGraphicsItemsToScene(MyGraphicsScene * scene)
{
    scene->clear();

    double meanDrawnDepth = getMeanDepth(true);

    //First make the GraphicsItemNode objects
    QMapIterator<QString, DeBruijnNode*> i(m_deBruijnGraphNodes);
    while (i.hasNext())
    {
        i.next();
        DeBruijnNode * node = i.value();

        if (node->isDrawn())
        {
            if (meanDrawnDepth == 0)
                node->setDepthRelativeToMeanDrawnDepth(1.0);
            else
                node->setDepthRelativeToMeanDrawnDepth(node->getDepth() / meanDrawnDepth);
            GraphicsItemNode * graphicsItemNode = new GraphicsItemNode(node, m_graphAttributes);
            node->setGraphicsItemNode(graphicsItemNode);
            graphicsItemNode->setFlag(QGraphicsItem::ItemIsSelectable);
            graphicsItemNode->setFlag(QGraphicsItem::ItemIsMovable);
        }
    }

    resetAllNodeColours();

    //Then make the GraphicsItemEdge objects and add them to the scene first
    //so they are drawn underneath
    QMapIterator<QPair<DeBruijnNode*, DeBruijnNode*>, DeBruijnEdge*> j(m_deBruijnGraphEdges);
    while (j.hasNext())
    {
        j.next();
        DeBruijnEdge * edge = j.value();

        if (edge->isDrawn())
        {
            GraphicsItemEdge * graphicsItemEdge = new GraphicsItemEdge(edge);
            edge->setGraphicsItemEdge(graphicsItemEdge);
            graphicsItemEdge->setFlag(QGraphicsItem::ItemIsSelectable);
            scene->addItem(graphicsItemEdge);
        }
    }

    //Now add the GraphicsItemNode objects to the scene so they are drawn
    //on top
    QMapIterator<QString, DeBruijnNode*> k(m_deBruijnGraphNodes);
    while (k.hasNext())
    {
        k.next();
        DeBruijnNode * node = k.value();
        if (node->hasGraphicsItem())
            scene->addItem(node->getGraphicsItemNode());
    }
}




std::vector<DeBruijnNode *> AssemblyGraph::getStartingNodes(QString * errorTitle, QString * errorMessage, bool doubleMode,
                                                            QString nodesList, QString blastQueryName, QString pathName)
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

    else if (g_settings->graphScope == DEPTH_RANGE)
    {
        if (g_settings->minDepthRange > g_settings->maxDepthRange)
        {
            *errorTitle = "Invalid depth range";
            *errorMessage = "The maximum depth must be greater than or equal to the minimum depth.";
            return startingNodes;
        }

        std::vector<DeBruijnNode *> startingNodes = getNodesInDepthRange(g_settings->minDepthRange,
                                                                             g_settings->maxDepthRange);

        if (startingNodes.size() == 0)
        {
            *errorTitle = "No nodes in range";
            *errorMessage = "There are no nodes with depths in the specified range.";
            return startingNodes;
        }
    }

    else if (g_settings->graphScope == AROUND_PATHS)
    {
        if (m_deBruijnGraphPaths.count(pathName) == 0)
        {
            *errorTitle = "Invalid path";
            *errorMessage = "No path with such name is loaded";
            return startingNodes;
        }
    }

    g_settings->doubleMode = doubleMode;
    clearOgdfGraphAndResetNodes();

    if (g_settings->graphScope == AROUND_NODE)
        startingNodes = getNodesFromString(nodesList, g_settings->startingNodesExactMatch);
    else if (g_settings->graphScope == AROUND_BLAST_HITS)
        startingNodes = getNodesFromBlastHits(blastQueryName);
    else if (g_settings->graphScope == DEPTH_RANGE)
        startingNodes = getNodesInDepthRange(g_settings->minDepthRange,
                                                 g_settings->maxDepthRange);
    else if (g_settings->graphScope == AROUND_PATHS) {
        QList<DeBruijnNode *> nodes = m_deBruijnGraphPaths[pathName]->getNodes();

        for (QList<DeBruijnNode *>::iterator i = nodes.begin(); i != nodes.end(); ++i)
            startingNodes.push_back(*i);
    }

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

std::vector<DeBruijnNode *> AssemblyGraph::getNodesInDepthRange(double min,
                                                                    double max)
{
    std::vector<DeBruijnNode *> returnVector;

    QMapIterator<QString, DeBruijnNode*> i(m_deBruijnGraphNodes);
    while (i.hasNext())
    {
        i.next();
        DeBruijnNode * node = i.value();

        if (node->isInDepthRange(min, max))
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
    GraphLayoutWorker * graphLayoutWorker = new GraphLayoutWorker(&fmmm, m_graphAttributes, m_edgeArray,
                                                                  g_settings->graphLayoutQuality,
                                                                  useLinearLayout(),
                                                                  g_settings->componentSeparation);
    graphLayoutWorker->layoutGraph();
}


void AssemblyGraph::setAllEdgesExactOverlap(int overlap)
{
    QMapIterator<QPair<DeBruijnNode*, DeBruijnNode*>, DeBruijnEdge*> i(m_deBruijnGraphEdges);
    while (i.hasNext())
    {
        i.next();
        i.value()->setExactOverlap(overlap);
    }
}



void AssemblyGraph::autoDetermineAllEdgesExactOverlap()
{
    int edgeCount = int(m_deBruijnGraphEdges.size());
    if (edgeCount == 0)
        return;

    //Determine the overlap for each edge.
    QMapIterator<QPair<DeBruijnNode*, DeBruijnNode*>, DeBruijnEdge*> i(m_deBruijnGraphEdges);
    while (i.hasNext())
    {
        i.next();
        i.value()->autoDetermineExactOverlap();
    }

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
    QMapIterator<QPair<DeBruijnNode*, DeBruijnNode*>, DeBruijnEdge*> j(m_deBruijnGraphEdges);
    while (j.hasNext())
    {
        j.next();
        DeBruijnEdge * edge = j.value();
        for (size_t k = 0; k < sortedOverlaps.size(); ++k)
        {
            if (edge->getOverlap() == sortedOverlaps[k])
                break;
            else if (edge->testExactOverlap(sortedOverlaps[k]))
            {
                edge->setOverlap(sortedOverlaps[k]);
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

    QMapIterator<QPair<DeBruijnNode*, DeBruijnNode*>, DeBruijnEdge*> i(m_deBruijnGraphEdges);
    while (i.hasNext())
    {
        i.next();
        int overlap = i.value()->getOverlap();

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


void AssemblyGraph::readFastaOrFastqFile(QString filename, std::vector<QString> * names,
                                         std::vector<QByteArray> * sequences) {
    QChar firstChar = 0;
    QFile inputFile(filename);
    if (inputFile.open(QIODevice::ReadOnly)) {
        QTextStream in(&inputFile);
        QString firstLine = in.readLine();
        firstChar = firstLine.at(0);
        inputFile.close();
    }
    if (firstChar == '>')
        readFastaFile(filename, names, sequences);
    else if (firstChar == '@')
        readFastqFile(filename, names, sequences);
}



void AssemblyGraph::readFastaFile(QString filename, std::vector<QString> * names, std::vector<QByteArray> * sequences)
{
    QFile inputFile(filename);
    if (inputFile.open(QIODevice::ReadOnly))
    {
        QString name = "";
        QByteArray sequence = "";

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


void AssemblyGraph::readFastqFile(QString filename, std::vector<QString> * names, std::vector<QByteArray> * sequences)
{
    QFile inputFile(filename);
    if (inputFile.open(QIODevice::ReadOnly))
    {
        QTextStream in(&inputFile);
        while (!in.atEnd())
        {
            QApplication::processEvents();

            QString name = in.readLine().simplified();
            QByteArray sequence = in.readLine().simplified().toLocal8Bit();
            in.readLine();  // separator
            in.readLine();  // qualities

            if (name.length() == 0)
                continue;
            if (sequence.length() == 0)
                continue;
            if (name.at(0) != '@')
                continue;
            name.remove(0, 1); //Remove '@' from start
            names->push_back(name);
            sequences->push_back(sequence);
        }
        inputFile.close();
    }
}


void AssemblyGraph::recalculateAllDepthsRelativeToDrawnMean()
{
    double meanDrawnDepth = getMeanDepth(true);
    QMapIterator<QString, DeBruijnNode*> k(m_deBruijnGraphNodes);
    while (k.hasNext())
    {
        k.next();
        DeBruijnNode * node = k.value();

        double depthRelativeToMeanDrawnDepth;
        if (meanDrawnDepth == 0)
            depthRelativeToMeanDrawnDepth = 1.0;
        else
            depthRelativeToMeanDrawnDepth = node->getDepth() / meanDrawnDepth;

        node->setDepthRelativeToMeanDrawnDepth(depthRelativeToMeanDrawnDepth);
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
        DeBruijnNode * startingNode = edge->getStartingNode();
        DeBruijnNode * endingNode = edge->getEndingNode();

        m_deBruijnGraphEdges.remove(QPair<DeBruijnNode*, DeBruijnNode*>(startingNode, endingNode));
        startingNode->removeEdge(edge);
        endingNode->removeEdge(edge);

        delete edge;
    }
}



//This function assumes it is receiving a positive node.  It will duplicate both
//the positive and negative node in the pair.  It divided their depth in
//two, giving half to each node.
void AssemblyGraph::duplicateNodePair(DeBruijnNode * node, MyGraphicsScene * scene)
{
    DeBruijnNode * originalPosNode = node;
    DeBruijnNode * originalNegNode = node->getReverseComplement();

    QString newNodeBaseName = getNewNodeName(originalPosNode->getName());
    QString newPosNodeName = newNodeBaseName + "+";
    QString newNegNodeName = newNodeBaseName + "-";

    double newDepth = node->getDepth() / 2.0;

    //Create the new nodes.
    DeBruijnNode * newPosNode = new DeBruijnNode(newPosNodeName, newDepth, originalPosNode->getSequence());
    DeBruijnNode * newNegNode = new DeBruijnNode(newNegNodeName, newDepth, originalNegNode->getSequence());
    newPosNode->setReverseComplement(newNegNode);
    newNegNode->setReverseComplement(newPosNode);

    //Copy over additional stuff from the original nodes.
    newPosNode->setCustomColour(originalPosNode->getCustomColour());
    newNegNode->setCustomColour(originalNegNode->getCustomColour());
    newPosNode->setCustomLabel(originalPosNode->getCustomLabel());
    newNegNode->setCustomLabel(originalNegNode->getCustomLabel());
    newPosNode->setCsvData(originalPosNode->getAllCsvData());
    newNegNode->setCsvData(originalNegNode->getAllCsvData());

    m_deBruijnGraphNodes.insert(newPosNodeName, newPosNode);
    m_deBruijnGraphNodes.insert(newNegNodeName, newNegNode);

    std::vector<DeBruijnEdge *> leavingEdges = originalPosNode->getLeavingEdges();
    for (size_t i = 0; i < leavingEdges.size(); ++i)
    {
        DeBruijnEdge * edge = leavingEdges[i];
        DeBruijnNode * downstreamNode = edge->getEndingNode();
        createDeBruijnEdge(newPosNodeName, downstreamNode->getName(),
                           edge->getOverlap(), edge->getOverlapType());
    }

    std::vector<DeBruijnEdge *> enteringEdges = originalPosNode->getEnteringEdges();
    for (size_t i = 0; i < enteringEdges.size(); ++i)
    {
        DeBruijnEdge * edge = enteringEdges[i];
        DeBruijnNode * upstreamNode = edge->getStartingNode();
        createDeBruijnEdge(upstreamNode->getName(), newPosNodeName,
                           edge->getOverlap(), edge->getOverlapType());
    }

    originalPosNode->setDepth(newDepth);
    originalNegNode->setDepth(newDepth);

    double meanDrawnDepth = getMeanDepth(true);
    double depthRelativeToMeanDrawnDepth;
    if (meanDrawnDepth == 0)
        depthRelativeToMeanDrawnDepth = 1.0;
    else
        depthRelativeToMeanDrawnDepth = originalPosNode->getDepth() / meanDrawnDepth;

    originalPosNode->setDepthRelativeToMeanDrawnDepth(depthRelativeToMeanDrawnDepth);
    originalNegNode->setDepthRelativeToMeanDrawnDepth(depthRelativeToMeanDrawnDepth);
    newPosNode->setDepthRelativeToMeanDrawnDepth(depthRelativeToMeanDrawnDepth);
    newPosNode->setDepthRelativeToMeanDrawnDepth(depthRelativeToMeanDrawnDepth);

    duplicateGraphicsNode(originalPosNode, newPosNode, scene);
    duplicateGraphicsNode(originalNegNode, newNegNode, scene);
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

    GraphicsItemNode * newGraphicsItemNode = new GraphicsItemNode(newNode, originalGraphicsItemNode);

    newNode->setGraphicsItemNode(newGraphicsItemNode);
    newGraphicsItemNode->setFlag(QGraphicsItem::ItemIsSelectable);
    newGraphicsItemNode->setFlag(QGraphicsItem::ItemIsMovable);

    originalGraphicsItemNode->shiftPointsLeft();
    newGraphicsItemNode->shiftPointsRight();
    originalGraphicsItemNode->fixEdgePaths();

    originalGraphicsItemNode->setNodeColour();
    newGraphicsItemNode->setNodeColour();

    originalGraphicsItemNode->setWidth();

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


//This function will merge the given nodes, if possible.  Nodes can only be
//merged if they are in a simple, unbranching path with no extra edges.  If the
//merge is successful, it returns true, otherwise false.
bool AssemblyGraph::mergeNodes(QList<DeBruijnNode *> nodes, MyGraphicsScene * scene,
                               bool recalulateDepth)
{
    if (nodes.size() == 0)
        return true;

    //We now need to sort the nodes into merge order.
    QList<DeBruijnNode *> orderedList;
    orderedList.push_back(nodes[0]);
    nodes.pop_front();

    bool addedNode;
    do
    {
        addedNode = false;
        for (int i = 0; i < nodes.size(); ++i)
        {
            DeBruijnNode * potentialNextNode = nodes[i];

            //Check if the node can be added to the end of the list.
            if (canAddNodeToEndOfMergeList(&orderedList, potentialNextNode))
            {
                orderedList.push_back(potentialNextNode);
                nodes.removeAt(i);
                addedNode = true;
                break;
            }

            //Check if the node can be added to the front of the list.
            if (canAddNodeToStartOfMergeList(&orderedList, potentialNextNode))
            {
                orderedList.push_front(potentialNextNode);
                nodes.removeAt(i);
                addedNode = true;
                break;
            }

            //If neither of those worked, then we should try the node's reverse
            //complement.
            DeBruijnNode * potentialNextNodeRevComp = potentialNextNode->getReverseComplement();
            if (canAddNodeToEndOfMergeList(&orderedList, potentialNextNodeRevComp))
            {
                orderedList.push_back(potentialNextNodeRevComp);
                nodes.removeAt(i);
                addedNode = true;
                break;
            }
            if (canAddNodeToStartOfMergeList(&orderedList, potentialNextNodeRevComp))
            {
                orderedList.push_front(potentialNextNodeRevComp);
                nodes.removeAt(i);
                addedNode = true;
                break;
            }
        }

        if (nodes.size() == 0)
            break;

    } while (addedNode);

    //If there are still nodes left in the first list, then they don't form a
    //nice simple path and the merge won't work.
    if (nodes.size() > 0)
        return false;

    double mergedNodeDepth = getMeanDepth(orderedList);

    Path posPath = Path::makeFromOrderedNodes(orderedList, false);
    QByteArray mergedNodePosSequence = posPath.getPathSequence();

    QList<DeBruijnNode *> revCompOrderedList;
    for (int i = 0; i < orderedList.size(); ++i)
        revCompOrderedList.push_front(orderedList[i]->getReverseComplement());

    Path negPath = Path::makeFromOrderedNodes(revCompOrderedList, false);
    QByteArray mergedNodeNegSequence = negPath.getPathSequence();

    QString newNodeBaseName;
    for (int i = 0; i < orderedList.size(); ++i)
    {
        newNodeBaseName += orderedList[i]->getNameWithoutSign();
        if (i < orderedList.size() - 1)
            newNodeBaseName += "_";
    }
    newNodeBaseName = getUniqueNodeName(newNodeBaseName);
    QString newPosNodeName = newNodeBaseName + "+";
    QString newNegNodeName = newNodeBaseName + "-";

    DeBruijnNode * newPosNode = new DeBruijnNode(newPosNodeName, mergedNodeDepth, mergedNodePosSequence);
    DeBruijnNode * newNegNode = new DeBruijnNode(newNegNodeName, mergedNodeDepth, mergedNodeNegSequence);

    newPosNode->setReverseComplement(newNegNode);
    newNegNode->setReverseComplement(newPosNode);

    m_deBruijnGraphNodes.insert(newPosNodeName, newPosNode);
    m_deBruijnGraphNodes.insert(newNegNodeName, newNegNode);

    std::vector<DeBruijnEdge *> leavingEdges = orderedList.back()->getLeavingEdges();
    for (size_t i = 0; i < leavingEdges.size(); ++i)
    {
        DeBruijnEdge * leavingEdge = leavingEdges[i];
        createDeBruijnEdge(newPosNodeName, leavingEdge->getEndingNode()->getName(), leavingEdge->getOverlap(),
                           leavingEdge->getOverlapType());
    }

    std::vector<DeBruijnEdge *> enteringEdges = orderedList.front()->getEnteringEdges();
    for (size_t i = 0; i < enteringEdges.size(); ++i)
    {
        DeBruijnEdge * enteringEdge = enteringEdges[i];
        createDeBruijnEdge(enteringEdge->getStartingNode()->getName(), newPosNodeName, enteringEdge->getOverlap(),
                           enteringEdge->getOverlapType());
    }

    if (recalulateDepth)
    {
        double meanDrawnDepth = getMeanDepth(true);
        double depthRelativeToMeanDrawnDepth;
        if (meanDrawnDepth == 0)
            depthRelativeToMeanDrawnDepth = 1.0;
        else
            depthRelativeToMeanDrawnDepth = newPosNode->getDepth() / meanDrawnDepth;

        newPosNode->setDepthRelativeToMeanDrawnDepth(depthRelativeToMeanDrawnDepth);
        newNegNode->setDepthRelativeToMeanDrawnDepth(depthRelativeToMeanDrawnDepth);
    }
    else
    {
        newPosNode->setDepthRelativeToMeanDrawnDepth(1.0);
        newNegNode->setDepthRelativeToMeanDrawnDepth(1.0);
    }

    mergeGraphicsNodes(&orderedList, &revCompOrderedList, newPosNode, scene);

    std::vector<DeBruijnNode *> nodesToDelete;
    for (int i = 0; i < orderedList.size(); ++i)
        nodesToDelete.push_back(orderedList[i]);
    deleteNodes(&nodesToDelete);

    return true;
}


bool AssemblyGraph::canAddNodeToStartOfMergeList(QList<DeBruijnNode *> * mergeList,
                                                 DeBruijnNode * potentialNode)
{
    DeBruijnNode * firstNode = mergeList->front();
    std::vector<DeBruijnEdge *> edgesEnteringFirstNode = firstNode->getEnteringEdges();
    std::vector<DeBruijnEdge *> edgesLeavingPotentialNode = potentialNode->getLeavingEdges();
    return (edgesEnteringFirstNode.size() == 1 &&
            edgesLeavingPotentialNode.size() == 1 &&
            edgesEnteringFirstNode[0]->getStartingNode() == potentialNode &&
            edgesLeavingPotentialNode[0]->getEndingNode() == firstNode);
}


bool AssemblyGraph::canAddNodeToEndOfMergeList(QList<DeBruijnNode *> * mergeList,
                                               DeBruijnNode * potentialNode)
{
    DeBruijnNode * lastNode = mergeList->back();
    std::vector<DeBruijnEdge *> edgesLeavingLastNode = lastNode->getLeavingEdges();
    std::vector<DeBruijnEdge *> edgesEnteringPotentialNode = potentialNode->getEnteringEdges();
    return (edgesLeavingLastNode.size() == 1 &&
            edgesEnteringPotentialNode.size() == 1 &&
            edgesLeavingLastNode[0]->getEndingNode() == potentialNode &&
            edgesEnteringPotentialNode[0]->getStartingNode() == lastNode);
}


QString AssemblyGraph::getUniqueNodeName(QString baseName)
{
    //If the base name is untaken, then that's it!
    if (!m_deBruijnGraphNodes.contains(baseName + "+"))
        return baseName;

    int suffix = 1;
    while (true)
    {
        ++suffix;
        QString potentialUniqueName = baseName + "_" + QString::number(suffix);
        if (!m_deBruijnGraphNodes.contains(potentialUniqueName + "+"))
            return potentialUniqueName;
    }

    //Code should never get here.
    return baseName;
}


void AssemblyGraph::mergeGraphicsNodes(QList<DeBruijnNode *> * originalNodes,
                                       QList<DeBruijnNode *> * revCompOriginalNodes,
                                       DeBruijnNode * newNode,
                                       MyGraphicsScene * scene)
{
    bool success = mergeGraphicsNodes2(originalNodes, newNode, scene);
    if (success)
        newNode->setAsDrawn();

    if (g_settings->doubleMode) {
        DeBruijnNode * newRevComp = newNode->getReverseComplement();
        bool revCompSuccess = mergeGraphicsNodes2(revCompOriginalNodes, newRevComp, scene);
        if (revCompSuccess)
            newRevComp->setAsDrawn();
    }

    std::vector<DeBruijnNode *> nodesToRemove;
    for (int i = 0; i < originalNodes->size(); ++i)
        nodesToRemove.push_back((*originalNodes)[i]);
    removeGraphicsItemNodes(&nodesToRemove, true, scene);
}


bool AssemblyGraph::mergeGraphicsNodes2(QList<DeBruijnNode *> * originalNodes,
                                        DeBruijnNode * newNode,
                                        MyGraphicsScene * scene)
{
    bool success = true;
    std::vector<QPointF> linePoints;

    for (int i = 0; i < originalNodes->size(); ++i)
    {
        DeBruijnNode * node = (*originalNodes)[i];

        //If we are in single mode, then we should check for a GraphicsItemNode only
        //in the positive nodes.
        bool opposite = false;
        if (!g_settings->doubleMode && node->isNegativeNode())
        {
            node = node->getReverseComplement();
            opposite = true;
        }

        GraphicsItemNode * originalGraphicsItemNode = node->getGraphicsItemNode();
        if (originalGraphicsItemNode == 0)
        {
            success = false;
            break;
        }

        std::vector<QPointF> originalLinePoints = originalGraphicsItemNode->m_linePoints;

        //Add the original line points to the new line point collection.  If we
        //are working with an opposite node, then we need to reverse the order.
        if (opposite)
        {
            for (size_t j = originalLinePoints.size(); j > 0; --j)
                linePoints.push_back(originalLinePoints[j-1]);
        }
        else
        {
            for (size_t j = 0; j < originalLinePoints.size(); ++j)
                linePoints.push_back(originalLinePoints[j]);
        }
    }

    if (success)
    {
        GraphicsItemNode * newGraphicsItemNode = new GraphicsItemNode(newNode, linePoints);

        newNode->setGraphicsItemNode(newGraphicsItemNode);
        newGraphicsItemNode->setFlag(QGraphicsItem::ItemIsSelectable);
        newGraphicsItemNode->setFlag(QGraphicsItem::ItemIsMovable);

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
    return success;
}



//If reverseComplement is true, this function will also remove the graphics items for reverse complements of the nodes.
void AssemblyGraph::removeGraphicsItemNodes(const std::vector<DeBruijnNode *> * nodes,
                                            bool reverseComplement,
                                            MyGraphicsScene * scene)
{
    QSet<GraphicsItemNode *> graphicsItemNodesToDelete;
    for (size_t i = 0; i < nodes->size(); ++i)
    {
        DeBruijnNode * node = (*nodes)[i];
        removeAllGraphicsEdgesFromNode(node, reverseComplement, scene);

        GraphicsItemNode * graphicsItemNode = node->getGraphicsItemNode();
        if (graphicsItemNode != 0 && !graphicsItemNodesToDelete.contains(graphicsItemNode))
            graphicsItemNodesToDelete.insert(graphicsItemNode);
        node->setGraphicsItemNode(0);

        if (reverseComplement)
        {
            DeBruijnNode * rcNode = node->getReverseComplement();
            GraphicsItemNode * rcGraphicsItemNode = rcNode->getGraphicsItemNode();
            if (rcGraphicsItemNode != 0 && !graphicsItemNodesToDelete.contains(rcGraphicsItemNode))
                graphicsItemNodesToDelete.insert(rcGraphicsItemNode);
            rcNode->setGraphicsItemNode(0);
        }
    }

    if (scene != 0)
        scene->blockSignals(true);
    QSetIterator<GraphicsItemNode *> i(graphicsItemNodesToDelete);
    while (i.hasNext())
    {
        GraphicsItemNode * graphicsItemNode = i.next();
        if (graphicsItemNode != 0)
        {
            if (scene != 0)
                scene->removeItem(graphicsItemNode);
            delete graphicsItemNode;
        }
    }
    if (scene != 0)
        scene->blockSignals(false);
}


void AssemblyGraph::removeAllGraphicsEdgesFromNode(DeBruijnNode * node, bool reverseComplement,
                                                   MyGraphicsScene * scene)
{
    const std::vector<DeBruijnEdge *> * edges = node->getEdgesPointer();
    removeGraphicsItemEdges(edges, reverseComplement, scene);
}

void AssemblyGraph::removeGraphicsItemEdges(const std::vector<DeBruijnEdge *> * edges,
                                            bool reverseComplement,
                                            MyGraphicsScene * scene)
{
    QSet<GraphicsItemEdge *> graphicsItemEdgesToDelete;
    for (size_t i = 0; i < edges->size(); ++i)
    {
        DeBruijnEdge * edge = (*edges)[i];

        GraphicsItemEdge * graphicsItemEdge = edge->getGraphicsItemEdge();
        if (graphicsItemEdge != 0 && !graphicsItemEdgesToDelete.contains(graphicsItemEdge))
            graphicsItemEdgesToDelete.insert(graphicsItemEdge);
        edge->setGraphicsItemEdge(0);

        if (reverseComplement)
        {
            DeBruijnEdge * rcEdge = edge->getReverseComplement();
            GraphicsItemEdge * rcGraphicsItemEdge = rcEdge->getGraphicsItemEdge();
            if (rcGraphicsItemEdge != 0 && !graphicsItemEdgesToDelete.contains(rcGraphicsItemEdge))
                graphicsItemEdgesToDelete.insert(rcGraphicsItemEdge);
            rcEdge->setGraphicsItemEdge(0);
        }
    }

    if (scene != 0)
        scene->blockSignals(true);
    QSetIterator<GraphicsItemEdge *> i(graphicsItemEdgesToDelete);
    while (i.hasNext())
    {
        GraphicsItemEdge * graphicsItemEdge = i.next();
        if (graphicsItemEdge != 0)
        {
            if (scene != 0)
                scene->removeItem(graphicsItemEdge);
            delete graphicsItemEdge;
        }
    }
    if (scene != 0)
        scene->blockSignals(false);
}


//This function simplifies the graph by merging all possible nodes in a simple
//line.  It returns the number of merges that it did.
//It gets a pointer to the progress dialog as well so it can check to see if the
//user has cancelled the merge.
int AssemblyGraph::mergeAllPossible(MyGraphicsScene * scene,
                                    MyProgressDialog * progressDialog)
{
    //Create a set of all nodes.
    QSet<DeBruijnNode *> uncheckedNodes;
    QMapIterator<QString, DeBruijnNode*> i(m_deBruijnGraphNodes);
    while (i.hasNext())
    {
        i.next();
        uncheckedNodes.insert(i.value());
    }

    //Create a list of all merges to be done.
    QList< QList<DeBruijnNode *> > allMerges;
    QMapIterator<QString, DeBruijnNode*> j(m_deBruijnGraphNodes);
    while (j.hasNext())
    {
        j.next();
        DeBruijnNode * node = j.value();

        //If the current node isn't checked, then we will find the longest
        //possible mergable sequence containing this node.
        if (uncheckedNodes.contains(node))
        {
            QList<DeBruijnNode *> nodesToMerge;
            nodesToMerge.push_back(node);

            uncheckedNodes.remove(node);
            uncheckedNodes.remove(node->getReverseComplement());

            //Extend forward as much as possible.
            bool extended;
            do
            {
                extended = false;
                DeBruijnNode * last = nodesToMerge.back();
                std::vector<DeBruijnEdge *> outgoingEdges = last->getLeavingEdges();
                if (outgoingEdges.size() == 1)
                {
                    DeBruijnEdge * potentialEdge = outgoingEdges[0];
                    DeBruijnNode * potentialNode = potentialEdge->getEndingNode();
                    std::vector<DeBruijnEdge *> edgesEnteringPotentialNode = potentialNode->getEnteringEdges();
                    if (edgesEnteringPotentialNode.size() == 1 &&
                            edgesEnteringPotentialNode[0] == potentialEdge &&
                            !nodesToMerge.contains(potentialNode) &&
                            uncheckedNodes.contains(potentialNode))
                    {
                        nodesToMerge.push_back(potentialNode);
                        uncheckedNodes.remove(potentialNode);
                        uncheckedNodes.remove(potentialNode->getReverseComplement());
                        extended = true;
                    }
                }
            } while (extended);

            //Extend backward as much as possible.
            do
            {
                extended = false;
                DeBruijnNode * first = nodesToMerge.front();
                std::vector<DeBruijnEdge *> incomingEdges = first->getEnteringEdges();
                if (incomingEdges.size() == 1)
                {
                    DeBruijnEdge * potentialEdge = incomingEdges[0];
                    DeBruijnNode * potentialNode = potentialEdge->getStartingNode();
                    std::vector<DeBruijnEdge *> edgesLeavingPotentialNode = potentialNode->getLeavingEdges();
                    if (edgesLeavingPotentialNode.size() == 1 &&
                            edgesLeavingPotentialNode[0] == potentialEdge &&
                            !nodesToMerge.contains(potentialNode) &&
                            uncheckedNodes.contains(potentialNode))
                    {
                        nodesToMerge.push_front(potentialNode);
                        uncheckedNodes.remove(potentialNode);
                        uncheckedNodes.remove(potentialNode->getReverseComplement());
                        extended = true;
                    }
                }
            } while (extended);

            if (nodesToMerge.size() > 1)
                allMerges.push_back(nodesToMerge);
        }
    }

    //Now do the actual merges.
    QApplication::processEvents();
    emit setMergeTotalCount(allMerges.size());
    for (int i = 0; i < allMerges.size(); ++i)
    {
        if (progressDialog != 0 && progressDialog->wasCancelled())
            break;

        mergeNodes(allMerges[i], scene, false);
        emit setMergeCompletedCount(i+1);
        QApplication::processEvents();
    }

    recalculateAllDepthsRelativeToDrawnMean();
    recalculateAllNodeWidths();

    return allMerges.size();
}

void AssemblyGraph::saveEntireGraphToFasta(QString filename)
{
    QFile file(filename);
    file.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream out(&file);

    QMapIterator<QString, DeBruijnNode*> i(m_deBruijnGraphNodes);
    while (i.hasNext())
    {
        i.next();
        out << i.value()->getFasta(true);
    }
}

void AssemblyGraph::saveEntireGraphToFastaOnlyPositiveNodes(QString filename)
{
    QFile file(filename);
    file.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream out(&file);

    QMapIterator<QString, DeBruijnNode*> i(m_deBruijnGraphNodes);
    while (i.hasNext())
    {
        i.next();
        DeBruijnNode * node = i.value();
        if (node->isPositiveNode())
            out << node->getFasta(false);
    }
}

bool AssemblyGraph::saveEntireGraphToGfa(QString filename)
{
    QFile file(filename);
    bool success = file.open(QIODevice::WriteOnly | QIODevice::Text);
    if (!success)
        return false;

    QTextStream out(&file);

    QMapIterator<QString, DeBruijnNode*> i(m_deBruijnGraphNodes);
    while (i.hasNext())
    {
        i.next();
        DeBruijnNode * node = i.value();
        if (node->isPositiveNode())
            out << node->getGfaSegmentLine(m_depthTag);
    }

    QList<DeBruijnEdge*> edgesToSave;
    QMapIterator<QPair<DeBruijnNode*, DeBruijnNode*>, DeBruijnEdge*> j(m_deBruijnGraphEdges);
    while (j.hasNext())
    {
        j.next();
        DeBruijnEdge * edge = j.value();
        if (edge->isPositiveEdge())
            edgesToSave.push_back(edge);
    }

    std::sort(edgesToSave.begin(), edgesToSave.end(), DeBruijnEdge::compareEdgePointers);

    for (int i = 0; i < edgesToSave.size(); ++i)
        out << edgesToSave[i]->getGfaLinkLine();

    return true;
}

bool AssemblyGraph::saveVisibleGraphToGfa(QString filename)
{
    QFile file(filename);
    bool success = file.open(QIODevice::WriteOnly | QIODevice::Text);
    if (!success)
        return false;

    QTextStream out(&file);

    QMapIterator<QString, DeBruijnNode*> i(m_deBruijnGraphNodes);
    while (i.hasNext())
    {
        i.next();
        DeBruijnNode * node = i.value();
        if (node->thisNodeOrReverseComplementIsDrawn() && node->isPositiveNode())
            out << node->getGfaSegmentLine(m_depthTag);
    }

    QList<DeBruijnEdge*> edgesToSave;
    QMapIterator<QPair<DeBruijnNode*, DeBruijnNode*>, DeBruijnEdge*> j(m_deBruijnGraphEdges);
    while (j.hasNext())
    {
        j.next();
        DeBruijnEdge * edge = j.value();
        if (edge->getStartingNode()->thisNodeOrReverseComplementIsDrawn() &&
                edge->getEndingNode()->thisNodeOrReverseComplementIsDrawn() &&
                edge->isPositiveEdge())
            edgesToSave.push_back(edge);
    }

    std::sort(edgesToSave.begin(), edgesToSave.end(), DeBruijnEdge::compareEdgePointers);

    for (int i = 0; i < edgesToSave.size(); ++i)
        out << edgesToSave[i]->getGfaLinkLine();

    return true;
}




//This function changes the name of a node pair.  The new and old names are
//both assumed to not include the +/- at the end.
void AssemblyGraph::changeNodeName(QString oldName, QString newName)
{
    if (checkNodeNameValidity(newName) != NODE_NAME_OKAY)
        return;

    QString posOldNodeName = oldName + "+";
    QString negOldNodeName = oldName + "-";

    if (!m_deBruijnGraphNodes.contains(posOldNodeName))
        return;
    if (!m_deBruijnGraphNodes.contains(negOldNodeName))
        return;

    DeBruijnNode * posNode = m_deBruijnGraphNodes[posOldNodeName];
    DeBruijnNode * negNode = m_deBruijnGraphNodes[negOldNodeName];

    m_deBruijnGraphNodes.remove(posOldNodeName);
    m_deBruijnGraphNodes.remove(negOldNodeName);

    QString posNewNodeName = newName + "+";
    QString negNewNodeName = newName + "-";

    posNode->setName(posNewNodeName);
    negNode->setName(negNewNodeName);

    m_deBruijnGraphNodes.insert(posNewNodeName, posNode);
    m_deBruijnGraphNodes.insert(negNewNodeName, negNode);
}



//This function checks whether a new node name is okay.  It takes a node name
//without a +/- at the end.
NodeNameStatus AssemblyGraph::checkNodeNameValidity(QString nodeName)
{
    if (nodeName.contains('\t'))
        return NODE_NAME_CONTAINS_TAB;

    if (nodeName.contains('\n'))
        return NODE_NAME_CONTAINS_NEWLINE;

    if (nodeName.contains(','))
        return NODE_NAME_CONTAINS_COMMA;

    if (nodeName.contains(' '))
        return NODE_NAME_CONTAINS_SPACE;

    if (m_deBruijnGraphNodes.contains(nodeName + "+"))
        return NODE_NAME_TAKEN;

    return NODE_NAME_OKAY;
}



void AssemblyGraph::changeNodeDepth(std::vector<DeBruijnNode *> * nodes,
                                        double newDepth)
{
    if (nodes->size() == 0)
        return;

    for (size_t i = 0; i < nodes->size(); ++i)
    {
        (*nodes)[i]->setDepth(newDepth);
        (*nodes)[i]->getReverseComplement()->setDepth(newDepth);
    }

    //If this graph does not already have a depthTag, give it a depthTag of KC
    //so the depth info will be saved.
    if (m_depthTag == "")
        m_depthTag = "KC";
}



//This function is used when making FASTA outputs - it breaks a sequence into
//separate lines.  The default interval is 70, as that seems to be what NCBI
//uses.
//The returned string always ends in a newline.
QByteArray AssemblyGraph::addNewlinesToSequence(QByteArray sequence,
                                                int interval)
{
    QByteArray output;

    int charactersRemaining = sequence.length();
    int currentIndex = 0;
    while (charactersRemaining > interval)
    {
        output += sequence.mid(currentIndex, interval);
        output += "\n";
        charactersRemaining -= interval;
        currentIndex += interval;
    }
    output += sequence.mid(currentIndex);
    output += "\n";

    return output;
}




//This function returns the number of dead ends in the graph.
//It looks only at positive nodes, which can have 0, 1 or 2 dead ends each.
//This value therefore varies between zero and twice the node count (specifically
//the positive node count).
int AssemblyGraph::getDeadEndCount() const
{
    int deadEndCount = 0;

    QMapIterator<QString, DeBruijnNode*> i(m_deBruijnGraphNodes);
    while (i.hasNext())
    {
        i.next();
        DeBruijnNode * node = i.value();
        if (node->isPositiveNode())
            deadEndCount += node->getDeadEndCount();
    }

    return deadEndCount;
}



void AssemblyGraph::getNodeStats(int * n50, int * shortestNode, int * firstQuartile, int * median, int * thirdQuartile, int * longestNode) const
{
    if (m_totalLength == 0.0)
        return;

    std::vector<int> nodeLengths;
    QMapIterator<QString, DeBruijnNode*> i(m_deBruijnGraphNodes);
    while (i.hasNext())
    {
        i.next();
        DeBruijnNode * node = i.value();
        if (node->isPositiveNode())
            nodeLengths.push_back(node->getLength());
    }

    if (nodeLengths.size() == 0)
        return;

    std::sort(nodeLengths.begin(), nodeLengths.end());

    *shortestNode = nodeLengths.front();
    *longestNode = nodeLengths.back();

    double firstQuartileIndex = (nodeLengths.size() - 1) / 4.0;
    double medianIndex = (nodeLengths.size() - 1) / 2.0;
    double thirdQuartileIndex = (nodeLengths.size() - 1) * 3.0 / 4.0;

    *firstQuartile = round(getValueUsingFractionalIndex(&nodeLengths, firstQuartileIndex));
    *median = round(getValueUsingFractionalIndex(&nodeLengths, medianIndex));
    *thirdQuartile = round(getValueUsingFractionalIndex(&nodeLengths, thirdQuartileIndex));

    double halfTotalLength = m_totalLength / 2.0;
    long long totalSoFar = 0;
    for (int i = int(nodeLengths.size()) - 1; i >= 0 ; --i)
    {
        totalSoFar += nodeLengths[i];
        if (totalSoFar >= halfTotalLength)
        {
            *n50 = nodeLengths[i];
            break;
        }
    }
}



//This function uses an algorithm adapted from: http://math.hws.edu/eck/cs327_s04/chapter9.pdf
void AssemblyGraph::getGraphComponentCountAndLargestComponentSize(int * componentCount, int * largestComponentLength) const
{
    *componentCount = 0;
    *largestComponentLength = 0;

    QSet<DeBruijnNode *> visitedNodes;
    QList< QList<DeBruijnNode *> > connectedComponents;

    //Loop through all positive nodes.
    QMapIterator<QString, DeBruijnNode*> i(m_deBruijnGraphNodes);
    while (i.hasNext())
    {
        i.next();
        DeBruijnNode * v = i.value();
        if (v->isNegativeNode())
            continue;

        //If the node has not yet been visited, then it must be the start of a new connected component.
        if (!visitedNodes.contains(v))
        {
            QList<DeBruijnNode *> connectedComponent;

            QQueue<DeBruijnNode *> q;
            q.enqueue(v);
            visitedNodes.insert(v);

            while (!q.isEmpty())
            {
                DeBruijnNode * w = q.dequeue();
                connectedComponent.push_back(w);

                std::vector<DeBruijnNode *> connectedNodes = w->getAllConnectedPositiveNodes();
                for (size_t j = 0; j < connectedNodes.size(); ++j)
                {
                    DeBruijnNode * k = connectedNodes[j];
                    if (!visitedNodes.contains(k))
                    {
                        visitedNodes.insert(k);
                        q.enqueue(k);
                    }
                }
            }

            connectedComponents.push_back(connectedComponent);
        }
    }

    //Now that the list of connected components is built, we look for the
    //largest one (as measured by total node length).
    *componentCount = connectedComponents.size();
    for (int i = 0; i < *componentCount; ++i)
    {
        int componentLength = 0;
        for (int j = 0; j < connectedComponents[i].size(); ++j)
            componentLength += connectedComponents[i][j]->getLength();

        if (componentLength > *largestComponentLength)
            *largestComponentLength = componentLength;
    }
}

bool compareNodeDepth(DeBruijnNode * a, DeBruijnNode * b) {return (a->getDepth() < b->getDepth());}



double AssemblyGraph::getMedianDepthByBase() const
{
    if (m_totalLength == 0)
        return 0.0;

    //Make a list of all nodes.
    long long totalLength = 0;
    QList<DeBruijnNode *> nodeList;
    QMapIterator<QString, DeBruijnNode*> i(m_deBruijnGraphNodes);
    while (i.hasNext())
    {
        i.next();
        DeBruijnNode * node = i.value();
        if (node->isPositiveNode())
        {
            nodeList.push_back(node);
            totalLength += node->getLength();
        }
    }

    //If there is only one node, then its depth is the median.
    if (nodeList.size() == 1)
        return nodeList[0]->getDepth();

    //Sort the node list from low to high depth.
    std::sort(nodeList.begin(), nodeList.end(), compareNodeDepth);

    if (totalLength % 2 == 0) //Even total length
    {
        long long medianIndex2 = totalLength / 2;
        long long medianIndex1 = medianIndex2 - 1;
        double depth1 = findDepthAtIndex(&nodeList, medianIndex1);
        double depth2 = findDepthAtIndex(&nodeList, medianIndex2);
        return (depth1 + depth2) / 2.0;
    }
    else //Odd total length
    {
        long long medianIndex = (totalLength - 1) / 2;
        return findDepthAtIndex(&nodeList, medianIndex);
    }
}



//This function takes a node list sorted by depth and a target index (in terms of
//the whole sequence length).  It returns the depth at that index.
double AssemblyGraph::findDepthAtIndex(QList<DeBruijnNode *> * nodeList, long long targetIndex) const
{
    long long lengthSoFar = 0;
    for (int i = 0; i < nodeList->size(); ++i)
    {
        DeBruijnNode * node = (*nodeList)[i];

        lengthSoFar += node->getLength();
        long long currentIndex = lengthSoFar - 1;

        if (currentIndex >= targetIndex)
            return node->getDepth();
    }
    return 0.0;
}



long long AssemblyGraph::getEstimatedSequenceLength() const
{
    return getEstimatedSequenceLength(getMedianDepthByBase());
}



long long AssemblyGraph::getEstimatedSequenceLength(double medianDepthByBase) const
{
    long long estimatedSequenceLength = 0;
    if (medianDepthByBase == 0.0)
        return 0;

    QMapIterator<QString, DeBruijnNode*> i(m_deBruijnGraphNodes);
    while (i.hasNext())
    {
        i.next();
        DeBruijnNode * node = i.value();

        if (node->isPositiveNode())
        {
            int nodeLength = node->getLengthWithoutTrailingOverlap();
            double relativeDepth = node->getDepth() / medianDepthByBase;

            int closestIntegerDepth = round(relativeDepth);
            int lengthAdjustedForDepth = nodeLength * closestIntegerDepth;

            estimatedSequenceLength += lengthAdjustedForDepth;
        }
    }

    return estimatedSequenceLength;
}



long long AssemblyGraph::getTotalLengthMinusEdgeOverlaps() const
{
    long long totalLength = 0;
    QMapIterator<QString, DeBruijnNode*> i(m_deBruijnGraphNodes);
    while (i.hasNext())
    {
        i.next();
        DeBruijnNode * node = i.value();
        if (node->isPositiveNode())
        {
            totalLength += node->getLength();
            const std::vector<DeBruijnEdge *> * edges = node->getEdgesPointer();
            int maxOverlap = 0;
            for (size_t j = 0; j < edges->size(); ++j)
            {
                int edgeOverlap = (*edges)[j]->getOverlap();
                maxOverlap = std::max(maxOverlap, edgeOverlap);
            }
            totalLength -= maxOverlap;
        }
    }

    return totalLength;
}


QPair<int, int> AssemblyGraph::getOverlapRange() const
{
    int smallestOverlap = std::numeric_limits<int>::max();
    int largestOverlap = 0;
    QMapIterator<QPair<DeBruijnNode*, DeBruijnNode*>, DeBruijnEdge*> i(m_deBruijnGraphEdges);
    while (i.hasNext())
    {
        i.next();
        int overlap = i.value()->getOverlap();
        if (overlap < smallestOverlap)
            smallestOverlap = overlap;
        if (overlap > largestOverlap)
            largestOverlap = overlap;
    }
    if (smallestOverlap == std::numeric_limits<int>::max())
        smallestOverlap = 0;
    return QPair<int, int>(smallestOverlap, largestOverlap);
}



//This function will look to see if there is a FASTA file (.fa or .fasta) with
//the same base name as the graph. If so, it will load it and give its
//sequences to the graph nodes with matching names. This is useful for GFA
//files which have no sequences (just '*') like ABySS makes.
//Returns true if any sequences were loaded (doesn't have to be all sequences
//in the graph).
bool AssemblyGraph::attemptToLoadSequencesFromFasta()
{
    if (m_sequencesLoadedFromFasta == NOT_READY || m_sequencesLoadedFromFasta == TRIED)
        return false;

    m_sequencesLoadedFromFasta = TRIED;

    QFileInfo gfaFileInfo(m_filename);
    QString baseName = gfaFileInfo.completeBaseName();
    QString fastaName = gfaFileInfo.dir().filePath(baseName + ".fa");
    QFileInfo fastaFileInfo(fastaName);
    if (!fastaFileInfo.exists())
    {
        fastaName = gfaFileInfo.dir().filePath(baseName + ".fasta");
        fastaFileInfo = QFileInfo(fastaName);
    }
    if (!fastaFileInfo.exists())
    {
        fastaName = gfaFileInfo.dir().filePath(baseName + ".contigs.fasta");
        fastaFileInfo = QFileInfo(fastaName);
    }
    if (!fastaFileInfo.exists())
        return false;

    bool atLeastOneNodeSequenceLoaded = false;
    std::vector<QString> names;
    std::vector<QByteArray> sequences;
    readFastaFile(fastaName, &names, &sequences);

    for (size_t i = 0; i < names.size(); ++i)
    {
        QString name = names[i];
        name = simplifyCanuNodeName(name);
        name = name.split(QRegExp("\\s+"))[0];
        if (m_deBruijnGraphNodes.contains(name + "+"))
        {
            DeBruijnNode * posNode = m_deBruijnGraphNodes[name + "+"];
            if (posNode->sequenceIsMissing())
            {
                atLeastOneNodeSequenceLoaded = true;
                posNode->setSequence(sequences[i]);
                DeBruijnNode * negNode = m_deBruijnGraphNodes[name + "-"];
                negNode->setSequence(getReverseComplement(sequences[i]));
            }
        }
    }

    return atLeastOneNodeSequenceLoaded;
}

// Returns true if every node name in the graph starts with the string.
bool AssemblyGraph::allNodesStartWith(QString start) const
{
    QMapIterator<QString, DeBruijnNode*> i(m_deBruijnGraphNodes);
    while (i.hasNext())
    {
        i.next();
        DeBruijnNode * node = i.value();
        if (!node->getName().startsWith(start))
            return false;
    }
    return true;
}


QString AssemblyGraph::simplifyCanuNodeName(QString oldName) const
{
    QString newName;

    // Remove "tig" from front.
    if (!oldName.startsWith("tig"))
        return oldName;
    newName = oldName.remove(0, 3);
    if (newName.isEmpty())
        return oldName;

    // Remove +/- from end.
    QChar sign = oldName[oldName.length()-1];
    newName.chop(1);
    if (newName.isEmpty())
        return oldName;

    // Remove leading zeros.
    while (newName.length() > 1 && newName[0] == '0')
        newName.remove(0, 1);
    return newName + sign;
}

long long AssemblyGraph::getTotalLengthOrphanedNodes() const {
    long long total = 0;
    QMapIterator<QString, DeBruijnNode*> i(m_deBruijnGraphNodes);
    while (i.hasNext())
    {
        i.next();
        DeBruijnNode * node = i.value();
        if (node->isPositiveNode() && node->getDeadEndCount() == 2)
            total += node->getLength();
    }
    return total;
}


bool AssemblyGraph::useLinearLayout() const {
    // If the graph has no edges, then we use a linear layout. Otherwise check the setting.
    if (m_edgeCount == 0)
        return true;
    else
        return g_settings->linearLayout;
}
