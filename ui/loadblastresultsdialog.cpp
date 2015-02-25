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
#include "../blast/blasttarget.h"
#include <QStandardItemModel>
#include "../program/globals.h"
#include "../graph/debruijnnode.h"

LoadBlastResultsDialog::LoadBlastResultsDialog(QMap<int, DeBruijnNode *> * deBruijnGraphNodes,
                                               QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LoadBlastResultsDialog),
    m_deBruijnGraphNodes(deBruijnGraphNodes)
{
    ui->setupUi(this);

    connect(ui->loadBlastDatabaseButton, SIGNAL(clicked()), this, SLOT(loadBlastDatabase()));
    connect(ui->loadBlastOutputButton, SIGNAL(clicked()), this, SLOT(loadBlastOutput()));
}

LoadBlastResultsDialog::~LoadBlastResultsDialog()
{
    delete ui;
}

void LoadBlastResultsDialog::loadBlastDatabase()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Load BLAST database");

    if (fileName != "") //User did not hit cancel
    {
        //If there a BLAST results object, delete it now and make a new one.
        if (g_blastSearchResults != 0)
            delete g_blastSearchResults;
        g_blastSearchResults = new BlastSearchResults();

        QFile inputFile(fileName);
        if (inputFile.open(QIODevice::ReadOnly))
        {
            int sequenceLength = 0;
            QString targetName = "";

            QTextStream in(&inputFile);
            while (!in.atEnd())
            {
                QString line = in.readLine();

                if (line.length() > 0 && line.at(0) == '>')
                {
                    //If there is a current target, add it to the results now.
                    if (targetName.length() > 0)
                        g_blastSearchResults->m_targets.push_back(BlastTarget(targetName, sequenceLength));

                    line.remove(0, 1); //Remove '>' from start
                    targetName = line;
                    sequenceLength = 0;
                }

                else //It's a sequence line
                    sequenceLength += line.simplified().length();
            }

            //Add the last target to the results now.
            if (targetName.length() > 0)
                g_blastSearchResults->m_targets.push_back(BlastTarget(targetName, sequenceLength));
        }

        fillTargetsTable();

        ui->blastTargetsTableView->setEnabled(true);
        ui->loadBlastOutputButton->setEnabled(true);
    }
}

void LoadBlastResultsDialog::loadBlastOutput()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Load BLAST output");

    if (fileName != "") //User did not hit cancel
    {
        QFile inputFile(fileName);
        if (inputFile.open(QIODevice::ReadOnly))
        {
            QTextStream in(&inputFile);
            while (!in.atEnd())
            {
                QString line = in.readLine();
                QStringList alignmentParts = line.split('\t');

                QString nodeLabel = alignmentParts[0];
                QString targetName = alignmentParts[1];
                int nodeStart = alignmentParts[6].toInt();
                int nodeEnd = alignmentParts[7].toInt();
                int targetStart = alignmentParts[8].toInt();
                int targetEnd = alignmentParts[9].toInt();

                int nodeNumber = getNodeNumberFromString(nodeLabel);
                DeBruijnNode * node;

                //Only save BLAST results for positive nodes, as results for
                //negative nodes should be redundant.
                if (m_deBruijnGraphNodes->contains(nodeNumber) && nodeNumber > 0)
                    node = (*m_deBruijnGraphNodes)[nodeNumber];
                else
                    continue;

                BlastTarget * target = getTargetFromString(targetName);

                g_blastSearchResults->m_hits.push_back(BlastHit(node, nodeStart, nodeEnd,
                                                                target, targetStart, targetEnd));
            }
        }

        fillHitsTable();
    }
}


int LoadBlastResultsDialog::getNodeNumberFromString(QString nodeString)
{
    //Find the first occurrence of "NODE" and then the first number after that.
    int nodeWordIndex = nodeString.indexOf("NODE");

    QString numberString = "";
    bool firstDigitFound = false;
    for (int i = nodeWordIndex; i < nodeString.size(); ++i)
    {
        if (nodeString.at(i).digitValue() >= 0)
        {
            numberString += nodeString.at(i);
            firstDigitFound = true;
        }
        else if (firstDigitFound)
            break;
    }

    int nodeNumber = numberString.toInt();

    //If the node string ends with an inverted comma, then this
    //is the FASTG indication for a negative node.
    QStringList fastgParts = nodeString.split(':');
    QString firstPart = fastgParts[0];
    if (firstPart.at(firstPart.length() - 1) == '\'')
        nodeNumber *= -1;

    return nodeNumber;
}


BlastTarget * LoadBlastResultsDialog::getTargetFromString(QString targetName)
{
    for (size_t i = 0; i < g_blastSearchResults->m_targets.size(); ++i)
    {
        if (g_blastSearchResults->m_targets[i].m_name == targetName)
            return &(g_blastSearchResults->m_targets[i]);
    }
    return 0;
}


void LoadBlastResultsDialog::fillTargetsTable()
{
    size_t targetCount = g_blastSearchResults->m_targets.size();
    QStandardItemModel * model = new QStandardItemModel(targetCount, 2, this); //2 Columns
    model->setHorizontalHeaderItem(0, new QStandardItem("Target name"));
    model->setHorizontalHeaderItem(1, new QStandardItem("Target length"));
    for (size_t i = 0; i < targetCount; ++i)
    {
        model->setItem(i, 0, new QStandardItem(g_blastSearchResults->m_targets[i].m_name));
        model->setItem(i, 1, new QStandardItem(formatIntForDisplay(g_blastSearchResults->m_targets[i].m_length)));
    }
    ui->blastTargetsTableView->setModel(model);
    ui->blastTargetsTableView->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
}


void LoadBlastResultsDialog::fillHitsTable()
{
    size_t hitCount = g_blastSearchResults->m_hits.size();
    QStandardItemModel * model = new QStandardItemModel(hitCount, 7, this); //7 Columns
    model->setHorizontalHeaderItem(0, new QStandardItem("Node number"));
    model->setHorizontalHeaderItem(1, new QStandardItem("Node length"));
    model->setHorizontalHeaderItem(2, new QStandardItem("Node start"));
    model->setHorizontalHeaderItem(3, new QStandardItem("Node end"));
    model->setHorizontalHeaderItem(4, new QStandardItem("Target name"));
    model->setHorizontalHeaderItem(5, new QStandardItem("Target start"));
    model->setHorizontalHeaderItem(6, new QStandardItem("Target end"));
    for (size_t i = 0; i < hitCount; ++i)
    {
        model->setItem(i, 0, new QStandardItem(formatIntForDisplay(g_blastSearchResults->m_hits[i].m_node->m_number)));
        model->setItem(i, 1, new QStandardItem(formatIntForDisplay(g_blastSearchResults->m_hits[i].m_node->m_length)));
        model->setItem(i, 2, new QStandardItem(formatIntForDisplay(g_blastSearchResults->m_hits[i].m_nodeStart)));
        model->setItem(i, 3, new QStandardItem(formatIntForDisplay(g_blastSearchResults->m_hits[i].m_nodeEnd)));
        model->setItem(i, 4, new QStandardItem(g_blastSearchResults->m_hits[i].m_target->m_name));
        model->setItem(i, 5, new QStandardItem(formatIntForDisplay(g_blastSearchResults->m_hits[i].m_targetStart)));
        model->setItem(i, 6, new QStandardItem(formatIntForDisplay(g_blastSearchResults->m_hits[i].m_targetEnd)));
    }
    ui->blastHitsTableView->setModel(model);
    ui->blastHitsTableView->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);

    ui->blastHitsTableView->setEnabled(true);
}
