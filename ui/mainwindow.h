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
#include "ogdf/basic/Graph.h"
#include "ogdf/basic/GraphAttributes.h"
#include "../graph/debruijnnode.h"
#include "../graph/debruijnedge.h"
#include "../graph/graphicsitemnode.h"
#include "../graph/graphicsitemedge.h"
#include "../graph/ogdfnode.h"
#include "graphicsviewzoom.h"
#include "mygraphicsview.h"
#include "mygraphicsscene.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    MyGraphicsScene * m_scene;

    QMap<int, DeBruijnNode*> m_deBruijnGraphNodes;
    std::vector<DeBruijnEdge*> m_deBruijnGraphEdges;
    ogdf::Graph * m_ogdfGraph;
    ogdf::GraphAttributes * m_graphAttributes;
    GraphicsViewZoom * m_graphicsViewZoom;
    double m_previousZoomSpinBoxValue;

    void cleanUp();
    void buildDeBruijnGraphFromLastGraph(QString fullFileName);
    bool buildDeBruijnGraphFromFastg(QString fullFileName);
    void displayGraphDetails(int nodeCount, int edgeCount, long long totalLength, double meanCoverage);
    void resetScene();
    void clearOgdfGraphAndResetNodes();
    void createDeBruijnEdge(int nodeNumber1, int nodeNumber2);
    QByteArray getReverseComplement(QByteArray forwardSequence);
    void buildOgdfGraphFromNodesAndEdges();
    void layoutGraph();
    void addGraphicsItemsToScene();
    double getMeanDeBruijnGraphCoverage(bool drawnNodesOnly = false);
    double getMaxDeBruijnGraphCoverageOfDrawnNodes();
    void zoomToFitRect(QRectF rect);
    void zoomToFitScene();
    void setZoomSpinBoxStep();
    void resetNodeContiguityStatus();
    void resetAllNodeColours();
    void removeAllGraphicsEdgesFromNode(DeBruijnNode * node);
    void getSelectedNodeInfo(int & selectedNodeCount, QString & selectedNodeCountText, QString & selectedNodeListText, QString & selectedNodeLengthText);
    QString getSelectedEdgeListText();
    double getGraphicsItemNodeWidth(double coverageRelativeToMean);
    std::vector<DeBruijnNode *> getNodeNumbersFromLineEdit(QLineEdit * lineEdit, std::vector<QString> * nodesNotInGraph = 0);
    void setRandomColourFactor();
    void setSceneRectangle();
    bool checkFileIsLastGraph(QString fullFileName);
    bool checkFileIsFastG(QString fullFileName);
    bool checkFirstLineOfFile(QString fullFileName, QString regExp);
    void loadGraphFile(QString graphFileType);

private slots:
    void loadLastGraph();
    void loadFastg();
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
    void setNodeColour();
    void setNodeCustomColour();
    void setNodeCustomLabel();
    void removeNodes();
    void openSettingsDialog();
    void openAboutDialog();
    void selectUserSpecifiedNodes();
    void graphLayoutFinished();
};

#endif // MAINWINDOW_H
