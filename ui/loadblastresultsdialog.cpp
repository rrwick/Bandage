#include "loadblastresultsdialog.h"
#include "ui_loadblastresultsdialog.h"

#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QString>
#include "../blast/blastresult.h"
#include "../blast/blasttarget.h"
#include <QStandardItemModel>
#include "../program/globals.h"
#include "../graph/debruijnnode.h"

LoadBlastResultsDialog::LoadBlastResultsDialog(QMap<int, DeBruijnNode *> * deBruijnGraphNodes,
                                               QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LoadBlastResultsDialog), m_blastSearchResults(0),
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
        if (m_blastSearchResults != 0)
            delete m_blastSearchResults;
        m_blastSearchResults = new BlastSearchResults();

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
                        m_blastSearchResults->m_targets.push_back(BlastTarget(targetName, sequenceLength));

                    line.remove(0, 1); //Remove '>' from start
                    targetName = line;
                    sequenceLength = 0;
                }

                else //It's a sequence line
                    sequenceLength += line.simplified().length();
            }

            //Add the last target to the results now.
            if (targetName.length() > 0)
                m_blastSearchResults->m_targets.push_back(BlastTarget(targetName, sequenceLength));
        }

        //Fill in the targets table
        size_t targetCount = m_blastSearchResults->m_targets.size();
        QStandardItemModel * model = new QStandardItemModel(targetCount, 2, this); //2 Columns
        model->setHorizontalHeaderItem(0, new QStandardItem("Target name"));
        model->setHorizontalHeaderItem(1, new QStandardItem("Target length"));
        for (size_t i = 0; i < targetCount; ++i)
        {
            model->setItem(i, 0, new QStandardItem(m_blastSearchResults->m_targets[i].m_name));
            model->setItem(i, 1, new QStandardItem(formatIntForDisplay(m_blastSearchResults->m_targets[i].m_length)));
        }
        ui->blastTargetsTableView->setModel(model);
        ui->blastTargetsTableView->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);

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
                QString targetLabel = alignmentParts[1];
                int nodeStart = alignmentParts[6].toInt();
                int nodeEnd = alignmentParts[7].toInt();
                int targetStart = alignmentParts[8].toInt();
                int targetEnd = alignmentParts[9].toInt();

                int nodeNumber = getNodeNumberFromString(nodeLabel);
                DeBruijnNode * node;
                if (m_deBruijnGraphNodes->contains(nodeNumber))
                    node = (*m_deBruijnGraphNodes)[nodeNumber];
                else
                    continue;

                BlastTarget * target = getTargetFromString(targetLabel);

                m_blastSearchResults->m_results.push_back(BlastResult(node, target,
                                                                      nodeStart, nodeEnd,
                                                                      targetStart, targetEnd));
            }
        }

        //Fill in the hits table
        size_t hitCount = m_blastSearchResults->m_results.size();
        QStandardItemModel * model = new QStandardItemModel(hitCount, 8, this); //8 Columns
        model->setHorizontalHeaderItem(0, new QStandardItem("Node number"));
        model->setHorizontalHeaderItem(1, new QStandardItem("Node length"));
        model->setHorizontalHeaderItem(2, new QStandardItem("Node start"));
        model->setHorizontalHeaderItem(3, new QStandardItem("Node end"));
        model->setHorizontalHeaderItem(4, new QStandardItem("Target name"));
        model->setHorizontalHeaderItem(5, new QStandardItem("Target length"));
        model->setHorizontalHeaderItem(6, new QStandardItem("Target start"));
        model->setHorizontalHeaderItem(7, new QStandardItem("Target end"));
        for (size_t i = 0; i < hitCount; ++i)
        {
            model->setItem(i, 0, new QStandardItem(formatIntForDisplay(m_blastSearchResults->m_results[i].m_node->m_number)));
            model->setItem(i, 1, new QStandardItem(formatIntForDisplay(m_blastSearchResults->m_results[i].m_node->m_length)));
            model->setItem(i, 2, new QStandardItem(formatIntForDisplay(m_blastSearchResults->m_results[i].m_nodeStart)));
            model->setItem(i, 3, new QStandardItem(formatIntForDisplay(m_blastSearchResults->m_results[i].m_nodeEnd)));
            model->setItem(i, 4, new QStandardItem(m_blastSearchResults->m_results[i].m_target->m_name));
            model->setItem(i, 5, new QStandardItem(formatIntForDisplay(m_blastSearchResults->m_results[i].m_target->m_length)));
            model->setItem(i, 6, new QStandardItem(formatIntForDisplay(m_blastSearchResults->m_results[i].m_targetStart)));
            model->setItem(i, 7, new QStandardItem(formatIntForDisplay(m_blastSearchResults->m_results[i].m_targetEnd)));
        }
        ui->blastHitsTableView->setModel(model);
        ui->blastHitsTableView->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);

        ui->blastHitsTableView->setEnabled(true);
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
    for (size_t i = 0; i < m_blastSearchResults->m_targets.size(); ++i)
    {
        if (m_blastSearchResults->m_targets[i].m_name == targetName)
            return &(m_blastSearchResults->m_targets[i]);
    }
    return 0;
}
