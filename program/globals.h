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


#ifndef GLOBALS_H
#define GLOBALS_H

#include <QString>

class Settings;
class MyGraphicsView;
class BlastSearchResults;

enum NodeColourScheme {ONE_COLOUR, RANDOM_COLOURS, COVERAGE_COLOUR, CONTIGUITY_COLOUR, CUSTOM_COLOURS};
enum GraphScope {WHOLE_GRAPH, AROUND_NODE};
enum ContiguityStatus {STARTING, CONTIGUOUS, CONTIGUOUS_UNTIL_BRANCHING, MAYBE_CONTIGUOUS, NOT_CONTIGUOUS};
enum NodeDragging {ONE_PIECE, NEARBY_PIECES, ALL_PIECES};
enum ZoomSource {MOUSE_WHEEL, SPIN_BOX, KEYBOARD};

extern Settings * g_settings;
extern MyGraphicsView * g_graphicsView;
extern double g_absoluteZoom;
extern int g_randomColourFactor;
extern BlastSearchResults * g_blastSearchResults;

QString formatIntForDisplay(int num);
QString formatIntForDisplay(long long num);
QString formatDoubleForDisplay(double num, double decimalPlacesToDisplay);


#endif // GLOBALS_H
