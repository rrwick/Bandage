//Copyright 2016 Ryan Wick

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


#include "enteroneblastquerydialog.h"
#include "ui_enteroneblastquerydialog.h"
#include "../program/settings.h"
#include "../program/globals.h"
#include <QDialogButtonBox>
#include <QPushButton>

EnterOneBlastQueryDialog::EnterOneBlastQueryDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::EnterOneBlastQueryDialog)
{
    ui->setupUi(this);
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    connect(ui->sequenceTextEdit, SIGNAL(textChanged()), this, SLOT(sequenceChanged()));
}

EnterOneBlastQueryDialog::~EnterOneBlastQueryDialog()
{
    delete ui;
}

QString EnterOneBlastQueryDialog::getName()
{
    QString name = ui->nameLineEdit->text().simplified();
    if (name == "")
        name = g_settings->unnamedQueryDefaultName;
    return name;
}

QString EnterOneBlastQueryDialog::getSequence()
{
    return ui->sequenceTextEdit->toPlainText().simplified().replace(" ", "");
}

void EnterOneBlastQueryDialog::sequenceChanged()
{
    bool someText = !ui->sequenceTextEdit->toPlainText().isEmpty();
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(someText);
}
