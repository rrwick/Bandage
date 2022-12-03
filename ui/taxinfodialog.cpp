#include "taxinfodialog.h"
#include "ui_taxinfodialog.h"

#include "../program/globals.h"
#include "../graph/assemblygraph.h"
#include <QPair>
#include <QFileDialog>
#include <QMessageBox>

TaxInfoDialog::TaxInfoDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TaxInfoDialog)
{
    ui->setupUi(this);
    setInfoTexts();
}

TaxInfoDialog::TaxInfoDialog(QWidget* parent, int taxId) :
    QDialog(parent),
    ui(new Ui::TaxInfoDialog)
{
    ui->setupUi(this);
    if (taxId != 0)
        setSpecialTaxInfoTexts(taxId);
    else
        setErrorText();
}

TaxInfoDialog::~TaxInfoDialog()
{
    delete ui;
}

void TaxInfoDialog::setInfoTexts()
{
    QMap<int, std::vector<tax*>>* stat = &(g_assemblyGraph->m_taxData->m_statistic);
    QString text;
    text += "There is statistic about taxonometry analyse. Ten most represented taxes in every of 8 ranks were displayed.\n";
    text += "Every record contains tax name, tax id, total length of contigs under this tax, number of taxes under this tax";
    for (int rank = 1; rank < 9; rank++) {
        text += "<tr><td><b>";
        text += g_assemblyGraph->m_taxData->getLevelByRank(rank);
        text += "</b></tr></td>";
        std::vector<tax*>* tempRankStat = &((*stat)[rank]);
        for (int i = 0; i < std::min(10, (int)tempRankStat->size()); i++) {
            tax* curTax = tempRankStat->at(i);
            text += "<tr><td>";
            text += (curTax->getName());
            text += " </td><td>";
            text += QString::number(curTax->getTaxId());
            text += " </td><td>";
            text += QString::number(curTax->getContigLen());
            text += "</td><td>";
            text += QString::number(curTax->getContigCount());
            text += "</td></tr>";
        }
    }
    ui->taxPlainTextEdit->setHtml(text);
}

bool taxPairCmp(QPair<tax*, int> a, QPair<tax*, int> b)
{
    return a.second > b.second;
}

void TaxInfoDialog::setErrorText() {
    QString text = "Tax Id was not filled.";
    ui->taxPlainTextEdit->setHtml(text);
}

void TaxInfoDialog::setErrorText(int taxId) {
    QString text = "Tax Id (" +QString::number(taxId)+") was not found.";
    ui->taxPlainTextEdit->setHtml(text);
}

void TaxInfoDialog::setSpecialTaxInfoTexts(int taxId) {
    tax* currentTax = g_assemblyGraph->m_taxData->m_taxMap[taxId];
    QString text;
    if (currentTax != NULL) {
        std::vector<QPair<tax*, int>> res = g_assemblyGraph->getHiCConnectedTaxes(currentTax);
        res.push_back(qMakePair(currentTax, currentTax->hicLinksToThemself));
        std::sort(res.begin(), res.end(), taxPairCmp);
        text += "<table>";
        text += ("<tr><td><b>|tax name\t</b></td><td><b>|tax id\t</b></td><td><b>|num of Hi-C links\t</b></td><td><b>|contig len\t</b></td><td><b>|% of all Hi-C links\t</b></td></tr>");
        for (int i = 0; i < res.size(); i++) {
            QPair<tax*, int>* pair = &res[i];
            text += ("<tr><td><b>|" + (pair->first->getName()) + "</b></td>");
            text += ("<td><b>|</b>" + QString::number(pair->first->getTaxId()) + "</td>");
            text += ("<td><b>|</b>" + QString::number(pair->second) + "</td>");
            text += ("<td><b>|</b>" + QString::number(pair->first->getContigLen()) + "</td>");
            text += ("<td><b>|" + QString::number((double)((double)pair->second/(double)pair->first->hicLinksWeight)*100) + "%</b></td></tr>");
        }
        text += "</table>";
        ui->taxPlainTextEdit->setHtml(text);
    }
    else {
        setErrorText(taxId);
    }
}

QString TaxInfoDialog::getHiCTaxInfoInTxt(int taxId) {
    tax* currentTax = g_assemblyGraph->m_taxData->m_taxMap[taxId];
    QString text = NULL;
    if (currentTax != NULL) {
        std::vector<QPair<tax*, int>> res = g_assemblyGraph->getHiCConnectedTaxes(currentTax);
        res.push_back(qMakePair(currentTax, currentTax->hicLinksToThemself));
        std::sort(res.begin(), res.end(), taxPairCmp);
        text += ("tax name,\ttax id,\tnum of Hi-C links,\tcontig len,\t% of all Hi-C links;\n");
        for (int i = 0; i < res.size(); i++) {
            QPair<tax*, int>* pair = &res[i];
            text += (pair->first->getName() + ",\t");
            text += (QString::number(pair->first->getTaxId()) + ",\t");
            text += (QString::number(pair->second) + ",\t");
            text += (QString::number(pair->first->getContigLen()) + ",\t");
            text += (QString::number((double)((double)pair->second / (double)pair->first->hicLinksWeight) * 100) + "%;\n");
        }
    }
    return text;
}

QString TaxInfoDialog::getCommonTaxInfoInTxt() {
    QMap<int, std::vector<tax*>>* stat = &(g_assemblyGraph->m_taxData->m_statistic);
    QString text;
    text += "There is statistic about taxonometry analyse. Ten most represented taxes in every of 8 ranks were displayed.\n";
    text += "Every record contains tax name, tax id, total length of contigs under this tax, number of taxes under this tax";
    text += ";\n";
    for (int rank = 1; rank < 9; rank++) {
        text += g_assemblyGraph->m_taxData->getLevelByRank(rank);
        text += ";\n";
        std::vector<tax*>* tempRankStat = &((*stat)[rank]);
        for (int i = 0; i < tempRankStat->size(); i++) {
            tax* curTax = tempRankStat->at(i);
            text += (curTax->getName());
            text += ",\t";
            text += QString::number(curTax->getTaxId());
            text += ",\t";
            text += QString::number(curTax->getContigLen());
            text += ",\t";
            text += QString::number(curTax->getContigCount());
            text += ";\n";
        }
    }
    return text;
}
