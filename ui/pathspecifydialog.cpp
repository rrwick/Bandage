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
#include "../program/memory.h"

PathSpecifyDialog::PathSpecifyDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PathSpecifyDialog)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() | Qt::Tool);

    ui->pathTextEdit->setPlainText(g_memory->userSpecifiedPathString);
    ui->circularPathCheckBox->setChecked(g_memory->userSpecifiedPathCircular);
    g_memory->pathDialogIsVisible = true;
    checkPathValidity();

    ui->circularPathInfoText->setInfoText("Tick this box to indicate that the path is circular, i.e. there is an edge connecting the "
                                          "last node in the list to the first.<br><br>"
                                          "Circular paths must contain the entirety of their nodes and therefore cannot contain "
                                          "start/end positions.");

    connect(ui->pathTextEdit, SIGNAL(textChanged()), this, SLOT(checkPathValidity()));
    connect(ui->pathTextEdit, SIGNAL(textChanged()), g_graphicsView->viewport(), SLOT(update()));
    connect(ui->circularPathCheckBox, SIGNAL(toggled(bool)), this, SLOT(checkPathValidity()));
    connect(ui->circularPathCheckBox, SIGNAL(toggled(bool)), g_graphicsView->viewport(), SLOT(update()));
    connect(ui->copyButton, SIGNAL(clicked(bool)), this, SLOT(copyPathToClipboard()));
    connect(ui->saveButton, SIGNAL(clicked(bool)), this, SLOT(savePathToFile()));
    connect(this, SIGNAL(rejected()), this, SLOT(deleteLater()));
}


PathSpecifyDialog::~PathSpecifyDialog()
{
    g_memory->pathDialogIsVisible = false;
    delete ui;
}



void PathSpecifyDialog::checkPathValidity()
{
    g_memory->userSpecifiedPathString = ui->pathTextEdit->toPlainText();
    g_memory->userSpecifiedPathCircular = ui->circularPathCheckBox->isChecked();
    g_memory->userSpecifiedPath = Path();

    //Clear out the Path object.  If the string makes a valid path,
    //it will be rebuilt.
    g_memory->userSpecifiedPath = Path();

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
    g_memory->userSpecifiedPath = Path::makeFromString(pathText,
                                                         ui->circularPathCheckBox->isChecked(),
                                                         &pathStringFailure);

    //If the Path turned out to be empty, that means that makeFromString failed.
    if (g_memory->userSpecifiedPath.isEmpty())
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
        int pathLength = g_memory->userSpecifiedPath.getLength();
        ui->validPathLabel->setText("Valid path: " + formatIntForDisplay(pathLength) + " bp");
        setPathValidityUiElements(true);
    }
}


void PathSpecifyDialog::setPathValidityUiElements(bool pathValid)
{
    QPixmap tickCross;
    if (pathValid)
        tickCross = QPixmap(":/icons/tick-128.png");
    else
        tickCross = QPixmap(":/icons/cross-128.png");
    tickCross.setDevicePixelRatio(devicePixelRatio()); //This is a workaround for a Qt bug.  Can possibly remove in the future.  https://bugreports.qt.io/browse/QTBUG-46846
    ui->tickCrossLabel->setPixmap(tickCross);

    ui->copyButton->setEnabled(pathValid);
    ui->saveButton->setEnabled(pathValid);
}


void PathSpecifyDialog::copyPathToClipboard()
{
    QClipboard * clipboard = QApplication::clipboard();
    clipboard->setText(g_memory->userSpecifiedPath.getPathSequence());
}



void PathSpecifyDialog::savePathToFile()
{
    QString defaultFileNameAndPath = g_memory->rememberedPath + "/path_sequence.fasta";
    QString fullFileName = QFileDialog::getSaveFileName(this, "Save path sequence", defaultFileNameAndPath, "FASTA (*.fasta)");

    if (fullFileName != "") //User did not hit cancel
    {
        QFile file(fullFileName);
        file.open(QIODevice::WriteOnly | QIODevice::Text);
        QTextStream out(&file);
        out << g_memory->userSpecifiedPath.getFasta();
        g_memory->rememberedPath = QFileInfo(fullFileName).absolutePath();
    }
}


void PathSpecifyDialog::addNodeName(DeBruijnNode * node)
{
    QString pathText = ui->pathTextEdit->toPlainText();

    //If the node fits on the end of the path add it there.
    Path extendedPath = g_memory->userSpecifiedPath;
    if (g_memory->userSpecifiedPath.canNodeFitOnEnd(node, &extendedPath))
        pathText = extendedPath.getString(true);

    //If not, try the front of the path.
    else if (g_memory->userSpecifiedPath.canNodeFitAtStart(node, &extendedPath))
        pathText = extendedPath.getString(true);

    //If neither of these work, try the reverse complement, first
    //at the end and then at the front.
    //But only do this if we are in single mode.
    else if (!g_settings->doubleMode &&
             g_memory->userSpecifiedPath.canNodeFitOnEnd(node->getReverseComplement(), &extendedPath))
        pathText = extendedPath.getString(true);
    else if (!g_settings->doubleMode &&
             g_memory->userSpecifiedPath.canNodeFitAtStart(node->getReverseComplement(), &extendedPath))
        pathText = extendedPath.getString(true);

    //If all of the above failed, we do nothing.  I.e. if the node cannot be
    //added to the path, it isn't added to the text.

    ui->pathTextEdit->setPlainText(pathText);
}
