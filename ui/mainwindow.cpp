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
#include "myprogressdialog.h"
#include <limits>
#include <QDesktopServices>
#include <QSvgGenerator>
#include "../graph/path.h"
#include "pathspecifydialog.h"
#include "../program/memory.h"

MainWindow::MainWindow(QString fileToLoadOnStartup, bool drawGraphAfterLoad) :
    QMainWindow(0),
    ui(new Ui::MainWindow), m_layoutThread(0), m_imageFilter("PNG (*.png)"),
    m_fileToLoadOnStartup(fileToLoadOnStartup), m_drawGraphAfterLoad(drawGraphAfterLoad),
    m_uiState(NO_GRAPH_LOADED), m_blastSearchDialog(0), m_alreadyShown(false)
{
    ui->setupUi(this);

    QApplication::setWindowIcon(QIcon(QPixmap(":/icons/icon.png")));
    ui->graphicsViewWidget->layout()->addWidget(g_graphicsView);

    srand(time(NULL));

    //Make a temp directory to hold the BLAST files.
    //Since Bandage is running in GUI mode, we make it in the system's
    //temp area - out of the way for the user.
    g_blastSearch->m_tempDirectory = QDir::tempPath() + "/bandage_temp-" + QString::number(QCoreApplication::applicationPid()) + "/";
    if (!QDir().mkdir(g_blastSearch->m_tempDirectory))
    {
        QMessageBox::warning(this, "Error", "A temporary directory could not be created.  BLAST search functionality will not be available");
        return;
    }
    else
        g_blastSearch->m_blastQueries.createTempQueryFiles();

    m_previousZoomSpinBoxValue = ui->zoomSpinBox->value();
    ui->zoomSpinBox->setMinimum(g_settings->minZoom * 100.0);
    ui->zoomSpinBox->setMaximum(g_settings->maxZoom * 100.0);

    //The normal height of the QPlainTextEdit objects is a bit much,
    //so fix them at a smaller height.
    ui->selectedNodesTextEdit->setFixedHeight(ui->selectedNodesTextEdit->sizeHint().height() / 2.5);
    ui->selectedEdgesTextEdit->setFixedHeight(ui->selectedEdgesTextEdit->sizeHint().height() / 2.5);

    setUiState(NO_GRAPH_LOADED);

    m_graphicsViewZoom = new GraphicsViewZoom(g_graphicsView);
    g_graphicsView->m_zoom = m_graphicsViewZoom;

    m_scene = new MyGraphicsScene(this);
    g_graphicsView->setScene(m_scene);

    setInfoTexts();

    //Nothing is selected yet, so this will hide the appropriate labels.
    selectionChanged();

    //The user may have specified settings on the command line, so it is now
    //necessary to update the UI to match these settings.
    setWidgetsFromSettings();
    setTextDisplaySettings();

    graphScopeChanged();
    switchColourScheme();

    //If this is a Mac, change the 'Delete' shortcuts to 'Backspace' instead.
#ifdef Q_OS_MAC
    ui->actionHide_selected_nodes->setShortcut(Qt::Key_Backspace);
    ui->actionRemove_selection_from_graph->setShortcut(Qt::SHIFT + Qt::Key_Backspace);
#endif

    connect(ui->drawGraphButton, SIGNAL(clicked()), this, SLOT(drawGraph()));
    connect(ui->actionLoad_graph, SIGNAL(triggered()), this, SLOT(loadGraph()));
    connect(ui->actionLoad_CSV, SIGNAL(triggered(bool)), this, SLOT(loadCSV()));
    connect(ui->actionExit, SIGNAL(triggered()), this, SLOT(close()));
    connect(ui->graphScopeComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(graphScopeChanged()));
    connect(ui->zoomSpinBox, SIGNAL(valueChanged(double)), this, SLOT(zoomSpinBoxChanged()));
    connect(m_graphicsViewZoom, SIGNAL(zoomed()), this, SLOT(zoomedWithMouseWheel()));
    connect(ui->actionCopy_selected_node_sequences_to_clipboard, SIGNAL(triggered()), this, SLOT(copySelectedSequencesToClipboardActionTriggered()));
    connect(ui->actionSave_selected_node_sequences_to_FASTA, SIGNAL(triggered()), this, SLOT(saveSelectedSequencesToFileActionTriggered()));
    connect(ui->actionCopy_selected_node_path_to_clipboard, SIGNAL(triggered(bool)), this, SLOT(copySelectedPathToClipboard()));
    connect(ui->actionSave_selected_node_path_to_FASTA, SIGNAL(triggered(bool)), this, SLOT(saveSelectedPathToFile()));
    connect(ui->coloursComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(switchColourScheme()));
    connect(ui->actionSave_image_current_view, SIGNAL(triggered()), this, SLOT(saveImageCurrentView()));
    connect(ui->actionSave_image_entire_scene, SIGNAL(triggered()), this, SLOT(saveImageEntireScene()));
    connect(ui->nodeCustomLabelsCheckBox, SIGNAL(toggled(bool)), this, SLOT(setTextDisplaySettings()));
    connect(ui->nodeNamesCheckBox, SIGNAL(toggled(bool)), this, SLOT(setTextDisplaySettings()));
    connect(ui->nodeLengthsCheckBox, SIGNAL(toggled(bool)), this, SLOT(setTextDisplaySettings()));
    connect(ui->nodeReadDepthCheckBox, SIGNAL(toggled(bool)), this, SLOT(setTextDisplaySettings()));
    connect(ui->csvCheckBox, SIGNAL(toggled(bool)), this, SLOT(setTextDisplaySettings()));
    connect(ui->csvComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(setTextDisplaySettings()));
    connect(ui->blastHitsCheckBox, SIGNAL(toggled(bool)), this, SLOT(setTextDisplaySettings()));
    connect(ui->textOutlineCheckBox, SIGNAL(toggled(bool)), this, SLOT(setTextDisplaySettings()));
    connect(ui->fontButton, SIGNAL(clicked()), this, SLOT(fontButtonPressed()));
    connect(ui->setNodeCustomColourButton, SIGNAL(clicked()), this, SLOT(setNodeCustomColour()));
    connect(ui->setNodeCustomLabelButton, SIGNAL(clicked()), this, SLOT(setNodeCustomLabel()));
    connect(ui->actionSettings, SIGNAL(triggered()), this, SLOT(openSettingsDialog()));
    connect(ui->selectNodesButton, SIGNAL(clicked()), this, SLOT(selectUserSpecifiedNodes()));
    connect(ui->selectionSearchNodesLineEdit, SIGNAL(returnPressed()), this, SLOT(selectUserSpecifiedNodes()));
    connect(ui->actionAbout, SIGNAL(triggered()), this, SLOT(openAboutDialog()));
    connect(ui->blastSearchButton, SIGNAL(clicked()), this, SLOT(openBlastSearchDialog()));
    connect(ui->blastQueryComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(blastQueryChanged()));
    connect(ui->actionControls_panel, SIGNAL(toggled(bool)), this, SLOT(showHidePanels()));
    connect(ui->actionSelection_panel, SIGNAL(toggled(bool)), this, SLOT(showHidePanels()));
    connect(ui->contiguityButton, SIGNAL(clicked()), this, SLOT(determineContiguityFromSelectedNode()));
    connect(ui->actionBring_selected_nodes_to_front, SIGNAL(triggered()), this, SLOT(bringSelectedNodesToFront()));
    connect(ui->actionSelect_nodes_with_BLAST_hits, SIGNAL(triggered()), this, SLOT(selectNodesWithBlastHits()));
    connect(ui->actionSelect_all, SIGNAL(triggered()), this, SLOT(selectAll()));
    connect(ui->actionSelect_none, SIGNAL(triggered()), this, SLOT(selectNone()));
    connect(ui->actionInvert_selection, SIGNAL(triggered()), this, SLOT(invertSelection()));
    connect(ui->actionZoom_to_selection, SIGNAL(triggered()), this, SLOT(zoomToSelection()));
    connect(ui->actionSelect_contiguous_nodes, SIGNAL(triggered()), this, SLOT(selectContiguous()));
    connect(ui->actionSelect_possibly_contiguous_nodes, SIGNAL(triggered()), this, SLOT(selectMaybeContiguous()));
    connect(ui->actionSelect_not_contiguous_nodes, SIGNAL(triggered()), this, SLOT(selectNotContiguous()));
    connect(ui->actionBandage_online_help, SIGNAL(triggered()), this, SLOT(openBandageUrl()));
    connect(ui->nodeDistanceSpinBox, SIGNAL(valueChanged(int)), this, SLOT(nodeDistanceChanged()));
    connect(ui->minReadDepthSpinBox, SIGNAL(valueChanged(double)), this, SLOT(readDepthRangeChanged()));
    connect(ui->maxReadDepthSpinBox, SIGNAL(valueChanged(double)), this, SLOT(readDepthRangeChanged()));
    connect(ui->startingNodesExactMatchRadioButton, SIGNAL(toggled(bool)), this, SLOT(startingNodesExactMatchChanged()));
    connect(ui->actionSpecify_exact_path_for_copy_save, SIGNAL(triggered()), this, SLOT(openPathSpecifyDialog()));
    connect(ui->nodeWidthSpinBox, SIGNAL(valueChanged(double)), this, SLOT(nodeWidthChanged()));
    connect(g_graphicsView, SIGNAL(copySelectedSequencesToClipboard()), this, SLOT(copySelectedSequencesToClipboard()));
    connect(g_graphicsView, SIGNAL(saveSelectedSequencesToFile()), this, SLOT(saveSelectedSequencesToFile()));
    connect(ui->actionSave_entire_graph_to_FASTA, SIGNAL(triggered(bool)), this, SLOT(saveEntireGraphToFasta()));
    connect(ui->actionSave_entire_graph_to_FASTA_only_positive_nodes, SIGNAL(triggered(bool)), this, SLOT(saveEntireGraphToFastaOnlyPositiveNodes()));
    connect(ui->actionSave_entire_graph_to_GFA, SIGNAL(triggered(bool)), this, SLOT(saveEntireGraphToGfa()));
    connect(ui->actionSave_visible_graph_to_GFA, SIGNAL(triggered(bool)), this, SLOT(saveVisibleGraphToGfa()));
    connect(ui->actionWeb_BLAST_selected_nodes, SIGNAL(triggered(bool)), this, SLOT(webBlastSelectedNodes()));
    connect(ui->actionHide_selected_nodes, SIGNAL(triggered(bool)), this, SLOT(hideNodes()));
    connect(ui->actionRemove_selection_from_graph, SIGNAL(triggered(bool)), this, SLOT(removeSelection()));
    connect(ui->actionDuplicate_selected_nodes, SIGNAL(triggered(bool)), this, SLOT(duplicateSelectedNodes()));
    connect(ui->actionMerge_selected_nodes, SIGNAL(triggered(bool)), this, SLOT(mergeSelectedNodes()));
    connect(ui->actionMerge_all_possible_nodes, SIGNAL(triggered(bool)), this, SLOT(mergeAllPossible()));

    connect(this, SIGNAL(windowLoaded()), this, SLOT(afterMainWindowShow()), Qt::ConnectionType(Qt::QueuedConnection | Qt::UniqueConnection));
}


//This function runs after the MainWindow has been shown.  This code is not
//included in the contructor because it can perform a BLAST search, which
//will fill the BLAST query combo box and screw up widget sizes.
void MainWindow::afterMainWindowShow()
{
    if (m_alreadyShown)
        return;

    //If the user passed a filename as a command line argument, try to open it now.
    if (m_fileToLoadOnStartup != "")
        loadGraph(m_fileToLoadOnStartup);

    //If a BLAST query filename is present, do the BLAST search now automatically.
    if (g_settings->blastQueryFilename != "")
    {
        BlastSearchDialog blastSearchDialog(this, g_settings->blastQueryFilename);
        setupBlastQueryComboBox();
    }

    //If the draw option was used and the graph appears to have loaded (i.e. there
    //is at least one node), then draw the graph.
    if (m_fileToLoadOnStartup != "" && m_drawGraphAfterLoad && g_assemblyGraph->m_deBruijnGraphNodes.size() > 0)
        drawGraph();

    m_alreadyShown = true;
}

MainWindow::~MainWindow()
{
    cleanUp();
    delete m_graphicsViewZoom;
    delete ui;

    if (g_blastSearch->m_tempDirectory != "" &&
            QDir(g_blastSearch->m_tempDirectory).exists() &&
            QDir(g_blastSearch->m_tempDirectory).dirName().contains("bandage_temp"))
        QDir(g_blastSearch->m_tempDirectory).removeRecursively();
}



void MainWindow::cleanUp()
{
    ui->blastQueryComboBox->clear();
    ui->blastQueryComboBox->addItem("none");

    g_blastSearch->cleanUp();
    g_assemblyGraph->cleanUp();
    setWindowTitle("Bandage");

    g_memory->userSpecifiedPath = Path();
    g_memory->userSpecifiedPathString = "";
    g_memory->userSpecifiedPathCircular = false;

    if (m_blastSearchDialog != 0)
    {
        delete m_blastSearchDialog;
        m_blastSearchDialog = 0;
    }

    ui->csvComboBox->clear();
    ui->csvComboBox->setEnabled(false);
    g_settings->displayNodeCsvDataCol = 0;
}

void MainWindow::loadCSV(QString fullFileName)
{
    QString selectedFilter = "Comma separated value (*.csv)";
    if (fullFileName == "")
    {
        fullFileName = QFileDialog::getOpenFileName(this, "Load CSV", g_memory->rememberedPath,
                                                    "Comma separated value (*.csv)",
                                                    &selectedFilter);
    }

    if (fullFileName == "")
        return; // user clicked on cancel

    QString errormsg;
    QStringList columns;

    try
    {
        MyProgressDialog progress(this, "Loading CSV...", false);
        progress.setWindowModality(Qt::WindowModal);
        progress.show();

        g_assemblyGraph->loadCSV(fullFileName, &columns, &errormsg);

        ui->csvComboBox->setEnabled(true);
        ui->csvComboBox->clear();
        ui->csvComboBox->addItems(columns);
        g_settings->displayNodeCsvDataCol = 0;
    }

    catch (...)
    {
        QString errorTitle = "Error loading CSV";
        QString errorMessage = "There was an error when attempting to load:\n"
                               + fullFileName + "\n\n"
                               "Please verify that this file has the correct format.";
        QMessageBox::warning(this, errorTitle, errorMessage);
    }
}


void MainWindow::loadGraph(QString fullFileName)
{
    QString selectedFilter = "Any supported graph (*)";
    if (fullFileName == "")
        fullFileName = QFileDialog::getOpenFileName(this, "Load graph", g_memory->rememberedPath,
                                                    "Any supported graph (*);;LastGraph (*LastGraph*);;FASTG (*.fastg);;GFA (*.gfa);;Trinity.fasta (*.fasta)",
                                                    &selectedFilter);

    if (fullFileName != "") //User did not hit cancel
    {
        GraphFileType detectedFileType = g_assemblyGraph->getGraphFileTypeFromFile(fullFileName);

        GraphFileType selectedFileType = ANY_FILE_TYPE;
        if (selectedFilter == "LastGraph (*LastGraph*)")
            selectedFileType = LAST_GRAPH;
        else if (selectedFilter == "FASTG (*.fastg)")
            selectedFileType = FASTG;
        else if (selectedFilter == "GFA (*.gfa)")
            selectedFileType = GFA;
        else if (selectedFilter == "Trinity.fasta (*.fasta)")
            selectedFileType = TRINITY;

        if (selectedFileType == ANY_FILE_TYPE)
        {
            //If the user chose any file type but it can't be determined, show an error and quit.
            if (detectedFileType == UNKNOWN_FILE_TYPE)
            {
                QMessageBox::warning(this, "Graph format not recognised", "Cannot load file. The selected file's format was not recognised as any supported graph type.");
                return;
            }

            //If the user chose any file type and it can be determined, then use that type.
            else
                selectedFileType = detectedFileType;
        }

        //If there is a discrepancy between the selected and detected file types, make sure the
        //user wants to continue.
        if (selectedFileType != detectedFileType)
        {
            QString graphFileTypeString = convertGraphFileTypeToString(selectedFileType);
            QMessageBox::StandardButton reply;
            reply = QMessageBox::question(this, graphFileTypeString + " file?",
                                          "This file does not appear to be a " + graphFileTypeString + " file."
                                          "\nDo you want to load it as a " + graphFileTypeString + " file anyway?",
                                          QMessageBox::Yes|QMessageBox::No);
            if (reply == QMessageBox::No)
                return;
        }

        loadGraph2(selectedFileType, fullFileName);
    }
}


void MainWindow::loadGraph2(GraphFileType graphFileType, QString fullFileName)
{
    resetScene();
    cleanUp();
    ui->selectionSearchNodesLineEdit->clear();

    try
    {
        MyProgressDialog progress(this, "Loading " + convertGraphFileTypeToString(graphFileType) + " file...", false);
        progress.setWindowModality(Qt::WindowModal);
        progress.show();

        if (graphFileType == LAST_GRAPH)
            g_assemblyGraph->buildDeBruijnGraphFromLastGraph(fullFileName);
        else if (graphFileType == FASTG)
            g_assemblyGraph->buildDeBruijnGraphFromFastg(fullFileName);
        else if (graphFileType == GFA)
            g_assemblyGraph->buildDeBruijnGraphFromGfa(fullFileName);
        else if (graphFileType == TRINITY)
            g_assemblyGraph->buildDeBruijnGraphFromTrinityFasta(fullFileName);

        setUiState(GRAPH_LOADED);
        setWindowTitle("Bandage - " + fullFileName);

        g_assemblyGraph->determineGraphInfo();
        displayGraphDetails();
        g_memory->rememberedPath = QFileInfo(fullFileName).absolutePath();
        g_memory->clearGraphSpecificMemory();
    }

    catch (...)
    {
        QString errorTitle = "Error loading " + convertGraphFileTypeToString(graphFileType);
        QString errorMessage = "There was an error when attempting to load:\n"
                               + fullFileName + "\n\n"
                               "Please verify that this file has the correct format.";
        QMessageBox::warning(this, errorTitle, errorMessage);
        resetScene();
        cleanUp();
        clearGraphDetails();
        setUiState(NO_GRAPH_LOADED);
    }
}



void MainWindow::displayGraphDetails()
{
    ui->nodeCountLabel->setText(formatIntForDisplay(g_assemblyGraph->m_nodeCount));
    ui->edgeCountLabel->setText(formatIntForDisplay(g_assemblyGraph->m_edgeCount));
    ui->totalLengthLabel->setText(formatIntForDisplay(g_assemblyGraph->m_totalLength));
}
void MainWindow::clearGraphDetails()
{
    ui->nodeCountLabel->setText("0");
    ui->edgeCountLabel->setText("0");
    ui->totalLengthLabel->setText("0");
}


void MainWindow::selectionChanged()
{
    std::vector<DeBruijnNode *> selectedNodes = m_scene->getSelectedNodes();
    std::vector<DeBruijnEdge *> selectedEdges = m_scene->getSelectedEdges();

    if (selectedNodes.size() == 0)
    {
        ui->selectedNodesTextEdit->setPlainText("");
        setSelectedNodesWidgetsVisibility(false);
    }

    else //One or more nodes selected
    {
        setSelectedNodesWidgetsVisibility(true);

        int selectedNodeCount;
        QString selectedNodeCountText;
        QString selectedNodeListText;
        QString selectedNodeLengthText;
        QString selectedNodeDepthText;

        getSelectedNodeInfo(selectedNodeCount, selectedNodeCountText, selectedNodeListText, selectedNodeLengthText, selectedNodeDepthText);

        if (selectedNodeCount == 1)
        {
            ui->selectedNodesTitleLabel->setText("Selected node");
            ui->selectedNodesLengthLabel->setText("Length: " + selectedNodeLengthText);
            ui->selectedNodesDepthLabel->setText("Read depth: " + selectedNodeDepthText);
        }
        else
        {
            ui->selectedNodesTitleLabel->setText("Selected nodes (" + selectedNodeCountText + ")");
            ui->selectedNodesLengthLabel->setText("Total length: " + selectedNodeLengthText);
            ui->selectedNodesDepthLabel->setText("Mean read depth: " + selectedNodeDepthText);
        }

        ui->selectedNodesTextEdit->setPlainText(selectedNodeListText);
    }


    if (selectedEdges.size() == 0)
    {
        ui->selectedEdgesTextEdit->setPlainText("");
        setSelectedEdgesWidgetsVisibility(false);
    }

    else //One or more edges selected
    {
        setSelectedEdgesWidgetsVisibility(true);
        if (selectedEdges.size() == 1)
            ui->selectedEdgesTitleLabel->setText("Selected edge");
        else
            ui->selectedEdgesTitleLabel->setText("Selected edges (" + formatIntForDisplay(int(selectedEdges.size())) + ")");

        ui->selectedEdgesTextEdit->setPlainText(getSelectedEdgeListText());
    }

}


void MainWindow::getSelectedNodeInfo(int & selectedNodeCount, QString & selectedNodeCountText, QString & selectedNodeListText, QString & selectedNodeLengthText, QString & selectedNodeDepthText)
{
    std::vector<DeBruijnNode *> selectedNodes = m_scene->getSelectedNodes();

    selectedNodeCount = int(selectedNodes.size());
    selectedNodeCountText = formatIntForDisplay(selectedNodeCount);

    long long totalLength = 0;

    for (int i = 0; i < selectedNodeCount; ++i)
    {
        QString nodeName = selectedNodes[i]->getName();

        //If we are in single mode, don't include +/i in the node name
        if (!g_settings->doubleMode)
            nodeName.chop(1);

        selectedNodeListText += nodeName;
        if (i != int(selectedNodes.size()) - 1)
            selectedNodeListText += ", ";

        totalLength += selectedNodes[i]->getLength();
    }

    selectedNodeLengthText = formatIntForDisplay(totalLength);
    selectedNodeDepthText = formatDoubleForDisplay(g_assemblyGraph->getMeanReadDepth(selectedNodes), 1);
}



bool compareEdgePointers(DeBruijnEdge * a, DeBruijnEdge * b)
{
    QString aStart = a->getStartingNode()->getName();
    QString bStart = b->getStartingNode()->getName();
    QString aStartNoSign = aStart;
    aStartNoSign.chop(1);
    QString bStartNoSign = bStart;
    bStartNoSign.chop(1);
    bool ok1;
    long long aStartNumber = aStartNoSign.toLongLong(&ok1);
    bool ok2;
    long long bStartNumber = bStartNoSign.toLongLong(&ok2);

    QString aEnd = a->getEndingNode()->getName();
    QString bEnd = b->getEndingNode()->getName();
    QString aEndNoSign = aEnd;
    aEndNoSign.chop(1);
    QString bEndNoSign = bEnd;
    bEndNoSign.chop(1);
    bool ok3;
    long long aEndNumber = aEndNoSign.toLongLong(&ok3);
    bool ok4;
    long long bEndNumber = bEndNoSign.toLongLong(&ok4);


    //If the node names are essentially numbers, then sort them as numbers.
    if (ok1 && ok2 && ok3 && ok4)
    {
        if (aStartNumber != bStartNumber)
            return aStartNumber < bStartNumber;

        if (aStartNumber == bStartNumber)
            return aEndNumber < bEndNumber;
    }

    //If the node names are strings, then just sort them as strings.
    return aStart < bStart;
}

QString MainWindow::getSelectedEdgeListText()
{
    std::vector<DeBruijnEdge *> selectedEdges = m_scene->getSelectedEdges();

    std::sort(selectedEdges.begin(), selectedEdges.end(), compareEdgePointers);

    QString edgeText;
    for (size_t i = 0; i < selectedEdges.size(); ++i)
    {
        edgeText += selectedEdges[i]->getStartingNode()->getName();
        edgeText += " to ";
        edgeText += selectedEdges[i]->getEndingNode()->getName();
        if (i != selectedEdges.size() - 1)
            edgeText += ", ";
    }

    return edgeText;
}



//This function shows/hides UI elements depending on which
//graph scope is currently selected.  It also reorganises
//the widgets in the layout to prevent gaps when widgets
//are hidden.
void MainWindow::graphScopeChanged()
{
    switch (ui->graphScopeComboBox->currentIndex())
    {
    case 0:
        g_settings->graphScope = WHOLE_GRAPH;

        setStartingNodesWidgetVisibility(false);
        setNodeDistanceWidgetVisibility(false);
        setReadDepthRangeWidgetVisibility(false);

        ui->graphDrawingGridLayout->addWidget(ui->nodeStyleInfoText, 1, 0, 1, 1);
        ui->graphDrawingGridLayout->addWidget(ui->nodeStyleLabel, 1, 1, 1, 1);
        ui->graphDrawingGridLayout->addWidget(ui->nodeStyleWidget, 1, 2, 1, 1);
        ui->graphDrawingGridLayout->addWidget(ui->drawGraphInfoText, 2, 0, 1, 1);
        ui->graphDrawingGridLayout->addWidget(ui->drawGraphButton, 2, 1, 1, 2);

        break;

    case 1:
        g_settings->graphScope = AROUND_NODE;

        setStartingNodesWidgetVisibility(true);
        setNodeDistanceWidgetVisibility(true);
        setReadDepthRangeWidgetVisibility(false);

        ui->nodeDistanceInfoText->setInfoText("Nodes will be drawn if they are specified in the above list or are "
                                              "within this many steps of those nodes.<br><br>"
                                              "A value of 0 will result in only the specified nodes being drawn. "
                                              "A large value will result in large sections of the graph around "
                                              "the specified nodes being drawn.");

        ui->graphDrawingGridLayout->addWidget(ui->startingNodesInfoText, 1, 0, 1, 1);
        ui->graphDrawingGridLayout->addWidget(ui->startingNodesLabel, 1, 1, 1, 1);
        ui->graphDrawingGridLayout->addWidget(ui->startingNodesLineEdit, 1, 2, 1, 1);
        ui->graphDrawingGridLayout->addWidget(ui->startingNodesMatchTypeInfoText, 2, 0, 1, 1);
        ui->graphDrawingGridLayout->addWidget(ui->startingNodesMatchTypeLabel, 2, 1, 1, 1);
        ui->graphDrawingGridLayout->addWidget(ui->startingNodesMatchTypeWidget, 2, 2, 1, 1);
        ui->graphDrawingGridLayout->addWidget(ui->nodeDistanceInfoText, 3, 0, 1, 1);
        ui->graphDrawingGridLayout->addWidget(ui->nodeDistanceLabel, 3, 1, 1, 1);
        ui->graphDrawingGridLayout->addWidget(ui->nodeDistanceSpinBox, 3, 2, 1, 1);
        ui->graphDrawingGridLayout->addWidget(ui->nodeStyleInfoText, 4, 0, 1, 1);
        ui->graphDrawingGridLayout->addWidget(ui->nodeStyleLabel, 4, 1, 1, 1);
        ui->graphDrawingGridLayout->addWidget(ui->nodeStyleWidget, 4, 2, 1, 1);
        ui->graphDrawingGridLayout->addWidget(ui->drawGraphInfoText, 5, 0, 1, 1);
        ui->graphDrawingGridLayout->addWidget(ui->drawGraphButton, 5, 1, 1, 2);

        break;

    case 2:
        g_settings->graphScope = AROUND_BLAST_HITS;

        setStartingNodesWidgetVisibility(false);
        setNodeDistanceWidgetVisibility(true);
        setReadDepthRangeWidgetVisibility(false);

        ui->nodeDistanceInfoText->setInfoText("Nodes will be drawn if they contain a BLAST hit or are within this "
                                              "many steps of nodes with a BLAST hit.<br><br>"
                                              "A value of 0 will result in only nodes with BLAST hits being drawn. "
                                              "A large value will result in large sections of the graph around "
                                              "nodes with BLAST hits being drawn.");

        ui->graphDrawingGridLayout->addWidget(ui->nodeDistanceInfoText, 1, 0, 1, 1);
        ui->graphDrawingGridLayout->addWidget(ui->nodeDistanceLabel, 1, 1, 1, 1);
        ui->graphDrawingGridLayout->addWidget(ui->nodeDistanceSpinBox, 1, 2, 1, 1);
        ui->graphDrawingGridLayout->addWidget(ui->nodeStyleInfoText, 2, 0, 1, 1);
        ui->graphDrawingGridLayout->addWidget(ui->nodeStyleLabel, 2, 1, 1, 1);
        ui->graphDrawingGridLayout->addWidget(ui->nodeStyleWidget, 2, 2, 1, 1);
        ui->graphDrawingGridLayout->addWidget(ui->drawGraphInfoText, 3, 0, 1, 1);
        ui->graphDrawingGridLayout->addWidget(ui->drawGraphButton, 3, 1, 1, 2);

        break;

    case 3:
        g_settings->graphScope = READ_DEPTH_RANGE;

        setStartingNodesWidgetVisibility(false);
        setNodeDistanceWidgetVisibility(false);
        setReadDepthRangeWidgetVisibility(true);

        ui->graphDrawingGridLayout->addWidget(ui->minReadDepthInfoText, 1, 0, 1, 1);
        ui->graphDrawingGridLayout->addWidget(ui->minReadDepthLabel, 1, 1, 1, 1);
        ui->graphDrawingGridLayout->addWidget(ui->minReadDepthSpinBox, 1, 2, 1, 1);
        ui->graphDrawingGridLayout->addWidget(ui->maxReadDepthInfoText, 2, 0, 1, 1);
        ui->graphDrawingGridLayout->addWidget(ui->maxReadDepthLabel, 2, 1, 1, 1);
        ui->graphDrawingGridLayout->addWidget(ui->maxReadDepthSpinBox, 2, 2, 1, 1);
        ui->graphDrawingGridLayout->addWidget(ui->nodeStyleInfoText, 3, 0, 1, 1);
        ui->graphDrawingGridLayout->addWidget(ui->nodeStyleLabel, 3, 1, 1, 1);
        ui->graphDrawingGridLayout->addWidget(ui->nodeStyleWidget, 3, 2, 1, 1);
        ui->graphDrawingGridLayout->addWidget(ui->drawGraphInfoText, 4, 0, 1, 1);
        ui->graphDrawingGridLayout->addWidget(ui->drawGraphButton, 4, 1, 1, 2);

        break;
    }
}


void MainWindow::setStartingNodesWidgetVisibility(bool visible)
{
    ui->startingNodesInfoText->setVisible(visible);
    ui->startingNodesLabel->setVisible(visible);
    ui->startingNodesLineEdit->setVisible(visible);
    ui->startingNodesMatchTypeInfoText->setVisible(visible);
    ui->startingNodesMatchTypeLabel->setVisible(visible);
    ui->startingNodesExactMatchRadioButton->setVisible(visible);
    ui->startingNodesPartialMatchRadioButton->setVisible(visible);
}
void MainWindow::setNodeDistanceWidgetVisibility(bool visible)
{
    ui->nodeDistanceInfoText->setVisible(visible);
    ui->nodeDistanceLabel->setVisible(visible);
    ui->nodeDistanceSpinBox->setVisible(visible);
}
void MainWindow::setReadDepthRangeWidgetVisibility(bool visible)
{
    ui->minReadDepthInfoText->setVisible(visible);
    ui->minReadDepthLabel->setVisible(visible);
    ui->minReadDepthSpinBox->setVisible(visible);
    ui->maxReadDepthInfoText->setVisible(visible);
    ui->maxReadDepthLabel->setVisible(visible);
    ui->maxReadDepthSpinBox->setVisible(visible);
}


void MainWindow::drawGraph()
{
    QString errorTitle;
    QString errorMessage;
    std::vector<DeBruijnNode *> startingNodes = g_assemblyGraph->getStartingNodes(&errorTitle, &errorMessage,
                                                                                  ui->doubleNodesRadioButton->isChecked(),
                                                                                  ui->startingNodesLineEdit->text(),
                                                                                  ui->blastQueryComboBox->currentText());

    if (errorMessage != "")
    {
        QMessageBox::information(this, errorTitle, errorMessage);
        return;
    }

    resetScene();
    g_assemblyGraph->buildOgdfGraphFromNodesAndEdges(startingNodes, g_settings->nodeDistance);
    layoutGraph();
}


void MainWindow::graphLayoutFinished()
{
    delete m_fmmm;
    m_layoutThread = 0;
    g_assemblyGraph->addGraphicsItemsToScene(m_scene);
    m_scene->setSceneRectangle();
    zoomToFitScene();
    selectionChanged();

    setUiState(GRAPH_DRAWN);

    //Move the focus to the view so the user can use keyboard controls to navigate.
    g_graphicsView->setFocus();
}


void MainWindow::graphLayoutCancelled()
{
    m_fmmm->fixedIterations(0);
    m_fmmm->fineTuningIterations(0);
    m_fmmm->threshold(std::numeric_limits<double>::max());
}


void MainWindow::resetScene()
{
    m_scene->blockSignals(true);

    g_assemblyGraph->resetEdges();
    g_assemblyGraph->m_contiguitySearchDone = false;

    g_graphicsView->setScene(0);
    delete m_scene;
    m_scene = new MyGraphicsScene(this);

    g_graphicsView->setScene(m_scene);
    connect(m_scene, SIGNAL(selectionChanged()), this, SLOT(selectionChanged()));
    selectionChanged();

    g_graphicsView->undoRotation();
}


std::vector<DeBruijnNode *> MainWindow::getNodesFromLineEdit(QLineEdit * lineEdit, bool exactMatch, std::vector<QString> * nodesNotInGraph)
{
    return g_assemblyGraph->getNodesFromString(lineEdit->text(), exactMatch, nodesNotInGraph);
}




void MainWindow::layoutGraph()
{
    //The actual layout is done in a different thread so the UI will stay responsive.
    MyProgressDialog * progress = new MyProgressDialog(this, "Laying out graph...", true, "Cancel layout", "Cancelling layout...",
                                                       "Clicking this button will halt the graph layout and display "
                                                       "the graph in its current, incomplete state.<br><br>"
                                                       "Layout can take a long time for very large graphs.  There are "
                                                       "three strategies to reduce the amount of time required:<ul>"
                                                       "<li>Change the scope of the graph from 'Entire graph' to either "
                                                       "'Around nodes' or 'Around BLAST hits'.  This will reduce the "
                                                       "number of nodes that are drawn to the screen.</li>"
                                                       "<li>Increase the 'Base pairs per segment' setting.  This will "
                                                       "result in shorter contigs which take less time to lay out.</li>"
                                                       "<li>Reduce the 'Graph layout iterations' setting.</li></ul>");
    progress->setWindowModality(Qt::WindowModal);
    progress->show();

    m_fmmm = new ogdf::FMMMLayout();

    m_layoutThread = new QThread;
    GraphLayoutWorker * graphLayoutWorker = new GraphLayoutWorker(m_fmmm, g_assemblyGraph->m_graphAttributes,
                                                                  g_settings->graphLayoutQuality, g_settings->segmentLength);
    graphLayoutWorker->moveToThread(m_layoutThread);

    connect(progress, SIGNAL(halt()), this, SLOT(graphLayoutCancelled()));
    connect(m_layoutThread, SIGNAL(started()), graphLayoutWorker, SLOT(layoutGraph()));
    connect(graphLayoutWorker, SIGNAL(finishedLayout()), m_layoutThread, SLOT(quit()));
    connect(graphLayoutWorker, SIGNAL(finishedLayout()), graphLayoutWorker, SLOT(deleteLater()));
    connect(graphLayoutWorker, SIGNAL(finishedLayout()), this, SLOT(graphLayoutFinished()));
    connect(m_layoutThread, SIGNAL(finished()), m_layoutThread, SLOT(deleteLater()));
    connect(m_layoutThread, SIGNAL(finished()), progress, SLOT(deleteLater()));
    m_layoutThread->start();
}




void MainWindow::zoomSpinBoxChanged()
{
    double newValue = ui->zoomSpinBox->value();
    double zoomFactor = newValue / m_previousZoomSpinBoxValue;
    setZoomSpinBoxStep();

    m_graphicsViewZoom->gentleZoom(zoomFactor, SPIN_BOX);

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
        m_graphicsViewZoom->gentleZoom(newZoomFactor, SPIN_BOX);
        g_absoluteZoom *= newZoomFactor;
        g_absoluteZoom = g_settings->minZoom;
        newSpinBoxValue = g_settings->minZoom * 100.0;
    }
    if (g_absoluteZoom > g_settings->maxZoom)
    {
        double newZoomFactor = g_settings->maxZoom / g_absoluteZoom;
        m_graphicsViewZoom->gentleZoom(newZoomFactor, SPIN_BOX);
        g_absoluteZoom *= newZoomFactor;
        g_absoluteZoom = g_settings->maxZoom;
        newSpinBoxValue = g_settings->maxZoom * 100.0;
    }

    ui->zoomSpinBox->blockSignals(true);
    ui->zoomSpinBox->setValue(newSpinBoxValue);
    m_previousZoomSpinBoxValue = newSpinBoxValue;
    ui->zoomSpinBox->blockSignals(false);
}



//This function copies selected sequences to clipboard, if any sequences are
//selected.  If there aren't, then it will prompt the user.
void MainWindow::copySelectedSequencesToClipboardActionTriggered()
{
    std::vector<DeBruijnNode *> selectedNodes = m_scene->getSelectedNodes();
    if (selectedNodes.size() == 0)
        QMessageBox::information(this, "Copy sequences to clipboard", "No nodes are selected.\n\n"
                                                                      "You must first select nodes in the graph before you can copy their sequences to the clipboard.");
    else
        copySelectedSequencesToClipboard();
}


//This function copies selected sequences to clipboard, if any sequences are
//selected.
void MainWindow::copySelectedSequencesToClipboard()
{
    std::vector<DeBruijnNode *> selectedNodes = m_scene->getSelectedNodes();
    if (selectedNodes.size() == 0)
        return;

    QClipboard * clipboard = QApplication::clipboard();
    QString clipboardText;

    for (size_t i = 0; i < selectedNodes.size(); ++i)
    {
        clipboardText += selectedNodes[i]->getSequence();
        if (i != selectedNodes.size() - 1)
            clipboardText += "\n";
    }

    clipboard->setText(clipboardText);
}


//This function saves selected sequences to file, with a save file prompt, if
//any sequences are selected.  If there aren't, then it will prompt the user.
void MainWindow::saveSelectedSequencesToFileActionTriggered()
{
    std::vector<DeBruijnNode *> selectedNodes = m_scene->getSelectedNodes();
    if (selectedNodes.size() == 0)
        QMessageBox::information(this, "Save sequences to FASTA", "No nodes are selected.\n\n"
                                                                  "You must first select nodes in the graph before you can save their sequences to a FASTA file.");
    else
        saveSelectedSequencesToFile();
}


//This function saves selected sequences to file, with a save file prompt, if
//any sequences are selected.
void MainWindow::saveSelectedSequencesToFile()
{
    std::vector<DeBruijnNode *> selectedNodes = m_scene->getSelectedNodes();
    if (selectedNodes.size() == 0)
        return;

    QString defaultFileNameAndPath = g_memory->rememberedPath + "/selected_sequences.fasta";

    QString fullFileName = QFileDialog::getSaveFileName(this, "Save node sequences", defaultFileNameAndPath, "FASTA (*.fasta)");

    if (fullFileName != "") //User did not hit cancel
    {
        QFile file(fullFileName);
        file.open(QIODevice::WriteOnly | QIODevice::Text);
        QTextStream out(&file);

        for (size_t i = 0; i < selectedNodes.size(); ++i)
            out << selectedNodes[i]->getFasta();

        g_memory->rememberedPath = QFileInfo(fullFileName).absolutePath();
    }
}

void MainWindow::copySelectedPathToClipboard()
{
    std::vector<DeBruijnNode *> selectedNodes = m_scene->getSelectedNodes();
    if (selectedNodes.size() == 0)
    {
        QMessageBox::information(this, "Copy path sequence to clipboard", "No nodes are selected.\n\n"
                                                                          "You must first select nodes in the graph which define a unambiguous "
                                                                          "path before you can copy their path sequence to the clipboard.");
        return;
    }

    Path nodePath = Path::makeFromUnorderedNodes(selectedNodes, g_settings->doubleMode);
    if (nodePath.isEmpty())
    {
        QMessageBox::information(this, "Copy path sequence to clipboard", "Invalid path.\n\n"
                                                                          "To use copy a path sequence to the clipboard, the nodes must follow "
                                                                          "an unambiguous path through the graph.\n\n"
                                                                          "Complex paths can be defined using the '" + ui->actionSpecify_exact_path_for_copy_save->text() +
                                                                          "' tool.");
        return;
    }

    QClipboard * clipboard = QApplication::clipboard();
    clipboard->setText(nodePath.getPathSequence());
}



void MainWindow::saveSelectedPathToFile()
{
    std::vector<DeBruijnNode *> selectedNodes = m_scene->getSelectedNodes();
    if (selectedNodes.size() == 0)
    {
        QMessageBox::information(this, "Save path sequence to FASTA", "No nodes are selected.\n\n"
                                                                      "You must first select nodes in the graph which define a unambiguous "
                                                                      "path before you can save their path sequence to a FASTA file.");
        return;
    }
    Path nodePath = Path::makeFromUnorderedNodes(selectedNodes, g_settings->doubleMode);
    if (nodePath.isEmpty())
    {
        QMessageBox::information(this, "Save path sequence to FASTA", "Invalid path.\n\n"
                                                                      "To use copy a path sequence to the clipboard, the nodes must follow "
                                                                      "an unambiguous path through the graph.\n\n"
                                                                      "Complex paths can be defined using the '" + ui->actionSpecify_exact_path_for_copy_save->text() +
                                                                      "' tool.");
        return;
    }

    QString defaultFileNameAndPath = g_memory->rememberedPath + "/path_sequence.fasta";

    QString fullFileName = QFileDialog::getSaveFileName(this, "Save path sequence", defaultFileNameAndPath, "FASTA (*.fasta)");

    if (fullFileName != "") //User did not hit cancel
    {
        QFile file(fullFileName);
        file.open(QIODevice::WriteOnly | QIODevice::Text);
        QTextStream out(&file);
        out << nodePath.getFasta();
        g_memory->rememberedPath = QFileInfo(fullFileName).absolutePath();
    }
}




void MainWindow::switchColourScheme()
{
    switch (ui->coloursComboBox->currentIndex())
    {
    case 0:
        g_settings->nodeColourScheme = RANDOM_COLOURS;
        ui->contiguityButton->setVisible(false);
        ui->contiguityInfoText->setVisible(false);
        break;
    case 1:
        g_settings->nodeColourScheme = UNIFORM_COLOURS;
        ui->contiguityButton->setVisible(false);
        ui->contiguityInfoText->setVisible(false);
        break;
    case 2:
        g_settings->nodeColourScheme = READ_DEPTH_COLOUR;
        ui->contiguityButton->setVisible(false);
        ui->contiguityInfoText->setVisible(false);
        break;
    case 3:
        g_settings->nodeColourScheme = BLAST_HITS_SOLID_COLOUR;
        ui->contiguityButton->setVisible(false);
        ui->contiguityInfoText->setVisible(false);
        break;
    case 4:
        g_settings->nodeColourScheme = BLAST_HITS_RAINBOW_COLOUR;
        ui->contiguityButton->setVisible(false);
        ui->contiguityInfoText->setVisible(false);
        break;
    case 5:
        g_settings->nodeColourScheme = CONTIGUITY_COLOUR;
        ui->contiguityButton->setVisible(true);
        ui->contiguityInfoText->setVisible(true);
        break;
    case 6:
        g_settings->nodeColourScheme = CUSTOM_COLOURS;
        ui->contiguityButton->setVisible(false);
        ui->contiguityInfoText->setVisible(false);
        break;
    }

    g_assemblyGraph->resetAllNodeColours();
    g_graphicsView->viewport()->update();
}



void MainWindow::determineContiguityFromSelectedNode()
{
    g_assemblyGraph->resetNodeContiguityStatus();
    g_assemblyGraph->resetAllNodeColours();

    std::vector<DeBruijnNode *> selectedNodes = m_scene->getSelectedNodes();
    if (selectedNodes.size() > 0)
    {
        MyProgressDialog progress(this, "Determining contiguity...", false);
        progress.setWindowModality(Qt::WindowModal);
        progress.show();

        for (size_t i = 0; i < selectedNodes.size(); ++i)
            (selectedNodes[i])->determineContiguity();

        g_assemblyGraph->m_contiguitySearchDone = true;
        g_assemblyGraph->resetAllNodeColours();
        g_graphicsView->viewport()->update();
    }
    else
        QMessageBox::information(this, "No nodes selected", "Please select one or more nodes for which "
                                                            "contiguity is to be determined.");
}


QString MainWindow::getDefaultImageFileName()
{
    QString fileNameAndPath = g_memory->rememberedPath + "/graph";

    if (m_imageFilter == "PNG (*.png)")
        fileNameAndPath += ".png";
    else if (m_imageFilter == "JPEG (*.jpg)")
        fileNameAndPath += ".jpg";
    else if (m_imageFilter == "SVG (*.svg)")
        fileNameAndPath += ".svg";
    else
        fileNameAndPath += ".png";

    return fileNameAndPath;
}


void MainWindow::saveImageCurrentView()
{
    if (!checkForImageSave())
        return;

    QString defaultFileNameAndPath = getDefaultImageFileName();

    QString selectedFilter = m_imageFilter;
    QString fullFileName = QFileDialog::getSaveFileName(this, "Save graph image (current view)",
                                                        defaultFileNameAndPath,
                                                        "PNG (*.png);;JPEG (*.jpg);;SVG (*.svg)",
                                                        &selectedFilter);

    bool pixelImage = true;
    if (selectedFilter == "PNG (*.png)" || selectedFilter == "JPEG (*.jpg)")
        pixelImage = true;
    else if (selectedFilter == "SVG (*.svg)")
        pixelImage = false;

    if (fullFileName != "") //User did not hit cancel
    {
        m_imageFilter = selectedFilter;

        QPainter painter;
        if (pixelImage)
        {
            QImage image(g_graphicsView->viewport()->rect().size(), QImage::Format_ARGB32);
            image.fill(Qt::white);
            painter.begin(&image);
            painter.setRenderHint(QPainter::Antialiasing);
            painter.setRenderHint(QPainter::TextAntialiasing);
            g_graphicsView->render(&painter);
            image.save(fullFileName);
            g_memory->rememberedPath = QFileInfo(fullFileName).absolutePath();
            painter.end();
        }
        else //SVG
        {
            QSvgGenerator generator;
            generator.setFileName(fullFileName);
            QSize size = g_graphicsView->viewport()->rect().size();
            generator.setSize(size);
            generator.setViewBox(QRect(0, 0, size.width(), size.height()));
            painter.begin(&generator);
            painter.fillRect(0, 0, size.width(), size.height(), Qt::white);
            painter.setRenderHint(QPainter::Antialiasing);
            painter.setRenderHint(QPainter::TextAntialiasing);
            g_graphicsView->render(&painter);
            painter.end();
        }
    }
}

void MainWindow::saveImageEntireScene()
{
    if (!checkForImageSave())
        return;

    QString defaultFileNameAndPath = getDefaultImageFileName();

    QString selectedFilter = m_imageFilter;
    QString fullFileName = QFileDialog::getSaveFileName(this,
                                                        "Save graph image (entire scene)",
                                                        defaultFileNameAndPath,
                                                        "PNG (*.png);;JPEG (*.jpg);;SVG (*.svg)",
                                                        &selectedFilter);

    bool pixelImage = true;
    if (selectedFilter == "PNG (*.png)" || selectedFilter == "JPEG (*.jpg)")
        pixelImage = true;
    else if (selectedFilter == "SVG (*.svg)")
        pixelImage = false;

    if (fullFileName != "") //User did not hit cancel
    {
        //The positionTextNodeCentre setting must be used for the entire scene
        //or else only the labels in the current viewport will be shown.
        bool positionTextNodeCentreSettingBefore = g_settings->positionTextNodeCentre;
        g_settings->positionTextNodeCentre = true;

        //Temporarily undo any rotation so labels appear upright.
        double rotationBefore = g_graphicsView->getRotation();
        g_graphicsView->undoRotation();

        m_imageFilter = selectedFilter;

        QPainter painter;
        if (pixelImage)
        {
            QSize imageSize = g_absoluteZoom * m_scene->sceneRect().size().toSize();

            if (imageSize.width() > 32767 || imageSize.height() > 32767)
            {
                QString error = "Images can not be taller or wider than 32767 pixels, but at the "
                                "current zoom level, the image to be saved would be ";
                error += QString::number(imageSize.width()) + "x" + QString::number(imageSize.height()) + " pixels.\n\n";
                error += "Please reduce the zoom level before saving the entire scene to image or use the SVG format.";

                QMessageBox::information(this, "Image too large", error);
                return;
            }

            if (imageSize.width() * imageSize.height() > 50000000) //50 megapixels is used as an arbitrary large image cutoff
            {
                QString warning = "At the current zoom level, the image will be ";
                warning += QString::number(imageSize.width()) + "x" + QString::number(imageSize.height()) + " pixels. ";
                warning += "An image of this large size may take significant time and space to save.\n\n"
                           "The image size can be reduced by decreasing the zoom level or using the SVG format.\n\n"
                           "Do you want to continue saving the image?";
                QMessageBox::StandardButton response = QMessageBox::question(this, "Large image", warning);
                if (response == QMessageBox::No || response == QMessageBox::Cancel)
                    return;
            }

            QImage image(imageSize, QImage::Format_ARGB32);
            image.fill(Qt::white);
            painter.begin(&image);
            painter.setRenderHint(QPainter::Antialiasing);
            painter.setRenderHint(QPainter::TextAntialiasing);
            m_scene->setSceneRectangle();
            m_scene->render(&painter);
            image.save(fullFileName);
            g_memory->rememberedPath = QFileInfo(fullFileName).absolutePath();
            painter.end();
        }
        else //SVG
        {
            QSvgGenerator generator;
            generator.setFileName(fullFileName);
            QSize size = g_absoluteZoom * m_scene->sceneRect().size().toSize();
            generator.setSize(size);
            generator.setViewBox(QRect(0, 0, size.width(), size.height()));
            painter.begin(&generator);
            painter.fillRect(0, 0, size.width(), size.height(), Qt::white);
            painter.setRenderHint(QPainter::Antialiasing);
            painter.setRenderHint(QPainter::TextAntialiasing);
            m_scene->setSceneRectangle();
            m_scene->render(&painter);
            painter.end();
        }

        g_settings->positionTextNodeCentre = positionTextNodeCentreSettingBefore;
        g_graphicsView->setRotation(rotationBefore);
    }
}



//This function makes sure that a graph is loaded and drawn so that an image can be saved.
//It returns true if everything is fine.  If things aren't ready, it displays a message
//to the user and returns false.
bool MainWindow::checkForImageSave()
{
    if (m_uiState == NO_GRAPH_LOADED)
    {
        QMessageBox::information(this, "No image to save", "You must first load and then draw a graph before you can save an image to file.");
        return false;
    }
    if (m_uiState == GRAPH_LOADED)
    {
        QMessageBox::information(this, "No image to save", "You must first draw the graph before you can save an image to file.");
        return false;
    }
    return true;
}


void MainWindow::setTextDisplaySettings()
{
    g_settings->displayNodeCustomLabels = ui->nodeCustomLabelsCheckBox->isChecked();
    g_settings->displayNodeNames = ui->nodeNamesCheckBox->isChecked();
    g_settings->displayNodeLengths = ui->nodeLengthsCheckBox->isChecked();
    g_settings->displayNodeReadDepth = ui->nodeReadDepthCheckBox->isChecked();
    g_settings->displayBlastHits = ui->blastHitsCheckBox->isChecked();
    g_settings->displayNodeCsvData = ui->csvCheckBox->isChecked();
    g_settings->displayNodeCsvDataCol = ui->csvComboBox->currentIndex();
    g_settings->textOutline = ui->textOutlineCheckBox->isChecked();

    g_graphicsView->viewport()->update();
}


void MainWindow::fontButtonPressed()
{
    bool ok;
    g_settings->labelFont = QFontDialog::getFont(&ok, g_settings->labelFont, this);
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

    QColor newColour = QColorDialog::getColor(selectedNodes[0]->getCustomColour(), this, dialogTitle);
    if (newColour.isValid())
    {
        //If the colouring scheme is not currently custom, change it to custom now
        if (g_settings->nodeColourScheme != CUSTOM_COLOURS)
            setNodeColourSchemeComboBox(CUSTOM_COLOURS);

        for (size_t i = 0; i < selectedNodes.size(); ++i)
        {
            selectedNodes[i]->setCustomColour(newColour);
            if (selectedNodes[i]->getGraphicsItemNode() != 0)
                selectedNodes[i]->getGraphicsItemNode()->setNodeColour();

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
                                             selectedNodes[0]->getCustomLabel(), &ok);

    if (ok)
    {
        //If the custom label option isn't currently on, turn it on now.
        ui->nodeCustomLabelsCheckBox->setChecked(true);

        for (size_t i = 0; i < selectedNodes.size(); ++i)
            selectedNodes[i]->setCustomLabel(newLabel);
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

        //If the settings affecting node width was changed, reset the width on
        //each GraphicsItemNode.
        if (settingsBefore.readDepthEffectOnWidth != g_settings->readDepthEffectOnWidth ||
                settingsBefore.readDepthPower != g_settings->readDepthPower)
            g_assemblyGraph->recalculateAllNodeWidths();

        //If any of the colours changed, reset the node colours now.
        if (settingsBefore.uniformPositiveNodeColour != g_settings->uniformPositiveNodeColour ||
                settingsBefore.uniformNegativeNodeColour != g_settings->uniformNegativeNodeColour ||
                settingsBefore.uniformNodeSpecialColour != g_settings->uniformNodeSpecialColour ||
                settingsBefore.autoReadDepthValue != g_settings->autoReadDepthValue ||
                settingsBefore.lowReadDepthColour != g_settings->lowReadDepthColour ||
                settingsBefore.highReadDepthColour != g_settings->highReadDepthColour ||
                settingsBefore.lowReadDepthValue != g_settings->lowReadDepthValue ||
                settingsBefore.highReadDepthValue != g_settings->highReadDepthValue ||
                settingsBefore.noBlastHitsColour != g_settings->noBlastHitsColour ||
                settingsBefore.contiguousStrandSpecificColour != g_settings->contiguousStrandSpecificColour ||
                settingsBefore.contiguousEitherStrandColour != g_settings->contiguousEitherStrandColour ||
                settingsBefore.notContiguousColour != g_settings->notContiguousColour ||
                settingsBefore.maybeContiguousColour != g_settings->maybeContiguousColour ||
                settingsBefore.contiguityStartingColour != g_settings->contiguityStartingColour ||
                settingsBefore.randomColourPositiveOpacity != g_settings->randomColourPositiveOpacity ||
                settingsBefore.randomColourNegativeOpacity != g_settings->randomColourNegativeOpacity ||
                settingsBefore.randomColourPositiveSaturation != g_settings->randomColourPositiveSaturation ||
                settingsBefore.randomColourNegativeSaturation != g_settings->randomColourNegativeSaturation ||
                settingsBefore.randomColourPositiveLightness != g_settings->randomColourPositiveLightness ||
                settingsBefore.randomColourNegativeLightness != g_settings->randomColourNegativeLightness)
        {
            g_assemblyGraph->resetAllNodeColours();
        }

        g_graphicsView->setAntialiasing(g_settings->antialiasing);
        g_graphicsView->viewport()->update();
    }
}

void MainWindow::selectUserSpecifiedNodes()
{
    if (g_assemblyGraph->checkIfStringHasNodes(ui->selectionSearchNodesLineEdit->text()))
    {
        QMessageBox::information(this, "No starting nodes",
                                 "Please enter at least one node when drawing the graph using the 'Around node(s)' scope. "
                                 "Separate multiple nodes with commas.");
        return;
    }




    if (ui->selectionSearchNodesLineEdit->text().length() == 0)
    {
        QMessageBox::information(this, "No nodes given", "Please enter the numbers of the nodes to find, separated by commas.");
        return;
    }

    m_scene->blockSignals(true);
    m_scene->clearSelection();
    std::vector<QString> nodesNotInGraph;
    std::vector<DeBruijnNode *> nodesToSelect = getNodesFromLineEdit(ui->selectionSearchNodesLineEdit,
                                                                     ui->selectionSearchNodesExactMatchRadioButton->isChecked(),
                                                                     &nodesNotInGraph);

    //Select each node that actually has a GraphicsItemNode, and build a bounding
    //rectangle so the viewport can focus on the selected node.
    std::vector<QString> nodesNotFound;
    int foundNodes = 0;
    for (size_t i = 0; i < nodesToSelect.size(); ++i)
    {
        GraphicsItemNode * graphicsItemNode = nodesToSelect[i]->getGraphicsItemNode();

        //If the GraphicsItemNode isn't found, try the reverse complement.  This
        //is only done for single node mode.
        if (graphicsItemNode == 0 && !g_settings->doubleMode)
            graphicsItemNode = nodesToSelect[i]->getReverseComplement()->getGraphicsItemNode();

        if (graphicsItemNode != 0)
        {
            graphicsItemNode->setSelected(true);
            ++foundNodes;
        }
        else
            nodesNotFound.push_back(nodesToSelect[i]->getName());
    }

    if (foundNodes > 0)
        zoomToSelection();

    if (nodesNotInGraph.size() > 0 || nodesNotFound.size() > 0)
    {
        QString errorMessage;
        if (nodesNotInGraph.size() > 0)
        {
            errorMessage += g_assemblyGraph->generateNodesNotFoundErrorMessage(nodesNotInGraph,
                                                                               ui->selectionSearchNodesExactMatchRadioButton->isChecked());
        }
        if (nodesNotFound.size() > 0)
        {
            if (errorMessage.length() > 0)
                errorMessage += "\n";
            errorMessage += "The following nodes are in the graph but not currently displayed:\n";
            for (size_t i = 0; i < nodesNotFound.size(); ++i)
            {
                errorMessage += nodesNotFound[i];
                if (i != nodesNotFound.size() - 1)
                    errorMessage += ", ";
            }
            errorMessage += "\n\nRedraw the graph with an increased scope to see these nodes.\n";
        }
        QMessageBox::information(this, "Nodes not found", errorMessage);
    }

    m_scene->blockSignals(false);
    g_graphicsView->viewport()->update();
    selectionChanged();
}


void MainWindow::openAboutDialog()
{
    AboutDialog aboutDialog(this);
    aboutDialog.exec();
}


void MainWindow::openBlastSearchDialog()
{
    //If a BLAST search dialog does not current exist, make it.
    if (m_blastSearchDialog == 0)
    {
        m_blastSearchDialog = new BlastSearchDialog(this);
        connect(m_blastSearchDialog, SIGNAL(blastChanged()), this, SLOT(blastChanged()));
        connect(m_blastSearchDialog, SIGNAL(queryPathSelectionChanged()), g_graphicsView->viewport(), SLOT(update()));
    }

    m_blastSearchDialog->show();
}


//This function is called whenever the user does something in the
//BlastSearchDialog that should be reflected here in MainWindow.
void MainWindow::blastChanged()
{
    QString blastQueryText = ui->blastQueryComboBox->currentText();
    BlastQuery * queryBefore = g_blastSearch->m_blastQueries.getQueryFromName(blastQueryText);

    //If we didn't find a currently selected query but it isn't "none" or "all",
    //then maybe the user changed the name of the currently selected query, and
    //that's why we didn't find it.  In that case, try to find it using the
    //index.
    if (queryBefore == 0 && blastQueryText != "none" && blastQueryText != "all")
    {
        int blastQueryIndex = ui->blastQueryComboBox->currentIndex();
        if (ui->blastQueryComboBox->count() > 1)
            --blastQueryIndex;
        if (blastQueryIndex < g_blastSearch->m_blastQueries.getQueryCount())
            queryBefore = g_blastSearch->m_blastQueries.m_queries[blastQueryIndex];
    }

    //Rebuild the query combo box, in case the user changed the queries or
    //their names.
    setupBlastQueryComboBox();

    //Look to see if the query selected before is still present.  If so,
    //set the combo box to have that query selected.  If not (or if no
    //query was previously selected), leave the combo box a index 0.
    if (queryBefore != 0 && g_blastSearch->m_blastQueries.isQueryPresent(queryBefore))
    {
        int indexOfQuery = ui->blastQueryComboBox->findText(queryBefore->getName());
        if (indexOfQuery != -1)
            ui->blastQueryComboBox->setCurrentIndex(indexOfQuery);
    }

    if (g_blastSearch->m_blastQueries.getQueryCount() > 0)
    {
        //If the colouring scheme is not currently BLAST hits, change it to BLAST hits now
        if (g_settings->nodeColourScheme != BLAST_HITS_RAINBOW_COLOUR &&
                g_settings->nodeColourScheme != BLAST_HITS_SOLID_COLOUR)
        {
            //If there is only one query, use BLAST rainbow.  Otherwise, use
            //BLAST solid.
            if (g_blastSearch->m_blastQueries.getQueryCount() == 1)
                setNodeColourSchemeComboBox(BLAST_HITS_RAINBOW_COLOUR);
            else
                setNodeColourSchemeComboBox(BLAST_HITS_SOLID_COLOUR);
        }
    }

    g_blastSearch->blastQueryChanged(ui->blastQueryComboBox->currentText());
    g_graphicsView->viewport()->update();
}


void MainWindow::setupBlastQueryComboBox()
{
    ui->blastQueryComboBox->clear();
    QStringList comboBoxItems;
    for (size_t i = 0; i < g_blastSearch->m_blastQueries.m_queries.size(); ++i)
    {
        if (g_blastSearch->m_blastQueries.m_queries[i]->hasHits())
            comboBoxItems.push_back(g_blastSearch->m_blastQueries.m_queries[i]->getName());
    }

    if (comboBoxItems.size() > 1)
        comboBoxItems.push_front("all");

    if (comboBoxItems.size() > 0)
    {
        ui->blastQueryComboBox->addItems(comboBoxItems);
        ui->blastQueryComboBox->setEnabled(true);
    }
    else
    {
        ui->blastQueryComboBox->addItem("none");
        ui->blastQueryComboBox->setEnabled(false);
    }
}


void MainWindow::blastQueryChanged()
{
    g_blastSearch->blastQueryChanged(ui->blastQueryComboBox->currentText());
    g_graphicsView->viewport()->update();
}


void MainWindow::setInfoTexts()
{
    QString control = "Ctrl";
    QString settingsDialogTitle = "settings";
#ifdef Q_OS_MAC
    QString command(QChar(0x2318));
    control = command;
    settingsDialogTitle = "preferences";
#endif

    ui->graphInformationInfoText->setInfoText("Node codes, edge count and total length are calculated using single "
                                              "nodes, not double nodes.<br><br>"
                                              "For example, node 5+ and node 5- would only be counted once.");
    ui->graphScopeInfoText->setInfoText("This controls how much of the assembly graph will be drawn:<ul>"
                                        "<li>'Entire graph': all nodes in the graph will be drawn. This is "
                                        "appropriate for smaller graphs, but large graphs may take "
                                        "longer and use large amounts of memory to draw in their entirety.</li>"
                                        "<li>'Around nodes': you can specify nodes and a distance to "
                                        "limit the drawing to a smaller region of the graph.</li>"
                                        "<li>'Around BLAST hits': if you have conducted a BLAST search "
                                        "on this graph, this option will draw the region(s) of the graph "
                                        "around nodes that contain hits.</li></ul>");
    ui->startingNodesInfoText->setInfoText("Enter a comma-delimited list of node names here. This will "
                                           "define which regions of the graph will be drawn.<br><br>"
                                           "When in double mode, you can include '+' or '-' at the end "
                                           "of the node name to specify which strand to draw. If you do "
                                           "not include '+' or '-', then nodes for both strands will be drawn.");
    ui->startingNodesMatchTypeInfoText->setInfoText("When 'Exact' match is used, the graph will only be drawn around nodes "
                                                    "that exactly match your above input.<br><br>"
                                                    "When 'Partial' match is used, the graph will be drawn around "
                                                    "nodes where any part of their name matches your above input.");
    ui->selectionSearchNodesMatchTypeInfoText->setInfoText("When 'Exact' match is used, nodes will only be selected if "
                                                           "their name exactly matches your input above.<br><br>"
                                                           "When 'Partial' match is used, nodes will be selected if any "
                                                           "part of their name matches your input above.");
    ui->nodeStyleInfoText->setInfoText("'Single' mode will only one node for each positive/negative pair. "
                                       "This produces a simpler graph visualisation, but "
                                       "strand-specific sequences and directionality will be less clear.<br><br>"
                                       "'Double' mode will draw both nodes and their complement nodes. The nodes "
                                       "will show directionality with an arrow head. They will initially be "
                                       "drawn on top of each other, but can be manually moved to separate them.");
    ui->drawGraphInfoText->setInfoText("Clicking this button will conduct the graph layout and draw the graph to "
                                       "the screen. This process is fast for small graphs but can be "
                                       "resource-intensive for large graphs.<br><br>"
                                       "The layout algorithm uses a random seed, so each time this button is "
                                       "clicked you will give different layouts of the same graph.");
    ui->zoomInfoText->setInfoText("This value controls how large the graph appears in Bandage. The zoom level "
                                  "can also be changed by:<ul>"
                                  "<li>Holding the " + control + " key and using the mouse wheel over the graph.</li>"
                                  "<li>Clicking on the graph display and then using the '+' and '-' keys.</li></ul>");
    ui->nodeWidthInfoText->setInfoText("This is the average width for each node. The exact width for each node is "
                                       "also influenced by the node's read depth. The effect of read depth on width "
                                       "can be adjusted in Bandage " + settingsDialogTitle + ".");
    ui->nodeColourInfoText->setInfoText("This controls the colour of the nodes in the graph:<ul>"
                                        "<li>'Random colours': Nodes will be coloured randomly. Each time this is "
                                        "selected, new random colours will be chosen. Negative nodes (visible "
                                        "in 'Double' mode) will be a darker shade of their complement positive "
                                        "nodes.</li>"
                                        "<li>'Uniform colour': For graphs drawn with the 'Entire graph' scope, all "
                                        "nodes will be the same colour. For graphs drawn with the 'Around nodes' "
                                        "scope, your specified nodes will be drawn in a separate colour. For "
                                        "graphs drawn with the 'Around BLAST hits' scope, nodes with BLAST hits "
                                        "will be drawn in a separate colour.</li>"
                                        "<li>'Colour by read depth': Node colours will be defined by their "
                                        "read depth. The details of this relationship are configurable in "
                                        "Bandage " + settingsDialogTitle + ".</li>"
                                        "<li>'BLAST hits (rainbow)': Nodes will be drawn in a light grey colour "
                                        "and BLAST hits for the currently selected query will be drawn using a "
                                        "rainbow. Red indicates the start of the query sequence and violet "
                                        "indicates the end.</li>"
                                        "<li>'BLAST hits (solid)': Nodes will be drawn in a light grey colour "
                                        "and BLAST hits for the currently selected query will be drawn using "
                                        "the query's colour. Query colours can be specified in the 'Create/view"
                                        "BLAST search' window.</li>"
                                        "<li>'Colour by contiguity': This option will display a 'Determine "
                                        "contiguity button. When pressed, the nodes will be coloured based "
                                        "on their contiguity with the selected node(s).</li>"
                                        "<li>'Custom colours': Nodes will be coloured using colours of your "
                                        "choice. Select one or more nodes and then click the 'Set colour' button "
                                        "to define their colour.</li></ul>"
                                        "See the 'Colours' section of the Bandage " + settingsDialogTitle + " "
                                        "to control various colouring options.");
    ui->contiguityInfoText->setInfoText("Select one or more nodes and then click this button. Bandage will "
                                        "then colour which other nodes in the graph are likely to be contiguous "
                                        "with your selected node(s).");
    ui->nodeLabelsInfoText->setInfoText("Tick any of the node labelling options to display those labels over "
                                        "nodes in the graph.<br><br>"
                                        "'Name', 'Length' and 'Read depth' labels are created automatically. "
                                        "'Custom' labels must be assigned by clicking the 'Set "
                                        "label' button when one or more nodes are selected.<br><br>"
                                        "When 'BLAST hits' labels are shown, they are displayed over any "
                                        "BLAST hits present in the node.<br><br>"
                                        "The 'CSV data' option allows you to import custom labels. To use this, "
                                        "you must first load a CSV file (using the 'Load CSV label data' item in "
                                        "the 'File' menu) which contains the node names in the first column and "
                                        "custom labels in subsequent columns. The CSV file must also contain a "
                                        "header row.");
    ui->nodeFontInfoText->setInfoText("Click the 'Font' button to choose the font used for node labels. The "
                                      "colour of the font is configurable in Bandage's " + settingsDialogTitle + ".<br><br>"
                                      "Ticking 'Text outline' will surround the text with a white outline. "
                                      "This can help to make text more readable, but will obscure more of the "
                                      "underlying graph. The thickness of the text outline is configurable in "
                                      "Bandage's " + settingsDialogTitle + ".");
    ui->blastSearchInfoText->setInfoText("Click this button to open a dialog where a BLAST search for one "
                                         "or more queries can be carried out on the graph's nodes.<br><br>"
                                         "After a BLAST search is complete, it will be possible to use the "
                                         "'Around BLAST hits' graph scope and the 'BLAST "
                                         "hits' colour modes.");
    ui->blastQueryInfoText->setInfoText("After a BLAST search is completed, you can select a query here for use "
                                        "with the 'Around BLAST hits' graph scope and the 'BLAST "
                                        "hits' colour modes.");
    ui->selectionSearchInfoText->setInfoText("Type a comma-delimited list of one or mode node numbers and then click "
                                             "the 'Find node(s)' button to search for nodes in the graph. "
                                             "If the search is successful, the view will zoom to the found nodes "
                                             "and they will be selected.");
    ui->setColourAndLabelInfoText->setInfoText("Custom colours and labels can be applied to selected nodes using "
                                               "these buttons. They will only be visible when the colouring "
                                               "mode is set to 'Custom colours' and the 'Custom' label option "
                                               "is ticked.");
    ui->minReadDepthInfoText->setInfoText("This is the lower bound for the read depth range. Nodes with a read "
                                          "depth less than this value will not be drawn.");
    ui->maxReadDepthInfoText->setInfoText("This is the uper bound for the read depth range. Nodes with a read "
                                          "depth greater than this value will not be drawn.");
}



void MainWindow::setUiState(UiState uiState)
{
    m_uiState = uiState;

    switch (uiState)
    {
    case NO_GRAPH_LOADED:
        ui->graphDetailsWidget->setEnabled(false);
        ui->graphDrawingWidget->setEnabled(false);
        ui->graphDisplayWidget->setEnabled(false);
        ui->nodeLabelsWidget->setEnabled(false);
        ui->blastSearchWidget->setEnabled(false);
        ui->selectionScrollAreaWidgetContents->setEnabled(false);
        ui->actionLoad_CSV->setEnabled(false);
        break;
    case GRAPH_LOADED:
        ui->graphDetailsWidget->setEnabled(true);
        ui->graphDrawingWidget->setEnabled(true);
        ui->graphDisplayWidget->setEnabled(false);
        ui->nodeLabelsWidget->setEnabled(false);
        ui->blastSearchWidget->setEnabled(true);
        ui->selectionScrollAreaWidgetContents->setEnabled(false);
        ui->actionLoad_CSV->setEnabled(true);
        break;
    case GRAPH_DRAWN:
        ui->graphDetailsWidget->setEnabled(true);
        ui->graphDrawingWidget->setEnabled(true);
        ui->graphDisplayWidget->setEnabled(true);
        ui->nodeLabelsWidget->setEnabled(true);
        ui->blastSearchWidget->setEnabled(true);
        ui->selectionScrollAreaWidgetContents->setEnabled(true);
        ui->actionZoom_to_selection->setEnabled(true);
        ui->actionLoad_CSV->setEnabled(true);
        break;
    }
}


void MainWindow::showHidePanels()
{
    ui->controlsScrollArea->setVisible(ui->actionControls_panel->isChecked());
    ui->selectionScrollArea->setVisible(ui->actionSelection_panel->isChecked());
}


void MainWindow::bringSelectedNodesToFront()
{
    m_scene->blockSignals(true);

    std::vector<DeBruijnNode *> selectedNodes = m_scene->getSelectedNodes();
    if (selectedNodes.size() == 0)
    {
        QMessageBox::information(this, "No nodes selected", "You must first select nodes in the graph before using "
                                                            "the 'Bring selected nodes to front' function.");
        return;
    }

    double topZ = m_scene->getTopZValue();
    double newZ = topZ + 1.0;

    for (size_t i = 0; i < selectedNodes.size(); ++i)
    {
        GraphicsItemNode * graphicsItemNode = selectedNodes[i]->getGraphicsItemNode();

        if (graphicsItemNode == 0)
            continue;

        graphicsItemNode->setZValue(newZ);
    }
    m_scene->blockSignals(false);
    g_graphicsView->viewport()->update();
}


void MainWindow::selectNodesWithBlastHits()
{
    if (ui->blastQueryComboBox->currentText() == "none")
    {
        QMessageBox::information(this, "No BLAST hits",
                                       "To select nodes with BLAST hits, you must first conduct a BLAST search.");
        return;
    }

    m_scene->blockSignals(true);
    m_scene->clearSelection();

    bool atLeastOneNodeHasBlastHits = false;
    bool atLeastOneNodeSelected = false;

    QMapIterator<QString, DeBruijnNode*> i(g_assemblyGraph->m_deBruijnGraphNodes);
    while (i.hasNext())
    {
        i.next();
        DeBruijnNode * node = i.value();

        bool nodeHasBlastHits;

        //If we're in double mode, only select a node if it has a BLAST hit itself.
        if (g_settings->doubleMode)
            nodeHasBlastHits = node->thisNodeHasBlastHits();

        //In single mode, select a node if it or its reverse complement has a BLAST hit.
        else
            nodeHasBlastHits = node->thisNodeOrReverseComplementHasBlastHits();

        if (nodeHasBlastHits)
            atLeastOneNodeHasBlastHits = true;

        GraphicsItemNode * graphicsItemNode = node->getGraphicsItemNode();

        if (graphicsItemNode == 0)
            continue;

        if (nodeHasBlastHits)
        {
            graphicsItemNode->setSelected(true);
            atLeastOneNodeSelected = true;
        }
    }
    m_scene->blockSignals(false);
    g_graphicsView->viewport()->update();
    selectionChanged();

    if (!atLeastOneNodeHasBlastHits)
    {
        QMessageBox::information(this, "No BLAST hits",
                                       "To select nodes with BLAST hits, you must first conduct a BLAST search.");
        return;
    }

    if (!atLeastOneNodeSelected)
        QMessageBox::information(this, "No BLAST hits in visible nodes",
                                       "No nodes with BLAST hits are currently visible, so there is nothing to select. "
                                       "Adjust the graph scope to make the nodes with BLAST hits visible.");
    else
        zoomToSelection();
}


void MainWindow::selectAll()
{
    m_scene->blockSignals(true);
    QList<QGraphicsItem *> allItems = m_scene->items();
    for (int i = 0; i < allItems.size(); ++i)
    {
        QGraphicsItem * item = allItems[i];
        item->setSelected(true);
    }
    m_scene->blockSignals(false);
    g_graphicsView->viewport()->update();
    selectionChanged();
}


void MainWindow::selectNone()
{
    m_scene->blockSignals(true);
    QList<QGraphicsItem *> allItems = m_scene->items();
    for (int i = 0; i < allItems.size(); ++i)
    {
        QGraphicsItem * item = allItems[i];
        item->setSelected(false);
    }
    m_scene->blockSignals(false);
    g_graphicsView->viewport()->update();
    selectionChanged();
}

void MainWindow::invertSelection()
{
    m_scene->blockSignals(true);
    QList<QGraphicsItem *> allItems = m_scene->items();
    for (int i = 0; i < allItems.size(); ++i)
    {
        QGraphicsItem * item = allItems[i];
        item->setSelected(!item->isSelected());
    }
    m_scene->blockSignals(false);
    g_graphicsView->viewport()->update();
    selectionChanged();
}



void MainWindow::zoomToSelection()
{
    QList<QGraphicsItem *> selection = m_scene->selectedItems();
    if (selection.size() == 0)
    {
        QMessageBox::information(this, "No nodes selected", "You must first select nodes in the graph before using "
                                                            "the 'Zoom to fit selection' function.");
        return;
    }

    QRectF boundingBox;
    for (int i = 0; i < selection.size(); ++i)
    {
        QGraphicsItem * selectedItem = selection[i];
        boundingBox = boundingBox | selectedItem->boundingRect();
    }

    zoomToFitRect(boundingBox);
}



void MainWindow::selectContiguous()
{
    selectBasedOnContiguity(CONTIGUOUS_EITHER_STRAND);
}

void MainWindow::selectMaybeContiguous()
{
    selectBasedOnContiguity(MAYBE_CONTIGUOUS);
}

void MainWindow::selectNotContiguous()
{
    selectBasedOnContiguity(NOT_CONTIGUOUS);
}



void MainWindow::selectBasedOnContiguity(ContiguityStatus targetContiguityStatus)
{
    if (!g_assemblyGraph->m_contiguitySearchDone)
    {
        QMessageBox::information(this, "Contiguity determination not done",
                                       "To select nodes by their contiguity status, while in 'Colour "
                                       "by contiguity' mode, you must select a node and then click "
                                       "'Determine contiguity'.");
        return;
    }

    m_scene->blockSignals(true);
    m_scene->clearSelection();

    QMapIterator<QString, DeBruijnNode*> i(g_assemblyGraph->m_deBruijnGraphNodes);
    while (i.hasNext())
    {
        i.next();
        DeBruijnNode * node = i.value();
        GraphicsItemNode * graphicsItemNode = node->getGraphicsItemNode();

        if (graphicsItemNode == 0)
            continue;

        //For single nodes, choose the greatest contiguity status of this
        //node and its complement.
        ContiguityStatus nodeContiguityStatus = node->getContiguityStatus();
        if (!g_settings->doubleMode)
        {
            ContiguityStatus twinContiguityStatus = node->getReverseComplement()->getContiguityStatus();
            if (twinContiguityStatus < nodeContiguityStatus)
                nodeContiguityStatus = twinContiguityStatus;
        }

        if (targetContiguityStatus == CONTIGUOUS_EITHER_STRAND &&
                (nodeContiguityStatus == CONTIGUOUS_STRAND_SPECIFIC || nodeContiguityStatus == CONTIGUOUS_EITHER_STRAND))
            graphicsItemNode->setSelected(true);
        else if (targetContiguityStatus == MAYBE_CONTIGUOUS &&
                 nodeContiguityStatus == MAYBE_CONTIGUOUS)
            graphicsItemNode->setSelected(true);
        else if (targetContiguityStatus == NOT_CONTIGUOUS &&
                 nodeContiguityStatus == NOT_CONTIGUOUS)
            graphicsItemNode->setSelected(true);
    }

    m_scene->blockSignals(false);
    g_graphicsView->viewport()->update();
    selectionChanged();
    zoomToSelection();
}



void MainWindow::openBandageUrl()
{
    QDesktopServices::openUrl(QUrl("https://github.com/rrwick/Bandage/wiki"));
}







void MainWindow::setWidgetsFromSettings()
{
    ui->singleNodesRadioButton->setChecked(!g_settings->doubleMode);
    ui->doubleNodesRadioButton->setChecked(g_settings->doubleMode);

    ui->nodeWidthSpinBox->setValue(g_settings->averageNodeWidth);

    ui->nodeNamesCheckBox->setChecked(g_settings->displayNodeNames);
    ui->nodeLengthsCheckBox->setChecked(g_settings->displayNodeLengths);
    ui->nodeReadDepthCheckBox->setChecked(g_settings->displayNodeReadDepth);
    ui->blastHitsCheckBox->setChecked(g_settings->displayBlastHits);
    ui->textOutlineCheckBox->setChecked(g_settings->textOutline);

    ui->startingNodesExactMatchRadioButton->setChecked(g_settings->startingNodesExactMatch);
    ui->startingNodesPartialMatchRadioButton->setChecked(!g_settings->startingNodesExactMatch);

    setNodeColourSchemeComboBox(g_settings->nodeColourScheme);

    setGraphScopeComboBox(g_settings->graphScope);
    ui->nodeDistanceSpinBox->setValue(g_settings->nodeDistance);
    ui->startingNodesLineEdit->setText(g_settings->startingNodes);

    ui->minReadDepthSpinBox->setValue(g_settings->minReadDepthRange);
    ui->maxReadDepthSpinBox->setValue(g_settings->maxReadDepthRange);
}



void MainWindow::setNodeColourSchemeComboBox(NodeColourScheme nodeColourScheme)
{
    switch (nodeColourScheme)
    {
    case RANDOM_COLOURS: ui->coloursComboBox->setCurrentIndex(0); break;
    case UNIFORM_COLOURS: ui->coloursComboBox->setCurrentIndex(1); break;
    case READ_DEPTH_COLOUR: ui->coloursComboBox->setCurrentIndex(2); break;
    case BLAST_HITS_SOLID_COLOUR: ui->coloursComboBox->setCurrentIndex(3); break;
    case BLAST_HITS_RAINBOW_COLOUR: ui->coloursComboBox->setCurrentIndex(4); break;
    case CONTIGUITY_COLOUR: ui->coloursComboBox->setCurrentIndex(5); break;
    case CUSTOM_COLOURS: ui->coloursComboBox->setCurrentIndex(6); break;
    }
}

void MainWindow::setGraphScopeComboBox(GraphScope graphScope)
{
    switch (graphScope)
    {
    case WHOLE_GRAPH: ui->graphScopeComboBox->setCurrentIndex(0); break;
    case AROUND_NODE: ui->graphScopeComboBox->setCurrentIndex(1); break;
    case AROUND_BLAST_HITS: ui->graphScopeComboBox->setCurrentIndex(2); break;
    case READ_DEPTH_RANGE: ui->graphScopeComboBox->setCurrentIndex(3); break;
    }
}

void MainWindow::nodeDistanceChanged()
{
    g_settings->nodeDistance = ui->nodeDistanceSpinBox->value();
}

void MainWindow::readDepthRangeChanged()
{
    g_settings->minReadDepthRange = ui->minReadDepthSpinBox->value();
    g_settings->maxReadDepthRange = ui->maxReadDepthSpinBox->value();
}

void MainWindow::showEvent(QShowEvent *ev)
{
    QMainWindow::showEvent(ev);
    emit windowLoaded();
}


void MainWindow::startingNodesExactMatchChanged()
{
    g_settings->startingNodesExactMatch = ui->startingNodesExactMatchRadioButton->isChecked();
}


void MainWindow::openPathSpecifyDialog()
{
    //Don't open a second dialog if one's already up.
    if (g_memory->pathDialogIsVisible)
        return;

    PathSpecifyDialog * pathSpecifyDialog = new PathSpecifyDialog(this);
    connect(g_graphicsView, SIGNAL(doubleClickedNode(DeBruijnNode*)), pathSpecifyDialog, SLOT(addNodeName(DeBruijnNode*)));
    pathSpecifyDialog->show();
}


QString MainWindow::convertGraphFileTypeToString(GraphFileType graphFileType)
{
    QString graphFileTypeString;
    switch (graphFileType)
    {
    case LAST_GRAPH: graphFileTypeString = "LastGraph"; break;
    case FASTG: graphFileTypeString = "FASTG"; break;
    case GFA: graphFileTypeString = "GFA"; break;
    case TRINITY: graphFileTypeString = "Trinity.fasta"; break;
    case ANY_FILE_TYPE: graphFileTypeString = "any"; break;
    case UNKNOWN_FILE_TYPE: graphFileTypeString = "unknown"; break;
    }
    return graphFileTypeString;
}


void MainWindow::setSelectedNodesWidgetsVisibility(bool visible)
{
    ui->selectedNodesTitleLabel->setVisible(visible);
    ui->selectedNodesLine1->setVisible(visible);
    ui->selectedNodesLine2->setVisible(visible);
    ui->selectedNodesTextEdit->setVisible(visible);
    ui->selectedNodesModificationWidget->setVisible(visible);
    ui->selectedNodesLengthLabel->setVisible(visible);
    ui->selectedNodesDepthLabel->setVisible(visible);

    if (visible)
        ui->selectedNodesSpacer->changeSize(20, 60);
    else
        ui->selectedNodesSpacer->changeSize(20, 0);

}

void MainWindow::setSelectedEdgesWidgetsVisibility(bool visible)
{
    ui->selectedEdgesTitleLabel->setVisible(visible);
    ui->selectedEdgesTextEdit->setVisible(visible);
    ui->selectedEdgesLine->setVisible(visible);

    if (visible)
        ui->selectedEdgesSpacer->changeSize(20, 60);
    else
        ui->selectedEdgesSpacer->changeSize(20, 0);
}


void MainWindow::nodeWidthChanged()
{
    g_settings->averageNodeWidth = ui->nodeWidthSpinBox->value();
    g_assemblyGraph->recalculateAllNodeWidths();
    g_graphicsView->viewport()->update();
}


void MainWindow::saveEntireGraphToFasta()
{
    QString defaultFileNameAndPath = g_memory->rememberedPath + "/all_graph_nodes.fasta";
    QString fullFileName = QFileDialog::getSaveFileName(this, "Save entire graph", defaultFileNameAndPath, "FASTA (*.fasta)");

    if (fullFileName != "") //User did not hit cancel
    {
        QFile file(fullFileName);
        file.open(QIODevice::WriteOnly | QIODevice::Text);
        QTextStream out(&file);

        g_memory->rememberedPath = QFileInfo(fullFileName).absolutePath();

        QMapIterator<QString, DeBruijnNode*> i(g_assemblyGraph->m_deBruijnGraphNodes);
        while (i.hasNext())
        {
            i.next();
            out << i.value()->getFasta();
        }
    }
}

void MainWindow::saveEntireGraphToFastaOnlyPositiveNodes()
{
    QString defaultFileNameAndPath = g_memory->rememberedPath + "/all_positive_graph_nodes.fasta";
    QString fullFileName = QFileDialog::getSaveFileName(this, "Save entire graph (only positive nodes)", defaultFileNameAndPath, "FASTA (*.fasta)");

    if (fullFileName != "") //User did not hit cancel
    {
        QFile file(fullFileName);
        file.open(QIODevice::WriteOnly | QIODevice::Text);
        QTextStream out(&file);

        g_memory->rememberedPath = QFileInfo(fullFileName).absolutePath();

        QMapIterator<QString, DeBruijnNode*> i(g_assemblyGraph->m_deBruijnGraphNodes);
        while (i.hasNext())
        {
            i.next();
            DeBruijnNode * node = i.value();
            if (node->isPositiveNode())
                out << node->getFasta();
        }
    }
}


void MainWindow::saveEntireGraphToGfa()
{
    QString defaultFileNameAndPath = g_memory->rememberedPath + "/graph.gfa";
    QString fullFileName = QFileDialog::getSaveFileName(this, "Save entire graph", defaultFileNameAndPath, "GFA (*.gfa)");

    if (fullFileName != "") //User did not hit cancel
    {
        QFile file(fullFileName);
        file.open(QIODevice::WriteOnly | QIODevice::Text);
        QTextStream out(&file);

        g_memory->rememberedPath = QFileInfo(fullFileName).absolutePath();

        QMapIterator<QString, DeBruijnNode*> i(g_assemblyGraph->m_deBruijnGraphNodes);
        while (i.hasNext())
        {
            i.next();
            out << i.value()->getGfaSegmentLine();
        }
        for (size_t i = 0; i < g_assemblyGraph->m_deBruijnGraphEdges.size(); ++i)
        {
            DeBruijnEdge * edge = g_assemblyGraph->m_deBruijnGraphEdges[i];
            out << edge->getGfaLinkLine();
        }
    }
}

void MainWindow::saveVisibleGraphToGfa()
{
    QString defaultFileNameAndPath = g_memory->rememberedPath + "/graph.gfa";
    QString fullFileName = QFileDialog::getSaveFileName(this, "Save visible graph", defaultFileNameAndPath, "GFA (*.gfa)");

    if (fullFileName != "") //User did not hit cancel
    {
        QFile file(fullFileName);
        file.open(QIODevice::WriteOnly | QIODevice::Text);
        QTextStream out(&file);

        g_memory->rememberedPath = QFileInfo(fullFileName).absolutePath();

        QMapIterator<QString, DeBruijnNode*> i(g_assemblyGraph->m_deBruijnGraphNodes);
        while (i.hasNext())
        {
            i.next();
            DeBruijnNode * node = i.value();
            if (node->thisOrReverseComplementHasGraphicsItemNode())
                out << node->getGfaSegmentLine();
        }
        for (size_t i = 0; i < g_assemblyGraph->m_deBruijnGraphEdges.size(); ++i)
        {
            DeBruijnEdge * edge = g_assemblyGraph->m_deBruijnGraphEdges[i];
            if (edge->getStartingNode()->thisOrReverseComplementHasGraphicsItemNode() &&
                    edge->getEndingNode()->thisOrReverseComplementHasGraphicsItemNode())
                out << edge->getGfaLinkLine();
        }
    }
}


void MainWindow::webBlastSelectedNodes()
{
    std::vector<DeBruijnNode *> selectedNodes = m_scene->getSelectedNodes();
    if (selectedNodes.size() == 0)
    {
        QMessageBox::information(this, "Web BLAST selected nodes", "No nodes are selected.\n\n"
                                                                   "You must first select nodes in the graph before you can can use web BLAST.");
        return;
    }

    QByteArray selectedNodesFasta;
    for (size_t i = 0; i < selectedNodes.size(); ++i)
        selectedNodesFasta += selectedNodes[i]->getFastaNoNewLinesInSequence();
    selectedNodesFasta.chop(1); //remove last newline

    QByteArray urlSafeFasta = makeStringUrlSafe(selectedNodesFasta);
    QByteArray url = "http://blast.ncbi.nlm.nih.gov/Blast.cgi?PROGRAM=blastn&PAGE_TYPE=BlastSearch&LINK_LOC=blasthome&QUERY=" + urlSafeFasta;
    
    if (url.length() < 8190)
        QDesktopServices::openUrl(QUrl(url));

    else
    {
        QMessageBox::information(this, "Long sequences", "The selected node sequences are too long to pass to the BLAST web "
                                                         "interface via the URL.  Bandage has put them in your clipboard so "
                                                         "you can paste them in.");
        QClipboard * clipboard = QApplication::clipboard();
        clipboard->setText(selectedNodesFasta);

        QByteArray url = "http://blast.ncbi.nlm.nih.gov/Blast.cgi?PROGRAM=blastn&PAGE_TYPE=BlastSearch&LINK_LOC=blasthome";
        QDesktopServices::openUrl(QUrl(url));
    }
}

//http://www.ncbi.nlm.nih.gov/staff/tao/URLAPI/new/node101.html#sub:Escape-of-Unsafe
QByteArray MainWindow::makeStringUrlSafe(QByteArray s)
{
    s.replace("%", "%25");
    s.replace(">", "%3E");
    s.replace("[", "%5B");
    s.replace("]", "%5D");
    s.replace("\n", "%0D%0A");
    s.replace("|", "%7C");
    s.replace("@", "%40");
    s.replace("#", "%23");
    s.replace("+", "%2B");
    s.replace(" ", "+");
    s.replace("\t", "+");

    return s;
}


//This function removes nodes from the visualisation, but leaves them in the
//actual graph.
void MainWindow::hideNodes()
{
    std::vector<DeBruijnNode *> selectedNodes = m_scene->getSelectedNodes();
    g_assemblyGraph->removeGraphicsItemNodes(&selectedNodes, false, m_scene);
}


//This function removes selected nodes/edges from the graph.
void MainWindow::removeSelection()
{
    std::vector<DeBruijnEdge *> selectedEdges = m_scene->getSelectedEdges();
    std::vector<DeBruijnNode *> selectedNodes = m_scene->getSelectedNodes();

    g_assemblyGraph->removeGraphicsItemEdges(&selectedEdges, true, m_scene);
    g_assemblyGraph->removeGraphicsItemNodes(&selectedNodes, true, m_scene);

    g_assemblyGraph->deleteEdges(&selectedEdges);
    g_assemblyGraph->deleteNodes(&selectedNodes);

    g_assemblyGraph->determineGraphInfo();
    displayGraphDetails();
    cleanUpAllBlast();
}



void MainWindow::duplicateSelectedNodes()
{
    std::vector<DeBruijnNode *> selectedNodes = m_scene->getSelectedNodes();
    if (selectedNodes.size() == 0)
    {
        QMessageBox::information(this, "No nodes selected", "You must first select one or more nodes before using the 'Duplicate selected nodes' function.");
        return;
    }

    //Nodes are always duplicated in pairs (both positive and negative), so we
    //want to compile a list of only positive nodes.
    QList<DeBruijnNode *> nodesToDuplicate;
    for (size_t i = 0; i < selectedNodes.size(); ++i)
    {
        DeBruijnNode * node = selectedNodes[i];
        if (node->isNegativeNode())
            node = node->getReverseComplement();
        if (!nodesToDuplicate.contains(node))
            nodesToDuplicate.push_back(node);
    }

    for (int i = 0; i < nodesToDuplicate.size(); ++i)
        g_assemblyGraph->duplicateNodePair(nodesToDuplicate[i], m_scene);

    g_assemblyGraph->determineGraphInfo();
    displayGraphDetails();
    cleanUpAllBlast();
}

void MainWindow::mergeSelectedNodes()
{
    std::vector<DeBruijnNode *> selectedNodes = m_scene->getSelectedNodes();
    if (selectedNodes.size() < 2)
    {
        QMessageBox::information(this, "Not enough nodes selected", "You must first select two or more nodes before using the 'Merge selected nodes' function.");
        return;
    }

    //Nodes are always merged in pairs (both positive and negative), so we
    //want to compile a list of only positive nodes.
    QList<DeBruijnNode *> nodesToMerge;
    for (size_t i = 0; i < selectedNodes.size(); ++i)
    {
        DeBruijnNode * node = selectedNodes[i];
        if (node->isNegativeNode())
            node = node->getReverseComplement();
        if (!nodesToMerge.contains(node))
            nodesToMerge.push_back(node);
    }

    if (nodesToMerge.size() < 2)
    {
        QMessageBox::information(this, "Not enough nodes selected", "You must first select two or more nodes before using the 'Merge selected nodes' function. "
                                                                    "Note that two complementary nodes only count as a single node regarding a merge.");
        return;
    }

    bool success = g_assemblyGraph->mergeNodes(nodesToMerge, m_scene);

    if (!success)
    {
        QMessageBox::information(this, "Nodes cannot be merged", "You can only merge nodes that are in a single, unbranching path with no extra edges.");
        return;
    }

    g_assemblyGraph->determineGraphInfo();
    displayGraphDetails();
    cleanUpAllBlast();
}

void MainWindow::mergeAllPossible()
{
    int merges = g_assemblyGraph->mergeAllPossible(m_scene);
    if (merges > 0)
    {
        g_assemblyGraph->determineGraphInfo();
        displayGraphDetails();
        cleanUpAllBlast();
    }
    else
        QMessageBox::information(this, "No possible merges", "The graph contains no nodes that can be merged.");
}


void MainWindow::cleanUpAllBlast()
{
    g_blastSearch->cleanUp();
    g_assemblyGraph->clearAllBlastHitPointers();
}
