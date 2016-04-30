//Copyright 2016 Ryan Wick

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


#ifndef OGDFNODE_H
#define OGDFNODE_H

#include <vector>
#include "../ogdf/basic/Graph.h"
#include <QPainterPath>
#include "../ogdf/basic/GraphAttributes.h"
#include <QPointF>

class OgdfNode
{
public:
    OgdfNode() {}

    std::vector<ogdf::node> m_ogdfNodes;

    void addOgdfNode(ogdf::node newNode) {m_ogdfNodes.push_back(newNode);}
    ogdf::node getFirst() {if (m_ogdfNodes.size() == 0) return 0; else return m_ogdfNodes[0];}
    ogdf::node getSecond() {if (m_ogdfNodes.size() < 2) return 0; else return m_ogdfNodes[1];}
    ogdf::node getLast() {if (m_ogdfNodes.size() == 0) return 0; else return m_ogdfNodes[m_ogdfNodes.size()-1];}
    ogdf::node getSecondLast() {if (m_ogdfNodes.size() < 2) return 0; else return m_ogdfNodes[m_ogdfNodes.size()-2];}
};

#endif // OGDFNODE_H
