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

Settings::Settings()
{
    doubleMode = false;

    basePairsPerSegment = 500;
    segmentLength = 15.0;
    graphLayoutQuality = 1;

    minimumContigWidth = 2.0;
    coverageContigWidth = 4.0;
    maxContigWidth = 30.0;
    edgeWidth = 2.0;
    outlineThickness = 1.0;
    highlightThickness = 2.0;
    arrowheadSize = 0.01;
    textOutlineThickness = 0.5;

    blastPartsPerTarget = 30;

    graphScope = WHOLE_GRAPH;

    minZoom = 0.01;
    maxZoom = 20.0;
    zoomFactor = 1.0005;

    dragStrength = 100.0;

    displayNodeCustomLabels = false;
    displayNodeNumbers = false;
    displayNodeLengths = false;
    displayNodeCoverages = false;
    displayFont = QFont();
    textOutline = false;
    antialiasing = true;

    nodeDragging = NEARBY_PIECES;

    nodeColourScheme = ONE_COLOUR;
    positiveNodeColour = QColor(225, 125, 125);
    setNegativeNodeColour();

    highlightColour = QColor(0, 0, 255);
    edgeColour = QColor(0, 0, 0, 180);
    noSequenceColour = QColor(190, 190, 190);
    startingColour = QColor(0, 255, 0);
    contiguousColour = QColor(0, 155, 0);
    maybeContiguousColour = QColor(190, 190, 190);
    notContiguousColour = QColor(190, 190, 190);
    minCoverageColour = QColor(0, 0, 0);
    maxCoverageColour = QColor(255, 20, 20);
    noBlastHitsColour = QColor(240, 240, 240);
}


//The negative node colour is a bit darker than the positive colour.
//It is only used when double graphs are drawn.
//The number in the darker function controls how much darker this
//colour is.
void Settings::setNegativeNodeColour()
{
    negativeNodeColour = positiveNodeColour.darker(120);
}



double Settings::widthScale(double zoom)
{
    return 1.0 / ((zoom + 7.0) / 8.0);
}
