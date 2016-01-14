#include "graphinfodialog.h"
#include "ui_graphinfodialog.h"

GraphInfoDialog::GraphInfoDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GraphInfoDialog)
{
    ui->setupUi(this);
}

GraphInfoDialog::~GraphInfoDialog()
{
    delete ui;
}
