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
    setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);

    ui->pathTextEdit->setPlainText(g_assemblyGraph->m_userSpecifiedPath.getString());
    ui->circularPathCheckBox->setChecked(g_assemblyGraph->m_userSpecifiedPath.isCircular());
    g_assemblyGraph->m_pathDialogIsVisible = true;
    checkPathValidity();

    ui->circularPathInfoText->setInfoText("Tick this box to indicate that the path is circular, i.e. there is an edge connecting the "
                                          "last node in the list to the first.");

    connect(ui->pathTextEdit, SIGNAL(textChanged()), this, SLOT(checkPathValidity()));
    connect(ui->circularPathCheckBox, SIGNAL(toggled(bool)), this, SLOT(checkPathValidity()));
    connect(ui->copyButton, SIGNAL(clicked(bool)), this, SLOT(copyPathToClipboard()));
    connect(ui->saveButton, SIGNAL(clicked(bool)), this, SLOT(savePathToFile()));
    connect(this, SIGNAL(finished(int)), this, SLOT(deleteLater()));
}

PathSpecifyDialog::~PathSpecifyDialog()
{
    g_assemblyGraph->m_pathDialogIsVisible = false;
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

    g_assemblyGraph->m_userSpecifiedPath = Path::makeFromOrderedNodes(pathNodes, ui->circularPathCheckBox->isChecked());

    if (g_assemblyGraph->m_userSpecifiedPath.isEmpty())
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
    clipboard->setText(g_assemblyGraph->m_userSpecifiedPath.getPathSequence());
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
        out << g_assemblyGraph->m_userSpecifiedPath.getFasta();
        g_settings->rememberedPath = QFileInfo(fullFileName).absolutePath();
    }
}


void PathSpecifyDialog::addNodeName(DeBruijnNode * node)
{
    QString pathText = ui->pathTextEdit->toPlainText();

    //If the node fits on the end of the path add it there.
    if (g_assemblyGraph->m_userSpecifiedPath.canNodeFitOnEnd(node))
    {
        if (pathText.length() > 0)
            pathText += ", ";
        pathText += node->m_name;
    }

    //If not, try the front of the path.
    else if (g_assemblyGraph->m_userSpecifiedPath.canNodeFitAtStart(node))
        pathText = node->m_name + ", " + pathText;

    //If neither of these work, try the reverse complement, first
    //at the end and then at the front.
    //But only do this if we are in single mode.
    else if (!g_settings->doubleMode && g_assemblyGraph->m_userSpecifiedPath.canNodeFitOnEnd(node->m_reverseComplement))
        pathText += ", " + node->m_reverseComplement->m_name;
    else if (!g_settings->doubleMode && g_assemblyGraph->m_userSpecifiedPath.canNodeFitAtStart(node->m_reverseComplement))
        pathText = node->m_reverseComplement->m_name + ", " + pathText;

    //If all of the above failed, just add the node to the end of
    //the list, which will make the list invalid.
    else
        pathText += ", " + node->m_name;

    ui->pathTextEdit->setPlainText(pathText);
}
