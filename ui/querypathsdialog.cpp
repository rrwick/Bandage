#include "querypathsdialog.h"
#include "ui_querypathsdialog.h"

#include "../blast/blastquery.h"
#include "../blast/blastquerypath.h"
#include "tablewidgetitemint.h"
#include "tablewidgetitemdouble.h"

QueryPathsDialog::QueryPathsDialog(QWidget * parent, BlastQuery * query) :
    QDialog(parent),
    ui(new Ui::QueryPathsDialog)
{
    ui->setupUi(this);

    ui->tableWidget->setHorizontalHeaderLabels(QStringList() << "Path" << "Length\n(bp)" << "Query\ncovered\nby path" <<
                                               "Query\ncovered\nby hits" << "Mean hit\nidentity"  << "Total\nhit mis-\nmatches" <<
                                               "Total\nhit gap\nopens" << "Length\ndiscre-\npancy" << "E-value\nproduct");

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
        TableWidgetItemDouble * pathLengthDisc = new TableWidgetItemDouble(formatDoubleForDisplay(100.0 * lengthDisc, 2) + "%", lengthDisc);
        pathLengthDisc->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);

        double evalueProduct = queryPath->getEvalueProduct();
        TableWidgetItemDouble * pathEvalueProduct = new TableWidgetItemDouble(QString::number(evalueProduct), evalueProduct);
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


}

QueryPathsDialog::~QueryPathsDialog()
{
    delete ui;
}
