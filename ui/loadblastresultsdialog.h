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


#ifndef LOADBLASTRESULTSDIALOG_H
#define LOADBLASTRESULTSDIALOG_H

#include <QDialog>
#include "../blast/blastsearch.h"
#include <QMap>

class DeBruijnNode;
class BlastQuery;

namespace Ui {
class LoadBlastResultsDialog;
}

class LoadBlastResultsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LoadBlastResultsDialog(QMap<int, DeBruijnNode*> * deBruijnGraphNodes,
                                    QWidget *parent = 0);
    ~LoadBlastResultsDialog();

private:
    Ui::LoadBlastResultsDialog *ui;
    QMap<int, DeBruijnNode*> * m_deBruijnGraphNodes;

    int getNodeNumberFromString(QString nodeString);
    void readFastaFile(QString filename, std::vector<QString> * names, std::vector<QString> * sequences);
    void makeQueryFile();
    void setUiStep(int step);
    void clearBlastHits();

private slots:
    void buildBlastDatabase1();
    void buildBlastDatabase2();
    void loadBlastQueriesFromFastaFile();
    void enterQueryManually();
    void clearQueries();
    void runBlastSearch();

    void loadBlastQueries();
    void loadBlastHits();
    void fillQueriesTable();
    void fillHitsTable();

signals:
    void createAllNodesFasta(QString path);

};

#endif // LOADBLASTRESULTSDIALOG_H
