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

class Settings
{
public:
    Settings();

    bool doubleMode;

    int basePairsPerSegment;
    double segmentLength;
    int graphLayoutQuality;

    double averageNodeWidth;
    double coverageEffectOnWidth;
    double edgeWidth;
    double outlineThickness;
    double selectionThickness;
    double arrowheadSize;
    double textOutlineThickness;

    int blastPartsPerQuery;

    GraphScope graphScope;

    double minZoom;
    double maxZoom;
    double zoomFactor;

    double dragStrength;

    bool displayNodeCustomLabels;
    bool displayNodeNumbers;
    bool displayNodeLengths;
    bool displayNodeCoverages;
    QFont labelFont;
    bool textOutline;
    bool antialiasing;

    NodeDragging nodeDragging;

    QColor edgeColour;
    QColor outlineColour;
    QColor selectionColour;
    QColor textColour;

    NodeColourScheme nodeColourScheme;
    QColor uniformPositiveNodeColour;
    QColor uniformNegativeNodeColour;
    QColor uniformNodeSpecialColour;

    int randomColourOpacity;

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

    QString blastSearchParameters;
    int blastTimeoutSeconds;

    QString rememberedPath;

    void setNegativeNodeColour();
    bool anyNodeDisplayText() {return displayNodeCustomLabels || displayNodeNumbers || displayNodeLengths || displayNodeCoverages;}
};

#endif // SETTINGS_H
