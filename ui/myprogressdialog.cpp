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

    ui->cancelWidget->setVisible(showCancelButton);

    setFixedHeight(sizeHint().height());
    setFixedWidth(sizeHint().width() * 1.2);

    ui->cancelInfoText->setInfoText("Clicking this button will halt the graph layout and display "
                                    "the graph in its current, incomplete state.<br><br>"
                                    "Layout can take a long time for very large graphs.  There are "
                                    "three strategies to reduce the amount of time required:<ul>"
                                    "<li>Change the scope of the graph from 'Entire graph' to either "
                                    "'Around nodes' or 'Around BLAST hits'.  This will reduce the "
                                    "number of nodes that are drawn to the screen.</li>"
                                    "<li>Increase the 'Base pairs per segment' setting.  This will "
                                    "result in shorter contigs which take less time to lay out.</li>"
                                    "<li>Reduce the 'Graph layout iterations' setting.</li></ul>");

    connect(ui->cancelButton, SIGNAL(clicked()), this, SLOT(cancelLayout()));
}

MyProgressDialog::~MyProgressDialog()
{
    delete ui;
}


void MyProgressDialog::cancelLayout()
{
    ui->messageLabel->setText("Cancelling layout...");
    ui->cancelButton->setEnabled(false);
    emit haltLayout();
}
