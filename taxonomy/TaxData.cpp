#include "TaxData.h"
#include <QFile>
#include <QVector>
#include <QTextStream>
#include <QApplication>

TaxData::TaxData(tax* root) :
	m_treeRoot(root)
{}

tax* TaxData::addTax(QVector<QPair<QString, unsigned int>>* taxonomy) {
	tax* curTax = m_treeRoot;
	for (int i = 0; i < taxonomy->size(); i++) {
		QPair<QString, int> pair = taxonomy->at(i);
		if (m_taxMap.contains(pair.second)) {
			curTax = m_taxMap[pair.second];
		}
		else {
			tax* newTax = new tax(pair.second, pair.first, i + 1, curTax);
			m_taxMap[pair.second] = newTax;
			m_statistic[i + 1].push_back(newTax);
			curTax = newTax;
		}
	}
	return curTax;
}

void TaxData::addDeBruineNode(DeBruijnNode* node, tax* curTax) {
	curTax->addContigLen(node->getLength());
	curTax->incContigCount();
	while (curTax->getPrevTax() != NULL)
	{
		tax* prevTax = curTax->getPrevTax();
		prevTax->addContigLen(node->getLength());
		prevTax->incContigCount();
		curTax = prevTax;
	}
}

bool taxContigLenCmp(tax* a, tax* b)
{
	return a->getContigLen() > b->getContigLen();
}

void TaxData::calcStatistic() {
	//QMap<int, tax* [10]> m_statistic;
	for (int i = 1; i < m_statistic.size(); i++) {
		std::vector<tax*>* allRankTaxes = &m_statistic[i];
		std::sort(allRankTaxes->begin(), allRankTaxes->end(), taxContigLenCmp);
	}
	setColour();
}

void TaxData::setColour() {
	for (std::vector<tax*> allRankTaxes : m_statistic.values()) {
		std::sort(allRankTaxes.begin(), allRankTaxes.end(), taxContigLenCmp);
		bool random = false;
		int h = 0;
		int s = 255;
		int l = 127;
		for (tax* curTax : allRankTaxes) {
			if (h > 340) {
				h = 0;
				s -= 50;
				if (s < 100) {
					random = true;
				}
			}
			if (!random) {
				QColor posColour;
				
				posColour.setHsl(h, s, l);
				posColour.setAlpha(g_settings->randomColourPositiveOpacity);
				h += 20;
				curTax->setColor(posColour);
			}
			else {
				int hue = rand() % 360;
				QColor posColour;
				posColour.setHsl(hue,
					g_settings->randomColourPositiveSaturation,
					g_settings->randomColourPositiveLightness);
				posColour.setAlpha(g_settings->randomColourPositiveOpacity);
				curTax->setColor(posColour);
			}
		}
	}
}

QString TaxData::getLevelByRank(int rank) {
	switch (rank)
	{
	case 0:
		return "Root";
	case 1:
		return "Domain";
	case 2:
		return "Kingdom";
	case 3:
		return "Phylum";
	case 4:
		return "Class";
	case 5:
		return "Order";
	case 6:
		return "Family";
	case 7:
		return "Genus";
	case 8:
		return "Species";
	default:
		return "No rank";
	}
}