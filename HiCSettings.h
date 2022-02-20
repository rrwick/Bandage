#ifndef HICSETTINGS_H
#define HICSETTINGS_H

#include <QObject>
#include <vector>

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

	//int numOfNodes = 0;
	int minWeight = 0;
	int minLength = 1;
	int minDist = 1;
	int maxWeight = 0;
	//0 - no filter
	//1 - all edges between components
	//2 - one edge between component
	int filterHiC = 0;
	int componentNum = 0;
	QMap<QPair<int, int>, DeBruijnEdge*> componentEdgeMap;
	bool isDrawn(DeBruijnEdge* edge);
	bool HiCSettings::addEdgeIfNeeded(DeBruijnEdge* edge);
	bool HiCSettings::contains(DeBruijnEdge* edge);
private:
};
#endif //HICSETTINGS_H

