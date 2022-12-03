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


#ifndef GRAPHLAYOUTWORKER_H
#define GRAPHLAYOUTWORKER_H

#include <QObject>
#include "../ogdf/energybased/FMMMLayout.h"
#include "../ogdf/basic/GraphAttributes.h"



class GraphLayoutWorker : public QObject
{
    Q_OBJECT

public:
    GraphLayoutWorker(ogdf::FMMMLayout * fmmm, ogdf::GraphAttributes * graphAttributes,
        ogdf::EdgeArray<double> * edgeArray, int graphLayoutQuality, bool linearLayout,
                      double graphLayoutComponentSeparation, double aspectRatio = 1.333333);
    GraphLayoutWorker(ogdf::FMMMLayout* fmmm, ogdf::GraphAttributes* graphAttributes,
        ogdf::EdgeArray<double>* edgeArray, int graphLayoutQuality, bool linearLayout,
        double graphLayoutComponentSeparation, int randSeed, double aspectRatio = 1.333333);

    ogdf::FMMMLayout * m_fmmm;
    ogdf::GraphAttributes * m_graphAttributes;
    ogdf::EdgeArray<double> * m_edgeArray;
    int m_graphLayoutQuality;
    bool m_linearLayout;
    double m_graphLayoutComponentSeparation;
    double m_aspectRatio;
    int m_randSeed;

public slots:
    void layoutGraph();

signals:
    void finishedLayout();
};

#endif // GRAPHLAYOUTWORKER_H
