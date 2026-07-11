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


#include "treelayoutworker.h"
#include <time.h>
#include "ogdf/basic/geometry.h"
#include <QLineF>
#include "../program/settings.h"
#include "../program/globals.h"

using namespace ogdf;

TreeLayoutWorker::TreeLayoutWorker(ogdf::TreeLayout* treeLayout, ogdf::GraphAttributes* graphAttributes,
    ogdf::EdgeArray<double>* edgeArray) :

    m_treeLayout(treeLayout), m_graphAttributes(graphAttributes), m_edgeArray(edgeArray)
{
}

void TreeLayoutWorker::layoutGraph()
{
    m_treeLayout->orientation(ogdf::Orientation::topToBottom);
    m_treeLayout->subtreeDistance(50.0);
    m_treeLayout->siblingDistance(50.0);
    m_treeLayout->callMultiLine(*m_graphAttributes);

    emit finishedLayout();
}

