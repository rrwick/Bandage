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

#include <vector>
#include <QString>

class Settings;
class MyGraphicsView;
class BlastSearch;
class AssemblyGraph;

enum NodeColourScheme {ONE_COLOUR, RANDOM_COLOURS, COVERAGE_COLOUR, BLAST_HITS_COLOUR, CONTIGUITY_COLOUR, CUSTOM_COLOURS};
enum GraphScope {WHOLE_GRAPH, AROUND_NODE, AROUND_BLAST_HITS};
enum ContiguityStatus {STARTING, CONTIGUOUS_STRAND_SPECIFIC, CONTIGUOUS_EITHER_STRAND, MAYBE_CONTIGUOUS, NOT_CONTIGUOUS};
enum NodeDragging {ONE_PIECE, NEARBY_PIECES, ALL_PIECES};
enum ZoomSource {MOUSE_WHEEL, SPIN_BOX, KEYBOARD};
enum UiState {NO_GRAPH_LOADED, GRAPH_LOADED, GRAPH_DRAWN};
enum NodeLengthMode {AUTO_NODE_LENGTH, MANUAL_NODE_LENGTH};
enum GraphFileType {LAST_GRAPH, FASTG, GFA, TRINITY};

extern Settings * g_settings;
extern MyGraphicsView * g_graphicsView;
extern double g_absoluteZoom;
extern BlastSearch * g_blastSearch;
extern QString g_tempDirectory;
extern AssemblyGraph * g_assemblyGraph;

QString formatIntForDisplay(int num);
QString formatIntForDisplay(long long num);
QString formatDoubleForDisplay(double num, double decimalPlacesToDisplay);

long long getFullTrinityNodeNumberFromParts(int transcript, int component, long long node);
void getTrinityPartsFromFullNodeNumber(long long fullNodeNumber, int * transcript, int * component, long long * node);
long long getFullTrinityNodeNumberFromName(QString name);
QString getTrinityNodeNameFromFullNodeNumber(long long fullNodeNumber);


void emptyTempDirectory();

void readFastaFile(QString filename, std::vector<QString> * names, std::vector<QString> * sequences);

#endif // GLOBALS_H
