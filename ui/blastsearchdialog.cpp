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
#include "../graph/debruijnnode.h"
#include <QMessageBox>
#include <QDir>
#include "enteroneblastquerydialog.h"
#include "../graph/assemblygraph.h"
#include "../blast/blastsearch.h"
#include <QProcessEnvironment>

BlastSearchDialog::BlastSearchDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::BlastSearchDialog), m_makeblastdbCommand("makeblastdb"), m_blastnCommand("blastn")
{
    ui->setupUi(this);

    //If a BLAST database already exists, move to step 2.
    QFile databaseFile(g_tempDirectory + "all_nodes.fasta");
    if (databaseFile.exists())
        setUiStep(2);

    //If there isn't a BLAST database, clear the entire temporary directory
    //and move to step 1.
    else
    {
        emptyTempDirectory();
        setUiStep(1);
    }

    //If queries already exist, display them and move to step 3.
    if (g_blastSearch->m_blastQueries.m_queries.size() > 0)
    {
        fillQueriesTable();
        setUiStep(3);
    }

    //If results already exist, display them and move to step 3.
    if (g_blastSearch->m_hits.size() > 0)
    {
        fillHitsTable();
        setUiStep(3);
    }

    setInfoTexts();

    connect(ui->buildBlastDatabaseButton, SIGNAL(clicked()), this, SLOT(buildBlastDatabase1()));
    connect(ui->loadQueriesFromFastaButton, SIGNAL(clicked()), this, SLOT(loadBlastQueriesFromFastaFile()));
    connect(ui->enterQueryManuallyButton, SIGNAL(clicked()), this, SLOT(enterQueryManually()));
    connect(ui->clearQueriesButton, SIGNAL(clicked()), this, SLOT(clearQueries()));
    connect(ui->startBlastSearchButton, SIGNAL(clicked()), this, SLOT(runBlastSearch()));
}

BlastSearchDialog::~BlastSearchDialog()
{
    delete ui;
}


void BlastSearchDialog::readFastaFile(QString filename, std::vector<QString> * names, std::vector<QString> * sequences)
{
    QFile inputFile(filename);
    if (inputFile.open(QIODevice::ReadOnly))
    {
        QString name = "";
        QString sequence = "";

        QTextStream in(&inputFile);
        while (!in.atEnd())
        {
            QString line = in.readLine();

            if (line.length() == 0)
                continue;

            if (line.at(0) == '>')
            {
                //If there is a current sequence, add it to the vectors now.
                if (name.length() > 0)
                {
                    names->push_back(name);
                    sequences->push_back(sequence);
                }

                line.remove(0, 1); //Remove '>' from start
                name = line;
                sequence = "";
            }

            else //It's a sequence line
                sequence += line.simplified();
        }

        //Add the last target to the results now.
        if (name.length() > 0)
        {
            names->push_back(name);
            sequences->push_back(sequence);
        }

        inputFile.close();
    }
}

void BlastSearchDialog::clearBlastHits()
{
    g_blastSearch->m_hits.clear();
    g_blastSearch->m_blastQueries.clearSearchResults();
    ui->blastHitsTableView->setModel(0);
}

void BlastSearchDialog::loadBlastHits(QString blastHits)
{
    QStringList blastHitList = blastHits.split("\n", QString::SkipEmptyParts);

    if (blastHitList.size() == 0)
    {
        QMessageBox::information(this, "No hits", "No BLAST hits were found for the given queries and parameters.");
        return;
    }

    for (int i = 0; i < blastHitList.size(); ++i)
    {
        QString hit = blastHitList[i];
        QStringList alignmentParts = hit.split('\t');

        QString queryName = alignmentParts[0];
        QString nodeLabel = alignmentParts[1];
        int queryStart = alignmentParts[6].toInt();
        int queryEnd = alignmentParts[7].toInt();
        int nodeStart = alignmentParts[8].toInt();
        int nodeEnd = alignmentParts[9].toInt();
        QString eValue = alignmentParts[10];

        //Only save BLAST hits that are on forward strands.
        if (nodeStart > nodeEnd)
            continue;

        int nodeNumber = getNodeNumberFromString(nodeLabel);
        DeBruijnNode * node;
        if (g_assemblyGraph->m_deBruijnGraphNodes.contains(nodeNumber))
            node = g_assemblyGraph->m_deBruijnGraphNodes[nodeNumber];
        else
            return;

        BlastQuery * query = g_blastSearch->m_blastQueries.getQueryFromName(queryName);
        if (query == 0)
            return;

        g_blastSearch->m_hits.push_back(BlastHit(node, nodeStart, nodeEnd,
                                                        query, queryStart, queryEnd, eValue));

        ++(query->m_hits);
    }

    fillQueriesTable();
    fillHitsTable();
}


int BlastSearchDialog::getNodeNumberFromString(QString nodeString)
{
    QStringList nodeStringParts = nodeString.split("_");
    return nodeStringParts[1].toInt();
}



void BlastSearchDialog::fillQueriesTable()
{
    int queryCount = int(g_blastSearch->m_blastQueries.m_queries.size());
    if (queryCount == 0)
        return;

    QStandardItemModel * model = new QStandardItemModel(queryCount, 3, this); //3 Columns
    model->setHorizontalHeaderItem(0, new QStandardItem("Target name"));
    model->setHorizontalHeaderItem(1, new QStandardItem("Target length"));
    model->setHorizontalHeaderItem(2, new QStandardItem("Hits"));
    for (int i = 0; i < queryCount; ++i)
    {
        BlastQuery * query = &(g_blastSearch->m_blastQueries.m_queries[i]);
        model->setItem(i, 0, new QStandardItem(query->m_name));
        model->setItem(i, 1, new QStandardItem(formatIntForDisplay(query->m_length)));

        //If the search hasn't yet been run, don't put a number in the hits column.
        if (query->m_searchedFor)
            model->setItem(i, 2, new QStandardItem(formatIntForDisplay(query->m_hits)));
        else
            model->setItem(i, 2, new QStandardItem("-"));
    }
    ui->blastQueriesTableView->setModel(model);
    ui->blastQueriesTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    setUiStep(3);
}


void BlastSearchDialog::fillHitsTable()
{
    int hitCount = int(g_blastSearch->m_hits.size());

    QStandardItemModel * model = new QStandardItemModel(hitCount, 8, this); //8 Columns
    model->setHorizontalHeaderItem(0, new QStandardItem("Node number"));
    model->setHorizontalHeaderItem(1, new QStandardItem("Node length"));
    model->setHorizontalHeaderItem(2, new QStandardItem("Node start"));
    model->setHorizontalHeaderItem(3, new QStandardItem("Node end"));
    model->setHorizontalHeaderItem(4, new QStandardItem("Query name"));
    model->setHorizontalHeaderItem(5, new QStandardItem("Query start"));
    model->setHorizontalHeaderItem(6, new QStandardItem("Query end"));
    model->setHorizontalHeaderItem(7, new QStandardItem("E-value"));

    for (int i = 0; i < hitCount; ++i)
    {
        BlastHit * hit = &(g_blastSearch->m_hits[i]);
        model->setItem(i, 0, new QStandardItem(formatIntForDisplay(hit->m_node->m_number)));
        model->setItem(i, 1, new QStandardItem(formatIntForDisplay(hit->m_node->m_length)));
        model->setItem(i, 2, new QStandardItem(formatIntForDisplay(hit->m_nodeStart)));
        model->setItem(i, 3, new QStandardItem(formatIntForDisplay(hit->m_nodeEnd)));
        model->setItem(i, 4, new QStandardItem(hit->m_query->m_name));
        model->setItem(i, 5, new QStandardItem(formatIntForDisplay(hit->m_queryStart)));
        model->setItem(i, 6, new QStandardItem(formatIntForDisplay(hit->m_queryEnd)));
        model->setItem(i, 7, new QStandardItem(hit->m_eValue));
    }
    ui->blastHitsTableView->setModel(model);
    ui->blastHitsTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    ui->blastHitsTableView->setEnabled(true);
}


void BlastSearchDialog::buildBlastDatabase1()
{
    QString findMakeblastdbCommand = "which makeblastdb";
#ifdef Q_OS_WIN32
    findMakeblastdbCommand = "WHERE makeblastdb";
#endif

    QProcess findMakeblastdb;

    //On Mac, it's necessary to do some stuff with the PATH variable in order
    //for which to work.
#ifdef Q_OS_MAC
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    QStringList envlist = env.toStringList();
    envlist.replaceInStrings(QRegularExpression("^(?i)PATH=(.*)"), "PATH=/opt/local/bin:/usr/local/bin:$HOME/bin:\\1");
    findMakeblastdb.setEnvironment(envlist);
#endif

    findMakeblastdb.start(findMakeblastdbCommand);
    findMakeblastdb.waitForFinished();

    //On Mac, the command for makeblastdb needs to be the absolute path.
#ifdef Q_OS_MAC
    m_makeblastdbCommand = QString(findMakeblastdb.readAll()).simplified();
#endif

    if (findMakeblastdb.exitCode() != 0)
    {
        QMessageBox::warning(this, "Error", "The program makeblastdb was not found.  Please install NCBI BLAST to use this feature.");
        return;
    }

    ui->buildBlastDatabaseButton->setEnabled(false);
    ui->buildBlastDatabaseInfoText->setEnabled(false);

    QApplication::processEvents();

    emit createAllNodesFasta(g_tempDirectory, false);
}


void BlastSearchDialog::buildBlastDatabase2()
{
    QProcess makeblastdb;
    makeblastdb.start(m_makeblastdbCommand + " -in " + g_tempDirectory + "all_nodes.fasta " + "-dbtype nucl");
    makeblastdb.waitForFinished();

    if (makeblastdb.exitCode() != 0)
    {
        QMessageBox::warning(this, "Error", "There was a problem building the BLAST database.");
        return;
    }

    setUiStep(2);
}


void BlastSearchDialog::loadBlastQueriesFromFastaFile()
{
    QString fullFileName = QFileDialog::getOpenFileName(this, "Load queries FASTA");

    if (fullFileName != "") //User did not hit cancel
    {
        std::vector<QString> queryNames;
        std::vector<QString> querySequences;
        readFastaFile(fullFileName, &queryNames, &querySequences);

        for (size_t i = 0; i < queryNames.size(); ++i)
            g_blastSearch->m_blastQueries.addQuery(BlastQuery(queryNames[i].replace(" ", "_"),
                                                              querySequences[i]));
        fillQueriesTable();
        clearBlastHits();
    }
}


void BlastSearchDialog::enterQueryManually()
{
    EnterOneBlastQueryDialog enterOneBlastQueryDialog(this);

    if (enterOneBlastQueryDialog.exec())
    {
        g_blastSearch->m_blastQueries.addQuery(BlastQuery(enterOneBlastQueryDialog.getName(),
                                                          enterOneBlastQueryDialog.getSequence()));
        fillQueriesTable();
        clearBlastHits();
    }
}



void BlastSearchDialog::clearQueries()
{
    g_blastSearch->m_blastQueries.clearQueries();
    ui->blastQueriesTableView->setModel(0);
    ui->clearQueriesButton->setEnabled(false);

    clearBlastHits();
    setUiStep(2);
}



void BlastSearchDialog::runBlastSearch()
{    
    QString findBlastnCommand = "which blastn";
#ifdef Q_OS_WIN32
    findBlastnCommand = "WHERE blastn";
#endif

    QProcess findBlastn;

    //On Mac, it's necessary to do some stuff with the PATH variable in order
    //for which to work.
#ifdef Q_OS_MAC
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    QStringList envlist = env.toStringList();
    envlist.replaceInStrings(QRegularExpression("^(?i)PATH=(.*)"), "PATH=/opt/local/bin:/usr/local/bin:$HOME/bin:\\1");
    findBlastn.setEnvironment(envlist);
#endif

    findBlastn.start(findBlastnCommand);
    findBlastn.waitForFinished();

    //On Mac, the command for makeblastdb needs to be the absolute path.
#ifdef Q_OS_MAC
    m_blastnCommand = QString(findBlastn.readAll()).simplified();
#endif

    if (findBlastn.exitCode() != 0)
    {
        QMessageBox::warning(this, "Error", "The program blastn was not found.  Please install NCBI BLAST to use this feature.");
        return;
    }

    ui->startBlastSearchButton->setEnabled(false);
    ui->parametersInfoText->setEnabled(false);
    ui->parametersLineEdit->setEnabled(false);
    ui->parametersLabel->setEnabled(false);

    QApplication::processEvents();

    QProcess blastn;
    QString extraCommandLineOptions = ui->parametersLineEdit->text().simplified() + " ";
    QString fullBlastnCommand = m_blastnCommand + " -query " + g_tempDirectory + "queries.fasta -db " + g_tempDirectory + "all_nodes.fasta -outfmt 6 " + extraCommandLineOptions;

    blastn.start(fullBlastnCommand);
    blastn.waitForFinished();

    QString blastHits = blastn.readAll();

    if (blastn.exitCode() != 0)
    {
        QMessageBox::warning(this, "Error", "There was a problem running the BLAST search.");

        ui->startBlastSearchButton->setEnabled(true);
        ui->parametersInfoText->setEnabled(true);
        ui->parametersLineEdit->setEnabled(true);
        ui->parametersLabel->setEnabled(true);

        return;
    }

    clearBlastHits();
    g_blastSearch->m_blastQueries.searchOccurred();
    loadBlastHits(blastHits);
    setUiStep(4);
}



void BlastSearchDialog::setUiStep(int step)
{
    QPixmap tick(":/icons/tick-128.png");
    QPixmap tickScaled = tick.scaled(32, 32);

    switch (step)
    {
    //Step 1 is for when the BLAST database has not yet been made.
    case 1:
        ui->step1Label->setEnabled(true);
        ui->buildBlastDatabaseButton->setEnabled(true);
        ui->step2Label->setEnabled(false);
        ui->loadQueriesFromFastaButton->setEnabled(false);
        ui->enterQueryManuallyButton->setEnabled(false);
        ui->blastQueriesTableView->setEnabled(false);
        ui->step3Label->setEnabled(false);
        ui->parametersLabel->setEnabled(false);
        ui->parametersLineEdit->setEnabled(false);
        ui->startBlastSearchButton->setEnabled(false);
        ui->clearQueriesButton->setEnabled(false);
        ui->hitsLabel->setEnabled(false);
        ui->step1TickLabel->setPixmap(QPixmap());
        ui->step2TickLabel->setPixmap(QPixmap());
        ui->step3TickLabel->setPixmap(QPixmap());
        ui->buildBlastDatabaseInfoText->setEnabled(true);
        ui->loadQueriesFromFastaInfoText->setEnabled(false);
        ui->enterQueryManuallyInfoText->setEnabled(false);
        ui->parametersInfoText->setEnabled(false);
        ui->startBlastSearchInfoText->setEnabled(false);
        ui->clearQueriesInfoText->setEnabled(false);
        break;

    //Step 2 is for loading queries
    case 2:
        ui->step1Label->setEnabled(true);
        ui->buildBlastDatabaseButton->setEnabled(false);
        ui->step2Label->setEnabled(true);
        ui->loadQueriesFromFastaButton->setEnabled(true);
        ui->enterQueryManuallyButton->setEnabled(true);
        ui->blastQueriesTableView->setEnabled(true);
        ui->step3Label->setEnabled(false);
        ui->parametersLabel->setEnabled(false);
        ui->parametersLineEdit->setEnabled(false);
        ui->startBlastSearchButton->setEnabled(false);
        ui->clearQueriesButton->setEnabled(false);
        ui->hitsLabel->setEnabled(false);
        ui->step1TickLabel->setPixmap(tickScaled);
        ui->step2TickLabel->setPixmap(QPixmap());
        ui->step3TickLabel->setPixmap(QPixmap());
        ui->buildBlastDatabaseInfoText->setEnabled(false);
        ui->loadQueriesFromFastaInfoText->setEnabled(true);
        ui->enterQueryManuallyInfoText->setEnabled(true);
        ui->parametersInfoText->setEnabled(false);
        ui->startBlastSearchInfoText->setEnabled(false);
        ui->clearQueriesInfoText->setEnabled(false);
        break;

    //Step 3 is for running the BLAST search
    case 3:
        ui->step1Label->setEnabled(true);
        ui->buildBlastDatabaseButton->setEnabled(false);
        ui->step2Label->setEnabled(true);
        ui->loadQueriesFromFastaButton->setEnabled(true);
        ui->enterQueryManuallyButton->setEnabled(true);
        ui->blastQueriesTableView->setEnabled(true);
        ui->step3Label->setEnabled(true);
        ui->parametersLabel->setEnabled(true);
        ui->parametersLineEdit->setEnabled(true);
        ui->startBlastSearchButton->setEnabled(true);
        ui->clearQueriesButton->setEnabled(true);
        ui->hitsLabel->setEnabled(false);
        ui->step1TickLabel->setPixmap(tickScaled);
        ui->step2TickLabel->setPixmap(tickScaled);
        ui->step3TickLabel->setPixmap(QPixmap());
        ui->buildBlastDatabaseInfoText->setEnabled(false);
        ui->loadQueriesFromFastaInfoText->setEnabled(true);
        ui->enterQueryManuallyInfoText->setEnabled(true);
        ui->parametersInfoText->setEnabled(true);
        ui->startBlastSearchInfoText->setEnabled(true);
        ui->clearQueriesInfoText->setEnabled(true);
        break;

    //Step 4 is after the BLAST search has been run.
    case 4:
        ui->step1Label->setEnabled(true);
        ui->buildBlastDatabaseButton->setEnabled(false);
        ui->step2Label->setEnabled(true);
        ui->loadQueriesFromFastaButton->setEnabled(true);
        ui->enterQueryManuallyButton->setEnabled(true);
        ui->blastQueriesTableView->setEnabled(true);
        ui->step3Label->setEnabled(true);
        ui->parametersLabel->setEnabled(true);
        ui->parametersLineEdit->setEnabled(true);
        ui->startBlastSearchButton->setEnabled(true);
        ui->clearQueriesButton->setEnabled(true);
        ui->hitsLabel->setEnabled(true);
        ui->step1TickLabel->setPixmap(tickScaled);
        ui->step2TickLabel->setPixmap(tickScaled);
        ui->step3TickLabel->setPixmap(tickScaled);
        ui->buildBlastDatabaseInfoText->setEnabled(false);
        ui->loadQueriesFromFastaInfoText->setEnabled(true);
        ui->enterQueryManuallyInfoText->setEnabled(true);
        ui->parametersInfoText->setEnabled(true);
        ui->startBlastSearchInfoText->setEnabled(true);
        ui->clearQueriesInfoText->setEnabled(true);
        break;
    }
}



void BlastSearchDialog::setInfoTexts()
{
    ui->buildBlastDatabaseInfoText->setInfoText("This step runs makeblastdb on the contig sequences, "
                                                "preparing them for a BLAST search.<br><br>"
                                                "The database files generated are temporary and will "
                                                "be deleted when Bandage is closed.");
    ui->loadQueriesFromFastaInfoText->setInfoText("Click this button to load a FASTA file.  Each "
                                                  "sequence in the FASTA file will be a separate "
                                                  "query.");
    ui->enterQueryManuallyInfoText->setInfoText("Click this button to type or paste a single query sequence.");
    ui->parametersInfoText->setInfoText("You may add additional blastn parameters here, exactly as they "
                                        "would be typed at the command line.");
    ui->startBlastSearchInfoText->setInfoText("Click this to conduct a blastn search for the above "
                                              "queries on the graph nodes.<br><br>"
                                              "If no parameters were added above, this will run:<br>"
                                              "blastn -query queries.fasta -db all_nodes.fasta -outfmt 6<br><br>"
                                              "If, for example, '-evalue 0.01' was entered in the above "
                                              "parameters field, then this will run:<br>"
                                              "blastn -query queries.fasta -db all_nodes.fasta -outfmt 6 -evalue 0.01");
    ui->clearQueriesInfoText->setInfoText("Click this button to remove all queries in the below list.");
}
