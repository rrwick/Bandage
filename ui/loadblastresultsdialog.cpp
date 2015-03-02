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


#include "loadblastresultsdialog.h"
#include "ui_loadblastresultsdialog.h"

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

LoadBlastResultsDialog::LoadBlastResultsDialog(QMap<int, DeBruijnNode *> * deBruijnGraphNodes,
                                               QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LoadBlastResultsDialog),
    m_deBruijnGraphNodes(deBruijnGraphNodes)
{
    ui->setupUi(this);

    //If a BLAST database already exists, move to step 2.
    QString checkForQueryFileCommmand = "test -e " + g_tempDirectory + "all_nodes.fasta";
    bool databaseExists =  system(checkForQueryFileCommmand.toLocal8Bit().constData()) == 0;
    if (databaseExists)
        setUiStep(2);

    //If there isn't a BLAST database, clear the entire temporary directory
    //and move to step 1.
    else
    {
        QString clearTempDirCommand = "rm -r " + g_tempDirectory + "*";
        system(clearTempDirCommand.toLocal8Bit().constData());

        setUiStep(1);
    }

    //If queries already exist, display them and move to step 3.
    if (g_blastSearchResults->m_queries.size() > 0)
    {
        fillQueriesTable();
        setUiStep(3);
    }

    //If results already exist, display them and move to step 3.
    if (g_blastSearchResults->m_hits.size() > 0)
    {
        fillHitsTable();
        setUiStep(3);
    }

    connect(ui->buildBlastDatabaseButton, SIGNAL(clicked()), this, SLOT(buildBlastDatabase1()));
    connect(ui->loadQueriesFromFastaButton, SIGNAL(clicked()), this, SLOT(loadBlastQueriesFromFastaFile()));
    connect(ui->enterQueryManuallyButton, SIGNAL(clicked()), this, SLOT(enterQueryManually()));
    connect(ui->clearQueriesButton, SIGNAL(clicked()), this, SLOT(clearQueries()));
    connect(ui->startBlastSearchButton, SIGNAL(clicked()), this, SLOT(runBlastSearch()));
}

LoadBlastResultsDialog::~LoadBlastResultsDialog()
{
    delete ui;


}

void LoadBlastResultsDialog::loadBlastQueries()
{
    g_blastSearchResults->m_queries.clear();
    ui->blastQueriesTableView->setModel(0);
    ui->clearQueriesButton->setEnabled(false);

    std::vector<QString> queryNames;
    std::vector<QString> querySequences;
    readFastaFile(g_tempDirectory + "queries.fasta", &queryNames, &querySequences);

    for (size_t i = 0; i < queryNames.size(); ++i)
        g_blastSearchResults->m_queries.push_back(BlastQuery(queryNames[i], querySequences[i].length()));
}

void LoadBlastResultsDialog::readFastaFile(QString filename, std::vector<QString> * names, std::vector<QString> * sequences)
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

void LoadBlastResultsDialog::clearBlastHits()
{
    g_blastSearchResults->m_hits.clear();
    for (size_t i = 0 ; i < g_blastSearchResults->m_queries.size(); ++i)
        g_blastSearchResults->m_queries[i].m_hits = 0;
    ui->blastHitsTableView->setModel(0);
}

void LoadBlastResultsDialog::loadBlastHits()
{
    clearBlastHits();

    QFile inputFile(g_tempDirectory + "blast_results");
    if (inputFile.open(QIODevice::ReadOnly))
    {
        if (inputFile.size() == 0)
        {
            QMessageBox::information(this, "No hits", "No BLAST hits were found for the given queries and parameters.");
            return;
        }

        QTextStream in(&inputFile);
        while (!in.atEnd())
        {
            QString line = in.readLine();
            QStringList alignmentParts = line.split('\t');

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
            if (m_deBruijnGraphNodes->contains(nodeNumber))
                node = (*m_deBruijnGraphNodes)[nodeNumber];
            else
                return;

            BlastQuery * query = g_blastSearchResults->getQueryFromName(queryName);
            if (query == 0)
                return;

            g_blastSearchResults->m_hits.push_back(BlastHit(node, nodeStart, nodeEnd,
                                                            query, queryStart, queryEnd, eValue));

            ++(query->m_hits);
        }
    }

    fillQueriesTable();
    fillHitsTable();
}


int LoadBlastResultsDialog::getNodeNumberFromString(QString nodeString)
{
    QStringList nodeStringParts = nodeString.split("_");
    return nodeStringParts[1].toInt();
}



void LoadBlastResultsDialog::fillQueriesTable()
{
    size_t queryCount = g_blastSearchResults->m_queries.size();
    if (queryCount == 0)
        return;

    QStandardItemModel * model = new QStandardItemModel(queryCount, 3, this); //3 Columns
    model->setHorizontalHeaderItem(0, new QStandardItem("Target name"));
    model->setHorizontalHeaderItem(1, new QStandardItem("Target length"));
    model->setHorizontalHeaderItem(2, new QStandardItem("Hits"));
    for (size_t i = 0; i < queryCount; ++i)
    {
        model->setItem(i, 0, new QStandardItem(g_blastSearchResults->m_queries[i].m_name));
        model->setItem(i, 1, new QStandardItem(formatIntForDisplay(g_blastSearchResults->m_queries[i].m_length)));
        model->setItem(i, 2, new QStandardItem(formatIntForDisplay(g_blastSearchResults->m_queries[i].m_hits)));
    }
    ui->blastQueriesTableView->setModel(model);
    ui->blastQueriesTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    setUiStep(3);
}


void LoadBlastResultsDialog::fillHitsTable()
{
    QStandardItemModel * model = new QStandardItemModel(g_blastSearchResults->m_hits.size(), 8, this); //8 Columns
    model->setHorizontalHeaderItem(0, new QStandardItem("Node number"));
    model->setHorizontalHeaderItem(1, new QStandardItem("Node length"));
    model->setHorizontalHeaderItem(2, new QStandardItem("Node start"));
    model->setHorizontalHeaderItem(3, new QStandardItem("Node end"));
    model->setHorizontalHeaderItem(4, new QStandardItem("Query name"));
    model->setHorizontalHeaderItem(5, new QStandardItem("Query start"));
    model->setHorizontalHeaderItem(6, new QStandardItem("Query end"));
    model->setHorizontalHeaderItem(7, new QStandardItem("E-value"));

    for (size_t i = 0; i < g_blastSearchResults->m_hits.size(); ++i)
    {
        model->setItem(i, 0, new QStandardItem(formatIntForDisplay(g_blastSearchResults->m_hits[i].m_node->m_number)));
        model->setItem(i, 1, new QStandardItem(formatIntForDisplay(g_blastSearchResults->m_hits[i].m_node->m_length)));
        model->setItem(i, 2, new QStandardItem(formatIntForDisplay(g_blastSearchResults->m_hits[i].m_nodeStart)));
        model->setItem(i, 3, new QStandardItem(formatIntForDisplay(g_blastSearchResults->m_hits[i].m_nodeEnd)));
        model->setItem(i, 4, new QStandardItem(g_blastSearchResults->m_hits[i].m_query->m_name));
        model->setItem(i, 5, new QStandardItem(formatIntForDisplay(g_blastSearchResults->m_hits[i].m_queryStart)));
        model->setItem(i, 6, new QStandardItem(formatIntForDisplay(g_blastSearchResults->m_hits[i].m_queryEnd)));
        model->setItem(i, 7, new QStandardItem(g_blastSearchResults->m_hits[i].m_eValue));
    }
    ui->blastHitsTableView->setModel(model);
    ui->blastHitsTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    ui->blastHitsTableView->setEnabled(true);
}


void LoadBlastResultsDialog::buildBlastDatabase1()
{
    if (!system(NULL))
    {
        QMessageBox::warning(this, "Error", "Bandage was unable to access the system's command line.");
        return;
    }

    if (system("which makeblastdb") != 0)
    {
        QMessageBox::warning(this, "Error", "The program makeblastdb was not found.  Please install NCBI BLAST to use this feature.");
        return;
    }

    emit createAllNodesFasta(g_tempDirectory);
}


void LoadBlastResultsDialog::buildBlastDatabase2()
{
    QString makeBlastDbCommand = "makeblastdb -in " + g_tempDirectory + "all_nodes.fasta " + "-dbtype nucl";
    if (system(makeBlastDbCommand.toLocal8Bit().constData()) != 0)
    {
        QMessageBox::warning(this, "Error", "There was a problem building the BLAST database.");
        return;
    }

    setUiStep(2);
    makeQueryFile();
}


void LoadBlastResultsDialog::loadBlastQueriesFromFastaFile()
{
    QString fullFileName = QFileDialog::getOpenFileName(this, "Load queries FASTA");

    if (fullFileName != "") //User did not hit cancel
    {
        QFile queriesFile(g_tempDirectory + "queries.fasta");
        queriesFile.open(QIODevice::Append | QIODevice::Text);
        QTextStream out(&queriesFile);

        std::vector<QString> queryNames;
        std::vector<QString> querySequences;
        readFastaFile(fullFileName, &queryNames, &querySequences);
        int newQueryCount = int(queryNames.size());

        if (queriesFile.pos() > 0 && newQueryCount > 0)
            out << "\n";

        for (int i = 0; i < newQueryCount; ++i)
        {
            out << ">" << queryNames[i].replace(" ", "_") << "\n";
            out << querySequences[i];

            if (i < newQueryCount - 1)
                out << "\n";
        }
        queriesFile.close();

        loadBlastQueries();
        fillQueriesTable();
        clearBlastHits();
    }
}


void LoadBlastResultsDialog::enterQueryManually()
{
    EnterOneBlastQueryDialog enterOneBlastQueryDialog(this);

    if (enterOneBlastQueryDialog.exec())
    {
        QFile queriesFile(g_tempDirectory + "queries.fasta");
        queriesFile.open(QIODevice::Append | QIODevice::Text);
        QTextStream out(&queriesFile);

        if (queriesFile.pos() > 0)
            out << "\n";

        out << ">";
        out << enterOneBlastQueryDialog.getName();
        out << "\n";
        out << enterOneBlastQueryDialog.getSequence();

        queriesFile.close();

        loadBlastQueries();
        fillQueriesTable();
        clearBlastHits();
    }
}



void LoadBlastResultsDialog::clearQueries()
{
    makeQueryFile();

    g_blastSearchResults->m_queries.clear();
    ui->blastQueriesTableView->setModel(0);
    ui->clearQueriesButton->setEnabled(false);

    clearBlastHits();
    setUiStep(2);
}


void LoadBlastResultsDialog::makeQueryFile()
{
    //If the query file already exists, delete it.
    QString checkForQueryFileCommmand = "test -e " + g_tempDirectory + "queries.fasta";
    if (system(checkForQueryFileCommmand.toLocal8Bit().constData()) == 0)
    {
        QString deleteQueryFastaCommand = "rm " + g_tempDirectory + "queries.fasta";
        system(deleteQueryFastaCommand.toLocal8Bit().constData());
    }

    //Make an empty query file.
    QString createQueryFastaCommand = "touch " + g_tempDirectory + "queries.fasta";
    system(createQueryFastaCommand.toLocal8Bit().constData());
}


void LoadBlastResultsDialog::runBlastSearch()
{
    if (system("which blastn") != 0)
    {
        QMessageBox::warning(this, "Error", "The program blastn was not found.  Please install NCBI BLAST to use this feature.");
        return;
    }

    QString extraCommandLineOptions = ui->parametersLineEdit->text().simplified();
    QString blastCommand = "blastn -query " + g_tempDirectory + "queries.fasta -db " + g_tempDirectory + "all_nodes.fasta -outfmt 6 " + extraCommandLineOptions + " > " + g_tempDirectory + "blast_results";
    system(blastCommand.toLocal8Bit().constData());

    loadBlastHits();
}



void LoadBlastResultsDialog::setUiStep(int step)
{
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
        break;


    }
}
