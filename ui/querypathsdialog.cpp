//Copyright 2015 Ryan Wick

//This file is part of Bandage.

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


#include "querypathsdialog.h"
#include "ui_querypathsdialog.h"

#include "../blast/blastquery.h"
#include "../blast/blastquerypath.h"
#include "tablewidgetitemint.h"
#include "tablewidgetitemdouble.h"
#include "../program/globals.h"
#include "../program/memory.h"

QueryPathsDialog::QueryPathsDialog(QWidget * parent, BlastQuery * query) :
    QDialog(parent),
    ui(new Ui::QueryPathsDialog)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);

    connect(this, SIGNAL(rejected()), this, SLOT(hidden()));
    connect(ui->tableWidget, SIGNAL(itemSelectionChanged()), this, SLOT(tableSelectionChanged()));

    g_memory->queryPathDialogIsVisible = true;
    g_memory->queryPaths.clear();

    ui->tableWidget->setHorizontalHeaderLabels(QStringList() << "Path" << "Length\n(bp)" << "Query\ncovered\nby path" <<
                                               "Query\ncovered\nby hits" << "Mean hit\nidentity"  << "Total\nhit mis-\nmatches" <<
                                               "Total\nhit gap\nopens" << "Length\ndiscre-\npancy" << "E-value\nproduct");

    QString queryDescription = "Query name: " + query->getName();
    queryDescription += "      type: " + query->getTypeString();
    queryDescription += "      length: " + formatIntForDisplay(query->getLength());
    if (query->getSequenceType() == PROTEIN)
        queryDescription += " (" + formatIntForDisplay(3 * query->getLength()) + " bp)";
    else
        queryDescription += " bp";
    ui->queryLabel->setText(queryDescription);

    ui->tableWidget->clearContents();
    ui->tableWidget->setSortingEnabled(false);

    int pathCount = query->getPathCount();
    ui->tableWidget->setRowCount(pathCount);

    if (pathCount == 0)
        return;

    QList<BlastQueryPath> paths = query->getPaths();

    for (int i = 0; i < pathCount; ++i)
    {
        BlastQueryPath * queryPath = &paths[i];

        QTableWidgetItem * pathString = new QTableWidgetItem(queryPath->getPath().getString(true));
        pathString->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);

        int length = queryPath->getPath().getLength();
        TableWidgetItemInt * pathLength = new TableWidgetItemInt(formatIntForDisplay(length), length);
        pathLength->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);

        double queryCoveragePath = queryPath->getPathQueryCoverage();
        TableWidgetItemDouble * pathQueryCoveragePath = new TableWidgetItemDouble(formatDoubleForDisplay(100.0 * queryCoveragePath, 2) + "%", queryCoveragePath);
        pathQueryCoveragePath->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        pathLength->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);

        double queryCoverageHits = queryPath->getHitsQueryCoverage();
        TableWidgetItemDouble * pathQueryCoverageHits = new TableWidgetItemDouble(formatDoubleForDisplay(100.0 * queryCoverageHits, 2) + "%", queryCoverageHits);
        pathQueryCoverageHits->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);

        double percIdentity = queryPath->getMeanHitPercIdentity();
        TableWidgetItemDouble * pathPercIdentity = new TableWidgetItemDouble(formatDoubleForDisplay(percIdentity, 2) + "%", percIdentity);
        pathPercIdentity->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);

        int mismatches = queryPath->getTotalHitMismatches();
        TableWidgetItemInt * pathMismatches = new TableWidgetItemInt(formatIntForDisplay(mismatches), mismatches);
        pathMismatches->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);

        int gapOpens = queryPath->getTotalHitGapOpens();
        TableWidgetItemInt * pathGapOpens = new TableWidgetItemInt(formatIntForDisplay(gapOpens), gapOpens);
        pathGapOpens->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);

        double lengthDisc = queryPath->getRelativeLengthDiscrepancy();
        QString lengthDiscSign = "";
        if (lengthDisc > 0.0)
            lengthDiscSign = "+";
        TableWidgetItemDouble * pathLengthDisc = new TableWidgetItemDouble(lengthDiscSign + formatDoubleForDisplay(100.0 * lengthDisc, 2) + "%", lengthDisc);
        pathLengthDisc->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);

        SciNot evalueProduct = queryPath->getEvalueProduct();
        TableWidgetItemDouble * pathEvalueProduct = new TableWidgetItemDouble(evalueProduct.asString(false), evalueProduct.toDouble());
        pathEvalueProduct->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);

        ui->tableWidget->setItem(i, 0, pathString);
        ui->tableWidget->setItem(i, 1, pathLength);
        ui->tableWidget->setItem(i, 2, pathQueryCoveragePath);
        ui->tableWidget->setItem(i, 3, pathQueryCoverageHits);
        ui->tableWidget->setItem(i, 4, pathPercIdentity);
        ui->tableWidget->setItem(i, 5, pathMismatches);
        ui->tableWidget->setItem(i, 6, pathGapOpens);
        ui->tableWidget->setItem(i, 7, pathLengthDisc);
        ui->tableWidget->setItem(i, 8, pathEvalueProduct);
    }

    ui->tableWidget->resizeColumns();
    ui->tableWidget->setSortingEnabled(true);

    ui->queryPathsInfoText->setInfoText("This table shows information about the possible paths through the graph which "
                                        "represent the query. These paths can be either simple (residing within a single "
                                        "node) or complex (spanning multiple nodes). The columns in the table are as "
                                        "follows:"
                                        "<br><br>"
                                        "<b>Path</b>: This is the query path through the graph, as written in Bandage's "
                                        "path notation. The nodes in the path are separated by commas. The start position "
                                        "in the first node is shown in parentheses at the beginning of the path. The end "
                                        "position in the last node is shown in parentheses at the end of the path."
                                        "<br><br>"
                                        "<b>Length</b>: This is the path length. It is shown in base pairs, whether the "
                                        "query is a nucleotide query or a protein query."
                                        "<br><br>"
                                        "<b>Query covered by path</b>: This is the fraction of the query which is covered "
                                        "by the path. It is calculated by taking 100% and subtracting the fraction of the "
                                        "query which is not captured by the start and the fraction of the query which is "
                                        "not captured by the end."
                                        "<br><br>"
                                        "<b>Query covered by hits</b>: This is the fraction of the query which is covered "
                                        "by the BLAST hits in this path. Since a path may contain nodes or parts of nodes which "
                                        "are not covered by BLAST hits, this value will be less than or equal to the 'Query "
                                        "covered by path' value."
                                        "<br><br>"
                                        "<b>Mean hit identity</b>: This is the mean of the percent identity for the BLAST "
                                        "hits in this path, weighted by the hits' lengths."
                                        "<br><br>"
                                        "<b>Total hit mismatches</b>: This is the sum of the mismatches for the BLAST hits "
                                        "in this path."
                                        "<br><br>"
                                        "<b>Total hit gap opens</b>: This is the sum of the gap opens for the BLAST hits "
                                        "in this path."
                                        "<br><br>"
                                        "<b>Length discrepancy</b>: This is the percent difference in the path length and "
                                        "the appropriate length for the relecant fraction of the query. A positive value "
                                        "indicates that the path is too long; a negative value indicates that the path is "
                                        "too short."
                                        "<br><br>"
                                        "<b>E-value product</b>: This is the product of the e-values for the BLAST hits "
                                        "in this path.");
}

QueryPathsDialog::~QueryPathsDialog()
{
    delete ui;
    g_memory->queryPathDialogIsVisible = false;
}


void QueryPathsDialog::hidden()
{
    g_memory->queryPathDialogIsVisible = false;
    emit selectionChanged();
}


void QueryPathsDialog::tableSelectionChanged()
{
    QList<QTableWidgetSelectionRange> selection = ui->tableWidget->selectedRanges();
    int totalSelectedRows = 0;
    for (int i = 0; i < selection.size(); ++i)
        totalSelectedRows += selection[i].rowCount();

    g_memory->queryPaths.clear();

    QList<int> selectedRows;
    for (int i = 0; i < selection.size(); ++i)
    {
        QTableWidgetSelectionRange * selectionRange = &(selection[i]);
        int top = selectionRange->topRow();
        int bottom = selectionRange->bottomRow();

        for (int row = top; row <= bottom; ++row)
        {
            if (!selectedRows.contains(row))
                selectedRows.push_back(row);
        }
    }

    for (int i = 0; i < selectedRows.size(); ++i)
    {
        int row = selectedRows[i];
        QString pathString = ui->tableWidget->item(row, 0)->text();
        QString pathStringFailure;
        g_memory->queryPaths.push_back(Path::makeFromString(pathString, false, &pathStringFailure));
    }

    emit selectionChanged();
}
