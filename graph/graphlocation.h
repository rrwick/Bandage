#ifndef GRAPHLOCATION_H
#define GRAPHLOCATION_H


class DeBruijnNode;

class GraphLocation
{
public:
    //CREATORS
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

    //MODIFERS
    void moveLocation(int change);


private:
    DeBruijnNode * m_node;
    int m_position;
};

#endif // GRAPHLOCATION_H
