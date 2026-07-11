#ifndef RANDOMFORESTNODE_H
#define RANDOMFORESTNODE_H

#include <QString>
#include <QColor>
#include <vector>
#include "../ogdf/basic/Graph.h"
#include "../ogdf/basic/GraphAttributes.h"
#include "../program/globals.h"
#include "../program/settings.h"

class OgdfNode;
class GraphicsItemFeatureNode;
class RandomForestEdge;

class RandomForestNode
{
public:
    RandomForestNode(QString name);
    ~RandomForestNode();

    void addChild(RandomForestNode * child);
    void setDepth(int depth) { m_depth = depth; }

    int getDepth() { return m_depth; }
    QString getName() { return m_name; }

    RandomForestNode* getParent() { return m_parent; }
    void setParent(RandomForestNode* parent) { m_parent = parent; }

    OgdfNode* getOgdfNode() const { return m_ogdfNode; }
    bool inOgdf() const { return m_ogdfNode != NULL; }

    GraphicsItemFeatureNode * getGraphicsItemFeatureNode() { return m_graphicsItemFeatureNode; }
    void setGraphicsItemFeatureNode(GraphicsItemFeatureNode* graphicsItemNode) { m_graphicsItemFeatureNode = graphicsItemNode; }
    bool hasGraphicsItemFeature() { return m_graphicsItemFeatureNode != NULL; }

    void addToOgdfGraph(ogdf::Graph* ogdfGraph);

    bool isDrawn() { return m_isDrawn; }
    void setAsDrawn(bool drawnStatus) { m_isDrawn = drawnStatus; }

    std::vector<RandomForestNode*> getChildren() { return m_children; }

    std::vector<QString> getQuerySequences() { return m_querySequences; }

    void addQuerySequence(QString seq) { m_querySequences.push_back(seq); }

    void setFeature(QString featureName, double threshold) { m_featureName = featureName; m_threshold = threshold; }
    QString getFeatureName() { return m_featureName; }
    double getThreshold() { return m_threshold; }
    void setClass(int classInd, QString clazz) { m_classInd = classInd; m_class = clazz; }
    QString getClass() { return m_class; }
    int getClassInd() { return m_classInd; }
    int getBlastColourInd() { return m_blastColourIndex; }
    void setBlastColourInd(int blastColourInd) { m_blastColourIndex = blastColourInd; }
    bool hasCustomColour() { return m_customColour.isValid(); }
    QColor getCustomColour();
    void setCustomColour(QColor colour) { m_customColour = colour; }
    QString getCustomLabel() const { return m_customLabel; }
    QStringList getCustomLabelForDisplay() const;
    void setCustomLabel(QString newLabel);
    std::vector<RandomForestEdge*>* getEdges() { return &m_edges; };
    void addEdge(RandomForestEdge* edge) { m_edges.push_back(edge); }

private:
    QString m_name;
    int m_depth = 0;
    std::vector<RandomForestNode*> m_children;
    RandomForestNode * m_parent = NULL;
    OgdfNode * m_ogdfNode = NULL;
    bool m_isDrawn = false;
    std::vector<QString> m_querySequences;

    GraphicsItemFeatureNode * m_graphicsItemFeatureNode = NULL;
    QString m_featureName = NULL;
    double m_threshold = 0;
    QString m_class = NULL;
    int m_classInd = -1;
    int m_blastColourIndex = -1;
    QColor m_customColour;
    QString m_customLabel = NULL;

    std::vector<RandomForestEdge*> m_edges;

};

#endif // RANDOMFORESTNODE_H

