#include "RandomForestNode.h"
#include "../graph/ogdfnode.h"

RandomForestNode::RandomForestNode(QString name) :
	m_name(name)
{}

RandomForestNode::~RandomForestNode()
{}

void RandomForestNode::addChild(RandomForestNode * child) {
	m_children.push_back(child);
	child -> setDepth(m_depth + 1);
	child->setParent(this);
}

void RandomForestNode::addToOgdfGraph(ogdf::Graph* ogdfGraph)
{
    m_ogdfNode = new OgdfNode();
    ogdf::node newNode = ogdfGraph->newNode();
    m_ogdfNode->addOgdfNode(newNode);
}

QColor RandomForestNode::getCustomColour() {
	if (hasCustomColour()) {
		return m_customColour;
	}
	else {
		return g_settings->defaultCustomNodeColour;
	}
}

QStringList RandomForestNode::getCustomLabelForDisplay() const
{
    QStringList customLabelLines;
    if (!getCustomLabel().isEmpty()) {
        QStringList labelLines = getCustomLabel().split("\\n");
        for (int i = 0; i < labelLines.size(); ++i)
            customLabelLines << labelLines[i];
    }
    return customLabelLines;
}

void RandomForestNode::setCustomLabel(QString newLabel)
{
    newLabel.replace("\t", "    ");
    m_customLabel = newLabel;
}