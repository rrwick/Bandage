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


#include "randomforestmainwindow.h"
#include "ui_randomforestmainwindow.h"
#include <QFileDialog>
#include <QLatin1String>
#include <QTextStream>
#include <QLocale>
#include <QRegExp>
#include <math.h>
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
#include "changenodenamedialog.h"
#include "changenodedepthdialog.h"
#include <limits>
#include "graphinfodialog.h"
#include "taxinfodialog.h"
#include<iostream>
#include<dos.h>
#include "../random_forest/assemblyforest.h"
#include "../random_forest/RandomForestNode.h"
#include "../random_forest/GraphicsItemFeatureNode.h"
#include "../ogdf/tree/TreeLayout.h"
#include "../program/TreeLayoutWorker.h"

RandomForestMainWindow::RandomForestMainWindow()
{
    m_graphicsViewZoom = new GraphicsViewZoom(g_graphicsViewFeaturesForest);
    g_graphicsViewFeaturesForest->m_zoom = m_graphicsViewZoom;

    m_scene = new MyGraphicsScene(this);
    g_graphicsViewFeaturesForest->setScene(m_scene);
}

RandomForestMainWindow::~RandomForestMainWindow()
{
    delete m_graphicsViewZoom;
}

void RandomForestMainWindow::getSelectedNodeInfo(int & selectedNodeCount, QString & selectedFeatureNodeText)
{
    std::vector<RandomForestNode *> selectedNodes = m_scene->getSelectedFeatureNodes();

    selectedNodeCount = int(selectedNodes.size());

    for (int i = 0; i < selectedNodeCount; ++i)
    {
        QString nodeName = selectedNodes[i]->getName();

        selectedFeatureNodeText += nodeName;
        selectedFeatureNodeText += '\n';
        if (selectedNodes[i]->getFeatureName() != NULL) {
            QString threshold = QString::number(selectedNodes[i]->getThreshold(), 'g', 2);
            selectedFeatureNodeText += "Feature_" + selectedNodes[i]->getFeatureName() + " <= " + threshold + "\n";
        }
        if (selectedNodes[i]->getClass() != NULL) {
            selectedFeatureNodeText += "Class: " + selectedNodes[i]->getClass() + "\n";
        }
        for (QString seq : selectedNodes[i]->getQuerySequences()) {
            selectedFeatureNodeText += "Seq: ";
            selectedFeatureNodeText += seq;
            selectedFeatureNodeText += "\n";

        }
    }
}

void RandomForestMainWindow::drawGraph()
{
    resetScene();

    g_assemblyForest->buildOgdfGraphFromNodesAndEdges();
    layoutGraph();
}

void RandomForestMainWindow::graphLayoutFinished()
{
    delete m_layout;
    m_layoutThread = 0;
    g_assemblyForest->addGraphicsItemsToScene(m_scene);
    m_scene->setSceneRectangle();
    g_graphicsViewFeaturesForest->setFocus();
    g_settings->addNewNodes = false;
}

void RandomForestMainWindow::graphLayoutCancelled()
{
}

void RandomForestMainWindow::resetScene()
{
    m_scene->blockSignals(true);

    g_graphicsViewFeaturesForest->setScene(0);
    delete m_scene;
    m_scene = new MyGraphicsScene(this);

    g_graphicsViewFeaturesForest->setScene(m_scene);

    g_graphicsViewFeaturesForest->undoRotation();
}

void RandomForestMainWindow::layoutGraph()
{
    m_layout = new ogdf::TreeLayout();

    m_layoutThread = new QThread;
    double aspectRatio = double(g_graphicsViewFeaturesForest->width()) / g_graphicsViewFeaturesForest->height();
    
    int m_clock;
    if (g_settings->m_clock == -1) {
        m_clock = clock();
        g_settings->m_clock = m_clock;
    }
    else {
        m_clock = g_settings->m_clock;
    }
    
    TreeLayoutWorker* layoutWorker = new TreeLayoutWorker(m_layout, g_assemblyForest->m_graphAttributes,
        g_assemblyForest->m_edgeArray);
    layoutWorker->moveToThread(m_layoutThread);

    connect(m_layoutThread, SIGNAL(started()), layoutWorker, SLOT(layoutGraph()));
    connect(layoutWorker, SIGNAL(finishedLayout()), m_layoutThread, SLOT(quit()));
    connect(layoutWorker, SIGNAL(finishedLayout()), layoutWorker, SLOT(deleteLater()));
    connect(layoutWorker, SIGNAL(finishedLayout()), this, SLOT(graphLayoutFinished()));
    connect(m_layoutThread, SIGNAL(finished()), m_layoutThread, SLOT(deleteLater()));
    m_layoutThread->start();
}