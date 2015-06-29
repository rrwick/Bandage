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
#include <ogdf/energybased/FMMMLayout.h>

class GraphicsViewZoom;
class MyGraphicsScene;
class DeBruijnNode;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QString filename);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    MyGraphicsScene * m_scene;

    GraphicsViewZoom * m_graphicsViewZoom;
    double m_previousZoomSpinBoxValue;
    QThread * m_layoutThread;
    ogdf::FMMMLayout * m_fmmm;

    void cleanUp();
    void displayGraphDetails();
    void clearGraphDetails();
    void resetScene();
    void layoutGraph();
    void addGraphicsItemsToScene();
    void zoomToFitRect(QRectF rect);
    void zoomToFitScene();
    void setZoomSpinBoxStep();
    void removeAllGraphicsEdgesFromNode(DeBruijnNode * node);
    void getSelectedNodeInfo(int & selectedNodeCount, QString & selectedNodeCountText, QString & selectedNodeListText, QString & selectedNodeLengthText);
    QString getSelectedEdgeListText();
    double getGraphicsItemNodeWidth(double coverageRelativeToMean);
    std::vector<DeBruijnNode *> getNodesFromLineEdit(QLineEdit * lineEdit, std::vector<QString> * nodesNotInGraph = 0);
    std::vector<DeBruijnNode *> getNodesFromBlastHits();
    void setSceneRectangle();
    bool checkFileIsLastGraph(QString fullFileName);
    bool checkFileIsFastG(QString fullFileName);
    bool checkFileIsGfa(QString fullFileName);
    bool checkFileIsTrinityFasta(QString fullFileName);
    bool checkFirstLineOfFile(QString fullFileName, QString regExp);
    void loadGraph2(GraphFileType graphFileType, QString filename);
    void buildDeBruijnGraphFromLastGraph(QString fullFileName);
    void buildDeBruijnGraphFromFastg(QString fullFileName);
    void buildDeBruijnGraphFromGfa(QString fullFileName);
    void buildDeBruijnGraphFromTrinityFasta(QString fullFileName);
    void buildOgdfGraphFromNodesAndEdges();
    void setInfoTexts();
    void enableDisableUiElements(UiState uiState);
    void makeReverseComplementNodeIfNecessary(DeBruijnNode * node);
    void pointEachNodeToItsReverseComplement();
    void selectBasedOnContiguity(ContiguityStatus contiguityStatus);

private slots:
    void loadGraph(QString fullFileName = "");
    void selectionChanged();
    void graphScopeChanged();
    void drawGraph();
    void zoomSpinBoxChanged();
    void zoomedWithMouseWheel();
    void copySelectedSequencesToClipboard();
    void saveSelectedSequencesToFile();
    void switchColourScheme();
    void determineContiguityFromSelectedNode();
    void saveImageCurrentView();
    void saveImageEntireScene();
    void setTextDisplaySettings();
    void fontButtonPressed();
    void setNodeCustomColour();
    void setNodeCustomLabel();
    void removeNodes();
    void openSettingsDialog();
    void openAboutDialog();
    void selectUserSpecifiedNodes();
    void graphLayoutFinished();
    void openBlastSearchDialog();
    void blastTargetChanged();
    void saveAllNodesToFasta(QString path, bool includeEmptyNodes, bool useTrinityNames);
    void showHidePanels();
    void graphLayoutCancelled();
    void bringSelectedNodesToFront();
    void selectNodesWithBlastHits();
    void selectAll();
    void selectNone();
    void invertSelection();
    void zoomToSelection();
    void selectContiguous();
    void selectMaybeContiguous();
    void selectNotContiguous();
    void openBandageUrl();

signals:
    void saveAllNodesToFastaFinished();
};

#endif // MAINWINDOW_H
