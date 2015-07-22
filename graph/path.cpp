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


#include "path.h"

Path::Path()
{

}


//These constructors will try to produce a path using the given nodes.
//It will only succeed if the nodes produce one and only one path.
//If they are disconnected, branching or ambiguous, it will fail
//and only contruct an empty Path.
Path::Path(QList<DeBruijnNode *> nodes, bool strandSpecific)
{
    buildUnambiguousPathFromNodes(nodes, strandSpecific);
}
Path::Path(std::vector<DeBruijnNode *> nodes, bool strandSpecific)
{
    QList<DeBruijnNode *> nodesList;
    for (size_t i = 0; i < nodes.size(); ++i)
        nodesList.push_back(nodes[i]);
    buildUnambiguousPathFromNodes(nodesList, strandSpecific);
}


//This constructor builds a path using an ordered list nodes. The path
//does not have to be unambiguous. If the nodes do not make a valid path,
//then it will fail and create an empty path.
Path::Path(QList<DeBruijnNode *> nodes)
{



}


void Path::buildUnambiguousPathFromNodes(QList<DeBruijnNode *> nodes, bool strandSpecific)
{
    if (nodes.isEmpty())
        return;

    //Loop through the nodes, trying to add them to the
    //Path.  If node can be added, then we fail and make
    //an empty Path.  If one can be added, we quit the
    //loop and try again with the remaining nodes.
    while (nodes.size() > 0)
    {
        bool addSuccess = false;
        for (int i = 0; i < nodes.size(); ++i)
        {
            addSuccess = addNode(nodes.at(i), strandSpecific);
            if (addSuccess)
            {
                nodes.removeAt(i);
                break;
            }
        }

        if (!addSuccess)
        {
            m_nodes.clear();
            m_edges.clear();
            return;
        }
    }

    //If the nodes in the path contain other edges which connect
    //them to each other, then the path is ambiguous and we fail.
    if (checkForOtherEdges())
    {
        m_nodes.clear();
        m_edges.clear();
    }
}


//This function will try to add a node to the path on either end.
//It will only succeed (and return true) if there is a single way
//to add the node on one of the path's ends.
//It can, however, add a node that connects the end to both ends,
//making a circular Path.
bool Path::addNode(DeBruijnNode * newNode, bool strandSpecific)
{
    //If the Path is empty, then this function always succeeds.
    if (m_nodes.isEmpty())
    {
        m_nodes.push_back(newNode);
        return true;
    }

    //If the Path is circular, then this function fails, as there
    //is no way to add a node to a circular sequence without making
    //it ambiguous.
    if (isCircular())
        return false;

    //Check to see if the node can be added anywhere in the middle
    //of the Path.  If so, this function fails.
    for (int i = 1; i < m_nodes.size() - 1; ++i)
    {
        DeBruijnNode * middleNode = m_nodes.at(i);
        if (middleNode->isNodeConnected(newNode))
            return false;
    }

    DeBruijnNode * firstNode = m_nodes.front();
    DeBruijnNode * lastNode = m_nodes.back();

    DeBruijnEdge * edgeIntoFirst = firstNode->doesNodeLeadIn(newNode);
    DeBruijnEdge * edgeAwayFromLast = lastNode->doesNodeLeadAway(newNode);

    //If not strand-specific, then we also check to see if the reverse
    //complement of the new node can be added.
    DeBruijnEdge * revCompEdgeIntoFirst = 0;
    DeBruijnEdge * revCompEdgeAwayFromLast = 0;
    if (!strandSpecific)
    {
        revCompEdgeIntoFirst = firstNode->doesNodeLeadIn(newNode->m_reverseComplement);
        revCompEdgeAwayFromLast = lastNode->doesNodeLeadAway(newNode->m_reverseComplement);
    }

    //To be successful, either:
    // 1) exactly one of the four edge pointers should be non-null.  This indicates
    //    the node extends a linear path.
    // 2) there is both an edge away from the last and an edge into the first.  This
    //    indicates that the node completes a circular Path.
    if (edgeIntoFirst == 0 && edgeAwayFromLast == 0 && revCompEdgeIntoFirst == 0 && revCompEdgeAwayFromLast == 0)
        return false;

    if (edgeIntoFirst != 0 && edgeAwayFromLast == 0 && revCompEdgeIntoFirst == 0 && revCompEdgeAwayFromLast == 0)
    {
        m_nodes.push_front(newNode);
        m_edges.push_front(edgeIntoFirst);
        return true;
    }

    if (edgeIntoFirst == 0 && edgeAwayFromLast != 0 && revCompEdgeIntoFirst == 0 && revCompEdgeAwayFromLast == 0)
    {
        m_nodes.push_back(newNode);
        m_edges.push_back(edgeAwayFromLast);
        return true;
    }

    if (edgeIntoFirst == 0 && edgeAwayFromLast == 0 && revCompEdgeIntoFirst != 0 && revCompEdgeAwayFromLast == 0)
    {
        m_nodes.push_front(newNode->m_reverseComplement);
        m_edges.push_front(revCompEdgeIntoFirst);
        return true;
    }

    if (edgeIntoFirst == 0 && edgeAwayFromLast == 0 && revCompEdgeIntoFirst == 0 && revCompEdgeAwayFromLast != 0)
    {
        m_nodes.push_back(newNode->m_reverseComplement);
        m_edges.push_back(revCompEdgeAwayFromLast);
        return true;
    }

    if (edgeIntoFirst != 0 && edgeAwayFromLast != 0 && revCompEdgeIntoFirst == 0 && revCompEdgeAwayFromLast == 0)
    {
        m_edges.push_back(edgeAwayFromLast);
        m_nodes.push_back(newNode);
        m_edges.push_back(edgeIntoFirst);
        return true;
    }

    if (edgeIntoFirst == 0 && edgeAwayFromLast == 0 && revCompEdgeIntoFirst != 0 && revCompEdgeAwayFromLast != 0)
    {
        m_edges.push_back(revCompEdgeAwayFromLast);
        m_nodes.push_back(newNode->m_reverseComplement);
        m_edges.push_back(revCompEdgeIntoFirst);
        return true;
    }

    //If the code got here, then there are multiple ways of adding the node, so we fail.
    return false;
}


//This function looks to see if there are other edges connecting path nodes
//that aren't in the list of path edges.  If so, it returns true.
//This is used to check whether a Path is ambiguous or node.
bool Path::checkForOtherEdges()
{
    //First compile a list of all edges which connect any
    //node in the Path to any other node in the Path.
    QList<DeBruijnEdge *> allConnectingEdges;
    for (int i = 0; i < m_nodes.size(); ++i)
    {
        DeBruijnNode * startingNode = m_nodes[i];
        for (int j = 0; j < m_nodes.size(); ++j)
        {
            DeBruijnNode * endingNode = m_nodes[j];
            for (size_t k = 0; k < startingNode->m_edges.size(); ++k)
            {
                DeBruijnEdge * edge = startingNode->m_edges[k];
                if (edge->m_startingNode == startingNode && edge->m_endingNode == endingNode)
                    allConnectingEdges.push_back(edge);
            }
        }
    }

    //If this list of edges is greater than the number of edges in the path, then
    //other edges exist.
    return allConnectingEdges.size() > m_edges.size();
}




//This function extracts the sequence for the whole path.  It uses the overlap
//value in the edges to remove sequences that are duplicated at the end of one
//node and the start of the next.
QByteArray Path::getPathSequence()
{
    if (m_nodes.empty())
        return "";

    QByteArray sequence;
    QByteArray firstNodeSequence = m_nodes[0]->m_sequence;

    //If the path is circular, we trim the overlap from
    //the first node so it is flush with the end.
    //If the path is linear, we include the whole first node.
    if (isCircular())
    {
        int overlap = m_edges.back()->m_overlap;
        int rightChars = firstNodeSequence.length() - overlap;
        if (rightChars > 0)
            sequence += firstNodeSequence.right(rightChars);
    }
    else
        sequence += firstNodeSequence;

    for (int i = 1; i < m_nodes.size(); ++i)
    {
        int overlap = m_edges[i-1]->m_overlap;
        QByteArray nodeSequence = m_nodes[i]->m_sequence;
        int rightChars = nodeSequence.length() - overlap;
        if (rightChars > 0)
            sequence += nodeSequence.right(rightChars);
    }

    return sequence;
}


QString Path::getFasta()
{
    //The description line is a comma-delimited list of the nodes in the path
    QString fasta = ">";
    for (int i = 0; i < m_nodes.size(); ++i)
    {
        fasta += m_nodes[i]->m_name;
        if (i < m_nodes.size() - 1)
            fasta += ", ";
    }
    if (isCircular())
        fasta += " (circular)";
    fasta += "\n";

    QString pathSequence = getPathSequence();
    int charactersOnLine = 0;
    for (int i = 0; i < pathSequence.length(); ++i)
    {
        fasta += pathSequence.at(i);
        ++charactersOnLine;
        if (charactersOnLine >= 70)
        {
            fasta += "\n";
            charactersOnLine = 0;
        }
    }
    fasta += "\n";

    return fasta;
}



//This function tests whether the last node in the path leads into the first.
bool Path::isCircular()
{
    if (isEmpty())
        return false;
    if (m_edges.empty())
        return false;

    //A circular path should have the same number of nodes and edges.
    if (m_nodes.size() != m_edges.size())
        return false;

    DeBruijnEdge * lastEdge = m_edges.back();
    DeBruijnNode * firstNode = m_nodes.front();
    DeBruijnNode * lastNode = m_nodes.back();

    return (lastEdge->m_startingNode == lastNode && lastEdge->m_endingNode == firstNode);
}
