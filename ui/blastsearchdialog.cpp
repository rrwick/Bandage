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


#include "blastsearchdialog.h"
#include "ui_blastsearchdialog.h"

#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QString>
#include "../blast/blasthit.h"
#include "../blast/blastquery.h"
#include <QStandardItemModel>
#include "../program/globals.h"
#include "../program/settings.h"
#include "../program/memory.h"
#include "../graph/debruijnnode.h"
#include <QMessageBox>
#include <QDir>
#include "enteroneblastquerydialog.h"
#include "../graph/assemblygraph.h"
#include "../blast/blastsearch.h"
#include <QProcessEnvironment>
#include <QMessageBox>
#include <QThread>
#include "../blast/buildblastdatabaseworker.h"
#include "../blast/runblastsearchworker.h"
#include "myprogressdialog.h"
#include "colourbutton.h"
#include <QSet>
#include "tablewidgetitemint.h"
#include "tablewidgetitemdouble.h"
#include <QCheckBox>

BlastSearchDialog::BlastSearchDialog(QWidget *parent, QString autoQuery) :
    QDialog(parent),
    m_blastSearchConducted(false),
    ui(new Ui::BlastSearchDialog),
    m_makeblastdbCommand("makeblastdb"), m_blastnCommand("blastn"), m_tblastnCommand("tblastn")

{
    ui->setupUi(this);
    ui->blastHitsTableWidget->m_smallFirstColumn = true;
    ui->blastQueriesTableWidget->m_smallFirstColumn = true;
    ui->blastQueriesTableWidget->m_smallSecondColumn = true;

    //Load any previous parameters the user might have entered when previously using this dialog.
    ui->parametersLineEdit->setText(g_settings->blastSearchParameters);

    //If the dialog is given an autoQuery parameter, then it will
    //carry out the entire process on its own.
    if (autoQuery != "")
    {
        buildBlastDatabase(false);
        clearAllQueries();
        loadBlastQueriesFromFastaFile(autoQuery);
        runBlastSearches(false);
        QMetaObject::invokeMethod(this, "close", Qt::QueuedConnection);
        return;
    }

    //Prepare the query and hits tables
    ui->blastHitsTableWidget->setHorizontalHeaderLabels(QStringList() << "" << "Query\nname" << "Node\nname" <<
                                                        "Percent\nidentity" << "Alignment\nlength" << "Mis-\nmatches" <<
                                                        "Gap\nopens" << "Query\nstart" << "Query\nend" << "Node\nstart" <<
                                                        "Node\nend" <<"E-\nvalue" << "Bit\nscore");
    QFont font = ui->blastQueriesTableWidget->horizontalHeader()->font();
    font.setBold(true);
    ui->blastQueriesTableWidget->horizontalHeader()->setFont(font);
    ui->blastHitsTableWidget->horizontalHeader()->setFont(font);


    //If a BLAST database already exists, move to step 2.
    QFile databaseFile(g_blastSearch->m_tempDirectory + "all_nodes.fasta");
    if (databaseFile.exists())
        setUiStep(BLAST_DB_BUILT_BUT_NO_QUERIES);

    //If there isn't a BLAST database, clear the entire temporary directory
    //and move to step 1.
    else
    {
        g_blastSearch->emptyTempDirectory();
        setUiStep(BLAST_DB_NOT_YET_BUILT);
    }

    //If queries already exist, display them and move to step 3.
    if (g_blastSearch->m_blastQueries.m_queries.size() > 0)
    {
        fillQueriesTable();
        setUiStep(READY_FOR_BLAST_SEARCH);
    }

    //If results already exist, display them and move to step 4.
    if (g_blastSearch->m_allHits.size() > 0)
    {
        fillHitsTable();
        setUiStep(BLAST_SEARCH_COMPLETE);
    }

    setInfoTexts();

    connect(ui->buildBlastDatabaseButton, SIGNAL(clicked()), this, SLOT(buildBlastDatabaseInThread()));
    connect(ui->loadQueriesFromFastaButton, SIGNAL(clicked()), this, SLOT(loadBlastQueriesFromFastaFileButtonClicked()));
    connect(ui->enterQueryManuallyButton, SIGNAL(clicked()), this, SLOT(enterQueryManually()));
    connect(ui->clearAllQueriesButton, SIGNAL(clicked()), this, SLOT(clearAllQueries()));
    connect(ui->clearSelectedQueriesButton, SIGNAL(clicked(bool)), this, SLOT(clearSelectedQueries()));
    connect(ui->runBlastSearchButton, SIGNAL(clicked()), this, SLOT(runBlastSearchesInThread()));
    connect(ui->blastQueriesTableWidget, SIGNAL(cellChanged(int,int)), this, SLOT(queryCellChanged(int,int)));
    connect(ui->blastQueriesTableWidget, SIGNAL(itemSelectionChanged()), this, SLOT(queryTableSelectionChanged()));
}

BlastSearchDialog::~BlastSearchDialog()
{
    delete ui;
}



void BlastSearchDialog::afterWindowShow()
{
    ui->blastQueriesTableWidget->resizeColumns();
    ui->blastHitsTableWidget->resizeColumns();
}

void BlastSearchDialog::clearBlastHits()
{
    g_blastSearch->clearBlastHits();
    ui->blastHitsTableWidget->clearContents();
    while (ui->blastHitsTableWidget->rowCount() > 0)
        ui->blastHitsTableWidget->removeRow(0);
}

void BlastSearchDialog::fillTablesAfterBlastSearch()
{
    if (g_blastSearch->m_allHits.empty())
        QMessageBox::information(this, "No hits", "No BLAST hits were found for the given queries and parameters.");

    fillQueriesTable();
    fillHitsTable();
}


void BlastSearchDialog::fillQueriesTable()
{
    //Turn off table widget signals for this function so the
    //queryCellChanged slot doesn't get called.
    ui->blastQueriesTableWidget->blockSignals(true);
    ui->blastQueriesTableWidget->setSortingEnabled(false);

    ui->blastQueriesTableWidget->clearContents();

    int queryCount = int(g_blastSearch->m_blastQueries.m_queries.size());
    if (queryCount == 0)
        return;

    ui->blastQueriesTableWidget->setRowCount(queryCount);

    for (int i = 0; i < queryCount; ++i)
        makeQueryRow(i);

    ui->blastQueriesTableWidget->resizeColumns();
    ui->blastQueriesTableWidget->setSortingEnabled(true);

    ui->blastQueriesTableWidget->setSortingEnabled(true);
    ui->blastQueriesTableWidget->blockSignals(false);
}

void BlastSearchDialog::makeQueryRow(int row)
{
    if (row >= int(g_blastSearch->m_blastQueries.m_queries.size()))
        return;

    BlastQuery * query = g_blastSearch->m_blastQueries.m_queries[row];

    QTableWidgetItem * name = new QTableWidgetItem(query->getName());
    name->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);

    QTableWidgetItem * type = new QTableWidgetItem(query->getTypeString());
    type->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);

    TableWidgetItemInt * length = new TableWidgetItemInt(formatIntForDisplay(query->getLength()));
    length->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);

    //If the search hasn't yet been run, some of the columns will just have
    //a dash.
    TableWidgetItemInt * hits;
    TableWidgetItemDouble * percent;
    QTableWidgetItem * paths;

    if (query->wasSearchedFor())
    {
        hits = new TableWidgetItemInt(formatIntForDisplay(query->hitCount()));
        percent = new TableWidgetItemDouble(formatDoubleForDisplay(100.0 * query->fractionCoveredByHits(), 2) + "%");
        paths = new QTableWidgetItem(query->getPathsString(g_settings->maxQueryPaths));
    }
    else
    {
        hits = new TableWidgetItemInt("-");
        percent = new TableWidgetItemDouble("-");
        paths = new QTableWidgetItem("-");
    }

    hits->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    percent->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    paths->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);

    QTableWidgetItem * colour = new QTableWidgetItem(query->getColour().name());
    ColourButton * colourButton = new ColourButton();
    colourButton->setColour(query->getColour());
    connect(colourButton, SIGNAL(colourChosen(QColor)), query, SLOT(setColour(QColor)));
    connect(colourButton, SIGNAL(colourChosen(QColor)), this, SLOT(fillHitsTable()));

    QTableWidgetItem * show = new QTableWidgetItem();
    show->setFlags(Qt::ItemIsEnabled);
    QWidget * showCheckBoxWidget = new QWidget;
    QCheckBox * showCheckBox = new QCheckBox();
    QHBoxLayout * layout = new QHBoxLayout(showCheckBoxWidget);
    layout->addWidget(showCheckBox);
    layout->setAlignment(Qt::AlignCenter);
    layout->setContentsMargins(0, 0, 0, 0);
    showCheckBoxWidget->setLayout(layout);
    showCheckBox->setChecked(query->isShown());
    connect(showCheckBox, SIGNAL(toggled(bool)), query, SLOT(setShown(bool)));
    connect(showCheckBox, SIGNAL(toggled(bool)), this, SLOT(fillHitsTable()));

    ui->blastQueriesTableWidget->setCellWidget(row, 0, colourButton);
    ui->blastQueriesTableWidget->setCellWidget(row, 1, showCheckBoxWidget);
    ui->blastQueriesTableWidget->setItem(row, 0, colour);
    ui->blastQueriesTableWidget->setItem(row, 1, show);
    ui->blastQueriesTableWidget->setItem(row, 2, name);
    ui->blastQueriesTableWidget->setItem(row, 3, type);
    ui->blastQueriesTableWidget->setItem(row, 4, length);
    ui->blastQueriesTableWidget->setItem(row, 5, hits);
    ui->blastQueriesTableWidget->setItem(row, 6, percent);
    ui->blastQueriesTableWidget->setItem(row, 7, paths);
}


void BlastSearchDialog::fillHitsTable()
{
    ui->blastHitsTableWidget->clearContents();
    ui->blastHitsTableWidget->setSortingEnabled(false);

    int hitCount = g_blastSearch->m_allHits.size();
    ui->blastHitsTableWidget->setRowCount(hitCount);

    if (hitCount == 0)
        return;

    for (int i = 0; i < hitCount; ++i)
    {
        BlastHit * hit = g_blastSearch->m_allHits[i].data();
        BlastQuery * hitQuery = hit->m_query;

        //Hits for hidden queries will be not enabled.
        Qt::ItemFlag enabled = Qt::NoItemFlags;
        if (hitQuery->isShown())
            enabled = Qt::ItemIsEnabled;

        QTableWidgetItem * queryColour = new QTableWidgetItem(hitQuery->getColour().name());
        queryColour->setFlags(enabled);
        queryColour->setBackground(hitQuery->getColour());
        QTableWidgetItem * queryName = new QTableWidgetItem(hitQuery->getName());
        queryName->setFlags(enabled | Qt::ItemIsSelectable);
        QTableWidgetItem * nodeName = new QTableWidgetItem(hit->m_node->m_name);
        nodeName->setFlags(enabled | Qt::ItemIsSelectable);
        TableWidgetItemDouble * percentIdentity = new TableWidgetItemDouble(QString::number(hit->m_percentIdentity) + "%");
        percentIdentity->setFlags(enabled | Qt::ItemIsSelectable);
        TableWidgetItemInt * alignmentLength = new TableWidgetItemInt(formatIntForDisplay(hit->m_alignmentLength));
        alignmentLength->setFlags(enabled | Qt::ItemIsSelectable);
        TableWidgetItemInt * numberMismatches = new TableWidgetItemInt(formatIntForDisplay(hit->m_numberMismatches));
        numberMismatches->setFlags(enabled | Qt::ItemIsSelectable);
        TableWidgetItemInt * numberGapOpens = new TableWidgetItemInt(formatIntForDisplay(hit->m_numberGapOpens));
        numberGapOpens->setFlags(enabled | Qt::ItemIsSelectable);
        TableWidgetItemInt * queryStart = new TableWidgetItemInt(formatIntForDisplay(hit->m_queryStart));
        queryStart->setFlags(enabled | Qt::ItemIsSelectable);
        TableWidgetItemInt * queryEnd = new TableWidgetItemInt(formatIntForDisplay(hit->m_queryEnd));
        queryEnd->setFlags(enabled | Qt::ItemIsSelectable);
        TableWidgetItemInt * nodeStart = new TableWidgetItemInt(formatIntForDisplay(hit->m_nodeStart));
        nodeStart->setFlags(enabled | Qt::ItemIsSelectable);
        TableWidgetItemInt * nodeEnd = new TableWidgetItemInt(formatIntForDisplay(hit->m_nodeEnd));
        nodeEnd->setFlags(enabled | Qt::ItemIsSelectable);
        TableWidgetItemDouble * eValue = new TableWidgetItemDouble(QString::number(hit->m_eValue));
        eValue->setFlags(enabled | Qt::ItemIsSelectable);
        TableWidgetItemDouble * bitScore = new TableWidgetItemDouble(QString::number(hit->m_bitScore));
        bitScore->setFlags(enabled | Qt::ItemIsSelectable);

        ui->blastHitsTableWidget->setItem(i, 0, queryColour);
        ui->blastHitsTableWidget->setItem(i, 1, queryName);
        ui->blastHitsTableWidget->setItem(i, 2, nodeName);
        ui->blastHitsTableWidget->setItem(i, 3, percentIdentity);
        ui->blastHitsTableWidget->setItem(i, 4, alignmentLength);
        ui->blastHitsTableWidget->setItem(i, 5, numberMismatches);
        ui->blastHitsTableWidget->setItem(i, 6, numberGapOpens);
        ui->blastHitsTableWidget->setItem(i, 7, queryStart);
        ui->blastHitsTableWidget->setItem(i, 8, queryEnd);
        ui->blastHitsTableWidget->setItem(i, 9, nodeStart);
        ui->blastHitsTableWidget->setItem(i, 10, nodeEnd);
        ui->blastHitsTableWidget->setItem(i, 11, eValue);
        ui->blastHitsTableWidget->setItem(i, 12, bitScore);
    }

    ui->blastHitsTableWidget->resizeColumns();
    ui->blastHitsTableWidget->setEnabled(true);
    ui->blastHitsTableWidget->setSortingEnabled(true);
}

void BlastSearchDialog::buildBlastDatabaseInThread()
{
    buildBlastDatabase(true);
}

void BlastSearchDialog::buildBlastDatabase(bool separateThread)
{
    setUiStep(BLAST_DB_BUILD_IN_PROGRESS);

    if (!g_blastSearch->findProgram("makeblastdb", &m_makeblastdbCommand))
    {
        QMessageBox::warning(this, "Error", "The program makeblastdb was not found.  Please install NCBI BLAST to use this feature.");
        setUiStep(BLAST_DB_NOT_YET_BUILT);
        return;
    }

    QApplication::processEvents();

    MyProgressDialog * progress = new MyProgressDialog(this, "Building BLAST database...", separateThread, "Cancel build", "Cancelling build...",
                                                       "Clicking this button will stop the BLAST database from being "
                                                       "built.");
    progress->setWindowModality(Qt::WindowModal);
    progress->show();

    if (separateThread)
    {
        m_buildBlastDatabaseThread = new QThread;
        BuildBlastDatabaseWorker * buildBlastDatabaseWorker = new BuildBlastDatabaseWorker(m_makeblastdbCommand);
        buildBlastDatabaseWorker->moveToThread(m_buildBlastDatabaseThread);

        connect(progress, SIGNAL(halt()), this, SLOT(buildBlastDatabaseCancelled()));
        connect(m_buildBlastDatabaseThread, SIGNAL(started()), buildBlastDatabaseWorker, SLOT(buildBlastDatabase()));
        connect(buildBlastDatabaseWorker, SIGNAL(finishedBuild(QString)), m_buildBlastDatabaseThread, SLOT(quit()));
        connect(buildBlastDatabaseWorker, SIGNAL(finishedBuild(QString)), buildBlastDatabaseWorker, SLOT(deleteLater()));
        connect(buildBlastDatabaseWorker, SIGNAL(finishedBuild(QString)), this, SLOT(blastDatabaseBuildFinished(QString)));
        connect(m_buildBlastDatabaseThread, SIGNAL(finished()), m_buildBlastDatabaseThread, SLOT(deleteLater()));
        connect(m_buildBlastDatabaseThread, SIGNAL(finished()), progress, SLOT(deleteLater()));

        m_buildBlastDatabaseThread->start();
    }
    else
    {
        BuildBlastDatabaseWorker buildBlastDatabaseWorker(m_makeblastdbCommand);
        buildBlastDatabaseWorker.buildBlastDatabase();
        progress->close();
        delete progress;
        blastDatabaseBuildFinished(buildBlastDatabaseWorker.m_error);
    }
}



void BlastSearchDialog::blastDatabaseBuildFinished(QString error)
{
    if (error != "")
    {
        QMessageBox::warning(this, "Error", error);
        setUiStep(BLAST_DB_NOT_YET_BUILT);
    }
    else
        setUiStep(BLAST_DB_BUILT_BUT_NO_QUERIES);
}


void BlastSearchDialog::buildBlastDatabaseCancelled()
{
    g_blastSearch->m_cancelBuildBlastDatabase = true;
    if (g_blastSearch->m_makeblastdb != 0)
        g_blastSearch->m_makeblastdb->kill();
}

void BlastSearchDialog::loadBlastQueriesFromFastaFileButtonClicked()
{
    QStringList fullFileNames = QFileDialog::getOpenFileNames(this, "Load queries FASTA", g_memory->rememberedPath);

    if (fullFileNames.size() > 0) //User did not hit cancel
    {
        for (int i = 0; i < fullFileNames.size(); ++i)
            loadBlastQueriesFromFastaFile(fullFileNames.at(i));
    }
}

void BlastSearchDialog::loadBlastQueriesFromFastaFile(QString fullFileName)
{
    MyProgressDialog * progress = new MyProgressDialog(this, "Loading queries...", false);
    progress->setWindowModality(Qt::WindowModal);
    progress->show();

    int queriesLoaded = g_blastSearch->loadBlastQueriesFromFastaFile(fullFileName);
    if (queriesLoaded > 0)
    {
        fillQueriesTable();
        clearBlastHits();
        g_memory->rememberedPath = QFileInfo(fullFileName).absolutePath();
        setUiStep(READY_FOR_BLAST_SEARCH);
    }

    progress->close();
    delete progress;

    if (queriesLoaded == 0)
        QMessageBox::information(this, "No queries loaded", "No queries could be loaded from the specified file.");
}



void BlastSearchDialog::enterQueryManually()
{
    EnterOneBlastQueryDialog enterOneBlastQueryDialog(this);

    if (enterOneBlastQueryDialog.exec())
    {
        QString queryName = g_blastSearch->cleanQueryName(enterOneBlastQueryDialog.getName());
        g_blastSearch->m_blastQueries.addQuery(new BlastQuery(queryName,
                                                              enterOneBlastQueryDialog.getSequence()));
        fillQueriesTable();
        clearBlastHits();

        setUiStep(READY_FOR_BLAST_SEARCH);
    }
}



void BlastSearchDialog::clearAllQueries()
{
    g_blastSearch->m_blastQueries.clearAllQueries();
    ui->blastQueriesTableWidget->clearContents();
    ui->clearAllQueriesButton->setEnabled(false);

    while (ui->blastQueriesTableWidget->rowCount() > 0)
        ui->blastQueriesTableWidget->removeRow(0);

    clearBlastHits();
    setUiStep(BLAST_DB_BUILT_BUT_NO_QUERIES);
}


void BlastSearchDialog::clearSelectedQueries()
{
    //Use the table selection to figure out which queries are to be removed.
    std::vector<BlastQuery *> queriesToRemove;
    QItemSelectionModel * select = ui->blastQueriesTableWidget->selectionModel();
    QModelIndexList selection = select->selectedIndexes();
    QSet<int> rowsWithSelectionSet;
    for (int i = 0; i < selection.size(); ++i)
        rowsWithSelectionSet.insert(selection[i].row());
    QSet<int>::const_iterator i = rowsWithSelectionSet.constBegin();
    while (i != rowsWithSelectionSet.constEnd())
    {
        size_t queryToRemoveIndex = *i;
        if (queryToRemoveIndex < g_blastSearch->m_blastQueries.m_queries.size())
        {
            BlastQuery * queryToRemove = g_blastSearch->m_blastQueries.m_queries[queryToRemoveIndex];
            queriesToRemove.push_back(queryToRemove);
        }
        ++i;
    }

    if (queriesToRemove.size() == g_blastSearch->m_blastQueries.m_queries.size())
    {
        clearAllQueries();
        return;
    }

    g_blastSearch->clearSomeQueries(queriesToRemove);

    fillQueriesTable();
    if (g_blastSearch->m_allHits.size() > 0)
        fillHitsTable();
}

void BlastSearchDialog::runBlastSearchesInThread()
{
    runBlastSearches(true);
}


void BlastSearchDialog::runBlastSearches(bool separateThread)
{
    setUiStep(BLAST_SEARCH_IN_PROGRESS);

    if (!g_blastSearch->findProgram("blastn", &m_blastnCommand))
    {
        QMessageBox::warning(this, "Error", "The program blastn was not found.  Please install NCBI BLAST to use this feature.");
        setUiStep(READY_FOR_BLAST_SEARCH);
        return;
    }
    if (!g_blastSearch->findProgram("tblastn", &m_tblastnCommand))
    {
        QMessageBox::warning(this, "Error", "The program tblastn was not found.  Please install NCBI BLAST to use this feature.");
        setUiStep(READY_FOR_BLAST_SEARCH);
        return;
    }

    clearBlastHits();

    MyProgressDialog * progress = new MyProgressDialog(this, "Running BLAST search...", separateThread, "Cancel search", "Cancelling search...",
                                                       "Clicking this button will stop the BLAST search.");
    progress->setWindowModality(Qt::WindowModal);
    progress->show();

    if (separateThread)
    {
        m_blastSearchThread = new QThread;
        RunBlastSearchWorker * runBlastSearchWorker = new RunBlastSearchWorker(m_blastnCommand, m_tblastnCommand, ui->parametersLineEdit->text().simplified());
        runBlastSearchWorker->moveToThread(m_blastSearchThread);

        connect(progress, SIGNAL(halt()), this, SLOT(runBlastSearchCancelled()));
        connect(m_blastSearchThread, SIGNAL(started()), runBlastSearchWorker, SLOT(runBlastSearch()));
        connect(runBlastSearchWorker, SIGNAL(finishedSearch(QString)), m_blastSearchThread, SLOT(quit()));
        connect(runBlastSearchWorker, SIGNAL(finishedSearch(QString)), runBlastSearchWorker, SLOT(deleteLater()));
        connect(runBlastSearchWorker, SIGNAL(finishedSearch(QString)), this, SLOT(runBlastSearchFinished(QString)));
        connect(m_blastSearchThread, SIGNAL(finished()), m_blastSearchThread, SLOT(deleteLater()));
        connect(m_blastSearchThread, SIGNAL(finished()), progress, SLOT(deleteLater()));

        m_blastSearchThread->start();
    }
    else
    {
        RunBlastSearchWorker runBlastSearchWorker(m_blastnCommand, m_tblastnCommand, ui->parametersLineEdit->text().simplified());;
        runBlastSearchWorker.runBlastSearch();
        progress->close();
        delete progress;
        runBlastSearchFinished(runBlastSearchWorker.m_error);
    }
}



void BlastSearchDialog::runBlastSearchFinished(QString error)
{
    if (error != "")
    {
        QMessageBox::warning(this, "Error", error);
        setUiStep(READY_FOR_BLAST_SEARCH);
    }
    else
    {
        m_blastSearchConducted = true;
        fillTablesAfterBlastSearch();
        g_settings->blastSearchParameters = ui->parametersLineEdit->text().simplified();
        setUiStep(BLAST_SEARCH_COMPLETE);
    }
}


void BlastSearchDialog::runBlastSearchCancelled()
{
    g_blastSearch->m_cancelRunBlastSearch = true;
    if (g_blastSearch->m_blast != 0)
        g_blastSearch->m_blast->kill();
}



void BlastSearchDialog::queryCellChanged(int row, int column)
{
    //Suspend signals for this function, as it is might change
    //the cell value again if the new name isn't unique.
    ui->blastQueriesTableWidget->blockSignals(true);

    //If a query name was changed, then we actually adjust that query name.
    if (column == 2)
    {
        QString newName = ui->blastQueriesTableWidget->item(row, column)->text();
        BlastQuery * query = g_blastSearch->m_blastQueries.m_queries[row];

        QString uniqueName = g_blastSearch->m_blastQueries.renameQuery(query, newName);

        //It's possible that the user gave the query a non-unique name, in which
        //case we now have to adjust it.
        if (uniqueName != newName)
            ui->blastQueriesTableWidget->item(row, column)->setText(uniqueName);

        //Resize the query table columns, as the name new might take up more or less space.
        ui->blastQueriesTableWidget->resizeColumns();

        //Rebuild the hits table, if necessary, to show the new name.
        if (query->hasHits())
            fillHitsTable();
    }

    //If anything else was changed, we want to change it back to what it was.
    else
        makeQueryRow(row);

    ui->blastQueriesTableWidget->blockSignals(false);
}


void BlastSearchDialog::queryTableSelectionChanged()
{
    //If there are any selected items, then the 'Clear selected' button
    //should be enabled.
    QItemSelectionModel * select = ui->blastQueriesTableWidget->selectionModel();
    bool hasSelection = select->hasSelection();

    ui->clearSelectedQueriesButton->setEnabled(hasSelection);
}



void BlastSearchDialog::setUiStep(BlastUiState blastUiState)
{
    QPixmap tick(":/icons/tick-128.png");
    QPixmap tickScaled = tick.scaled(32, 32);

    switch (blastUiState)
    {
    case BLAST_DB_NOT_YET_BUILT:
        ui->step1Label->setEnabled(true);
        ui->buildBlastDatabaseButton->setEnabled(true);
        ui->step2Label->setEnabled(false);
        ui->loadQueriesFromFastaButton->setEnabled(false);
        ui->enterQueryManuallyButton->setEnabled(false);
        ui->blastQueriesTableWidget->setEnabled(false);
        ui->blastQueriesTableInfoText->setEnabled(false);
        ui->step3Label->setEnabled(false);
        ui->parametersLabel->setEnabled(false);
        ui->parametersLineEdit->setEnabled(false);
        ui->runBlastSearchButton->setEnabled(false);
        ui->clearAllQueriesButton->setEnabled(false);
        ui->clearSelectedQueriesButton->setEnabled(false);
        ui->hitsLabel->setEnabled(false);
        ui->step1TickLabel->setPixmap(QPixmap());
        ui->step2TickLabel->setPixmap(QPixmap());
        ui->step3TickLabel->setPixmap(QPixmap());
        ui->buildBlastDatabaseInfoText->setEnabled(true);
        ui->loadQueriesFromFastaInfoText->setEnabled(false);
        ui->enterQueryManuallyInfoText->setEnabled(false);
        ui->parametersInfoText->setEnabled(false);
        ui->startBlastSearchInfoText->setEnabled(false);
        ui->clearAllQueriesInfoText->setEnabled(false);
        ui->clearSelectedQueriesInfoText->setEnabled(false);
        ui->blastHitsTableWidget->setEnabled(false);
        break;

    case BLAST_DB_BUILD_IN_PROGRESS:
        ui->step1Label->setEnabled(true);
        ui->buildBlastDatabaseButton->setEnabled(false);
        ui->step2Label->setEnabled(false);
        ui->loadQueriesFromFastaButton->setEnabled(false);
        ui->enterQueryManuallyButton->setEnabled(false);
        ui->blastQueriesTableWidget->setEnabled(false);
        ui->blastQueriesTableInfoText->setEnabled(false);
        ui->step3Label->setEnabled(false);
        ui->parametersLabel->setEnabled(false);
        ui->parametersLineEdit->setEnabled(false);
        ui->runBlastSearchButton->setEnabled(false);
        ui->clearAllQueriesButton->setEnabled(false);
        ui->clearSelectedQueriesButton->setEnabled(false);
        ui->hitsLabel->setEnabled(false);
        ui->step1TickLabel->setPixmap(QPixmap());
        ui->step2TickLabel->setPixmap(QPixmap());
        ui->step3TickLabel->setPixmap(QPixmap());
        ui->buildBlastDatabaseInfoText->setEnabled(false);
        ui->loadQueriesFromFastaInfoText->setEnabled(false);
        ui->enterQueryManuallyInfoText->setEnabled(false);
        ui->parametersInfoText->setEnabled(false);
        ui->startBlastSearchInfoText->setEnabled(false);
        ui->clearAllQueriesInfoText->setEnabled(false);
        ui->clearSelectedQueriesInfoText->setEnabled(false);
        ui->blastHitsTableWidget->setEnabled(false);
        break;

    case BLAST_DB_BUILT_BUT_NO_QUERIES:
        ui->step1Label->setEnabled(true);
        ui->buildBlastDatabaseButton->setEnabled(false);
        ui->step2Label->setEnabled(true);
        ui->loadQueriesFromFastaButton->setEnabled(true);
        ui->enterQueryManuallyButton->setEnabled(true);
        ui->blastQueriesTableWidget->setEnabled(true);
        ui->blastQueriesTableInfoText->setEnabled(true);
        ui->step3Label->setEnabled(false);
        ui->parametersLabel->setEnabled(false);
        ui->parametersLineEdit->setEnabled(false);
        ui->runBlastSearchButton->setEnabled(false);
        ui->clearAllQueriesButton->setEnabled(false);
        ui->clearAllQueriesButton->setEnabled(false);
        ui->hitsLabel->setEnabled(false);
        ui->step1TickLabel->setPixmap(tickScaled);
        ui->step2TickLabel->setPixmap(QPixmap());
        ui->step3TickLabel->setPixmap(QPixmap());
        ui->buildBlastDatabaseInfoText->setEnabled(false);
        ui->loadQueriesFromFastaInfoText->setEnabled(true);
        ui->enterQueryManuallyInfoText->setEnabled(true);
        ui->parametersInfoText->setEnabled(false);
        ui->startBlastSearchInfoText->setEnabled(false);
        ui->clearSelectedQueriesInfoText->setEnabled(false);
        ui->clearSelectedQueriesInfoText->setEnabled(false);
        ui->blastHitsTableWidget->setEnabled(false);
        break;

    case READY_FOR_BLAST_SEARCH:
        ui->step1Label->setEnabled(true);
        ui->buildBlastDatabaseButton->setEnabled(false);
        ui->step2Label->setEnabled(true);
        ui->loadQueriesFromFastaButton->setEnabled(true);
        ui->enterQueryManuallyButton->setEnabled(true);
        ui->blastQueriesTableWidget->setEnabled(true);
        ui->blastQueriesTableInfoText->setEnabled(true);
        ui->step3Label->setEnabled(true);
        ui->parametersLabel->setEnabled(true);
        ui->parametersLineEdit->setEnabled(true);
        ui->runBlastSearchButton->setEnabled(true);
        ui->clearAllQueriesButton->setEnabled(true);
        queryTableSelectionChanged();
        ui->hitsLabel->setEnabled(false);
        ui->step1TickLabel->setPixmap(tickScaled);
        ui->step2TickLabel->setPixmap(tickScaled);
        ui->step3TickLabel->setPixmap(QPixmap());
        ui->buildBlastDatabaseInfoText->setEnabled(false);
        ui->loadQueriesFromFastaInfoText->setEnabled(true);
        ui->enterQueryManuallyInfoText->setEnabled(true);
        ui->parametersInfoText->setEnabled(true);
        ui->startBlastSearchInfoText->setEnabled(true);
        ui->clearAllQueriesInfoText->setEnabled(true);
        ui->clearSelectedQueriesInfoText->setEnabled(true);
        ui->blastHitsTableWidget->setEnabled(false);
        break;

    case BLAST_SEARCH_IN_PROGRESS:
        ui->step1Label->setEnabled(true);
        ui->buildBlastDatabaseButton->setEnabled(false);
        ui->step2Label->setEnabled(true);
        ui->loadQueriesFromFastaButton->setEnabled(true);
        ui->enterQueryManuallyButton->setEnabled(true);
        ui->blastQueriesTableWidget->setEnabled(true);
        ui->blastQueriesTableInfoText->setEnabled(true);
        ui->step3Label->setEnabled(true);
        ui->parametersLabel->setEnabled(true);
        ui->parametersLineEdit->setEnabled(true);
        ui->runBlastSearchButton->setEnabled(false);
        ui->clearAllQueriesButton->setEnabled(true);
        queryTableSelectionChanged();
        ui->hitsLabel->setEnabled(false);
        ui->step1TickLabel->setPixmap(tickScaled);
        ui->step2TickLabel->setPixmap(tickScaled);
        ui->step3TickLabel->setPixmap(QPixmap());
        ui->buildBlastDatabaseInfoText->setEnabled(false);
        ui->loadQueriesFromFastaInfoText->setEnabled(true);
        ui->enterQueryManuallyInfoText->setEnabled(true);
        ui->parametersInfoText->setEnabled(true);
        ui->startBlastSearchInfoText->setEnabled(true);
        ui->clearAllQueriesInfoText->setEnabled(true);
        ui->clearSelectedQueriesInfoText->setEnabled(true);
        ui->blastHitsTableWidget->setEnabled(false);
        break;

    case BLAST_SEARCH_COMPLETE:
        ui->step1Label->setEnabled(true);
        ui->buildBlastDatabaseButton->setEnabled(false);
        ui->step2Label->setEnabled(true);
        ui->loadQueriesFromFastaButton->setEnabled(true);
        ui->enterQueryManuallyButton->setEnabled(true);
        ui->blastQueriesTableWidget->setEnabled(true);
        ui->blastQueriesTableInfoText->setEnabled(true);
        ui->step3Label->setEnabled(true);
        ui->parametersLabel->setEnabled(true);
        ui->parametersLineEdit->setEnabled(true);
        ui->runBlastSearchButton->setEnabled(true);
        ui->clearAllQueriesButton->setEnabled(true);
        queryTableSelectionChanged();
        ui->hitsLabel->setEnabled(true);
        ui->step1TickLabel->setPixmap(tickScaled);
        ui->step2TickLabel->setPixmap(tickScaled);
        ui->step3TickLabel->setPixmap(tickScaled);
        ui->buildBlastDatabaseInfoText->setEnabled(false);
        ui->loadQueriesFromFastaInfoText->setEnabled(true);
        ui->enterQueryManuallyInfoText->setEnabled(true);
        ui->parametersInfoText->setEnabled(true);
        ui->startBlastSearchInfoText->setEnabled(true);
        ui->clearAllQueriesInfoText->setEnabled(true);
        ui->clearSelectedQueriesInfoText->setEnabled(true);
        ui->blastHitsTableWidget->setEnabled(true);
        break;
    }
}



void BlastSearchDialog::setInfoTexts()
{
    QString settingsDialogTitle = "settings";
#ifdef Q_OS_MAC
    settingsDialogTitle = "preferences";
#endif

    ui->buildBlastDatabaseInfoText->setInfoText("This step runs makeblastdb on the contig sequences, "
                                                "preparing them for a BLAST search.<br><br>"
                                                "The database files generated are temporary and will "
                                                "be deleted when Bandage is closed.");
    ui->loadQueriesFromFastaInfoText->setInfoText("Click this button to load a FASTA file. Each "
                                                  "sequence in the FASTA file will be a separate "
                                                  "query.");
    ui->enterQueryManuallyInfoText->setInfoText("Click this button to type or paste a single query sequence.");
    ui->parametersInfoText->setInfoText("You may add additional blastn/tblastn parameters here, exactly as they "
                                        "would be typed at the command line.");
    ui->startBlastSearchInfoText->setInfoText("Click this to conduct search for the above "
                                              "queries on the graph nodes.<br><br>"
                                              "If no parameters were added above, this will run:<br>"
                                              "blastn -query queries.fasta -db all_nodes.fasta -outfmt 6<br><br>"
                                              "If, for example, '-evalue 0.01' was entered in the above "
                                              "parameters field, then this will run:<br>"
                                              "blastn -query queries.fasta -db all_nodes.fasta -outfmt 6 -evalue 0.01<br><br>"
                                              "For protein queries, tblastn will be used instead of blastn.");
    ui->clearSelectedQueriesInfoText->setInfoText("Click this button to remove any selected queries in the below list.");
    ui->clearAllQueriesInfoText->setInfoText("Click this button to remove all queries in the below list.");
    ui->blastQueriesTableInfoText->setInfoText("The BLAST queries are displayed in this table. Before a BLAST search "
                                               "is run, some information about the queries is not yet available and "
                                               "will show a dash.<br><br>"
                                               "Colour: Each query is automatically assigned a colour which is used for the "
                                               "'Blast hits (solid)' graph colour scheme. This colour can be changed by "
                                               "clicking in the table cell.<br><br>"
                                               "Show: if this box is ticked, the query's hits will be visible in the table "
                                               "below and it will be able to be viewed on the graph. If this box is not "
                                               "ticked, the query's hits will be hidden in the table below and the query "
                                               "will be hidden on the graph.<br><br>"
                                               "Query name: If a query is loaded from a FASTA file, its name is the sequence "
                                               "ID (the text between the '>' and the first space in the description line). "
                                               "Query names are editable by double clicking in their table cell.<br><br>"
                                               "Type: This is either 'nucl' for nucleotide sequences or 'prot' for protein "
                                               "sequences. Nucleotide sequences will be searched for using blastn, while "
                                               "protein sequences will be search for with tblastn. Both types can be used "
                                               "simultaneously.<br><br>"
                                               "Length: This is the length of the query, in bases (for nucleotide queries) "
                                               "or amino acids (for protein queries).<br><br>"
                                               "Hits: This is the number of BLAST hits acquired for the query.<br><br>"
                                               "Percent found: This is the total fraction of the query captured by all of "
                                               "the BLAST hits. However, the hits may not be proximal to each other. For "
                                               "example, if the first half a query was found in one part of the graph and "
                                               "the second half in a different part, this value would be 100%.<br><br>"
                                               "Nodes/paths: These are the possible paths through the graph which "
                                               "represent the entire query, as defined by the 'BLAST query paths' " + settingsDialogTitle + ". "
                                               "If a query is contained within a single node, the path will just be one "
                                               "node name. If the query spans multiple nodes, the path will be a comma-"
                                               "delimited list. If there are multiple paths, they are separated with "
                                               "semicolons.");
}


