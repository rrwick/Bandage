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


#include "settings.h"
#include <QDir>

Settings::Settings()
{
    doubleMode = false;

    nodeLengthMode = AUTO_NODE_LENGTH;
    autoBasePairsPerSegment = 100;
    manualBasePairsPerSegment = 500;
    meanSegmentsPerNode = 3;
    segmentLength = 15.0;
    graphLayoutQuality = 2;

    averageNodeWidth = 5.0;
    coverageEffectOnWidth = 0.5;
    coveragePower = 0.5;

    edgeWidth = 2.0;
    outlineThickness = 0.0;
    selectionThickness = 1.0;
    arrowheadSize = 0.01;
    textOutlineThickness = 0.3;

    blastRainbowPartsPerQuery = 100;

    graphScope = WHOLE_GRAPH;
    nodeDistance = 0;
    startingNodesExactMatch = true;
    startingNodes = "";
    blastQueryFilename = "";
    unnamedQueryDefaultName = "unnamed";

    minZoom = 0.01;
    maxZoom = 20.0;
    zoomFactor = 1.0005;

    dragStrength = 100.0;

    displayNodeCustomLabels = false;
    displayNodeNames = false;
    displayNodeLengths = false;
    displayNodeCoverages = false;
    displayBlastHits = false;
    labelFont = QFont();
    textOutline = true;
    antialiasing = true;
    positionTextNodeCentre = false;

    nodeDragging = NEARBY_PIECES;

    nodeColourScheme = RANDOM_COLOURS;
    uniformPositiveNodeColour = QColor(178, 34, 34);
    uniformNegativeNodeColour = QColor(128, 0, 0);
    uniformNodeSpecialColour = QColor(0, 128, 0);

    randomColourPositiveOpacity = 255;
    randomColourNegativeOpacity = 255;
    randomColourPositiveSaturation = 127;
    randomColourNegativeSaturation = 127;
    randomColourPositiveLightness = 127;
    randomColourNegativeLightness = 63;

    edgeColour = QColor(0, 0, 0, 180);
    outlineColour = QColor(0, 0, 0);
    selectionColour = QColor(0, 0, 255);
    textColour = QColor(0, 0, 0);
    textOutlineColour = QColor(255, 255, 255);

    contiguitySearchSteps = 15;
    contiguousStrandSpecificColour = QColor(0, 155, 0);
    contiguousEitherStrandColour = QColor(0, 155, 0);
    maybeContiguousColour = QColor(190, 240, 190);
    notContiguousColour = QColor(220, 220, 220);
    contiguityStartingColour = QColor(0, 255, 0);

    noBlastHitsColour = QColor(220, 220, 220);

    autoCoverageValue = true;
    lowCoverageValue = 5.0;
    lowCoverageColour = QColor(0, 0, 0);
    highCoverageValue = 50.0;
    highCoverageColour = QColor(255, 0, 0);

    pathHighlightShadingColour = QColor(0, 0, 0, 60);
    pathHighlightOutlineColour = QColor(0, 0, 0);

    blastSearchParameters = "";

    rememberedPath = QDir::homePath();

    commandLineCommand = NO_COMMAND;

    minAutoFindEdgeOverlap = 10;
    maxAutoFindEdgeOverlap = 200;

    queryRequiredCoverage = 0.95;
    queryAllowedLengthDiscrepancy = 0.05;
    maxQueryPathNodes = 6;
    maxQueryPaths = 4;

    pathDialogIsVisible = false;
    userSpecifiedPath = Path();
    userSpecifiedPathString = "";
    userSpecifiedPathCircular = false;
}


int Settings::getBasePairsPerSegment()
{
    if (nodeLengthMode == AUTO_NODE_LENGTH)
        return autoBasePairsPerSegment;
    else
        return manualBasePairsPerSegment;
}
