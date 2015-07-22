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
}


//This function will try to add a node to the path on either end.
//It will only succeed (and return true) if there is a single way
//to add the node, which is on one of the path's ends.  If there
//are multiple ways to add the node, it fails and returns false.
bool Path::addNode(DeBruijnNode * newNode, bool strandSpecific)
{
    //If the Path is empty, then this function always succeeds.
    if (m_nodes.isEmpty())
    {
        m_nodes.push_back(newNode);
        return true;
    }

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

    //To be successful, exactly one of the four edge pointers should be non-null.
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

    //If the code got here, then there are multiple ways of adding the node, so we fail.
    return false;
}




//This function extracts the sequence for the whole path.  It uses the overlap
//value in the edges to remove sequences that are duplicated at the end of one
//node and the start of the next.
QByteArray Path::getPathSequence()
{
    if (m_nodes.empty())
        return "";

    QByteArray sequence = m_nodes[0]->m_sequence;
    for (size_t i = 1; i < m_nodes.size(); ++i)
    {
        int overlap = m_edges[i-1]->m_overlap;
        QByteArray nextSequence = m_nodes[i]->m_sequence;
        int rightChars = nextSequence.length() - overlap;
        if (rightChars > 0)
            sequence += nextSequence.right(rightChars);
    }

    return sequence;
}


QString Path::getFasta()
{
    //The description line is a comma-delimited list of the nodes in the path
    QString fasta = ">";
    for (size_t i = 0; i < m_nodes.size(); ++i)
    {
        fasta += m_nodes[i]->m_name;
        if (i < m_nodes.size() - 1)
            fasta += ", ";
    }
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
