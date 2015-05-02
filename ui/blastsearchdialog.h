//Copyright 2015 Ryan Wick

//This file is part of Bandage

//Bandage is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//Bandage is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

//You should have received a copy of the GNU General Public License
//along with Bandage.  If not, see <http://www.gnu.org/licenses/>.


#ifndef BLASTSEARCHDIALOG_H
#define BLASTSEARCHDIALOG_H

#include <QDialog>
#include <QMap>

class DeBruijnNode;

namespace Ui {
class BlastSearchDialog;
}

class BlastSearchDialog : public QDialog
{
    Q_OBJECT

public:
    explicit BlastSearchDialog(QWidget *parent = 0);
    ~BlastSearchDialog();

private:
    Ui::BlastSearchDialog *ui;
    QMap<long long, DeBruijnNode*> * m_deBruijnGraphNodes;
    QString m_makeblastdbCommand;
    QString m_blastnCommand;

    long long getNodeNumberFromString(QString nodeString);
    void setUiStep(int step);
    void clearBlastHits();
    void setInfoTexts();
    QString cleanQueryName(QString queryName);

private slots:
    void buildBlastDatabase1();
    void buildBlastDatabase2();
    void loadBlastQueriesFromFastaFile();
    void enterQueryManually();
    void clearQueries();
    void runBlastSearch();
    void loadBlastHits(QString blastHits);
    void fillQueriesTable();
    void fillHitsTable();

signals:
    void createAllNodesFasta(QString path, bool includeEmptyNodes, bool useTrinityNames);

};

#endif // BLASTSEARCHDIALOG_H
