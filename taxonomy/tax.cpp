#include "tax.h"

tax::tax(int taxId, QString name, int rank, tax* prevTax) :
    m_taxId(taxId),
    m_name(name),
    m_rank(rank),
    m_prevTax(prevTax)
{}

std::vector<tax*> tax::getTaxHierarchy() {
	std::vector<tax*> res;
	res.push_back(this);
	tax* curTax = this;
	while (curTax->getPrevTax() != NULL && curTax->getRank() != 0) {
		curTax = curTax->getPrevTax();
		res.push_back(curTax);
	}
	return res;
}

tax* tax::getTaxHierarchy(int rank) {
	tax* res = NULL;
	tax* curTax = this;
	if (this->getRank() == rank) {
		return this;
	}
	while (curTax->getPrevTax() != NULL && curTax->getRank() != 0) {
		if (curTax->getRank() == rank)
			return curTax;
		curTax = curTax->getPrevTax();
	}
	return res;
}

bool tax::hasTax(unsigned int taxId) {
	tax* curTax = this;
	while (curTax->getPrevTax() != NULL && curTax->getRank() != 0) {
		if (curTax->getTaxId() == taxId) {
			return true;
		}
		curTax = curTax->getPrevTax();
	}
	return false;
}

bool tax::addWeightInHicLinkedTaxes(tax* curTax, int weight) {
	for (int i = 0; i < m_hicLinkedTaxes.size(); i++) {
		if (m_hicLinkedTaxes[i].first == curTax) {
			m_hicLinkedTaxes[i].second += weight;
			return true;
		}
	}
	return false;
}
