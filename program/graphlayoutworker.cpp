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


#include "graphlayoutworker.h"
#include <time.h>

GraphLayoutWorker::GraphLayoutWorker(ogdf::FMMMLayout * fmmm, ogdf::GraphAttributes * graphAttributes,
                                     int graphLayoutQuality, int segmentLength) :
    m_fmmm(fmmm), m_graphAttributes(graphAttributes),
    m_graphLayoutQuality(graphLayoutQuality), m_segmentLength(segmentLength)
{
}


void GraphLayoutWorker::layoutGraph()
{
    m_fmmm->randSeed(clock());
    m_fmmm->useHighLevelOptions(false);
    m_fmmm->initialPlacementForces(ogdf::FMMMLayout::ipfRandomRandIterNr);
    m_fmmm->unitEdgeLength(m_segmentLength);
    m_fmmm->allowedPositions(ogdf::FMMMLayout::apAll);

    switch (m_graphLayoutQuality)
    {
    case 0:
        m_fmmm->fixedIterations(15);
        m_fmmm->fineTuningIterations(10);
        m_fmmm->nmPrecision(2);
        break;
    case 1:
        m_fmmm->fixedIterations(30);
        m_fmmm->fineTuningIterations(20);
        m_fmmm->nmPrecision(4);
        break;
    case 2:
        m_fmmm->fixedIterations(60);
        m_fmmm->fineTuningIterations(40);
        m_fmmm->nmPrecision(6);
        break;
    case 3:
        m_fmmm->fixedIterations(120);
        m_fmmm->fineTuningIterations(80);
        m_fmmm->nmPrecision(8);
        break;
    }

    m_fmmm->call(*m_graphAttributes);

    emit finishedLayout();
}
