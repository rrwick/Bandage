#include "assemblyforest.h"
#include <QFile>
#include <QTextStream>
#include <QApplication>
#include "../graph/ogdfnode.h"
#include "../program/globals.h"
#include "GraphicsItemFeatureNode.h"
#include "GraphicsItemFeatureEdge.h"

AssemblyForest::AssemblyForest() {
    m_ogdfGraph = new ogdf::Graph();
    m_edgeArray = new ogdf::EdgeArray<double>(*m_ogdfGraph);
    m_graphAttributes = new ogdf::GraphAttributes(*m_ogdfGraph, ogdf::GraphAttributes::nodeGraphics |
        ogdf::GraphAttributes::edgeGraphics);
}

AssemblyForest::~AssemblyForest() {}

bool AssemblyForest::loadRandomForestFromFile(QString filename, QString* errormsg) {
    QFile featureForestFile(filename);

    if (!featureForestFile.open(QIODevice::ReadOnly))
    {
        *errormsg = "Unable to read from specified file.";
        return false;
    }

    QTextStream in(&featureForestFile);
    QApplication::processEvents();
    while (!in.atEnd())
    {
        QString line = in.readLine();
        QStringList data = line.split(QRegExp("\t"));
        if (data.size() <= 1) {
            break;
        }
        if (data[0] == "N") {
            QString nodeName = data[1];
            RandomForestNode* curnode = m_nodes[nodeName];
            if (curnode == nullptr) {
                curnode = new RandomForestNode(nodeName);
                m_nodes.insert(nodeName, curnode);
            }
            if (data.size() > 2 && data[2].size() > 0) {
                processChild(curnode, data[2]);
            }
            if (data.size() > 3 && data[3].size() > 0) {
                processChild(curnode, data[3]);
            }

        }
        if (data[0] == "S") {
            QString nodeName = data[1];
            QString seq = data[2].simplified();
            m_nodes[nodeName]->addQuerySequence(seq);
        }
        if (data[0] == "F") {
            QString nodeName = data[1];
            QString featureName = data[2];
            double threshold = data[3].toDouble();
            m_nodes[nodeName]->setFeature(featureName, threshold);
        }
        if (data[0] == "C") {
            QString nodeName = data[1];
            QString clazz = data[2].simplified();
            addClass(clazz);
            m_nodes[nodeName]->setClass(m_classes[clazz], clazz);
        }

    }
    featureForestFile.close();
    for (RandomForestNode* node : m_nodes) {
        if (node->getParent() == NULL) {
            m_roots[node->getName()] = node;
        }
    }
    return true;
}

void AssemblyForest::buildOgdfGraphFromNodesAndEdges()
{
    QMapIterator<QString, RandomForestNode*> i(m_nodes);
    
    while (i.hasNext())
    {
        i.next();

        RandomForestNode* node = i.value();
        node->setAsDrawn(true);
        node->addToOgdfGraph(m_ogdfGraph);
       
    }

    QMapIterator<QString, RandomForestNode*> j(m_nodes);
    while (j.hasNext())
    {
        j.next();
        RandomForestNode* node = j.value();
        if (node->isDrawn()) {
            for (RandomForestNode* child : node->getChildren()) {
                if (child->isDrawn()) {
                    addEdgeToOgdfGraph(node, child);
                }
            }
        }
    }
}

void AssemblyForest::recalculateAllNodeWidths()
{
    QMapIterator<QString, RandomForestNode*> i(m_nodes);
    while (i.hasNext())
    {
        i.next();
        GraphicsItemFeatureNode* graphicsItemNode = i.value()->getGraphicsItemFeatureNode();
        if (graphicsItemNode != 0)
            graphicsItemNode->m_width = g_settings->averageFeatureNodeWidth;
    }
}

void AssemblyForest::addEdgeToOgdfGraph(RandomForestNode* startingNode, RandomForestNode* endingNode) {
    ogdf::node firstEdgeOgdfNode;
    ogdf::node secondEdgeOgdfNode;

    if (startingNode->inOgdf())
        firstEdgeOgdfNode = startingNode->getOgdfNode()->getFirst();
    else
        return;

    if (endingNode->inOgdf())
        secondEdgeOgdfNode = endingNode->getOgdfNode()->getFirst();
    else
        return;
       
        
    ogdf::edge newEdge = m_ogdfGraph->newEdge(firstEdgeOgdfNode, secondEdgeOgdfNode);
    (*m_edgeArray)[newEdge] = g_settings->featureForestEdgeLength;
}

void AssemblyForest::cleanUp() {}

void AssemblyForest::processChild(RandomForestNode* parent, QString childName) {
    if (m_nodes.contains(childName)) {
        RandomForestNode* child = m_nodes[childName];
        parent->addChild(child);
        RandomForestEdge* edge = new RandomForestEdge();
        parent->addEdge(edge);
        child->addEdge(edge);
        m_edges.insert(QPair<RandomForestNode*, RandomForestNode*>(parent, child), edge);

    }
    else {
        RandomForestNode* child = new RandomForestNode(childName);
        m_nodes.insert(childName, child);
        parent->addChild(child);
        RandomForestEdge* edge = new RandomForestEdge();
        parent->addEdge(edge);
        child->addEdge(edge);
        m_edges.insert(QPair<RandomForestNode*, RandomForestNode*>(parent, child), edge);
    }
   
}

void AssemblyForest::addGraphicsItemsToScene(MyGraphicsScene* scene)
{
    scene->clear();

    //First make the GraphicsItemNode objects
    QMapIterator<QString, RandomForestNode*> i(m_nodes);
    while (i.hasNext())
    {
        i.next();
        RandomForestNode* node = i.value();
        if (node->isDrawn())
        {
            GraphicsItemFeatureNode* graphicsItemNode = new GraphicsItemFeatureNode(node, m_graphAttributes);
            node->setGraphicsItemFeatureNode(graphicsItemNode);
            graphicsItemNode->setFlag(QGraphicsItem::ItemIsSelectable);
            graphicsItemNode->setFlag(QGraphicsItem::ItemIsMovable);
        }
    }

    resetAllNodeColours();


    QMapIterator<QString, RandomForestNode*> j(m_nodes);
    while (j.hasNext())
    {
        j.next();
        RandomForestNode* node = j.value();
        if (node->isDrawn()) {
            for (RandomForestNode* child : node->getChildren()) {
                if (child->isDrawn()) {
                    GraphicsItemFeatureEdge* graphicsItemEdge = new GraphicsItemFeatureEdge(node, child);
                    graphicsItemEdge->setFlag(QGraphicsItem::ItemIsSelectable);
                    scene->addItem(graphicsItemEdge);
                    m_edges[QPair<RandomForestNode*, RandomForestNode*>(node, child)]->setGraphicsItemFeatureEdge(graphicsItemEdge);
                }
            }
        }
    }

    //Now add the GraphicsItemNode objects to the scene so they are drawn
    //on top
    QMapIterator<QString, RandomForestNode*> k(m_nodes);
    while (k.hasNext())
    {
        k.next();
        RandomForestNode* node = k.value();
        if (node->isDrawn() && node->hasGraphicsItemFeature())
            scene->addItem(node->getGraphicsItemFeatureNode());
    }
}

void AssemblyForest::resetAllNodeColours()
{
    QMapIterator<QString, RandomForestNode*> i(m_nodes);
    while (i.hasNext())
    {
        i.next();
        if (i.value()->isDrawn() && i.value()->getGraphicsItemFeatureNode() != 0)
            i.value()->getGraphicsItemFeatureNode()->setColour();
    }
}

void AssemblyForest::addClass(QString className) {
    int ind = m_classes.size();
    if (!m_classes.contains(className)) {
        m_classes[className] = ind;
    }
}

QString AssemblyForest::getClassFigureInfo() {
    QString res = "";
    for (QString className : m_classes.keys()) {
        res = res + className + ": ";
        if (m_classes[className] == 0) {
            res += QChar(0x2B1B);
            res += " (cube)\n";
        } 
        else if (m_classes[className] == 1) {
            res += QChar(0x25B2);
            res += " (triangle)\n";
        }
        else {
            res += QChar(0x2B24);
            res += " (circle)\n";
        }
    }
    return res;
}