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
#include "mygraphicsview.h"

PathSpecifyDialog::PathSpecifyDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PathSpecifyDialog)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);

    ui->pathTextEdit->setPlainText(g_settings->userSpecifiedPathString);
    ui->circularPathCheckBox->setChecked(g_settings->userSpecifiedPathCircular);
    g_settings->pathDialogIsVisible = true;
    checkPathValidity();

    ui->circularPathInfoText->setInfoText("Tick this box to indicate that the path is circular, i.e. there is an edge connecting the "
                                          "last node in the list to the first.");

    connect(ui->pathTextEdit, SIGNAL(textChanged()), this, SLOT(checkPathValidity()));
    connect(ui->pathTextEdit, SIGNAL(textChanged()), g_graphicsView->viewport(), SLOT(update()));
    connect(ui->circularPathCheckBox, SIGNAL(toggled(bool)), this, SLOT(checkPathValidity()));
    connect(ui->circularPathCheckBox, SIGNAL(toggled(bool)), g_graphicsView->viewport(), SLOT(update()));
    connect(ui->copyButton, SIGNAL(clicked(bool)), this, SLOT(copyPathToClipboard()));
    connect(ui->saveButton, SIGNAL(clicked(bool)), this, SLOT(savePathToFile()));
    connect(this, SIGNAL(finished(int)), this, SLOT(deleteLater()));
}

PathSpecifyDialog::~PathSpecifyDialog()
{
    g_settings->pathDialogIsVisible = false;
    delete ui;
}



void PathSpecifyDialog::checkPathValidity()
{
    g_settings->userSpecifiedPathString = ui->pathTextEdit->toPlainText();
    g_settings->userSpecifiedPathCircular = ui->circularPathCheckBox->isChecked();
    g_settings->userSpecifiedPath = Path();

    //Clear out the Path object.  If the string makes a valid path,
    //it will be rebuilt.
    m_path = Path();

    //If there is no graph loaded, then no path can be valid.
    if (g_assemblyGraph->m_deBruijnGraphNodes.size() == 0)
    {
        ui->validPathLabel->setText("Invalid path: no graph is currently loaded");
        setPathValidityUiElements(false);
        return;
    }

    //Create a path from the user-supplied string.
    QString pathStringFailure;
    QString pathText = ui->pathTextEdit->toPlainText().simplified();
    m_path = Path::makeFromString(pathText,
                                  ui->circularPathCheckBox->isChecked(),
                                  &pathStringFailure);
    g_settings->userSpecifiedPath = m_path;

    //If the Path turned out to be empty, that means that makeFromString failed.
    if (m_path.isEmpty())
    {
        if (pathText == "")
            ui->validPathLabel->setText("No path specified");
        else
            ui->validPathLabel->setText("Invalid path: " + pathStringFailure);

        setPathValidityUiElements(false);
    }

    //If the Path isn't empty, then we have succeeded!
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


void PathSpecifyDialog::addNodeName(DeBruijnNode * node)
{
    QString pathText = ui->pathTextEdit->toPlainText();

    //If the node fits on the end of the path add it there.
    Path extendedPath = m_path;
    if (m_path.canNodeFitOnEnd(node, &extendedPath))
        pathText = extendedPath.getString(true);

    //If not, try the front of the path.
    else if (m_path.canNodeFitAtStart(node, &extendedPath))
        pathText = extendedPath.getString(true);

    //If neither of these work, try the reverse complement, first
    //at the end and then at the front.
    //But only do this if we are in single mode.
    else if (!g_settings->doubleMode &&
             m_path.canNodeFitOnEnd(node->m_reverseComplement, &extendedPath))
        pathText = extendedPath.getString(true);
    else if (!g_settings->doubleMode &&
             m_path.canNodeFitAtStart(node->m_reverseComplement, &extendedPath))
        pathText = extendedPath.getString(true);

    //If all of the above failed, just add the node to the end of
    //the list, which will make the list invalid.
    else
    {
        Path pathCopy = m_path;
        pathCopy.extendPathToIncludeEntirityOfNodes();
        pathText = pathCopy.getString(true) + ", " + node->m_name;
    }

    ui->pathTextEdit->setPlainText(pathText);
}
