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


#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QLatin1String>
#include <QTextStream>
#include <QLocale>
#include <QRegExp>
#include <ogdf/energybased/FMMMLayout.h>
#include <cmath>
#include "../program/settings.h"
#include <QClipboard>
#include <QTransform>
#include <QFontDialog>
#include <QColorDialog>
#include <algorithm>
#include <QFile>
#include <QTextStream>
#include <QScrollBar>
#include "settingsdialog.h"
#include <stdlib.h>
#include <time.h>
#include <QProgressDialog>
#include <QThread>
#include "../program/graphlayoutworker.h"
#include <QRegExp>
#include <QMessageBox>
#include <QInputDialog>
#include <QShortcut>
#include "aboutdialog.h"
#include <QMainWindow>
#include "blastsearchdialog.h"
#include "../graph/assemblygraph.h"
#include "mygraphicsview.h"
#include "graphicsviewzoom.h"
#include "mygraphicsscene.h"
#include "../blast/blastsearch.h"
#include "../graph/debruijnnode.h"
#include "../graph/debruijnedge.h"
#include "../graph/graphicsitemnode.h"
#include "../graph/graphicsitemedge.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    g_assemblyGraph = new AssemblyGraph();

    QApplication::setWindowIcon(QIcon(QPixmap(":/icons/icon.png")));
    g_graphicsView = new MyGraphicsView();
    ui->graphicsViewWidget->layout()->addWidget(g_graphicsView);

    srand(time(NULL));
    setRandomColourFactor();

    //Make a temp directory to hold the BLAST files.
    g_tempDirectory = QDir::tempPath() + "/bandage_temp-" + QString::number(QCoreApplication::applicationPid()) + "/";
    if (!QDir().mkdir(g_tempDirectory))
    {
        QMessageBox::warning(this, "Error", "A temporary directory could not be created.  BLAST search functionality will not be available");
        return;
    }

    g_settings = new Settings();
    m_previousZoomSpinBoxValue = ui->zoomSpinBox->value();
    ui->zoomSpinBox->setMinimum(g_settings->minZoom * 100.0);
    ui->zoomSpinBox->setMaximum(g_settings->maxZoom * 100.0);

    //Fix the width of the objects in the selection panel so it doesn't
    //change size.
    int fixedRightPanelWidth = ui->selectedNodesWidget->sizeHint().width();
    ui->selectedNodesWidget->setFixedWidth(fixedRightPanelWidth);
    ui->selectionSearchWidget->setFixedWidth(fixedRightPanelWidth);
    ui->selectedEdgesWidget->setFixedWidth(fixedRightPanelWidth);

    //The normal height of the QPlainTextEdit objects is a bit much,
    //so fix them at a smaller height.
    ui->selectedNodesTextEdit->setFixedHeight(ui->selectedNodesTextEdit->sizeHint().height() / 2.5);
    ui->selectedEdgesTextEdit->setFixedHeight(ui->selectedEdgesTextEdit->sizeHint().height() / 2.5);

    enableDisableUiElements(NO_GRAPH_LOADED);

    m_graphicsViewZoom = new GraphicsViewZoom(g_graphicsView);
    g_graphicsView->m_zoom = m_graphicsViewZoom;

    m_scene = new MyGraphicsScene(this);
    g_graphicsView->setScene(m_scene);

    g_blastSearch = new BlastSearch();

    setInfoTexts();

    selectionChanged(); //Nothing is selected yet, so this will hide the appropriate labels.
    graphScopeChanged();
    switchColourScheme();

    connect(ui->drawGraphButton, SIGNAL(clicked()), this, SLOT(drawGraph()));
    connect(ui->actionLoad_LastGraph, SIGNAL(triggered()), this, SLOT(loadLastGraph()));
    connect(ui->actionLoad_fastg, SIGNAL(triggered()), this, SLOT(loadFastg()));
    connect(ui->actionExit, SIGNAL(triggered()), this, SLOT(close()));
    connect(ui->graphScopeComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(graphScopeChanged()));
    connect(ui->zoomSpinBox, SIGNAL(valueChanged(double)), this, SLOT(zoomSpinBoxChanged()));
    connect(m_graphicsViewZoom, SIGNAL(zoomed()), this, SLOT(zoomedWithMouseWheel()));
    connect(ui->nodeSequenceToClipboardButton, SIGNAL(clicked()), this, SLOT(copySelectedSequencesToClipboard()));
    connect(ui->nodeSequenceToFileButton, SIGNAL(clicked()), this, SLOT(saveSelectedSequencesToFile()));
    connect(ui->coloursComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(switchColourScheme()));
    connect(ui->actionSave_image_current_view, SIGNAL(triggered()), this, SLOT(saveImageCurrentView()));
    connect(ui->actionSave_image_entire_scene, SIGNAL(triggered()), this, SLOT(saveImageEntireScene()));
    connect(ui->nodeCustomLabelsCheckBox, SIGNAL(toggled(bool)), this, SLOT(setTextDisplaySettings()));
    connect(ui->nodeNumbersCheckBox, SIGNAL(toggled(bool)), this, SLOT(setTextDisplaySettings()));
    connect(ui->nodeLengthsCheckBox, SIGNAL(toggled(bool)), this, SLOT(setTextDisplaySettings()));
    connect(ui->nodeCoveragesCheckBox, SIGNAL(toggled(bool)), this, SLOT(setTextDisplaySettings()));
    connect(ui->textOutlineCheckBox, SIGNAL(toggled(bool)), this, SLOT(setTextDisplaySettings()));
    connect(ui->fontButton, SIGNAL(clicked()), this, SLOT(fontButtonPressed()));
    connect(ui->setNodeCustomColourButton, SIGNAL(clicked()), this, SLOT(setNodeCustomColour()));
    connect(ui->setNodeCustomLabelButton, SIGNAL(clicked()), this, SLOT(setNodeCustomLabel()));
    connect(ui->removeNodeButton, SIGNAL(clicked()), this, SLOT(removeNodes()));
    connect(ui->actionSettings, SIGNAL(triggered()), this, SLOT(openSettingsDialog()));
    connect(ui->selectNodesButton, SIGNAL(clicked()), this, SLOT(selectUserSpecifiedNodes()));
    connect(ui->selectionSearchNodesLineEdit, SIGNAL(returnPressed()), this, SLOT(selectUserSpecifiedNodes()));
    connect(ui->actionAbout, SIGNAL(triggered()), this, SLOT(openAboutDialog()));
    connect(ui->blastSearchButton, SIGNAL(clicked()), this, SLOT(openBlastSearchDialog()));
    connect(ui->blastQueryComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(blastTargetChanged()));
    connect(ui->actionControls_panel, SIGNAL(toggled(bool)), this, SLOT(showHidePanels()));
    connect(ui->actionSelection_panel, SIGNAL(toggled(bool)), this, SLOT(showHidePanels()));

    QShortcut *copyShortcut = new QShortcut(QKeySequence("Ctrl+C"), this);
    connect(copyShortcut, SIGNAL(activated()), this, SLOT(copySelectedSequencesToClipboard()));
    QShortcut *saveShortcut = new QShortcut(QKeySequence("Ctrl+S"), this);
    connect(saveShortcut, SIGNAL(activated()), this, SLOT(saveSelectedSequencesToFile()));
    QShortcut *colourShortcut = new QShortcut(QKeySequence("Ctrl+O"), this);
    connect(colourShortcut, SIGNAL(activated()), this, SLOT(setNodeCustomColour()));
    QShortcut *labelShortcut = new QShortcut(QKeySequence("Ctrl+L"), this);
    connect(labelShortcut, SIGNAL(activated()), this, SLOT(setNodeCustomLabel()));
    QShortcut *removeShortcut1 = new QShortcut(QKeySequence("Backspace"), this);
    connect(removeShortcut1, SIGNAL(activated()), this, SLOT(removeNodes()));
    QShortcut *removeShortcut2 = new QShortcut(QKeySequence("Delete"), this);
    connect(removeShortcut2, SIGNAL(activated()), this, SLOT(removeNodes()));

    //On the Mac, the shortcut keys will be using the command button, not the control button
    //so change the tooltips to reflect this.
#ifdef Q_OS_MAC
    QString command(QChar(0x2318));
    ui->nodeSequenceToClipboardButton->setToolTip(command + 'C');
    ui->nodeSequenceToFileButton->setToolTip(command + 'S');
    ui->setNodeCustomColourButton->setToolTip(command + 'O');
    ui->setNodeCustomLabelButton->setToolTip(command + 'L');
#endif
}

MainWindow::~MainWindow()
{
    cleanUp();
    delete m_graphicsViewZoom;
    delete g_assemblyGraph;
    delete ui;
    delete g_blastSearch;

    QDir(g_tempDirectory).removeRecursively();
}



void MainWindow::cleanUp()
{
    ui->blastQueryComboBox->clear();
    emptyTempDirectory();
    g_blastSearch->cleanUp();
    g_assemblyGraph->cleanUp();
}



void MainWindow::loadLastGraph()
{
    loadGraphFile("LastGraph");
}


void MainWindow::loadFastg()
{
    loadGraphFile("FASTG");
}


void MainWindow::loadGraphFile(QString graphFileType)
{
    QString fullFileName = QFileDialog::getOpenFileName(this, "Load " + graphFileType);

    if (fullFileName != "") //User did not hit cancel
    {
        if ( (graphFileType == "LastGraph" && !checkFileIsLastGraph(fullFileName)) ||
             (graphFileType == "FASTG" && !checkFileIsFastG(fullFileName)) )
        {
            QMessageBox::StandardButton reply;
            reply = QMessageBox::question(this, graphFileType + " file?",
                                          "This file does not appear to be a " + graphFileType + " file."
                                                                                                 "\nDo you want to load it as a " + graphFileType + " file anyway?",
                                          QMessageBox::Yes|QMessageBox::No);
            if (reply == QMessageBox::No)
                return;
        }

        resetScene();
        cleanUp();
        ui->selectionSearchNodesLineEdit->clear();

        if (graphFileType == "LastGraph")
            buildDeBruijnGraphFromLastGraph(fullFileName);
        else if (graphFileType == "FASTG")
        {
            bool succeeded = buildDeBruijnGraphFromFastg(fullFileName);
            if (!succeeded)
            {
                QMessageBox::information(this, "FASTG problem", "There was a problem loading the FASTG file.  "
                                                                "Are you sure it was made by SPAdes version 3.5.0 or later?",
                                         QMessageBox::Close);
                cleanUp();
            }
        }

        enableDisableUiElements(GRAPH_LOADED);
        setWindowTitle("Bandage - " + fullFileName);

        g_assemblyGraph->determineGraphInfo();
        displayGraphDetails();
    }
}




void MainWindow::buildDeBruijnGraphFromLastGraph(QString fullFileName)
{
    QProgressDialog progress("Reading LastGraph file", QString(), 0, 0, this);
    progress.setWindowModality(Qt::WindowModal);
    progress.show();

    QFile inputFile(fullFileName);
    if (inputFile.open(QIODevice::ReadOnly))
    {
        QTextStream in(&inputFile);
        while (!in.atEnd())
        {
            QApplication::processEvents();
            QString line = in.readLine();
            if (line.startsWith("NODE"))
            {
                QStringList nodeDetails = line.split(QRegExp("\\s+"));
                int nodeNumber = nodeDetails.at(1).toInt();
                int nodeLength = nodeDetails.at(2).toInt();

                double nodeCoverage;
                if (nodeLength > 0)
                    nodeCoverage = double(nodeDetails.at(3).toInt()) / nodeLength; //IS THIS COLUMN ($COV_SHORT1) THE BEST ONE TO USE?
                else
                    nodeCoverage = double(nodeDetails.at(3).toInt());

                QByteArray sequence = in.readLine().toLocal8Bit();
                QByteArray revCompSequence = in.readLine().toLocal8Bit();

                DeBruijnNode * node = new DeBruijnNode(nodeNumber, nodeLength, nodeCoverage, sequence);
                DeBruijnNode * reverseComplementNode = new DeBruijnNode(-nodeNumber, nodeLength, nodeCoverage, revCompSequence);
                node->m_reverseComplement = reverseComplementNode;
                reverseComplementNode->m_reverseComplement = node;
                g_assemblyGraph->m_deBruijnGraphNodes.insert(nodeNumber, node);
                g_assemblyGraph->m_deBruijnGraphNodes.insert(-nodeNumber, reverseComplementNode);
            }
            else if (line.startsWith("ARC"))
            {
                QStringList arcDetails = line.split(QRegExp("\\s+"));
                int node1Number = arcDetails.at(1).toInt();
                int node2Number = arcDetails.at(2).toInt();
                g_assemblyGraph->createDeBruijnEdge(node1Number, node2Number);
            }
        }
        inputFile.close();
    }
}



//Returns true if it succeeded, false if it failed.
bool MainWindow::buildDeBruijnGraphFromFastg(QString fullFileName)
{
    QProgressDialog progress("Reading FASTG file", QString(), 0, 0, this);
    progress.setWindowModality(Qt::WindowModal);
    progress.show();

    QFile inputFile(fullFileName);
    if (inputFile.open(QIODevice::ReadOnly))
    {
        std::vector<int> edgeStartingNodeNumbers;
        std::vector<int> edgeEndingNodeNumbers;
        DeBruijnNode * node = 0;

        QTextStream in(&inputFile);
        while (!in.atEnd())
        {
            QApplication::processEvents();

            int nodeNumber;
            int nodeLength;
            double nodeCoverage;

            QString line = in.readLine();

            //If the line starts with a '>', then we are beginning a new node.
            if (line.startsWith(">"))
            {
                line.remove(0, 1); //Remove '>' from start
                line.chop(1); //Remove ';' from end
                QStringList nodeDetails = line.split(":");
                QString thisNode = nodeDetails.at(0);

                //A single quote as the last character indicates a negative node.
                bool negativeNode = thisNode.at(thisNode.size() - 1) == '\'';

                QStringList thisNodeDetails = thisNode.split("_");
                nodeNumber = thisNodeDetails.at(1).toInt();
                nodeLength = thisNodeDetails.at(3).toInt();

                //If this is the second node and its node number is NOT the
                //same as the first node, then this is probably a FASTG file
                //made by an earlier version of SPAdes (before 3.5).
                if (g_assemblyGraph->m_deBruijnGraphNodes.size() == 1)
                {
                    if (g_assemblyGraph->m_deBruijnGraphNodes.first()->m_number != nodeNumber)
                        return false;
                }

                QString nodeCoverageString = thisNodeDetails.at(5);
                if (negativeNode)
                {
                    //It may be necessary to remove a single quote from the end of the coverage
                    if (nodeCoverageString.at(nodeCoverageString.size() - 1) == '\'')
                        nodeCoverageString.chop(1);

                    nodeNumber *= -1;
                }
                nodeCoverage = nodeCoverageString.toDouble();

                //Make the node
                node = new DeBruijnNode(nodeNumber, nodeLength, nodeCoverage, ""); //Sequence string is currently empty - will be added to on subsequent lines of the fastg file
                g_assemblyGraph->m_deBruijnGraphNodes.insert(nodeNumber, node);

                //The second part of nodeDetails is a comma-delimited list of edge nodes.
                //Edges aren't made right now (because the other node might not yet exist),
                //so they are saved into vectors and made after all the nodes have been made.
                if (nodeDetails.size() == 1)
                    continue;
                QStringList edgeNodes = nodeDetails.at(1).split(",");
                for (int i = 0; i < edgeNodes.size(); ++i)
                {
                    QString edgeNode = edgeNodes.at(i);

                    QChar lastChar = edgeNode.at(edgeNode.size() - 1);
                    bool reverseComplement = false;
                    if (lastChar == '\'')
                    {
                        reverseComplement = true;
                        edgeNode.chop(1);
                    }
                    QStringList edgeNodeDetails = edgeNode.split("_");

                    int edgeNodeNumber = edgeNodeDetails.at(1).toInt();
                    if (reverseComplement)
                        edgeNodeNumber *= -1;

                    edgeStartingNodeNumbers.push_back(nodeNumber);
                    edgeEndingNodeNumbers.push_back(edgeNodeNumber);
                }
            }

            //If the line does not start with a '>', then this line is part of the
            //sequence for the last node.
            else
            {
                QByteArray sequenceLine = line.simplified().toLocal8Bit();
                if (node != 0)
                    node->m_sequence.append(sequenceLine);
            }
        }

        inputFile.close();

        //Now for each node, find its reverse complement and make them point to each other.
        QMapIterator<int, DeBruijnNode*> i(g_assemblyGraph->m_deBruijnGraphNodes);
        while (i.hasNext())
        {
            i.next();
            DeBruijnNode * positiveNode = i.value();

            if (positiveNode->m_number > 0)
            {
                DeBruijnNode * negativeNode = g_assemblyGraph->m_deBruijnGraphNodes[-(positiveNode->m_number)];
                if (negativeNode != 0)
                {
                    positiveNode->m_reverseComplement = negativeNode;
                    negativeNode->m_reverseComplement = positiveNode;
                }
            }
        }

        //For each edge, add the edges to the corresponding Node objects.
        for (size_t i = 0; i < edgeStartingNodeNumbers.size(); ++i)
        {
            int node1Number = edgeStartingNodeNumbers[i];
            int node2Number = edgeEndingNodeNumbers[i];
            g_assemblyGraph->createDeBruijnEdge(node1Number, node2Number);
        }

    }

    return true;
}

void MainWindow::buildOgdfGraphFromNodesAndEdges()
{
    if (g_settings->graphScope == WHOLE_GRAPH)
    {
        QMapIterator<int, DeBruijnNode*> i(g_assemblyGraph->m_deBruijnGraphNodes);
        while (i.hasNext())
        {
            i.next();

            //If double mode is off, only positive nodes are drawn.  If it's
            //on, all nodes are drawn.
            if (i.value()->m_number > 0 || g_settings->doubleMode)
                i.value()->m_drawn = true;
        }
    }
    else //The scope is either around specified nodes or around nodes with BLAST hits
    {
        std::vector<DeBruijnNode *> startingNodes;

        if (g_settings->graphScope == AROUND_NODE)
            startingNodes = getNodesFromLineEdit(ui->startingNodesLineEdit);
        else if (g_settings->graphScope == AROUND_BLAST_HITS)
            startingNodes = getNodesFromBlastHits();

        int nodeDistance = ui->nodeDistanceSpinBox->value();

        for (size_t i = 0; i < startingNodes.size(); ++i)
        {
            DeBruijnNode * node = startingNodes[i];
            if (node->m_number < 0)
                node = node->m_reverseComplement;

            node->m_drawn = true;
            node->m_startingNode = true;
            node->labelNeighbouringNodesAsDrawn(nodeDistance, 0);
        }
    }

    //First loop through each node, adding it to OGDF if it is drawn.
    QMapIterator<int, DeBruijnNode*> i(g_assemblyGraph->m_deBruijnGraphNodes);
    while (i.hasNext())
    {
        i.next();
        if (i.value()->m_drawn)
            i.value()->addToOgdfGraph(g_assemblyGraph->m_ogdfGraph);
    }

    //Then loop through each determining its drawn status and adding it
    //to OGDF if it is drawn.
    for (size_t i = 0; i < g_assemblyGraph->m_deBruijnGraphEdges.size(); ++i)
    {
        g_assemblyGraph->m_deBruijnGraphEdges[i]->determineIfDrawn();
        if (g_assemblyGraph->m_deBruijnGraphEdges[i]->m_drawn)
            g_assemblyGraph->m_deBruijnGraphEdges[i]->addToOgdfGraph(g_assemblyGraph->m_ogdfGraph);
    }
}



//Cursory look to see if file appears to be a LastGraph file.
bool MainWindow::checkFileIsLastGraph(QString fullFileName)
{
    return checkFirstLineOfFile(fullFileName, "\\d+\\s+\\d+\\s+\\d+\\s+\\d+");
}

//Cursory look to see if file appears to be a FASTG file.
bool MainWindow::checkFileIsFastG(QString fullFileName)
{
    return checkFirstLineOfFile(fullFileName, ">NODE");
}


bool MainWindow::checkFirstLineOfFile(QString fullFileName, QString regExp)
{
    QFile inputFile(fullFileName);
    if (inputFile.open(QIODevice::ReadOnly))
    {
        QTextStream in(&inputFile);
        if (in.atEnd())
            return false;
        QRegExp rx(regExp);
        QString line = in.readLine();
        if (rx.indexIn(line) != -1)
            return true;
    }
    return false;
}



void MainWindow::displayGraphDetails()
{
    ui->nodeCountLabel->setText(formatIntForDisplay(g_assemblyGraph->m_deBruijnGraphNodes.size() / 2));
    ui->edgeCountLabel->setText(formatIntForDisplay(int(g_assemblyGraph->m_deBruijnGraphEdges.size()) / 2));
    ui->totalLengthLabel->setText(formatIntForDisplay(g_assemblyGraph->m_totalLength));
}



void MainWindow::selectionChanged()
{
    std::vector<DeBruijnNode *> selectedNodes = m_scene->getSelectedNodes();
    std::vector<DeBruijnEdge *> selectedEdges = m_scene->getSelectedEdges();

    if (selectedNodes.size() == 0)
    {
        ui->selectedNodesTextEdit->setPlainText("");
        ui->selectedNodesWidget->setVisible(false);
    }

    else //One or more nodes selected
    {
        ui->selectedNodesWidget->setVisible(true);

        int selectedNodeCount;
        QString selectedNodeCountText;
        QString selectedNodeListText;
        QString selectedNodeLengthText;

        getSelectedNodeInfo(selectedNodeCount, selectedNodeCountText, selectedNodeListText, selectedNodeLengthText);

        if (selectedNodeCount == 1)
        {
            ui->selectedNodesTitleLabel->setText("Selected node");
            ui->nodeSequenceToClipboardButton->setText("Copy sequence to clipboard");
            ui->nodeSequenceToFileButton->setText("Save sequence to FASTA file");
            ui->removeNodeButton->setText("Remove node");
            ui->selectedContigLengthLabel->setText("Length: " + selectedNodeLengthText);
        }
        else
        {
            ui->selectedNodesTitleLabel->setText("Selected nodes (" + selectedNodeCountText + ")");
            ui->nodeSequenceToClipboardButton->setText("Copy sequences to clipboard");
            ui->nodeSequenceToFileButton->setText("Save sequences to FASTA file");
            ui->removeNodeButton->setText("Remove nodes");
            ui->selectedContigLengthLabel->setText("Total length: " + selectedNodeLengthText);
        }

        ui->selectedNodesTextEdit->setPlainText(selectedNodeListText);
    }


    if (selectedEdges.size() == 0)
    {
        ui->selectedEdgesTextEdit->setPlainText("");
        ui->selectedEdgesWidget->setVisible(false);
    }

    else //One or more edges selected
    {
        ui->selectedEdgesWidget->setVisible(true);
        if (selectedEdges.size() == 1)
            ui->selectedEdgesTitleLabel->setText("Selected edge");
        else
            ui->selectedEdgesTitleLabel->setText("Selected edges (" + formatIntForDisplay(int(selectedEdges.size())) + ")");

        ui->selectedEdgesTextEdit->setPlainText(getSelectedEdgeListText());
    }

}


void MainWindow::getSelectedNodeInfo(int & selectedNodeCount, QString & selectedNodeCountText, QString & selectedNodeListText, QString & selectedNodeLengthText)
{
    std::vector<DeBruijnNode *> selectedNodes = m_scene->getSelectedNodes();

    selectedNodeCount = int(selectedNodes.size());
    selectedNodeCountText = formatIntForDisplay(selectedNodeCount);

    long long totalLength = 0;

    for (int i = 0; i < selectedNodeCount; ++i)
    {
        selectedNodeListText += QString::number(selectedNodes[i]->m_number);  //I don't use the formatIntForDisplay function because I don't want commas in the numbers - that will mess up comma-delimited lists
        if (i != int(selectedNodes.size()) - 1)
            selectedNodeListText += ", ";

        totalLength += selectedNodes[i]->m_length;
    }

    selectedNodeLengthText = formatIntForDisplay(totalLength);
}



bool compareEdgePointers(DeBruijnEdge * a, DeBruijnEdge * b)
{
    int aStart = abs(a->m_startingNode->m_number);
    int bStart = abs(b->m_startingNode->m_number);
    int positiveAStart = abs(aStart);
    int positiveBStart = abs(bStart);
    int aEnd = abs(a->m_endingNode->m_number);
    int bEnd = abs(b->m_endingNode->m_number);
    int positiveAEnd = abs(aEnd);
    int positiveBEnd = abs(bEnd);

    if (positiveAStart != positiveBStart)
        return positiveAStart < positiveBStart;

    if (aStart == bStart)
        return positiveAEnd < positiveBEnd;

    return aStart < bStart;
}

QString MainWindow::getSelectedEdgeListText()
{
    std::vector<DeBruijnEdge *> selectedEdges = m_scene->getSelectedEdges();


    std::sort(selectedEdges.begin(), selectedEdges.end(), compareEdgePointers);

    QString edgeText;
    for (size_t i = 0; i < selectedEdges.size(); ++i)
    {
        edgeText += formatIntForDisplay(selectedEdges[i]->m_startingNode->m_number);
        edgeText += " to ";
        edgeText += formatIntForDisplay(selectedEdges[i]->m_endingNode->m_number);
        if (i != selectedEdges.size() - 1)
            edgeText += ", ";
    }

    return edgeText;
}


void MainWindow::graphScopeChanged()
{
    switch (ui->graphScopeComboBox->currentIndex())
    {
    case 0:
        g_settings->graphScope = WHOLE_GRAPH;
        ui->nodeSelectionWidget->setVisible(false);
        ui->distanceWidget->setVisible(false);
        break;
    case 1:
        g_settings->graphScope = AROUND_NODE;
        ui->nodeSelectionWidget->setVisible(true);
        ui->nodeSelectionWidget->setEnabled(true);
        ui->distanceWidget->setVisible(true);
        ui->distanceWidget->setEnabled(true);
        ui->nodeDistanceInfoText->setInfoText("Nodes will be drawn if they are specified in the above list or are "
                                              "within this many steps away from any of those nodes.<br><br>"
                                              "A value of 0 will result in only the specified nodes being drawn. "
                                              "A large value will result in a large section of the graph around "
                                              "the specified nodes being drawn.");
        break;
    case 2:
        g_settings->graphScope = AROUND_BLAST_HITS;
        ui->nodeSelectionWidget->setVisible(false);
        ui->distanceWidget->setVisible(true);
        ui->distanceWidget->setEnabled(true);
        ui->nodeDistanceInfoText->setInfoText("Nodes will be drawn if they contain a BLAST hit or are within this "
                                              "many steps away from nodes with a BLAST hit.<br><br>"
                                              "A value of 0 will result in only nodes with BLAST hits being drawn. "
                                              "A large value will result in a large section of the graph around "
                                              "nodes with BLAST hits being drawn.");
        break;
    }
}



void MainWindow::drawGraph()
{
    if (g_settings->graphScope == AROUND_NODE)
    {
        std::vector<DeBruijnNode *> startingNodes = getNodesFromLineEdit(ui->startingNodesLineEdit);

        if (startingNodes.size() == 0)
        {
            QMessageBox::information(this, "No starting nodes",
                                     "Please enter at least one starting node when drawing the graph using the 'Around node(s)' scope.");
            return;
        }
    }
    else if (g_settings->graphScope == AROUND_BLAST_HITS)
    {
        std::vector<DeBruijnNode *> startingNodes = getNodesFromBlastHits();

        if (startingNodes.size() == 0)
        {
            QMessageBox::information(this, "No BLAST hits",
                                     "To draw the graph around BLAST hits, you must first conduct a BLAST search.");
            return;
        }
    }

    g_settings->doubleMode = ui->doubleNodesRadioButton->isChecked();
    resetScene();
    setRandomColourFactor();
    g_assemblyGraph->clearOgdfGraphAndResetNodes();
    buildOgdfGraphFromNodesAndEdges();
    layoutGraph();
}


void MainWindow::graphLayoutFinished()
{
    addGraphicsItemsToScene();
    setSceneRectangle();
    zoomToFitScene();
    selectionChanged();

    enableDisableUiElements(GRAPH_DRAWN);

    //Move the focus to the view so the user can use keyboard controls to navigate.
    g_graphicsView->setFocus();
}


void MainWindow::resetScene()
{
    m_scene->blockSignals(true);

    g_assemblyGraph->resetEdges();
    delete m_scene;
    m_scene = new MyGraphicsScene(this);
    g_graphicsView->setScene(m_scene);
    connect(m_scene, SIGNAL(selectionChanged()), this, SLOT(selectionChanged()));
    selectionChanged();

    //Undo the graphics view rotation
    g_graphicsView->rotate(-g_graphicsView->m_rotation);
    g_graphicsView->m_rotation = 0.0;
}


std::vector<DeBruijnNode *> MainWindow::getNodesFromLineEdit(QLineEdit * lineEdit, std::vector<QString> * nodesNotInGraph)
{
    std::vector<DeBruijnNode *> returnVector;

    QString nodesString = lineEdit->text();
    nodesString = nodesString.simplified();
    QStringList nodesList = nodesString.split(",");
    for (int i = 0; i < nodesList.size(); ++i)
    {
        int nodeNumber = nodesList.at(i).toInt();
        if (g_assemblyGraph->m_deBruijnGraphNodes.contains(nodeNumber))
            returnVector.push_back(g_assemblyGraph->m_deBruijnGraphNodes[nodeNumber]);
        else if (nodesNotInGraph != 0)
            nodesNotInGraph->push_back(nodesList.at(i).trimmed());
    }

    return returnVector;
}

std::vector<DeBruijnNode *> MainWindow::getNodesFromBlastHits()
{
    std::vector<DeBruijnNode *> returnVector;

    if (g_blastSearch->m_blastQueries.m_queries.size() == 0)
        return returnVector;

    BlastQuery * currentQuery = &(g_blastSearch->m_blastQueries.m_queries[ui->blastQueryComboBox->currentIndex()]);

    for (size_t i = 0; i < g_blastSearch->m_hits.size(); ++i)
    {
        if (g_blastSearch->m_hits[i].m_query == currentQuery)
            returnVector.push_back(g_blastSearch->m_hits[i].m_node);
    }

    return returnVector;
}


void MainWindow::layoutGraph()
{
    //The actual layout is done in a different thread so the UI will stay responsive.
    QProgressDialog * progress = new QProgressDialog("Laying out graph", QString(), 0, 0, this);
    progress->setWindowModality(Qt::WindowModal);
    progress->show();

    QThread * thread = new QThread;
    GraphLayoutWorker * graphLayoutWorker = new GraphLayoutWorker(g_assemblyGraph->m_graphAttributes, g_settings->graphLayoutQuality, g_settings->segmentLength);
    graphLayoutWorker->moveToThread(thread);

    connect(thread, SIGNAL(started()), graphLayoutWorker, SLOT(layoutGraph()));
    connect(graphLayoutWorker, SIGNAL(finishedLayout()), thread, SLOT(quit()));
    connect(graphLayoutWorker, SIGNAL(finishedLayout()), graphLayoutWorker, SLOT(deleteLater()));
    connect(graphLayoutWorker, SIGNAL(finishedLayout()), this, SLOT(graphLayoutFinished()));
    connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
    connect(thread, SIGNAL(finished()), progress, SLOT(deleteLater()));
    thread->start();
}



void MainWindow::addGraphicsItemsToScene()
{
    m_scene->clear();

    double meanDrawnCoverage = g_assemblyGraph->getMeanDeBruijnGraphCoverage(true);

    //First make the GraphicsItemNode objects
    QMapIterator<int, DeBruijnNode*> i(g_assemblyGraph->m_deBruijnGraphNodes);
    while (i.hasNext())
    {
        i.next();
        if (i.value()->m_drawn)
        {
            DeBruijnNode * node = i.value();
            node->m_coverageRelativeToMeanDrawnCoverage = node->m_coverage / meanDrawnCoverage;
            GraphicsItemNode * graphicsItemNode = new GraphicsItemNode(node, g_assemblyGraph->m_graphAttributes);
            node->m_graphicsItemNode = graphicsItemNode;
            graphicsItemNode->setFlag(QGraphicsItem::ItemIsSelectable);
            graphicsItemNode->setFlag(QGraphicsItem::ItemIsMovable);
        }
    }

    g_assemblyGraph->resetAllNodeColours();

    //Then make the GraphicsItemEdge objects and add them to the scene first
    //so they are drawn underneath
    for (size_t i = 0; i < g_assemblyGraph->m_deBruijnGraphEdges.size(); ++i)
    {
        if (g_assemblyGraph->m_deBruijnGraphEdges[i]->m_drawn)
        {
            GraphicsItemEdge * graphicsItemEdge = new GraphicsItemEdge(g_assemblyGraph->m_deBruijnGraphEdges[i]);
            g_assemblyGraph->m_deBruijnGraphEdges[i]->m_graphicsItemEdge = graphicsItemEdge;
            graphicsItemEdge->setFlag(QGraphicsItem::ItemIsSelectable);
            m_scene->addItem(graphicsItemEdge);
        }
    }

    //Now add the GraphicsItemNode objects to the scene so they are drawn
    //on top
    QMapIterator<int, DeBruijnNode*> j(g_assemblyGraph->m_deBruijnGraphNodes);
    while (j.hasNext())
    {
        j.next();
        DeBruijnNode * node = j.value();
        if (node->hasGraphicsItem())
            m_scene->addItem(node->m_graphicsItemNode);
    }

}



void MainWindow::zoomSpinBoxChanged()
{
    int newValue = ui->zoomSpinBox->value();
    double zoomFactor = double(newValue) / m_previousZoomSpinBoxValue;
    setZoomSpinBoxStep();

    m_graphicsViewZoom->gentle_zoom(zoomFactor, SPIN_BOX);

    m_previousZoomSpinBoxValue = newValue;
}

void MainWindow::setZoomSpinBoxStep()
{
    double newSingleStep = ui->zoomSpinBox->value() * (g_settings->zoomFactor - 1.0) * 100.0;

    //Round up to nearest 0.1
    newSingleStep = int((newSingleStep + 0.1) * 10.0) / 10.0;

    ui->zoomSpinBox->setSingleStep(newSingleStep);

}


void MainWindow::zoomedWithMouseWheel()
{
    ui->zoomSpinBox->blockSignals(true);
    double newSpinBoxValue = g_absoluteZoom * 100.0;
    ui->zoomSpinBox->setValue(newSpinBoxValue);
    setZoomSpinBoxStep();
    m_previousZoomSpinBoxValue = newSpinBoxValue;
    ui->zoomSpinBox->blockSignals(false);
}



void MainWindow::zoomToFitScene()
{
    zoomToFitRect(m_scene->sceneRect());
}


void MainWindow::zoomToFitRect(QRectF rect)
{
    double startingZoom = g_graphicsView->transform().m11();
    g_graphicsView->fitInView(rect, Qt::KeepAspectRatio);
    double endingZoom = g_graphicsView->transform().m11();
    double zoomFactor = endingZoom / startingZoom;
    g_absoluteZoom *= zoomFactor;
    double newSpinBoxValue = ui->zoomSpinBox->value() * zoomFactor;

    //Limit the zoom to the minimum and maximum
    if (g_absoluteZoom < g_settings->minZoom)
    {
        double newZoomFactor = g_settings->minZoom / g_absoluteZoom;
        m_graphicsViewZoom->gentle_zoom(newZoomFactor, SPIN_BOX);
        g_absoluteZoom *= newZoomFactor;
        g_absoluteZoom = g_settings->minZoom;
        newSpinBoxValue = g_settings->minZoom * 100.0;
    }
    if (g_absoluteZoom > g_settings->maxZoom)
    {
        double newZoomFactor = g_settings->maxZoom / g_absoluteZoom;
        m_graphicsViewZoom->gentle_zoom(newZoomFactor, SPIN_BOX);
        g_absoluteZoom *= newZoomFactor;
        g_absoluteZoom = g_settings->maxZoom;
        newSpinBoxValue = g_settings->maxZoom * 100.0;
    }

    ui->zoomSpinBox->blockSignals(true);
    ui->zoomSpinBox->setValue(newSpinBoxValue);
    m_previousZoomSpinBoxValue = newSpinBoxValue;
    ui->zoomSpinBox->blockSignals(false);
}


void MainWindow::copySelectedSequencesToClipboard()
{
    std::vector<DeBruijnNode *> selectedNodes = m_scene->getSelectedNodes();
    if (selectedNodes.size() == 0)
        return;

    QClipboard * clipboard = QApplication::clipboard();
    QString clipboardText;

    for (size_t i = 0; i < selectedNodes.size(); ++i)
    {
        clipboardText += selectedNodes[i]->m_sequence;
        if (i != selectedNodes.size() - 1)
            clipboardText += "\n";
    }

    clipboard->setText(clipboardText);
}



void MainWindow::saveSelectedSequencesToFile()
{
    std::vector<DeBruijnNode *> selectedNodes = m_scene->getSelectedNodes();
    if (selectedNodes.size() == 0)
        return;

    QString defaultFileNameAndPath = QDir::homePath() + "/selected_sequences.fasta";

    QString fullFileName = QFileDialog::getSaveFileName(this, "Save selected sequences", defaultFileNameAndPath, "FASTA (*.fasta)");

    if (fullFileName != "") //User did not hit cancel
    {
        QFile file(fullFileName);
        file.open(QIODevice::WriteOnly | QIODevice::Text);
        QTextStream out(&file);

        for (size_t i = 0; i < selectedNodes.size(); ++i)
        {
            out << selectedNodes[i]->getFasta();
            if (i != selectedNodes.size() - 1)
                out << "\n";
        }
    }
}



void MainWindow::switchColourScheme()
{
    switch (ui->coloursComboBox->currentIndex())
    {
    case 0:
        g_settings->nodeColourScheme = ONE_COLOUR;
        break;
    case 1:
        g_settings->nodeColourScheme = COVERAGE_COLOUR;
        break;
    case 2:
        setRandomColourFactor();
        g_settings->nodeColourScheme = RANDOM_COLOURS;
        break;
    case 3:
        setRandomColourFactor();
        g_settings->nodeColourScheme = BLAST_HITS_COLOUR;
        break;
    case 4:
        g_settings->nodeColourScheme = CUSTOM_COLOURS;
        break;
//    case 5:  //CONTIGUITY - THIS OPTION IS TEMPORARILY DISABLED!
//        resetNodeContiguityStatus();
//        g_settings->nodeColourScheme = CONTIGUITY_COLOUR;
//        ui->setNodeColourButton->setVisible(false);
//        ui->contiguityButton->setVisible(true);
//        break;
    }

    g_assemblyGraph->resetAllNodeColours();
    g_graphicsView->viewport()->update();
}



void MainWindow::determineContiguityFromSelectedNode()
{
    g_assemblyGraph->resetNodeContiguityStatus();
    g_assemblyGraph->resetAllNodeColours();

    DeBruijnNode * selectedNode = m_scene->getOneSelectedNode();
    if (selectedNode != 0)
    {
        selectedNode->determineContiguity(0);
        if (!g_settings->doubleMode)
            selectedNode->m_reverseComplement->determineContiguity(0);
        g_graphicsView->viewport()->update();
    }
}



void MainWindow::saveImageCurrentView()
{
    QString defaultFileNameAndPath = QDir::homePath() + "/graph.png";

    QString fullFileName = QFileDialog::getSaveFileName(this, "Save graph image (current view)", defaultFileNameAndPath, "PNG image (*.png)");

    if (fullFileName != "") //User did not hit cancel
    {
        QImage image(g_graphicsView->viewport()->rect().size(), QImage::Format_ARGB32);
        image.fill(Qt::white);
        QPainter painter(&image);
        painter.setRenderHint(QPainter::Antialiasing);

        g_graphicsView->render(&painter);
        image.save(fullFileName);
    }
}

void MainWindow::saveImageEntireScene()
{
    QString defaultFileNameAndPath = QDir::homePath() + "/graph.png";

    QString fullFileName = QFileDialog::getSaveFileName(this, "Save graph image (entire scene)", defaultFileNameAndPath, "PNG image (*.png)");

    if (fullFileName != "") //User did not hit cancel
    {
        QImage image(g_absoluteZoom * m_scene->sceneRect().size().toSize(), QImage::Format_ARGB32);  // Create the image with the exact size of the shrunk scene
        image.fill(Qt::white);
        QPainter painter(&image);
        painter.setRenderHint(QPainter::Antialiasing);
        setSceneRectangle();
        m_scene->render(&painter);
        image.save(fullFileName);
    }
}



void MainWindow::setTextDisplaySettings()
{
    g_settings->displayNodeCustomLabels = ui->nodeCustomLabelsCheckBox->isChecked();
    g_settings->displayNodeNumbers = ui->nodeNumbersCheckBox->isChecked();
    g_settings->displayNodeLengths = ui->nodeLengthsCheckBox->isChecked();
    g_settings->displayNodeCoverages = ui->nodeCoveragesCheckBox->isChecked();
    g_settings->textOutline = ui->textOutlineCheckBox->isChecked();
    g_graphicsView->viewport()->update();
}


void MainWindow::fontButtonPressed()
{
    bool ok;
    g_settings->displayFont = QFontDialog::getFont(&ok, g_settings->displayFont, this);
    if (ok)
        g_graphicsView->viewport()->update();
}



void MainWindow::setNodeCustomColour()
{
    std::vector<DeBruijnNode *> selectedNodes = m_scene->getSelectedNodes();
    if (selectedNodes.size() == 0)
        return;

    QString dialogTitle = "Select custom colour for selected node";
    if (selectedNodes.size() > 1)
        dialogTitle += "s";

    QColor newColour = QColorDialog::getColor(selectedNodes[0]->m_customColour, this, dialogTitle);
    if (newColour.isValid())
    {
        //If the colouring scheme is not currently custom, change it to custom now
        if (ui->coloursComboBox->currentIndex() != 4)
            ui->coloursComboBox->setCurrentIndex(4);

        for (size_t i = 0; i < selectedNodes.size(); ++i)
        {
            selectedNodes[i]->m_customColour = newColour;
            if (selectedNodes[i]->m_graphicsItemNode != 0)
                selectedNodes[i]->m_graphicsItemNode->setNodeColour();

        }
        g_graphicsView->viewport()->update();
    }
}

void MainWindow::setNodeCustomLabel()
{
    std::vector<DeBruijnNode *> selectedNodes = m_scene->getSelectedNodes();
    if (selectedNodes.size() == 0)
        return;

    QString dialogMessage = "Type a custom label for selected node";
    if (selectedNodes.size() > 1)
        dialogMessage += "s";
    dialogMessage += ":";

    bool ok;
    QString newLabel = QInputDialog::getText(this, "Custom label", dialogMessage, QLineEdit::Normal,
                                             selectedNodes[0]->m_customLabel, &ok);

    if (ok)
    {
        //If the custom label option isn't currently on, turn it on now.
        ui->nodeCustomLabelsCheckBox->setChecked(true);

        for (size_t i = 0; i < selectedNodes.size(); ++i)
            selectedNodes[i]->m_customLabel = newLabel;
    }
}


void MainWindow::removeNodes()
{
    std::vector<DeBruijnNode *> selectedNodes = m_scene->getSelectedNodes();
    if (selectedNodes.size() == 0)
        return;

    for (size_t i = 0; i < selectedNodes.size(); ++i)
    {
        //First remove any edges connected to this node
        removeAllGraphicsEdgesFromNode(selectedNodes[i]);

        //If the graph is on single mode, then also try to remove any
        //edges connected to the reverse complement node
        if (!g_settings->doubleMode)
            removeAllGraphicsEdgesFromNode(selectedNodes[i]->m_reverseComplement);

        //Now remove the node itself
        GraphicsItemNode * graphicsItemNode = selectedNodes[i]->m_graphicsItemNode;
        m_scene->removeItem(graphicsItemNode);
        delete graphicsItemNode;
        selectedNodes[i]->m_graphicsItemNode = 0;
    }
}

void MainWindow::removeAllGraphicsEdgesFromNode(DeBruijnNode * node)
{
    for (size_t i = 0; i < node->m_edges.size(); ++i)
    {
        DeBruijnEdge * deBruijnEdge = node->m_edges[i];
        GraphicsItemEdge * graphicsItemEdge = deBruijnEdge->m_graphicsItemEdge;
        if (graphicsItemEdge != 0)
        {
            m_scene->removeItem(graphicsItemEdge);
            delete graphicsItemEdge;
            deBruijnEdge->m_graphicsItemEdge = 0;
        }
    }
}



void MainWindow::openSettingsDialog()
{
    SettingsDialog settingsDialog(this);
    settingsDialog.setWidgetsFromSettings();

    if (settingsDialog.exec()) //The user clicked OK
    {
        Settings settingsBefore = *g_settings;

        settingsDialog.setSettingsFromWidgets();

        //If the contig width was changed, reset the width on each GraphicsItemNode.
        if (settingsBefore.minimumContigWidth != g_settings->minimumContigWidth ||
                settingsBefore.coverageContigWidth != g_settings->coverageContigWidth)
        {
            QMapIterator<int, DeBruijnNode*> i(g_assemblyGraph->m_deBruijnGraphNodes);
            while (i.hasNext())
            {
                i.next();
                GraphicsItemNode * graphicsItemNode = i.value()->m_graphicsItemNode;
                if (graphicsItemNode != 0)
                    graphicsItemNode->setWidth();
            }
        }

        //If any of the colour changed, reset the node colours now.
        if (settingsBefore.uniformPositiveNodeColour != g_settings->uniformPositiveNodeColour ||
                settingsBefore.uniformNegativeNodeColour != g_settings->uniformNegativeNodeColour ||
                settingsBefore.uniformNodeSpecialColour != g_settings->uniformNodeSpecialColour ||
                settingsBefore.lowCoverageColour != g_settings->lowCoverageColour ||
                settingsBefore.highCoverageColour != g_settings->highCoverageColour ||
                settingsBefore.selectionColour != g_settings->selectionColour ||
                settingsBefore.lowCoverageValue != g_settings->lowCoverageValue ||
                settingsBefore.highCoverageValue != g_settings->highCoverageValue)
        {
            g_assemblyGraph->resetAllNodeColours();
            g_graphicsView->viewport()->update();
        }

        g_graphicsView->setAntialiasing(g_settings->antialiasing);
        g_graphicsView->viewport()->update();
    }
}

void MainWindow::selectUserSpecifiedNodes()
{
    if (ui->selectionSearchNodesLineEdit->text().length() == 0)
    {
        QMessageBox::information(this, "No nodes given", "Please enter the numbers of the nodes to find, separated by commas.");
        return;
    }

    m_scene->clearSelection();
    std::vector<QString> nodesNotInGraph;
    std::vector<DeBruijnNode *> nodesToSelect = getNodesFromLineEdit(ui->selectionSearchNodesLineEdit, &nodesNotInGraph);


    //Select each node that actually has a GraphicsItemNode, and build a bounding
    //rectangle so the viewport can focus on the selected node.
    QRectF boundingBox;
    std::vector<int> nodesNotFound;
    int foundNodes = 0;
    for (size_t i = 0; i < nodesToSelect.size(); ++i)
    {
        GraphicsItemNode * graphicsItemNode = nodesToSelect[i]->m_graphicsItemNode;

        //If the GraphicsItemNode isn't found, try the reverse complement.  This
        //is only done for single node mode.
        if (graphicsItemNode == 0 && !g_settings->doubleMode)
            graphicsItemNode = nodesToSelect[i]->m_reverseComplement->m_graphicsItemNode;

        if (graphicsItemNode != 0)
        {
            graphicsItemNode->setSelected(true);
            QRectF thisNodeBoundingBox = graphicsItemNode->boundingRect();
            boundingBox = boundingBox | thisNodeBoundingBox;
            ++foundNodes;
        }
        else
            nodesNotFound.push_back(nodesToSelect[i]->m_number);
    }

    if (foundNodes > 0)
        zoomToFitRect(boundingBox);

    if (nodesNotInGraph.size() > 0 || nodesNotFound.size() > 0)
    {
        QString errorMessage;
        if (nodesNotInGraph.size() > 0)
        {
            errorMessage += "The following nodes are not in the graph:\n";
            for (size_t i = 0; i < nodesNotInGraph.size(); ++i)
            {
                errorMessage += nodesNotInGraph[i];
                if (i != nodesNotInGraph.size() - 1)
                    errorMessage += ", ";
            }
            errorMessage += "\n";
        }
        if (nodesNotFound.size() > 0)
        {
            if (errorMessage.length() > 0)
                errorMessage += "\n";
            errorMessage += "The following nodes are in the graph but not currently displayed:\n";
            for (size_t i = 0; i < nodesNotFound.size(); ++i)
            {
                errorMessage += QString::number(nodesNotFound[i]);
                if (i != nodesNotFound.size() - 1)
                    errorMessage += ", ";
            }
            errorMessage += "\nRedraw the graph with an increased scope to see these nodes.\n";
        }
        QMessageBox::information(this, "Nodes not found", errorMessage);
    }
}


//Choose a random colour factor, but not one too close to a multiple of
//60, as 360 is used in the random hue mod fuction.
void MainWindow::setRandomColourFactor()
{
    do
    {
        g_randomColourFactor = rand() % 1000;
    } while (g_randomColourFactor % 60 < 15 || g_randomColourFactor % 60 > 45);
}


//Expands the scene rectangle a bit beyond the items so they aren't drawn right to the edge.
void MainWindow::setSceneRectangle()
{
    QRectF itemsBoundingRect = m_scene->itemsBoundingRect();
    double width = itemsBoundingRect.width();
    double height = itemsBoundingRect.height();
    double larger = std::max(width, height);

    double margin = larger * 0.05; //5% margin

    m_scene->setSceneRect(itemsBoundingRect.left() - margin, itemsBoundingRect.top() - margin,
                          width + 2 * margin, height + 2 * margin);
}

void MainWindow::openAboutDialog()
{
    AboutDialog aboutDialog(this);
    aboutDialog.exec();
}


void MainWindow::openBlastSearchDialog()
{
    BlastSearchDialog blastSearchDialog(this);

    connect(&blastSearchDialog, SIGNAL(createAllNodesFasta(QString, bool)), this, SLOT(saveAllNodesToFasta(QString, bool)));
    connect(this, SIGNAL(saveAllNodesToFastaFinished()), &blastSearchDialog, SLOT(buildBlastDatabase2()));

    blastSearchDialog.exec();

    //Fill in the blast results combo box
    ui->blastQueryComboBox->clear();
    for (size_t i = 0; i < g_blastSearch->m_blastQueries.m_queries.size(); ++i)
    {
        if (g_blastSearch->m_blastQueries.m_queries[i].m_hits > 0)
            ui->blastQueryComboBox->addItem(g_blastSearch->m_blastQueries.m_queries[i].m_name);
    }

    if (ui->blastQueryComboBox->count() > 0)
    {
        //If the colouring scheme is not currently sequence, change it to custom now
        if (ui->coloursComboBox->currentIndex() != 3)
            ui->coloursComboBox->setCurrentIndex(3);
    }
}


void MainWindow::blastTargetChanged()
{
    g_assemblyGraph->clearAllBlastHitPointers();

    //Add the blast hit pointers to nodes that have a hit for
    //the selected target.
    BlastQuery * currentQuery = g_blastSearch->m_blastQueries.getQueryFromName(ui->blastQueryComboBox->currentText());
    for (size_t i = 0; i < g_blastSearch->m_hits.size(); ++i)
    {
        BlastHit * hit = &(g_blastSearch->m_hits[i]);
        if (hit->m_query == currentQuery)
            hit->m_node->m_blastHits.push_back(hit);
    }

    g_graphicsView->viewport()->update();
}



void MainWindow::saveAllNodesToFasta(QString path, bool includeEmptyNodes)
{
    QFile file(path + "all_nodes.fasta");
    file.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream out(&file);

    QMapIterator<int, DeBruijnNode*> i(g_assemblyGraph->m_deBruijnGraphNodes);
    while (i.hasNext())
    {
        i.next();
        if (includeEmptyNodes || i.value()->m_length > 0)
        {
            out << i.value()->getFasta();
            out << "\n";
        }
    }

    emit saveAllNodesToFastaFinished();
}



void MainWindow::setInfoTexts()
{
    ui->graphScopeInfoText->setInfoText("This controls how much of the assembly graph will be drawn:<ul>"
                                        "<li>'Entire graph': all nodes in the graph will be drawn.  This is "
                                        "appropriate for smaller assembly graphs, but large graphs may take a "
                                        "lot of time and memory to display in their entirety.</li>"
                                        "<li>'Around nodes': you can specify nodes and a distance to "
                                        "limit the graph drawing to a smaller region of the graph.</li>"
                                        "<li>'Around BLAST hits': if you have conducted a BLAST search "
                                        "on this graph, then this option will draw the region(s) of the graph "
                                        "around nodes that contain hits.</li></ul>");
    ui->startingNodesInfoText->setInfoText("Enter a comma-delimited list here of the node numbers. This will "
                                           "define which regions of the graph will be drawn.");
    ui->nodeStyleInfoText->setInfoText("'Single' mode will only draw nodes with positive numbers, not their "
                                       "complement nodes with negative numbers.  This gives a simpler graph, but "
                                       "strand-specific sequences and directionality will be less clear.<br><br>"
                                       "'Double' mode will draw both nodes and their complement nodes.  The nodes "
                                       "will show directionality with an arrow head.  They will initially be "
                                       "drawn on top of each other, but can be manually moved to separate them.");
    ui->drawGraphInfoText->setInfoText("Clicking this button will conduct the graph layout and draw the graph to "
                                       "the screen.  This process is fast for small graphs but can be "
                                       "resource-intensive for large graphs.<br><br>"
                                       "The layout algorithm uses a random seed, so each time this button is "
                                       "clicked you will give different layouts of the same graph.");
    ui->zoomInfoText->setInfoText("This value controls how large the graph appears in Bandage. The zoom level "
                                  "can also be changed by:<ul>"
                                  "<li>holding the Ctrl key and using the mouse wheel over the graph.</li>"
                                  "<li>clicking on the graph display and then using the '+' and '-' keys</li></ul>");
    ui->nodeColourInfoText->setInfoText("This controls the colour of the nodes in the graph:<ul>"
                                        "<li>'Uniform colour': for graphs drawn with the 'entire graph' scope, all "
                                        "nodes will be the same colour.  For graphs drawn with the 'Around nodes' "
                                        "scope, your defined nodes will be drawn in a separate colour.  For "
                                        "graphs drawn with the 'Around BLAST hits' scope, nodes with BLAST hits "
                                        "will be drawn in a separate colour.</li>"
                                        "<li>'Colour by coverage': node colours will be defined by their "
                                        "coverage.</li>"
                                        "<li>'Random colours': nodes will be coloured randomly.  Each time this is "
                                        "selected, new random colours will be chosen.  Negative nodes (when shown "
                                        "in 'Double' mode) will be a darker shade of their complement positive "
                                        "nodes.</li>"
                                        "<li>'Colour using BLAST hits': nodes will be drawn with a light colour "
                                        "and BLAST hits for the currently selected query will be drawn using a "
                                        "rainbow.  Red indicates the start of the query sequence and violet "
                                        "indicates the end.</li>"
                                        "<li>'Custom colours': nodes will be coloured using colours of your "
                                        "choice.  Select one or more nodes and then click the 'Set colour' button "
                                        "to define their colour.</li></ul>"
                                        "See the 'Colours' section of the Bandage settings to control various "
                                        "colouring options.");
    ui->nodeLabelsInfoText->setInfoText("Tick any of the node labelling options to display those labels over "
                                        "nodes in the graph."
                                        "'Number', 'Length' and 'Coverage' labels are created automatically. "
                                        "'Custom' labels must be assigned by you by clicking the 'Set "
                                        "label' button when one or more nodes are selected.");
    ui->nodeFontInfoText->setInfoText("Click the 'Font' button to choose the font used for node labels.<br><br>"
                                      "Ticking 'Text outline' will surround the text with a white outline."
                                      "This can help to make text more readable, especially over darkly "
                                      "coloured nodes, but will obscure a little bit more of the underlying "
                                      "graph.  The thickness of the text outline is configurable in Bandage's "
                                      "settings.");
    ui->blastSearchInfoText->setInfoText("Click this button to open a dialog where a BLAST search for one "
                                         "or more queries can be carried out on the graph.<br><br>"
                                         "After a BLAST search is complete, it will be possible to use the "
                                         "'Around BLAST hits' graph scope and the 'Colour using BLAST "
                                         "hits' colour mode.");
    ui->blastQueryInfoText->setInfoText("After a BLAST search is completed, you can select a query here for use "
                                        "with the 'Around BLAST hits' graph scope and the 'Colour using BLAST "
                                        "hits' colour mode.");
    ui->selectionSearchInfoText->setInfoText("Type a comma-delimited list one or mode node numbers and then click "
                                             "the 'Find node(s)' button to search for nodes in the graph. "
                                             "If the search is successful, the view will zoom into the found nodes "
                                             "and they will be selected.");
    ui->setColourAndLabelInfoText->setInfoText("Custom colours and labels can be applied to selected nodes using "
                                               "these buttons.  They will only be visible when the colouring "
                                               "mode is set to 'Custom colours' and the 'Custom' label option "
                                               "is ticked.");
    ui->removeNodesInfoText->setInfoText("Click this button to remove selected nodes from the drawn graph, along "
                                         "with any edges that connect to those nodes.  This makes no change to "
                                         "the underlying assembly graph, just the visualisation.<br><br>"
                                         "To see a removed nodes in the visualisation again, you must redraw the "
                                         "graph by clicking 'Draw graph'.");
    ui->copySequencesInfoText->setInfoText("Click this button to copy the sequences of all selected nodes to the "
                                           "clipboard.  If multiple nodes are selected, each node will be on its"
                                           "own line without headers.");
    ui->saveSequencesInfoText->setInfoText("Click this button to save the sequences of all selected nodes to a "
                                           "FASTA file.  Each node's' number, length and coverage will be "
                                           "included in its header.");
}



void MainWindow::enableDisableUiElements(UiState uiState)
{
    switch (uiState)
    {
    case NO_GRAPH_LOADED:
        ui->graphDetailsWidget->setEnabled(false);
        ui->graphDrawingWidget->setEnabled(false);
        ui->graphDisplayWidget->setEnabled(false);
        ui->nodeLabelsWidget->setEnabled(false);
        ui->blastSearchWidget->setEnabled(false);
        ui->selectionSearchWidget->setEnabled(false);
        break;
    case GRAPH_LOADED:
        ui->graphDetailsWidget->setEnabled(true);
        ui->graphDrawingWidget->setEnabled(true);
        ui->graphDisplayWidget->setEnabled(false);
        ui->nodeLabelsWidget->setEnabled(false);
        ui->blastSearchWidget->setEnabled(false);
        ui->selectionSearchWidget->setEnabled(false);
        break;
    case GRAPH_DRAWN:
        ui->graphDetailsWidget->setEnabled(true);
        ui->graphDrawingWidget->setEnabled(true);
        ui->graphDisplayWidget->setEnabled(true);
        ui->nodeLabelsWidget->setEnabled(true);
        ui->blastSearchWidget->setEnabled(true);
        ui->selectionSearchWidget->setEnabled(true);
        break;
    }
}


void MainWindow::showHidePanels()
{
    ui->controlsScrollArea->setVisible(ui->actionControls_panel->isChecked());
    ui->selectionScrollArea->setVisible(ui->actionSelection_panel->isChecked());
}
