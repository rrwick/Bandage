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

GraphLayoutWorker::GraphLayoutWorker(ogdf::GraphAttributes * graphAttributes, int graphLayoutQuality, int segmentLength) :
    m_graphAttributes(graphAttributes), m_graphLayoutQuality(graphLayoutQuality), m_segmentLength(segmentLength)
{
}


void GraphLayoutWorker::layoutGraph()
{
    ogdf::FMMMLayout fmmm;

    fmmm.useHighLevelOptions(false);
    fmmm.initialPlacementForces(ogdf::FMMMLayout::ipfRandomTime);
    fmmm.unitEdgeLength(m_segmentLength);
    fmmm.newInitialPlacement(true);

    switch (m_graphLayoutQuality)
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
    }

    fmmm.call(*m_graphAttributes);

    emit finishedLayout();
}
