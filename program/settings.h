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
#include "scinot.h"

class DeBruijnNode;

class Settings
{
public:
    Settings();

    bool doubleMode;

    NodeLengthMode nodeLengthMode;
    double autoNodeLengthPerMegabase;
    double manualNodeLengthPerMegabase;
    double meanNodeLength;
    int graphLayoutQuality;
    double minimumNodeLength;
    double edgeLength;
    double doubleModeNodeSeparation;
    double nodeSegmentLength;

    double averageNodeWidth;
    double readDepthEffectOnWidth;
    double readDepthPower;

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
    double minZoomOnGraphDraw;
    double maxZoom;
    double zoomFactor;
    double textZoomScaleFactor; // 1.0 = text size constant, regardless of zoom.  0.0 = text size changes exactly with zoom.

    double dragStrength;

    bool displayNodeCustomLabels;
    bool displayNodeNames;
    bool displayNodeLengths;
    bool displayNodeReadDepth;
    bool displayNodeCsvData;
    int  displayNodeCsvDataCol;
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

    bool autoReadDepthValue;
    double lowReadDepthValue;
    QColor lowReadDepthColour;
    double highReadDepthValue;
    QColor highReadDepthColour;

    QColor pathHighlightShadingColour;
    QColor pathHighlightOutlineColour;

    //These specify the range of overlaps to look for when Bandage determines
    //edge overlaps automatically.
    int minAutoFindEdgeOverlap;
    int maxAutoFindEdgeOverlap;

    //These control how Bandage finds the best path in the graph to represent
    //each BLAST query.  Some of these settings can be turned on and off and
    //therefore have a corresponding bool value.
    int maxQueryPathNodes;
    double minQueryCoveredByPath;
    bool minQueryCoveredByHitsOn;
    double minQueryCoveredByHits;
    bool minMeanHitIdentityOn;
    double minMeanHitIdentity;
    bool maxEValueProductOn;
    SciNot maxEValueProduct;
    bool minLengthPercentageOn;
    double minLengthPercentage;
    bool maxLengthPercentageOn;
    double maxLengthPercentage;
    bool minLengthBaseDiscrepancyOn;
    int minLengthBaseDiscrepancy;
    bool maxLengthBaseDiscrepancyOn;
    int maxLengthBaseDiscrepancy;

    //This holds the BLAST search parameters that a user can change before
    //running a BLAST search.
    QString blastSearchParameters;

    //These are the optional BLAST hit filters: whether or not they are used and
    //what their values are.
    bool blastAlignmentLengthFilterOn;
    int blastAlignmentLengthFilterValue;
    bool blastQueryCoverageFilterOn;
    double blastQueryCoverageFilterValue;
    bool blastIdentityFilterOn;
    double blastIdentityFilterValue;
    bool blastEValueFilterOn;
    SciNot blastEValueFilterValue;
    bool blastBitScoreFilterOn;
    double blastBitScoreFilterValue;

    //These are used for the 'Read depth range' graph scope.
    double minReadDepthRange;
    double maxReadDepthRange;
};

#endif // SETTINGS_H
