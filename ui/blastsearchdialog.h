//Copyright 2016 Ryan Wick

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
#include <QThread>
#include <QProcess>
#include "../program/globals.h"

class DeBruijnNode;
class BlastQuery;
class QueryPathsDialog;

namespace Ui {
class BlastSearchDialog;
}

class BlastSearchDialog : public QDialog
{
    Q_OBJECT

public:
    explicit BlastSearchDialog(QWidget *parent = 0, QString autoQuery = "");
    ~BlastSearchDialog();

private:
    Ui::BlastSearchDialog *ui;
    QMap<long long, DeBruijnNode*> * m_deBruijnGraphNodes;
    QString m_makeblastdbCommand;
    QString m_blastnCommand;
    QString m_tblastnCommand;
    QThread * m_buildBlastDatabaseThread;
    QThread * m_blastSearchThread;
    QueryPathsDialog * m_queryPathsDialog;

    void setUiStep(BlastUiState blastUiState);
    void clearBlastHits();
    void setInfoTexts();
    void loadBlastQueriesFromFastaFile(QString fullFileName);
    void buildBlastDatabase(bool separateThread);
    void runBlastSearches(bool separateThread);
    void makeQueryRow(int row);
    void deleteQueryPathsDialog();
    void setFilterText();

private slots:
    void afterWindowShow();
    void buildBlastDatabaseInThread();
    void loadBlastQueriesFromFastaFileButtonClicked();
    void enterQueryManually();
    void clearAllQueries();
    void clearSelectedQueries();
    void runBlastSearchesInThread();
    void fillTablesAfterBlastSearch();
    void fillQueriesTable();
    void fillHitsTable();
    void blastDatabaseBuildFinished(QString error);
    void runBlastSearchFinished(QString error);
    void buildBlastDatabaseCancelled();
    void runBlastSearchCancelled();
    void queryCellChanged(int row, int column);
    void queryTableSelectionChanged();
    void queryShownChanged();
    void showPathsDialog(BlastQuery * query);
    void queryPathSelectionChangedSlot();
    void openFiltersDialog();

signals:
    void blastChanged();
    void queryPathSelectionChanged();
};


#endif // BLASTSEARCHDIALOG_H
