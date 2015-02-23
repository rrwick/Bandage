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
#include "../program/globals.h"
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

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QApplication::setWindowIcon(QIcon(QPixmap(":/icons/icon.png")));
    g_graphicsView = new MyGraphicsView();
    ui->graphicsViewWidget->layout()->addWidget(g_graphicsView);

    srand(time(NULL));
    setRandomColourFactor();

    g_settings = new Settings();
    m_previousZoomSpinBoxValue = ui->zoomSpinBox->value();
    ui->zoomSpinBox->setMinimum(g_settings->minZoom * 100.0);
    ui->zoomSpinBox->setMaximum(g_settings->maxZoom * 100.0);

    //Disable these until a graph is loaded or drawn
    ui->graphDrawingGroupBox->setEnabled(false);
    ui->selectionSearchGroupBox->setEnabled(false);

    m_graphicsViewZoom = new GraphicsViewZoom(g_graphicsView);
    g_graphicsView->m_zoom = m_graphicsViewZoom;

    m_scene = new MyGraphicsScene(this);
    g_graphicsView->setScene(m_scene);

    m_ogdfGraph = new ogdf::Graph();
    m_graphAttributes = new ogdf::GraphAttributes(*m_ogdfGraph, ogdf::GraphAttributes::nodeGraphics |
                                                  ogdf::GraphAttributes::edgeGraphics);

    int fixedRightPanelWidth = ui->selectedNodesGroupBox->sizeHint().width();
    ui->selectedNodesGroupBox->setFixedWidth(fixedRightPanelWidth);
    ui->selectionSearchGroupBox->setFixedWidth(fixedRightPanelWidth);
    ui->selectedEdgesGroupBox->setFixedWidth(fixedRightPanelWidth);

    //Fix the scroll areas' sizes.  The numbers added on were what I needed to prevent the horizontal
    //scroll bar from showing.  I'm not sure why they are necessary.
    ui->controlsScrollArea->setFixedWidth(ui->controlsScrollAreaWidgetContents->sizeHint().width() + 45);
    ui->selectionScrollArea->setFixedWidth(fixedRightPanelWidth + 25);


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
    connect(ui->contiguityButton, SIGNAL(clicked()), this, SLOT(determineContiguityFromSelectedNode()));
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
    connect(ui->setNodeColourButton, SIGNAL(clicked()), this, SLOT(setNodeColour()));
    connect(ui->removeNodeButton, SIGNAL(clicked()), this, SLOT(removeNodes()));
    connect(ui->actionSettings, SIGNAL(triggered()), this, SLOT(openSettingsDialog()));
    connect(ui->selectNodesButton, SIGNAL(clicked()), this, SLOT(selectUserSpecifiedNodes()));
    connect(ui->selectionSearchNodesLineEdit, SIGNAL(returnPressed()), this, SLOT(selectUserSpecifiedNodes()));
    connect(ui->actionAbout, SIGNAL(triggered()), this, SLOT(openAboutDialog()));

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
    delete ui;
}



void MainWindow::cleanUp()
{
    //    clearOgdfGraph();

    QMapIterator<int, DeBruijnNode*> i(m_deBruijnGraphNodes);
    while (i.hasNext())
    {
        i.next();
        delete i.value();
    }
    m_deBruijnGraphNodes.clear();

    for (size_t i = 0; i < m_deBruijnGraphEdges.size(); ++i)
        delete m_deBruijnGraphEdges[i];
    m_deBruijnGraphEdges.clear();
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

        //Disable UI elements that aren't applicable until the graph is drawn
        ui->graphDisplayGroupBox->setEnabled(false);
        ui->nodeLabelsGroupBox->setEnabled(false);

        setWindowTitle("Bandage - " + fullFileName);
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

void MainWindow::buildDeBruijnGraphFromLastGraph(QString fullFileName)
{
    QProgressDialog progress("Reading LastGraph file", QString(), 0, 0, this);
    progress.setWindowModality(Qt::WindowModal);
    progress.show();

    int nodeCount = 0;
    int edgeCount = 0;
    long long totalLength = 0;

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
                m_deBruijnGraphNodes.insert(nodeNumber, node);
                m_deBruijnGraphNodes.insert(-nodeNumber, reverseComplementNode);

                ++nodeCount;
                totalLength += nodeLength;
            }
            else if (line.startsWith("ARC"))
            {
                QStringList arcDetails = line.split(QRegExp("\\s+"));
                int node1Number = arcDetails.at(1).toInt();
                int node2Number = arcDetails.at(2).toInt();
                createDeBruijnEdge(node1Number, node2Number);
                ++edgeCount;
            }
        }
        inputFile.close();
    }

    displayGraphDetails(nodeCount, edgeCount, totalLength, getMeanDeBruijnGraphCoverage());
    ui->graphDrawingGroupBox->setEnabled(true);
}



//Returns true if it succeeded, false if it failed.
bool MainWindow::buildDeBruijnGraphFromFastg(QString fullFileName)
{
    QProgressDialog progress("Reading FASTG file", QString(), 0, 0, this);
    progress.setWindowModality(Qt::WindowModal);
    progress.show();

    int nodeCount = 0;
    int edgeCount = 0;
    long long totalLength = 0;

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
                if (m_deBruijnGraphNodes.size() == 1)
                {
                    if (m_deBruijnGraphNodes.first()->m_number != nodeNumber)
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
                else
                {
                    ++nodeCount;
                    totalLength += nodeLength;
                }
                nodeCoverage = nodeCoverageString.toDouble();

                //Make the node
                node = new DeBruijnNode(nodeNumber, nodeLength, nodeCoverage, ""); //Sequence string is currently empty - will be added to on subsequent lines of the fastg file
                m_deBruijnGraphNodes.insert(nodeNumber, node);

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
        QMapIterator<int, DeBruijnNode*> i(m_deBruijnGraphNodes);
        while (i.hasNext())
        {
            i.next();
            DeBruijnNode * positiveNode = i.value();

            if (positiveNode->m_number > 0)
            {
                DeBruijnNode * negativeNode = m_deBruijnGraphNodes[-(positiveNode->m_number)];
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
            createDeBruijnEdge(node1Number, node2Number);
            ++edgeCount;
        }

    }

    displayGraphDetails(nodeCount, edgeCount/2, totalLength, getMeanDeBruijnGraphCoverage());
    ui->graphDrawingGroupBox->setEnabled(true);

    return true;
}



//This function makes a double edge: in one direction for the given nodes
//and the opposite direction for their reverse complements.  It adds the
//new edges to the vector here and to the nodes themselves.
void MainWindow::createDeBruijnEdge(int node1Number, int node2Number)
{
    //Quit if any of the nodes don't exist.
    if (!m_deBruijnGraphNodes.contains(node1Number) ||
            !m_deBruijnGraphNodes.contains(node2Number) ||
            !m_deBruijnGraphNodes.contains(-node1Number) ||
            !m_deBruijnGraphNodes.contains(-node2Number))
        return;

    DeBruijnNode * node1 = m_deBruijnGraphNodes[node1Number];
    DeBruijnNode * node2 = m_deBruijnGraphNodes[node2Number];
    DeBruijnNode * negNode1 = m_deBruijnGraphNodes[-node1Number];
    DeBruijnNode * negNode2 = m_deBruijnGraphNodes[-node2Number];

    //Quit if the edge already exists
    for (size_t i = 0; i < node1->m_edges.size(); ++i)
    {
        if (node1->m_edges[i]->m_endingNode == node2)
            return;
    }

    DeBruijnEdge * forwardEdge = new DeBruijnEdge(node1, node2);
    DeBruijnEdge * backwardEdge = new DeBruijnEdge(negNode2, negNode1);

    forwardEdge->m_reverseComplement = backwardEdge;
    backwardEdge->m_reverseComplement = forwardEdge;

    m_deBruijnGraphEdges.push_back(forwardEdge);
    m_deBruijnGraphEdges.push_back(backwardEdge);

    node1->addEdge(forwardEdge);
    node2->addEdge(forwardEdge);
    negNode1->addEdge(backwardEdge);
    negNode2->addEdge(backwardEdge);
}



void MainWindow::displayGraphDetails(int nodeCount, int edgeCount, long long totalLength, double meanCoverage)
{
    ui->graphDetailsGroupBox->setEnabled(true);
    ui->nodeCountLabel->setText(formatIntForDisplay(nodeCount));
    ui->edgeCountLabel->setText(formatIntForDisplay(edgeCount));
    ui->totalLengthLabel->setText(formatIntForDisplay(totalLength));
    ui->meanCoverageLabel->setText(formatDoubleForDisplay(meanCoverage, 1));
}



void MainWindow::selectionChanged()
{
    std::vector<DeBruijnNode *> selectedNodes = m_scene->getSelectedNodes();
    std::vector<DeBruijnEdge *> selectedEdges = m_scene->getSelectedEdges();

    if (selectedNodes.size() == 1)
        ui->contiguityButton->setEnabled(true);
    else
        ui->contiguityButton->setEnabled(false);

    //If both nodes and edges are selected, it's necessary to space
    //the UI elements out a bit
    if (selectedNodes.size() > 0 && selectedEdges.size() > 0)
        ui->currentSelectionSpacerWidget->setVisible(true);
    else
        ui->currentSelectionSpacerWidget->setVisible(false);

    if (selectedNodes.size() == 0)
    {
        ui->selectedNodesGroupBox->setVisible(false);
    }

    else //One or more nodes selected
    {
        ui->selectedNodesGroupBox->setVisible(true);


        if (selectedNodes.size() == 1)
        {
            ui->selectedNodesGroupBox->setTitle("Currently selected node");
            ui->nodeSequenceToClipboardButton->setText("Copy sequence to clipboard");
            ui->nodeSequenceToFileButton->setText("Save sequence to FASTA file");
            ui->removeNodeButton->setText("Remove node");
        }
        else
        {
            ui->selectedNodesGroupBox->setTitle("Currently selected nodes");
            ui->nodeSequenceToClipboardButton->setText("Copy sequences to clipboard");
            ui->nodeSequenceToFileButton->setText("Save sequences to FASTA file");
            ui->removeNodeButton->setText("Remove nodes");
        }

        int selectedNodeCount;
        QString selectedNodeCountText;
        QString selectedNodeListText;
        QString selectedNodeLengthText;

        getSelectedNodeInfo(selectedNodeCount, selectedNodeCountText, selectedNodeListText, selectedNodeLengthText);

        if (selectedNodeCount == 1)
        {
            ui->selectedContigNumberLabel->setText("<b>Selected node:</b> " + selectedNodeListText);
            ui->selectedContigLengthLabel->setText("<b>Length:</b> " + selectedNodeLengthText);
        }
        else
        {
            ui->selectedContigNumberLabel->setText("<b>Selected nodes (" + selectedNodeCountText + "):</b> " + selectedNodeListText);
            ui->selectedContigLengthLabel->setText("<b>Total length:</b> " + selectedNodeLengthText);
        }
    }


    if (selectedEdges.size() == 0)
    {
        ui->selectedEdgesGroupBox->setVisible(false);
    }

    else //One or more edges selected
    {
        ui->selectedEdgesGroupBox->setVisible(true);
        if (selectedEdges.size() == 1)
            ui->selectedEdgesGroupBox->setTitle("Currently selected edge");
        else
            ui->selectedEdgesGroupBox->setTitle("Currently selected edges");

        ui->selectedEdgesLabel->setText(getSelectedEdgeListText());
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
        break;
    }
}



void MainWindow::drawGraph()
{
    if (g_settings->graphScope == AROUND_NODE)
    {
        std::vector<DeBruijnNode *> startingNodes = getNodeNumbersFromLineEdit(ui->startingNodesLineEdit);

        if (startingNodes.size() == 0)
        {
            QMessageBox::information(this, "No starting nodes",
                                     "Please enter at least one starting node when drawing the graph using the 'Around node(s)' scope.");
            return;
        }
    }

    g_settings->doubleMode = ui->doubleNodesRadioButton->isChecked();
    resetScene();
    setRandomColourFactor();
    clearOgdfGraphAndResetNodes();
    buildOgdfGraphFromNodesAndEdges();
    layoutGraph();
}


void MainWindow::graphLayoutFinished()
{
    addGraphicsItemsToScene();
    setSceneRectangle();
    zoomToFitScene();
    selectionChanged();

    ui->graphDisplayGroupBox->setEnabled(true);
    ui->nodeLabelsGroupBox->setEnabled(true);
    ui->actionSave_image_current_view->setEnabled(true);
    ui->actionSave_image_entire_scene->setEnabled(true);
    ui->selectionSearchGroupBox->setEnabled(true);

    //Move the focus to the view so the user can use keyboard controls to navigate.
    g_graphicsView->setFocus();
}


void MainWindow::resetScene()
{
    //The scene to be deleted should emit signals, or else the selectionChanged
    //signal could alter what's in the startingNodes line edit.
    m_scene->blockSignals(true);

    for (size_t i = 0; i < m_deBruijnGraphEdges.size(); ++i)
        m_deBruijnGraphEdges[i]->reset();
    delete m_scene;
    m_scene = new MyGraphicsScene(this);
    g_graphicsView->setScene(m_scene);
    connect(m_scene, SIGNAL(selectionChanged()), this, SLOT(selectionChanged()));

    //Undo the graphics view rotation
    g_graphicsView->rotate(-g_graphicsView->m_rotation);
    g_graphicsView->m_rotation = 0.0;
}



void MainWindow::clearOgdfGraphAndResetNodes()
{
    QMapIterator<int, DeBruijnNode*> i(m_deBruijnGraphNodes);
    while (i.hasNext())
    {
        i.next();
        i.value()->resetNode();
    }

    m_ogdfGraph->clear();
}






//http://www.code10.info/index.php?option=com_content&view=article&id=62:articledna-reverse-complement&catid=49:cat_coding_algorithms_bioinformatics&Itemid=74
QByteArray MainWindow::getReverseComplement(QByteArray forwardSequence)
{
    QByteArray reverseComplement;

    for (int i = forwardSequence.length() - 1; i >= 0; --i)
    {
        char letter = forwardSequence.at(i);

        switch (letter)
        {
        case 'A': reverseComplement.append('T'); break;
        case 'T': reverseComplement.append('A'); break;
        case 'G': reverseComplement.append('C'); break;
        case 'C': reverseComplement.append('G'); break;
        case 'a': reverseComplement.append('t'); break;
        case 't': reverseComplement.append('a'); break;
        case 'g': reverseComplement.append('c'); break;
        case 'c': reverseComplement.append('g'); break;
        case 'R': reverseComplement.append('Y'); break;
        case 'Y': reverseComplement.append('R'); break;
        case 'S': reverseComplement.append('S'); break;
        case 'W': reverseComplement.append('W'); break;
        case 'K': reverseComplement.append('M'); break;
        case 'M': reverseComplement.append('K'); break;
        case 'r': reverseComplement.append('y'); break;
        case 'y': reverseComplement.append('r'); break;
        case 's': reverseComplement.append('s'); break;
        case 'w': reverseComplement.append('w'); break;
        case 'k': reverseComplement.append('m'); break;
        case 'm': reverseComplement.append('k'); break;
        case 'B': reverseComplement.append('V'); break;
        case 'D': reverseComplement.append('H'); break;
        case 'H': reverseComplement.append('D'); break;
        case 'V': reverseComplement.append('B'); break;
        case 'b': reverseComplement.append('v'); break;
        case 'd': reverseComplement.append('h'); break;
        case 'h': reverseComplement.append('d'); break;
        case 'v': reverseComplement.append('b'); break;
        case 'N': reverseComplement.append('N'); break;
        case 'n': reverseComplement.append('n'); break;
        case '.': reverseComplement.append('.'); break;
        case '-': reverseComplement.append('-'); break;
        case '?': reverseComplement.append('?'); break;
        }
    }

    return reverseComplement;
}



void MainWindow::buildOgdfGraphFromNodesAndEdges()
{
    if (g_settings->graphScope == WHOLE_GRAPH)
    {
        QMapIterator<int, DeBruijnNode*> i(m_deBruijnGraphNodes);
        while (i.hasNext())
        {
            i.next();

            //If double mode is off, only positive nodes are drawn.  If it's
            //on, all nodes are drawn.
            if (i.value()->m_number > 0 || g_settings->doubleMode)
                i.value()->m_drawn = true;
        }
    }
    else if (g_settings->graphScope == AROUND_NODE)
    {
        //If only some nodes are being drawn, they are the starting nodes.

        std::vector<DeBruijnNode *> startingNodes = getNodeNumbersFromLineEdit(ui->startingNodesLineEdit);
        int nodeDistance = ui->nodeDistanceSpinBox->value();

        for (size_t i = 0; i < startingNodes.size(); ++i)
        {
            startingNodes[i]->m_drawn = true;
            startingNodes[i]->m_startingNode = true;
            startingNodes[i]->labelNeighbouringNodesAsDrawn(nodeDistance, 0);
        }
    }

    //First loop through each node, adding it to OGDF if it is drawn.
    QMapIterator<int, DeBruijnNode*> i(m_deBruijnGraphNodes);
    while (i.hasNext())
    {
        i.next();
        if (i.value()->m_drawn)
            i.value()->addToOgdfGraph(m_ogdfGraph);
    }

    //Then loop through each determining its drawn status and adding it
    //to OGDF if it is drawn.
    for (size_t i = 0; i < m_deBruijnGraphEdges.size(); ++i)
    {
        m_deBruijnGraphEdges[i]->determineIfDrawn();
        if (m_deBruijnGraphEdges[i]->m_drawn)
            m_deBruijnGraphEdges[i]->addToOgdfGraph(m_ogdfGraph);
    }
}


std::vector<DeBruijnNode *> MainWindow::getNodeNumbersFromLineEdit(QLineEdit * lineEdit)
{
    std::vector<DeBruijnNode *> returnVector;

    QString nodesString = lineEdit->text();
    nodesString = nodesString.simplified();
    QStringList nodesList = nodesString.split(",");
    for (int i = 0; i < nodesList.size(); ++i)
    {
        int nodeNumber = nodesList.at(i).toInt();
        if (m_deBruijnGraphNodes.contains(nodeNumber))
            returnVector.push_back(m_deBruijnGraphNodes[nodeNumber]);
    }

    return returnVector;
}


void MainWindow::layoutGraph()
{
    //There is an unresolved issue with doing the graph layout in a thread on Mac OS X.
    //Therefore, if the OS is not Mac, a thread will be used to keep the UI responsive
    //during layout.  But if the OS is Mac, then the layout will be done in this function
    //in the main thread.

#ifndef Q_OS_MAC

    QProgressDialog * progress = new QProgressDialog("Laying out graph", QString(), 0, 0, this);
    progress->setWindowModality(Qt::WindowModal);
    progress->show();

    QThread * thread = new QThread;
    GraphLayoutWorker * graphLayoutWorker = new GraphLayoutWorker(m_graphAttributes, g_settings->graphLayoutQuality, g_settings->segmentLength);
    graphLayoutWorker->moveToThread(thread);

    connect(thread, SIGNAL(started()), graphLayoutWorker, SLOT(layoutGraph()));
    connect(graphLayoutWorker, SIGNAL(finishedLayout()), thread, SLOT(quit()));
    connect(graphLayoutWorker, SIGNAL(finishedLayout()), graphLayoutWorker, SLOT(deleteLater()));
    connect(graphLayoutWorker, SIGNAL(finishedLayout()), this, SLOT(graphLayoutFinished()));
    connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
    connect(thread, SIGNAL(finished()), progress, SLOT(deleteLater()));
    thread->start();

#else

    QProgressDialog progress("Laying out graph", QString(), 0, 0, this);
    progress.setWindowModality(Qt::WindowModal);
    progress.show();

    ogdf::FMMMLayout fmmm;

    fmmm.useHighLevelOptions(false);
    fmmm.initialPlacementForces(ogdf::FMMMLayout::ipfRandomTime);
    fmmm.unitEdgeLength(g_settings->segmentLength);
    fmmm.newInitialPlacement(true);

    switch (g_settings->graphLayoutQuality)
    {
    case 0:
        fmmm.fixedIterations(15);
        fmmm.fineTuningIterations(10);
        fmmm.nmPrecision(2);
        break;
    case 1:
        fmmm.fixedIterations(30);
        fmmm.fineTuningIterations(20);
        fmmm.nmPrecision(4);
        break;
    case 2:
        fmmm.fixedIterations(60);
        fmmm.fineTuningIterations(40);
        fmmm.nmPrecision(6);
        break;
    case 3:
        fmmm.fixedIterations(120);
        fmmm.fineTuningIterations(80);
        fmmm.nmPrecision(8);
        break;
    case 4:
        fmmm.fixedIterations(240);
        fmmm.fineTuningIterations(600);
        fmmm.nmPrecision(20);
        break;
    }

    fmmm.call(*m_graphAttributes);
    graphLayoutFinished();

#endif
}



void MainWindow::addGraphicsItemsToScene()
{
    m_scene->clear();

    double meanDrawnCoverage = getMeanDeBruijnGraphCoverage(true);

    //First make the GraphicsItemNode objects
    QMapIterator<int, DeBruijnNode*> i(m_deBruijnGraphNodes);
    while (i.hasNext())
    {
        i.next();
        if (i.value()->m_drawn)
        {
            DeBruijnNode * node = i.value();
            node->m_coverageRelativeToMeanDrawnCoverage = node->m_coverage / meanDrawnCoverage;
            GraphicsItemNode * graphicsItemNode = new GraphicsItemNode(node, m_graphAttributes);
            node->m_graphicsItemNode = graphicsItemNode;
            graphicsItemNode->setFlag(QGraphicsItem::ItemIsSelectable);
            graphicsItemNode->setFlag(QGraphicsItem::ItemIsMovable);
        }
    }

    resetAllNodeColours();

    //Then make the GraphicsItemEdge objects and add them to the scene first
    //so they are drawn underneath
    for (size_t i = 0; i < m_deBruijnGraphEdges.size(); ++i)
    {
        if (m_deBruijnGraphEdges[i]->m_drawn)
        {
            GraphicsItemEdge * graphicsItemEdge = new GraphicsItemEdge(m_deBruijnGraphEdges[i]);
            m_deBruijnGraphEdges[i]->m_graphicsItemEdge = graphicsItemEdge;
            graphicsItemEdge->setFlag(QGraphicsItem::ItemIsSelectable);
            m_scene->addItem(graphicsItemEdge);
        }
    }

    //Now add the GraphicsItemNode objects to the scene so they are drawn
    //on top
    QMapIterator<int, DeBruijnNode*> j(m_deBruijnGraphNodes);
    while (j.hasNext())
    {
        j.next();
        DeBruijnNode * node = j.value();
        if (node->hasGraphicsItem())
            m_scene->addItem(node->m_graphicsItemNode);
    }

}


double MainWindow::getMeanDeBruijnGraphCoverage(bool drawnNodesOnly)
{
    int nodeCount = 0;
    long double coverageSum = 0.0;
    long long totalLength = 0;

    QMapIterator<int, DeBruijnNode*> i(m_deBruijnGraphNodes);
    while (i.hasNext())
    {
        i.next();
        DeBruijnNode * node = i.value();

        if (drawnNodesOnly && !node->m_drawn)
            continue;

        if (node->m_number > 0)
        {
            ++nodeCount;
            totalLength += node->m_length;
            coverageSum += node->m_length * node->m_coverage;
        }
    }

    if (totalLength == 0)
        return 0.0;
    else
        return coverageSum / totalLength;
}

double MainWindow::getMaxDeBruijnGraphCoverageOfDrawnNodes()
{
    double maxCoverage = 1.0;

    QMapIterator<int, DeBruijnNode*> i(m_deBruijnGraphNodes);
    while (i.hasNext())
    {
        i.next();

        if (i.value()->m_graphicsItemNode != 0 && i.value()->m_coverage > maxCoverage)
            maxCoverage = i.value()->m_coverage;
    }

    return maxCoverage;
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
        ui->setNodeColourButton->setVisible(true);
        ui->contiguityButton->setVisible(false);
        break;
    case 1:
        g_settings->nodeColourScheme = COVERAGE_COLOUR;
        ui->setNodeColourButton->setVisible(false);
        ui->contiguityButton->setVisible(false);
        break;
    case 2:
        setRandomColourFactor();
        g_settings->nodeColourScheme = RANDOM_COLOURS;
        ui->setNodeColourButton->setVisible(false);
        ui->contiguityButton->setVisible(false);
        break;
    case 3:
        g_settings->nodeColourScheme = CUSTOM_COLOURS;
        ui->setNodeColourButton->setVisible(false);
        ui->contiguityButton->setVisible(false);
        break;
    case 4:  //CONTIGUITY - THIS OPTION IS TEMPORARILY DISABLED!
        resetNodeContiguityStatus();
        g_settings->nodeColourScheme = CONTIGUITY_COLOUR;
        ui->setNodeColourButton->setVisible(false);
        ui->contiguityButton->setVisible(true);
        break;
    }

    resetAllNodeColours();
    g_graphicsView->viewport()->update();
}



void MainWindow::resetNodeContiguityStatus()
{
    QMapIterator<int, DeBruijnNode*> i(m_deBruijnGraphNodes);
    while (i.hasNext())
    {
        i.next();
        i.value()->m_contiguityStatus = NOT_CONTIGUOUS;
    }
}

void MainWindow::resetAllNodeColours()
{
    QMapIterator<int, DeBruijnNode*> i(m_deBruijnGraphNodes);
    while (i.hasNext())
    {
        i.next();
        if (i.value()->m_graphicsItemNode != 0)
            i.value()->m_graphicsItemNode->setNodeColour();
    }
}


void MainWindow::determineContiguityFromSelectedNode()
{
    resetNodeContiguityStatus();
    resetAllNodeColours();

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


void MainWindow::setNodeColour()
{
    QColor newColour = QColorDialog::getColor(g_settings->positiveNodeColour, this, "Select colour for all nodes");

    if (newColour.isValid())
    {
        g_settings->positiveNodeColour = newColour;
        g_settings->setNegativeNodeColour();
        g_graphicsView->viewport()->update();
    }
    switchColourScheme();
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
        if (ui->coloursComboBox->currentIndex() != 3)
            ui->coloursComboBox->setCurrentIndex(3);

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
            QMapIterator<int, DeBruijnNode*> i(m_deBruijnGraphNodes);
            while (i.hasNext())
            {
                i.next();
                GraphicsItemNode * graphicsItemNode = i.value()->m_graphicsItemNode;
                if (graphicsItemNode != 0)
                    graphicsItemNode->setWidth();
            }
        }

        g_graphicsView->setAntialiasing(g_settings->antialiasing);
        g_graphicsView->viewport()->update();
    }
}

void MainWindow::selectUserSpecifiedNodes()
{
    if (ui->selectionSearchNodesLineEdit->text().length() == 0)
    {
        QMessageBox::information(this, "No nodes", "Please enter the numbers of the nodes to find, separated by commas.");
        return;
    }

    m_scene->clearSelection();
    std::vector<DeBruijnNode *> nodesToSelect = getNodeNumbersFromLineEdit(ui->selectionSearchNodesLineEdit);


    //Select each node that actually has a GraphicsItemNode, and build a bounding
    //rectangle so the viewport can focus on the selected node.
    QRectF boundingBox;
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
    }


    if (foundNodes == 0)
    {
        QMessageBox::information(this, "Nodes not found", "The nodes searched for were not found in the displayed graph.");
        return;
    }

    zoomToFitRect(boundingBox);
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
