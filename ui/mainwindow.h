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
    explicit MainWindow(QString filename = "", bool drawGraphAfterLoad = false);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    MyGraphicsScene * m_scene;

    GraphicsViewZoom * m_graphicsViewZoom;
    double m_previousZoomSpinBoxValue;
    QThread * m_layoutThread;
    ogdf::FMMMLayout * m_fmmm;
    QString m_imageFilter;

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
    std::vector<DeBruijnNode *> getNodesFromLineEdit(QLineEdit * lineEdit, bool exactMatch, std::vector<QString> * nodesNotInGraph = 0);
    std::vector<DeBruijnNode *> getNodesFromListExact(QStringList nodesList, std::vector<QString> * nodesNotInGraph);
    std::vector<DeBruijnNode *> getNodesFromListPartial(QStringList nodesList, std::vector<QString> * nodesNotInGraph);
    std::vector<DeBruijnNode *> getNodesFromBlastHits();
    void setSceneRectangle();
    void loadGraph2(GraphFileType graphFileType, QString filename);
    void setInfoTexts();
    void enableDisableUiElements(UiState uiState);
    void selectBasedOnContiguity(ContiguityStatus contiguityStatus);
    QString generateNodesNotFoundErrorMessage(std::vector<QString> nodesNotInGraph, bool exact);
    QStringList removeNullStringsFromList(QStringList in);
    bool checkIfLineEditHasNodes(QLineEdit * lineEdit);
    void setWidgetsFromSettings();
    QString getDefaultImageFileName();
    void setNodeColourSchemeComboBox(NodeColourScheme nodeColourScheme);

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
};

#endif // MAINWINDOW_H
