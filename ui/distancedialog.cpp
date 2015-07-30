#include "distancedialog.h"
#include "ui_distancedialog.h"

DistanceDialog::DistanceDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DistanceDialog)
{
    ui->setupUi(this);
}

DistanceDialog::~DistanceDialog()
{
    delete ui;
}
