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


#include "graphlayoutworker.h"
#include <time.h>
#include "ogdf/basic/geometry.h"
#include <QLineF>

//#include <ogdf/orthogonal/OrthoLayout.h>
//#include <ogdf/planarity/EmbedderMinDepthMaxFaceLayers.h>
//#include <ogdf/planarity/PlanarSubgraphFast.h>
//#include <ogdf/planarity/PlanarizationLayout.h>
//#include <ogdf/planarity/SubgraphPlanarizer.h>
//#include <ogdf/planarity/VariableEmbeddingInserter.h>

using namespace ogdf;

GraphLayoutWorker::GraphLayoutWorker(ogdf::FMMMLayout * fmmm, ogdf::GraphAttributes * graphAttributes,
    ogdf::EdgeArray<double> * edgeArray, int graphLayoutQuality, bool linearLayout,
    double graphLayoutComponentSeparation, double aspectRatio) :
    
    m_fmmm(fmmm), m_graphAttributes(graphAttributes), m_edgeArray(edgeArray), m_graphLayoutQuality(graphLayoutQuality),
    m_linearLayout(linearLayout), m_graphLayoutComponentSeparation(graphLayoutComponentSeparation),
    m_randSeed(-1), m_aspectRatio(aspectRatio)
{
}

GraphLayoutWorker::GraphLayoutWorker(ogdf::FMMMLayout* fmmm, ogdf::GraphAttributes* graphAttributes,
    ogdf::EdgeArray<double>* edgeArray, int graphLayoutQuality, bool linearLayout,
    double graphLayoutComponentSeparation, int randSeed, double aspectRatio) :
    m_fmmm(fmmm), m_graphAttributes(graphAttributes), m_edgeArray(edgeArray), m_graphLayoutQuality(graphLayoutQuality),
    m_linearLayout(linearLayout), m_graphLayoutComponentSeparation(graphLayoutComponentSeparation),
    m_randSeed(randSeed), m_aspectRatio(aspectRatio)
{
}

void GraphLayoutWorker::layoutGraph()
{
    m_fmmm->randSeed(m_randSeed);
    
    m_fmmm->useHighLevelOptions(false);
    m_fmmm->initialPlacementForces(ogdf::FMMMLayout::ipfRandomRandIterNr);
    if (m_edgeArray != NULL)
        m_fmmm->unitEdgeLength(1.0);
    else {
        m_fmmm->unitEdgeLength(15.0);
    }
    m_fmmm->allowedPositions(ogdf::FMMMLayout::apAll);
    m_fmmm->pageRatio(m_aspectRatio);
    m_fmmm->minDistCC(m_graphLayoutComponentSeparation);
    m_fmmm->stepsForRotatingComponents(50); // Helps to make linear graph components more horizontal.

    if (m_linearLayout)
        m_fmmm->initialPlacementForces(ogdf::FMMMLayout::ipfKeepPositions);
    else
        m_fmmm->initialPlacementForces(ogdf::FMMMLayout::ipfRandomTime);

    switch (m_graphLayoutQuality)
    {
    case 0:
        m_fmmm->fixedIterations(3);
        m_fmmm->fineTuningIterations(1);
        m_fmmm->nmPrecision(2);
        break;
    case 1:
        m_fmmm->fixedIterations(12);
        m_fmmm->fineTuningIterations(8);
        m_fmmm->nmPrecision(2);
        break;
    case 2:
        m_fmmm->fixedIterations(30);
        m_fmmm->fineTuningIterations(20);
        m_fmmm->nmPrecision(4);
        break;
    case 3:
        m_fmmm->fixedIterations(60);
        m_fmmm->fineTuningIterations(20);
        m_fmmm->nmPrecision(6);
        break;
    case 4:
        m_fmmm->fixedIterations(120);
        m_fmmm->fineTuningIterations(20);
        m_fmmm->nmPrecision(8);
        break;
    }
    m_fmmm->call(*m_graphAttributes, *m_edgeArray);

    emit finishedLayout();
}

