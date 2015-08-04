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


#ifndef SETTINGS_H
#define SETTINGS_H

#include <QColor>
#include <QFont>
#include "globals.h"
#include <QString>
#include "../graph/path.h"

class DeBruijnNode;

class Settings
{
public:
    Settings();

    bool doubleMode;

    NodeLengthMode nodeLengthMode;
    int autoBasePairsPerSegment;
    int manualBasePairsPerSegment;
    int meanSegmentsPerNode;
    double segmentLength;
    int graphLayoutQuality;

    double averageNodeWidth;
    double coverageEffectOnWidth;
    double coveragePower;

    double edgeWidth;
    double outlineThickness;
    double selectionThickness;
    double arrowheadSize;
    double textOutlineThickness;

    int blastRainbowPartsPerQuery;

    GraphScope graphScope;
    int nodeDistance;
    bool startingNodesExactMatch;
    QString startingNodes;
    QString blastQueryFilename;
    QString unnamedQueryDefaultName;

    double minZoom;
    double maxZoom;
    double zoomFactor;

    double dragStrength;

    bool displayNodeCustomLabels;
    bool displayNodeNames;
    bool displayNodeLengths;
    bool displayNodeCoverages;
    bool displayBlastHits;
    QFont labelFont;
    bool textOutline;
    bool antialiasing;
    bool positionTextNodeCentre;

    NodeDragging nodeDragging;

    QColor edgeColour;
    QColor outlineColour;
    QColor selectionColour;
    QColor textColour;
    QColor textOutlineColour;

    NodeColourScheme nodeColourScheme;
    QColor uniformPositiveNodeColour;
    QColor uniformNegativeNodeColour;
    QColor uniformNodeSpecialColour;

    int randomColourPositiveOpacity;
    int randomColourNegativeOpacity;
    int randomColourPositiveSaturation;
    int randomColourNegativeSaturation;
    int randomColourPositiveLightness;
    int randomColourNegativeLightness;

    int contiguitySearchSteps;
    QColor contiguousStrandSpecificColour;
    QColor contiguousEitherStrandColour;
    QColor maybeContiguousColour;
    QColor notContiguousColour;
    QColor contiguityStartingColour;

    QColor noBlastHitsColour;

    bool autoCoverageValue;
    double lowCoverageValue;
    QColor lowCoverageColour;
    double highCoverageValue;
    QColor highCoverageColour;

    QColor pathHighlightShadingColour;
    QColor pathHighlightOutlineColour;

    //These specify the range of overlaps to look for when Bandage determines
    //edge overlaps automatically.
    int minAutoFindEdgeOverlap;
    int maxAutoFindEdgeOverlap;

    //These control how Bandage finds the best path in the graph to represent
    //each BLAST query.
    double queryRequiredCoverage;
    double queryAllowedLengthDiscrepancy;
    int maxQueryPathNodes;
    int maxQueryPaths;

    //This holds the BLAST search parameters that a user can change before
    //running a BLAST search.
    QString blastSearchParameters;

    int distancePathSearchDepth;
    int minDistancePathLength;
    int maxDistancePathLength;
};

#endif // SETTINGS_H
