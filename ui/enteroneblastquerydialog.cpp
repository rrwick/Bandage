#include "enteroneblastquerydialog.h"
#include "ui_enteroneblastquerydialog.h"

EnterOneBlastQueryDialog::EnterOneBlastQueryDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::EnterOneBlastQueryDialog)
{
    ui->setupUi(this);
}

EnterOneBlastQueryDialog::~EnterOneBlastQueryDialog()
{
    delete ui;
}



QString EnterOneBlastQueryDialog::getName()
{
    return ui->nameLineEdit->text().simplified();
}
QString EnterOneBlastQueryDialog::getSequence()
{
    return ui->sequenceTextEdit->toPlainText().simplified();
}
