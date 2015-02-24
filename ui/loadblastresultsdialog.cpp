#include "loadblastresultsdialog.h"
#include "ui_loadblastresultsdialog.h"

LoadBlastResultsDialog::LoadBlastResultsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LoadBlastResultsDialog)
{
    ui->setupUi(this);
}

LoadBlastResultsDialog::~LoadBlastResultsDialog()
{
    delete ui;
}
