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

    basePairsPerSegment = 500;
    segmentLength = 15.0;
    graphLayoutQuality = 1;

    averageNodeWidth = 5.0;
    coverageEffectOnWidth = 0.5;
    edgeWidth = 2.0;
    outlineThickness = 0.5;
    selectionThickness = 1.5;
    arrowheadSize = 0.01;
    textOutlineThickness = 0.3;

    blastPartsPerQuery = 100;

    graphScope = WHOLE_GRAPH;

    minZoom = 0.01;
    maxZoom = 20.0;
    zoomFactor = 1.0005;

    dragStrength = 100.0;

    displayNodeCustomLabels = false;
    displayNodeNumbers = false;
    displayNodeLengths = false;
    displayNodeCoverages = false;
    labelFont = QFont();
    textOutline = false;
    antialiasing = true;

    nodeDragging = NEARBY_PIECES;

    nodeColourScheme = ONE_COLOUR;
    uniformPositiveNodeColour = QColor(225, 125, 125);
    setNegativeNodeColour();
    uniformNodeSpecialColour = QColor(0, 255, 0);

    edgeColour = QColor(0, 0, 0, 180);
    outlineColour = QColor(0, 0, 0);
    selectionColour = QColor(0, 0, 255);
    textColour = QColor(0, 0, 0);

    contiguitySearchSteps = 10;
    contiguousColour = QColor(0, 155, 0);
    maybeContiguousColour = QColor(190, 240, 190);
    notContiguousColour = QColor(240, 240, 240);
    contiguityStartingColour = QColor(0, 255, 0);

    noBlastHitsColour = QColor(240, 240, 240);

    autoCoverageValue = true;
    lowCoverageValue = 5.0;
    lowCoverageColour = QColor(0, 0, 0);
    highCoverageValue = 50.0;
    highCoverageColour = QColor(255, 20, 20);

    blastSearchParameters = "";

    rememberedPath = QDir::homePath();
}


//The negative node colour is a bit darker than the positive colour.
//It is only used when double graphs are drawn.
//The number in the darker function controls how much darker this
//colour is.
void Settings::setNegativeNodeColour()
{
    uniformNegativeNodeColour = uniformPositiveNodeColour.darker(120);
}



double Settings::widthScale(double zoom)
{
    return 1.0 / ((zoom + 7.0) / 8.0);
}
