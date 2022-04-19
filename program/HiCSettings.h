#ifndef HICSETTINGS_H
#define HICSETTINGS_H

#include <QObject>
#include <QVector>

#include <ogdf/basic/Graph_d.h>
#include <ogdf/basic/GraphAttributes.h>
#include <QMap>
#include <QPair>


#include "../program/globals.h"
#include "../ui/mygraphicsscene.h"
#include "../graph/debruijnedge.h"

class HiCSettings : public QObject
{
	Q_OBJECT

public:
	HiCSettings();
	~HiCSettings();

	int minWeight = 0;
	int minLength = 1;
	int minDist = 1;
	int maxWeight = 0;
	int maxComponentSize = 100;
	bool checkContigLength = true;
	unsigned long sumWeightBetweenComponent = 0;
	unsigned long countOfEdgesBetweenComponent = 0;

	HiCInclusionFilter inclusionFilter = ALL;
	int componentNum = 0;
	QMap<QPair<int, int>, DeBruijnEdge*> componentEdgeMap;
	QVector<int> targetComponents;
	QVector<int> componentSize;
	QVector<int> averageSize;

	bool isDrawnWithNode(DeBruijnEdge* edge);
	bool isDrawnWithNode(DeBruijnEdge* edge, HiCInclusionFilter filterHiC);
	bool isDrawn(DeBruijnEdge* edge);
	bool isDrawn(DeBruijnEdge* edge, HiCInclusionFilter filterHiC);
	bool addEdgeIfNeeded(DeBruijnEdge* edge);
	bool contains(DeBruijnEdge* edge);
	void addTargetComponentIfNeeded(int id);
	bool isTargetComponent(int componentId);
	bool isConnectedWithTargetComponent(int componentId);
	bool isBigComponent(int componentId);
	int getAverageWeightBetweenComponent() {
		if (countOfEdgesBetweenComponent == 0) {
			return 0;
		}
		return sumWeightBetweenComponent / countOfEdgesBetweenComponent;
	}

private:
	QPair<int, int> getComponentKey(DeBruijnEdge* edge);
	bool isValidContigLength(DeBruijnEdge* edge);
};
#endif //HICSETTINGS_H

