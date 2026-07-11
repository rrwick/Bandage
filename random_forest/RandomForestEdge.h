#ifndef RANDOMFORESTEDGE_H
#define RANDOMFORESTEDGE_H

#include "GraphicsItemFeatureEdge.h"

class RandomForestEdge
{
public:
	GraphicsItemFeatureEdge* getGraphicsItemFeatureEdge() {
		return m_graphicsEdge;
	};
	void setGraphicsItemFeatureEdge(GraphicsItemFeatureEdge* edge) { m_graphicsEdge = edge; };

private:
	GraphicsItemFeatureEdge* m_graphicsEdge;
};
#endif; //RANDOMFORESTEDGE_H
