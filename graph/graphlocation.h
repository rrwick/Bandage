//Copyright 2016 Ryan Wick

//This file is part of Bandage.

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


#ifndef GRAPHLOCATION_H
#define GRAPHLOCATION_H


class DeBruijnNode;

class GraphLocation
{
public:
    //CREATORS
    GraphLocation();
    GraphLocation(DeBruijnNode * node, int position);
    static GraphLocation startOfNode(DeBruijnNode * node);
    static GraphLocation endOfNode(DeBruijnNode * node);
    static GraphLocation null();

    //ACCESSORS
    DeBruijnNode * getNode() const {return m_node;}
    int getPosition() const {return m_position;}
    bool isValid() const;
    bool isNull() const;
    GraphLocation reverseComplementLocation() const;
    char getBase() const;
    bool isAtStartOfNode() const;
    bool isAtEndOfNode() const;
    bool operator==(GraphLocation const &other) const;

    //MODIFERS
    void moveLocation(int change);


private:
    DeBruijnNode * m_node;
    int m_position;
    void moveForward(int change);
    void moveBackward(int change);
};

#endif // GRAPHLOCATION_H
