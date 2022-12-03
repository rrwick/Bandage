//Copyright 2017 Ryan Wick

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


#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <QMap>
#include <QString>
#include <vector>
#include <QLineEdit>
#include <QRectF>
#include "../program/globals.h"
#include <QThread>
#include "../ogdf/energybased/FMMMLayout.h"
#include "../ui/randomforestmainwindow.h"
#include "../blast/BlastFeaturesNodesMatcher.h"

class GraphicsViewZoom;
class MyGraphicsScene;
class DeBruijnNode;
class DeBruijnEdge;
class BlastSearchDialog;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QString fileToLoadOnStartup = "", bool drawGraphAfterLoad = false);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    MyGraphicsScene * m_scene;
    MyGraphicsScene * m_featureForestScene;

    GraphicsViewZoom * m_graphicsViewZoom;
    GraphicsViewZoom * m_featuresForestViewZoom;
    double m_previousZoomSpinBoxValue;
    QThread * m_layoutThread;
    ogdf::FMMMLayout * m_fmmm;
    QString m_imageFilter;
    QString m_fileToLoadOnStartup;
    bool m_drawGraphAfterLoad;
    UiState m_uiState;
    UiState m_featuresUiState;
    BlastSearchDialog * m_blastSearchDialog;
    bool m_alreadyShown;
    RandomForestMainWindow* m_randomForestMainWindow;
    BlastFeaturesNodesMatcher* m_blastFeaturesNodesMatcher = NULL;

    void cleanUp();
    void displayGraphDetails();
    void clearGraphDetails();
    void resetScene();
    void layoutGraph();
    void zoomToFitRect(QRectF rect, MyGraphicsView* graphicsView);
    void zoomToFitScene();
    void zoomToFitFeatureScene();
    void setZoomSpinBoxStep();
    void getSelectedNodeInfo(int & selectedNodeCount, QString & selectedNodeCountText, QString & selectedNodeListText, QString & selectedNodeLengthText, QString &selectedNodeDepthText);
    void MainWindow::getSelectedNodeTaxInfo(QString& selectedNodeListText);
    QString getSelectedEdgeListText();
    std::vector<DeBruijnNode *> getNodesFromLineEdit(QLineEdit * lineEdit, bool exactMatch, std::vector<QString> * nodesNotInGraph = 0);
    void loadGraph2(GraphFileType graphFileType, QString filename);
    void setInfoTexts();
    void setUiState(UiState uiState);
    void setFeaturesUiState(UiState uiState);
    void selectBasedOnContiguity(ContiguityStatus contiguityStatus);
    void setWidgetsFromSettings();
    QString getDefaultGraphImageFileName();
    QString getDefaultFeaturesImageFileName();
    void setNodeColourSchemeComboBox(NodeColourScheme nodeColourScheme);
    void setGraphScopeComboBox(GraphScope graphScope);
    void setupBlastQueryComboBox();
    bool checkForGraphImageSave();
    bool checkForFeaturesImageSave();
    QString convertGraphFileTypeToString(GraphFileType graphFileType);
    void setSelectedNodesWidgetsVisibility(bool visible);
    void setSelectedEdgesWidgetsVisibility(bool visible);
    void setStartingNodesWidgetVisibility(bool visible);
    void setNodeDistanceWidgetVisibility(bool visible);
    void setDepthRangeWidgetVisibility(bool visible);
    void MainWindow::setHiCWidgetVisibility(bool visible);
    void MainWindow::setTaxVisibility(bool visible);
    static QByteArray makeStringUrlSafe(QByteArray s);
    void removeGraphicsItemNodes(const std::vector<DeBruijnNode *> * nodes, bool reverseComplement);
    void removeGraphicsItemEdges(const std::vector<DeBruijnEdge *> * edges, bool reverseComplement);
    void removeAllGraphicsEdgesFromNode(DeBruijnNode * node, bool reverseComplement);
    std::vector<DeBruijnNode*> addComplementaryNodes(std::vector<DeBruijnNode*> nodes);
    void layoutGraphUnzip();

private slots:
    void loadGraph(QString fullFileName = "");
    void loadCSV(QString fullFileName = "");
    void loadHiC(QString fullFileName = "");
    void loadTax(QString fullFileName = "");
    void loadFeaturesForest(QString fullFileName = "");
    void selectionChanged();
    void graphScopeChanged();
    void drawGraph();
    void zoomSpinBoxChanged();
    void zoomedWithMouseWheel();
    void copySelectedSequencesToClipboardActionTriggered();
    void copySelectedSequencesToClipboard();
    void saveSelectedSequencesToFileActionTriggered();
    void saveSelectedSequencesToFile();
    void copySelectedPathToClipboard();
    void saveSelectedPathToFile();
    void switchColourScheme();
    void determineContiguityFromSelectedNode();
    void saveImageCurrentView(QString defaultFileNameAndPath, MyGraphicsView* graphicsView);
    void saveImageGraphCurrentView();
    void saveImageFeaturesCurrentView();
    void saveImageEntireScene(QString defaultFileNameAndPath, MyGraphicsView* graphicsView, MyGraphicsScene* scene);
    void saveImageGraphEntireScene();
    void saveImageFeaturesEntireScene();
    void setTextDisplaySettings();
    void fontButtonPressed();
    void setNodeCustomColour();
    void setNodeCustomLabel();
    void hideNodes();
    void openSettingsDialog();
    void openAboutDialog();
    void selectUserSpecifiedNodes();
    void graphLayoutFinished();
    void openBlastSearchDialog();
    void blastChanged();
    void blastQueryChanged();
    void showHidePanels();
    void graphLayoutCancelled();
    void bringSelectedNodesToFront();
    void selectNodesWithBlastHits();
    void selectNodesWithDeadEnds();
    void selectAll();
    void selectNone();
    void invertSelection();
    void zoomToSelection();
    void selectContiguous();
    void selectMaybeContiguous();
    void selectNotContiguous();
    void openBandageUrl();
    void nodeDistanceChanged();
    void depthRangeChanged();
    void afterMainWindowShow();
    void startingNodesExactMatchChanged();
    void openPathSpecifyDialog();
    void nodeWidthChanged();
    void featureNodeWidthChanged();
    void saveEntireGraphToFasta();
    void saveEntireGraphToFastaOnlyPositiveNodes();
    void saveEntireGraphToGfa();
    void saveVisibleGraphToGfa();
    void webBlastSelectedNodes();
    void removeSelection();
    void duplicateSelectedNodes();
    void mergeSelectedNodes();
    void mergeAllPossible();
    void cleanUpAllBlast();
    void changeNodeName();
    void changeNodeDepth();
    void openGraphInfoDialog();
    void switchTaxRank();
    void setAroundTaxWidgetVisibility(bool visible);
    void openTaxInfoDialog();
    void openTaxInfoHiCDialog();
    void unzipSelectedNodes();
    void saveTaxInfo();
    void saveHiCTaxInfo();
    void drawFeaturesForest();
    void featureSelectionChanged();
    void matchSelectedFeatureNodes();
    void switchFeatureColourScheme();
    void setFeatureNodeCustomColour();
    void setFeatureNodeCustomLabel();
    void setFeatureTextDisplaySettings();

protected:
      void showEvent(QShowEvent *ev);

signals:
      void windowLoaded();

};

#endif // MAINWINDOW_H
