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


#ifndef SETTINGS_H
#define SETTINGS_H

#include <QColor>
#include <QFont>
#include "globals.h"
#include <QString>
#include "../graph/path.h"
#include "scinot.h"

class DeBruijnNode;



class IntSetting
{
public:
    IntSetting(int v, int minimum, int maximum, bool isOn = true) {val = v; min = minimum; max = maximum; on = isOn;}
    IntSetting() {val = 0; min = 0; max = 0;}
    int val, min, max;
    bool on;
    operator int() const {return val;}
    IntSetting & operator=(int newVal) {val = newVal; return *this;}
    bool inRange(int v) const {return v >= min && v <= max;}
};

class FloatSetting
{
public:
    FloatSetting(double v, double minimum, double maximum, bool isOn = true) {val = v; min = minimum; max = maximum; on = isOn;}
    FloatSetting() {val = 0.0; min = 0.0; max = 0.0;}
    double val, min, max;
    bool on;
    operator double() const {return val;}
    FloatSetting & operator=(double newVal) {val = newVal; return *this;}
    bool inRange(double v) const {return v >= min && v <= max;}
};

class SciNotSetting
{
public:
    SciNotSetting(SciNot v, SciNot minimum, SciNot maximum, bool isOn = true) {val = v; min = minimum; max = maximum; on = isOn;}
    SciNotSetting() {val = 0.0; min = 0.0; max = 0.0;}
    SciNot val, min, max;
    bool on;
    operator SciNot() const {return val;}
    SciNotSetting & operator=(SciNot newVal) {val = newVal; return *this;}
    bool inRange(SciNot v) const {return v >= min && v <= max;}
};


class Settings
{
public:
    Settings();

    bool doubleMode;

    NodeLengthMode nodeLengthMode;
    double autoNodeLengthPerMegabase;
    FloatSetting manualNodeLengthPerMegabase;
    double meanNodeLength;
    double minTotalGraphLength;
    IntSetting graphLayoutQuality;
    bool linearLayout;
    FloatSetting minimumNodeLength;
    FloatSetting edgeLength;
    FloatSetting doubleModeNodeSeparation;
    FloatSetting nodeSegmentLength;
    FloatSetting componentSeparation;

    FloatSetting averageNodeWidth;
    FloatSetting averageFeatureNodeWidth;
    FloatSetting depthEffectOnWidth;
    FloatSetting depthPower;

    FloatSetting edgeWidth;
    FloatSetting outlineThickness;
    double selectionThickness;
    double arrowheadSize;
    double arrowheadsInSingleMode;
    FloatSetting textOutlineThickness;

    int blastRainbowPartsPerQuery;

    GraphScope graphScope;
    IntSetting nodeDistance;
    bool startingNodesExactMatch;
    QString startingNodes;
    QString blastQueryFilename;
    QString unnamedQueryDefaultName;

    double minZoom;
    double minZoomOnGraphDraw;
    double maxZoom;
    double maxAutomaticZoom;
    double zoomFactor;
    double textZoomScaleFactor; // 1.0 = text size constant, regardless of zoom.  0.0 = text size changes exactly with zoom.

    double dragStrength;

    bool displayNodeCustomLabels;
    bool displayNodeNames;
    bool displayNodeLengths;
    bool displayNodeDepth;
    bool displayNodeCsvData;
    int  displayNodeCsvDataCol;
    bool displayBlastHits;
    bool displayTaxIdName;
    bool displayTaxIdRank;
    bool displayTaxNameRank;
    QFont labelFont;
    bool textOutline;
    bool antialiasing;
    bool positionTextNodeCentre;

    bool displayFeatureIdLabels;
    bool displayFeatureClassLabels;
    bool displayFeatureCustomLabels;
    bool displayFeatureClassLikeFigure;

    NodeDragging nodeDragging;

    QColor edgeColour;
    QColor outlineColour;
    QColor selectionColour;
    QColor textColour;
    QColor textOutlineColour;

    NodeColourScheme nodeColourScheme;
    NodeColourScheme featureColourScheme;
    QColor uniformPositiveNodeColour;
    QColor uniformNegativeNodeColour;
    QColor uniformNodeSpecialColour;

    IntSetting randomColourPositiveOpacity;
    IntSetting randomColourNegativeOpacity;
    IntSetting randomColourPositiveSaturation;
    IntSetting randomColourNegativeSaturation;
    IntSetting randomColourPositiveLightness;
    IntSetting randomColourNegativeLightness;

    IntSetting contiguitySearchSteps;
    QColor contiguousStrandSpecificColour;
    QColor contiguousEitherStrandColour;
    QColor maybeContiguousColour;
    QColor notContiguousColour;
    QColor contiguityStartingColour;

    QColor noBlastHitsColour;

    bool autoDepthValue;
    FloatSetting lowDepthValue;
    QColor lowDepthColour;
    FloatSetting highDepthValue;
    QColor highDepthColour;

    QColor pathHighlightShadingColour;
    QColor pathHighlightOutlineColour;

    QColor defaultCustomNodeColour;

    //These specify the range of overlaps to look for when Bandage determines
    //edge overlaps automatically.
    int minAutoFindEdgeOverlap;
    int maxAutoFindEdgeOverlap;

    //These control how Bandage finds the best path in the graph to represent
    //each BLAST query.  Some of these settings can be turned on and off and
    //therefore have a corresponding bool value.
    IntSetting maxHitsForQueryPath;
    IntSetting maxQueryPathNodes;
    FloatSetting minQueryCoveredByPath;
    FloatSetting minQueryCoveredByHits;
    FloatSetting minMeanHitIdentity;
    SciNotSetting maxEValueProduct;
    FloatSetting minLengthPercentage;
    FloatSetting maxLengthPercentage;
    IntSetting minLengthBaseDiscrepancy;
    IntSetting maxLengthBaseDiscrepancy;

    //This holds the BLAST search parameters that a user can change before
    //running a BLAST search.
    QString blastSearchParameters;

    //These are the optional BLAST hit filters: whether or not they are used and
    //what their values are.
    IntSetting blastAlignmentLengthFilter;
    FloatSetting blastQueryCoverageFilter;
    FloatSetting blastIdentityFilter;
    SciNotSetting blastEValueFilter;
    FloatSetting blastBitScoreFilter;

    //These are used for the 'Depth range' graph scope.
    FloatSetting minDepthRange;
    FloatSetting maxDepthRange;

    FloatSetting hicEdgeLength;
    FloatSetting hicEdgeWidth;

    HiCInclusionFilter hicInclusionFilter = ONE_FROM_TARGET_COMPONENT;
    HiCDrawingType hicDrawingType = ALL_EDGES;

    bool isAutoParameters = false;
    int taxRank;
    int taxId;
    bool makeZip = false;
    bool wasZipped = false;
    bool propagateTaxColour = false;
    int taxDistance;
    int displayAroundTaxWithHiC;
    bool wasCalcHiCLinkForTax = false;
    bool aroundTargetNodes = false;
    bool onlyBigComponent = false;
    bool wasComponentsFound = false;
    int m_clock = -1;
    bool addNewNodes = false;
    bool roundMode = true;

    int featureForestEdgeLength = 10;
};

#endif // SETTINGS_H
