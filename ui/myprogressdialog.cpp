#include "myprogressdialog.h"
#include "ui_myprogressdialog.h"
#include "../program/globals.h"

MyProgressDialog::MyProgressDialog(QWidget * parent, QString message, bool showCancelButton) :
    QDialog(parent),
    ui(new Ui::MyProgressDialog)
{
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);

    ui->setupUi(this);
    QFont font;
    QFont largeFont;
    largeFont.setPointSize(font.pointSize() * 2);
    ui->messageLabel->setText(message);
    ui->messageLabel->setFont(largeFont);

    ui->buttonBox->setVisible(showCancelButton);

    setFixedSize(sizeHint());
}

MyProgressDialog::~MyProgressDialog()
{
    delete ui;
}
