#include "changenodereaddepthdialog.h"
#include "ui_changenodereaddepthdialog.h"


#include "../program/globals.h"
#include "../graph/debruijnnode.h"

ChangeNodeReadDepthDialog::ChangeNodeReadDepthDialog(QWidget * parent,
                                                     std::vector<DeBruijnNode *> * nodes,
                                                     double oldDepth) :
    QDialog(parent),
    ui(new Ui::ChangeNodeReadDepthDialog)
{
    ui->setupUi(this);

    ui->currentReadDepthLabel2->setText(formatDoubleForDisplay(oldDepth, 1));
    ui->newReadDepthSpinBox->setValue(oldDepth);

    //Display the node names.  If there are lots of nodes (more than 10), just
    //display the first 10 and then an ellipsis.
    QString nodeNames;
    int numNodeNames = int(nodes->size());
    if (numNodeNames > 10)
        numNodeNames = 10;
    for (int i = 0; i < numNodeNames; ++i)
    {
        nodeNames += (*nodes)[i]->getNameWithoutSign();
        if (i < numNodeNames - 1)
            nodeNames += ", ";
    }
    if (int(nodes->size()) > numNodeNames)
        nodeNames += "...";

    //Set the dialog text to be either singular or plural as appropriate.
    if (nodes->size() == 1)
    {
        ui->nodeNameLabel1->setText("Node:");
        ui->currentReadDepthLabel1->setText("Current read depth:");
        ui->infoLabel->setText("Enter a new read depth. Both the node and its reverse complement will have their read depth set to the new value.");
    }
    else
    {
        ui->nodeNameLabel1->setText("Nodes (" + formatIntForDisplay(int(nodes->size())) + "):");
        ui->currentReadDepthLabel1->setText("Current mean read depth:");
        ui->infoLabel->setText("Enter a new read depth. The nodes and their reverse complements will have their read depths set to the new value.");
    }

    ui->nodeNameLabel2->setText(nodeNames);

    ui->newReadDepthSpinBox->setFocus();
}

ChangeNodeReadDepthDialog::~ChangeNodeReadDepthDialog()
{
    delete ui;
}

double ChangeNodeReadDepthDialog::getNewDepth() const
{
    return ui->newReadDepthSpinBox->value();
}
