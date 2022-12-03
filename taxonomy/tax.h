#ifndef TAX_H
#define TAX_H
#include <QObject>
#include <QColor>

class tax : public QObject
{
	Q_OBJECT

public:
	tax(int taxId, QString name, int rank, tax* prevTax = NULL);
	//~tax();

	tax* getPrevTax() {return m_prevTax; };
	void setPrevTax(tax* prevTax) { m_prevTax = prevTax; };
	int getRank() { return m_rank; };
	void setRank(int rank) { m_rank = rank; };
	QString getName() { return m_name; };
	void setName(QString name) { m_name = name; };
	bool isRoot() { return m_prevTax == NULL; };
	void setContigCount(unsigned int contigCount) { m_contigCount = contigCount; };
	void incContigCount() { m_contigCount += 1; };
	unsigned int getContigCount() { return m_contigCount; };
	void setContigLen(unsigned int contigLen) { m_contigLen = contigLen; };
	void addContigLen(unsigned int contigLen) { m_contigLen += contigLen; };
	unsigned int getContigLen() { return m_contigLen; };
	void setColor(QColor color) { m_color = color; };
	QColor getColor() { return m_color; };
	std::vector<tax*> getTaxHierarchy();
	tax* getTaxHierarchy(int rank);
	unsigned int getTaxId() { return m_taxId; };
	bool hasTax(unsigned int taxId);
	bool addWeightInHicLinkedTaxes(tax* curTax, int weight);

	int hicLinksWeight = 0;
	int hicLinksToThemself = 0;
	std::vector<QPair<tax*, int>> m_hicLinkedTaxes;

private:
	QColor m_color = QColor(200, 200, 200);
	tax* m_prevTax = NULL;
	unsigned int m_taxId;
	QString m_name;
	int m_rank;
	unsigned int m_contigLen = 0;
	unsigned int m_contigCount = 0;
};
#endif
