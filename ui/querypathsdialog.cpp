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

        double evalueProduct = queryPath->getEvalueProduct();
        TableWidgetItemDouble * pathEvalueProduct = new TableWidgetItemDouble(QString::number(evalueProduct), evalueProduct);
        pathEvalueProduct->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);

        double percIdentity = queryPath->getPercIdentity();
        TableWidgetItemDouble * pathPercIdentity = new TableWidgetItemDouble(QString::number(percIdentity), percIdentity);
        pathPercIdentity->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);

        ui->tableWidget->setItem(i, 0, pathString);
        ui->tableWidget->setItem(i, 1, pathLength);
        ui->tableWidget->setItem(i, 2, pathEvalueProduct);
        ui->tableWidget->setItem(i, 3, pathPercIdentity);
    }

    ui->tableWidget->resizeColumns();
    ui->tableWidget->setSortingEnabled(true);


}

QueryPathsDialog::~QueryPathsDialog()
{
    delete ui;
}
