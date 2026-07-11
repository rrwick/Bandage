#ifndef TAXDATA_H
#define TAXDATA_H
#include <QObject>
#include <QMap>
#include <QPair>
#include "tax.h"
#include "../graph/debruijnnode.h"
class TaxData :QObject
{
	Q_OBJECT
public:
	TaxData(tax* root);
	tax* addTax(QVector<QPair<QString, unsigned int>>* taxonomy);
	void addDeBruineNode(DeBruijnNode* node, tax* curTax);
	void calcStatistic();
	QString getLevelByRank(int rank);
	void setColour();

	QMap<unsigned int, tax*> m_taxMap;
	QMap<int, std::vector<tax*>> m_statistic;
	QMap<QPair<unsigned int, unsigned int>, unsigned int> hiCLinksWeight;
	QPair<unsigned int, unsigned int> getHiCLinksWeightKey(unsigned int tax1, unsigned int tax2) {
		return qMakePair(std::min(tax1, tax2), std::max(tax1, tax2));
	}
	double m_hiCMinNormalizedWeightByTax = -1;
private:
	tax* m_treeRoot;
};
#endif //TAXDATA_H

