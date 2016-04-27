#include "changenodedepthdialog.h"
#include "ui_changenodedepthdialog.h"


#include "../program/globals.h"
#include "../graph/debruijnnode.h"

ChangeNodeDepthDialog::ChangeNodeDepthDialog(QWidget * parent,
                                                     std::vector<DeBruijnNode *> * nodes,
                                                     double oldDepth) :
    QDialog(parent),
    ui(new Ui::ChangeNodeDepthDialog)
{
    ui->setupUi(this);

    ui->currentDepthLabel2->setText(formatDoubleForDisplay(oldDepth, 1));
    ui->newDepthSpinBox->setValue(oldDepth);

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
        ui->currentDepthLabel1->setText("Current depth:");
        ui->infoLabel->setText("Enter a new depth. Both the node and its reverse complement will have their depth set to the new value.");
    }
    else
    {
        ui->nodeNameLabel1->setText("Nodes (" + formatIntForDisplay(int(nodes->size())) + "):");
        ui->currentDepthLabel1->setText("Current mean depth:");
        ui->infoLabel->setText("Enter a new depth. The nodes and their reverse complements will have their depths set to the new value.");
    }

    ui->nodeNameLabel2->setText(nodeNames);

    ui->newDepthSpinBox->setFocus();
}

ChangeNodeDepthDialog::~ChangeNodeDepthDialog()
{
    delete ui;
}

double ChangeNodeDepthDialog::getNewDepth() const
{
    return ui->newDepthSpinBox->value();
}
