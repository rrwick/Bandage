//Copyright 2015 Ryan Wick

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


#include "pathspecifydialog.h"
#include "ui_pathspecifydialog.h"
#include "../program/globals.h"
#include "../program/settings.h"
#include "../graph/assemblygraph.h"
#include <QClipboard>
#include <QTextStream>
#include <QFileDialog>
#include <QFileInfo>

PathSpecifyDialog::PathSpecifyDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PathSpecifyDialog)
{
    ui->setupUi(this);
    checkPathValidity();

    ui->circularPathInfoText->setInfoText("Tick this box to indicate that the path is circular, i.e. there is an edge connecting the "
                                          "last node in the list to the first.");

    connect(ui->pathTextEdit, SIGNAL(textChanged()), this, SLOT(checkPathValidity()));
    connect(ui->circularPathCheckBox, SIGNAL(toggled(bool)), this, SLOT(checkPathValidity()));
    connect(ui->copyButton, SIGNAL(clicked(bool)), this, SLOT(copyPathToClipboard()));
    connect(ui->saveButton, SIGNAL(clicked(bool)), this, SLOT(savePathToFile()));
}

PathSpecifyDialog::~PathSpecifyDialog()
{
    delete ui;
}



void PathSpecifyDialog::checkPathValidity()
{
    //If there is no graph loaded, then no path can be valid.
    if (g_assemblyGraph->m_deBruijnGraphNodes.size() == 0)
    {
        ui->validPathLabel->setText("Invalid path: no graph is currently loaded");
        setPathValidityUiElements(false);
        return;
    }

    QStringList nodeNameList = ui->pathTextEdit->toPlainText().simplified().split(",", QString::SkipEmptyParts);
    if (nodeNameList.empty())
    {
        ui->validPathLabel->setText("Invalid path: no nodes specified");
        setPathValidityUiElements(false);
        return;
    }

    QList<DeBruijnNode *> pathNodes;
    QStringList nodesNotInGraph;
    for (int i = 0; i < nodeNameList.size(); ++i)
    {
        QString nodeName = nodeNameList[i].simplified();
        if (g_assemblyGraph->m_deBruijnGraphNodes.contains(nodeName))
            pathNodes.push_back(g_assemblyGraph->m_deBruijnGraphNodes[nodeName]);
        else
            nodesNotInGraph.push_back(nodeName);
    }

    //If any nodes aren't in the graph, the path isn't valid.
    if (!nodesNotInGraph.empty())
    {
        QString nodesNotInGraphString;
        for (int i = 0; i < nodesNotInGraph.size(); ++i)
        {
            nodesNotInGraphString += nodesNotInGraph[i];
            if (i < nodesNotInGraph.size() - 1)
                nodesNotInGraphString += ", ";
        }
        ui->validPathLabel->setText("Invalid path: the following nodes are not in the graph: " + nodesNotInGraphString);
        setPathValidityUiElements(false);
        return;
    }

    m_path = Path::makeFromOrderedNodes(pathNodes, ui->circularPathCheckBox->isChecked());

    if (m_path.isEmpty())
    {
        if (ui->circularPathCheckBox->isChecked())
            ui->validPathLabel->setText("Invalid path: the nodes do not form a circular path in the graph");
        else
            ui->validPathLabel->setText("Invalid path: the nodes do not form a path in the graph");
        setPathValidityUiElements(false);
    }
    else
    {
        ui->validPathLabel->setText("Valid path");
        setPathValidityUiElements(true);
    }
}


void PathSpecifyDialog::setPathValidityUiElements(bool pathValid)
{
    if (pathValid)
        ui->tickCrossLabel->setPixmap(QPixmap(":/icons/tick-128.png"));
    else
        ui->tickCrossLabel->setPixmap(QPixmap(":/icons/cross-128.png"));

    ui->copyButton->setEnabled(pathValid);
    ui->saveButton->setEnabled(pathValid);
}



void PathSpecifyDialog::copyPathToClipboard()
{
    QClipboard * clipboard = QApplication::clipboard();
    clipboard->setText(m_path.getPathSequence());
}



void PathSpecifyDialog::savePathToFile()
{
    QString defaultFileNameAndPath = g_settings->rememberedPath + "/path_sequence.fasta";
    QString fullFileName = QFileDialog::getSaveFileName(this, "Save path sequence", defaultFileNameAndPath, "FASTA (*.fasta)");

    if (fullFileName != "") //User did not hit cancel
    {
        QFile file(fullFileName);
        file.open(QIODevice::WriteOnly | QIODevice::Text);
        QTextStream out(&file);
        out << m_path.getFasta();
        g_settings->rememberedPath = QFileInfo(fullFileName).absolutePath();
    }
}
