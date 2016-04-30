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


#include "myprogressdialog.h"
#include "ui_myprogressdialog.h"
#include "../program/globals.h"

MyProgressDialog::MyProgressDialog(QWidget * parent, QString message, bool showCancelButton,
                                   QString cancelButtonText, QString cancelMessage, QString cancelInfoText) :
    QDialog(parent),
    ui(new Ui::MyProgressDialog),
    m_cancelMessage(cancelMessage),
    m_cancelled(false)
{
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);

    ui->setupUi(this);
    QFont font;
    QFont largeFont;
    largeFont.setPointSize(font.pointSize() * 2);
    ui->messageLabel->setText(message);
    ui->messageLabel->setFont(largeFont);

    ui->cancelWidget->setVisible(showCancelButton);
    ui->cancelButton->setText(cancelButtonText);

    setFixedHeight(sizeHint().height());
    setFixedWidth(sizeHint().width() * 1.2);

    ui->cancelInfoText->setInfoText(cancelInfoText);

    connect(ui->cancelButton, SIGNAL(clicked()), this, SLOT(cancel()));
}

MyProgressDialog::~MyProgressDialog()
{
    delete ui;
}


void MyProgressDialog::cancel()
{
    ui->messageLabel->setText(m_cancelMessage);
    ui->cancelButton->setEnabled(false);
    m_cancelled = true;
    emit halt();
}


void MyProgressDialog::setMaxValue(int max)
{
    ui->progressBar->setMaximum(max);
}

void MyProgressDialog::setValue(int value)
{
    ui->progressBar->setValue(value);
}
