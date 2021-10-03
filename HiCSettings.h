#ifndef HICSETTINGS_H
#define HICSETTINGS_H

#include <QObject>
#include <vector>

#include "../ogdf/basic/Graph.h"
#include "../ogdf/basic/GraphAttributes.h"
#include <QString>
#include <QMap>
#include "../program/globals.h"
#include "../ui/mygraphicsscene.h"
class HiCSettings : public QObject
{
	Q_OBJECT

public:
	HiCSettings();
	~HiCSettings();
	
	int numOfNodes = 0;
	int minWeight = 0;
	int minLength = 1;
	int minDist = 1;
	QVector <QString> kontigNames;
	QVector <QVector <int> > weightMatrix;
	QVector <QVector <int> > distanceMatrix;
	bool isDrawn(DeBruijnEdge* edge);
private:
	int getEdgeWeight(DeBruijnEdge* edge);


};
#endif //HICSETTINGS_H

