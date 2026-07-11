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


#ifndef RANDOMFORESTMAINWINDOW_H
#define RANDOMFORESTMAINWINDOW_H

#include <QMainWindow>
#include <QWindow>
#include <QDialog>
#include <QGraphicsScene>
#include <QMap>
#include <QString>
#include <vector>
#include <QLineEdit>
#include <QRectF>
#include "../program/globals.h"
#include <QThread>
#include "../ogdf/tree/TreeLayout.h"
#include "mainwindow.h"

class GraphicsViewZoom;
class MyGraphicsScene;
class RandomForestNode;
class DeBruijnNode;

class RandomForestMainWindow : QObject
{
    Q_OBJECT

public:
    explicit RandomForestMainWindow();
    ~RandomForestMainWindow();

    void drawGraph();

    //Ui::MainWindow* m_mainWindowUi = NULL;
    //Ui::MainWindow* ui;
    MyGraphicsScene * m_scene;

    GraphicsViewZoom * m_graphicsViewZoom;
    double m_previousZoomSpinBoxValue;
    QThread * m_layoutThread;
    ogdf::TreeLayout* m_layout;
    QString m_imageFilter;
    QString m_fileToLoadOnStartup;
    bool m_drawGraphAfterLoad;
    UiState m_uiState;
    bool m_alreadyShown;

    //void cleanUp();
   // void displayGraphDetails();
   // void clearGraphDetails();
    void resetScene();
    void layoutGraph();
 //   void zoomToFitRect(QRectF rect);
 //   void zoomToFitScene();
 //   void setZoomSpinBoxStep();
    void getSelectedNodeInfo(int & selectedNodeCount, QString & selectedFeatureNodeText);

    //QString getSelectedEdgeListText();
    //std::vector<DeBruijnNode *> getNodesFromLineEdit(QLineEdit * lineEdit, bool exactMatch, std::vector<QString> * nodesNotInGraph = 0);
    //
    //void setInfoTexts();
    //void setUiState(UiState uiState);
    //void setWidgetsFromSettings();
    //QString getDefaultImageFileName();
    //void setNodeColourSchemeComboBox(NodeColourScheme nodeColourScheme);
    //void setGraphScopeComboBox(GraphScope graphScope);
    //bool checkForImageSave();

    //void setSelectedNodesWidgetsVisibility(bool visible);
    //void setSelectedEdgesWidgetsVisibility(bool visible);
    //void setStartingNodesWidgetVisibility(bool visible);
    //void setNodeDistanceWidgetVisibility(bool visible);

    //std::vector<DeBruijnNode*> addComplementaryNodes(std::vector<DeBruijnNode*> nodes);

private slots:
//    void selectionChanged();
//    void graphScopeChanged();
//    
//    void zoomSpinBoxChanged();
//    void zoomedWithMouseWheel();
//    void switchColourScheme();
//    void saveImageCurrentView();
//    void saveImageEntireScene();
//    void setTextDisplaySettings();
//    void fontButtonPressed();
//    void setNodeCustomColour();
//    void setNodeCustomLabel();
//    void hideNodes();
//    void openSettingsDialog();
//    void openAboutDialog();
//    void selectUserSpecifiedNodes();
    void graphLayoutFinished();
//    void showHidePanels();
    void graphLayoutCancelled();
//    void bringSelectedNodesToFront();
//    void selectAll();
//    void selectNone();
//    void invertSelection();
//    void zoomToSelection();
//    void openBandageUrl();
//    void afterMainWindowShow();
//    void nodeWidthChanged();
//    void removeSelection();
//    void duplicateSelectedNodes();
//    void mergeSelectedNodes();
//    void changeNodeName();

signals:
      void windowLoaded();
};

#endif // RANDOMFORESTMAINWINDOW_H
