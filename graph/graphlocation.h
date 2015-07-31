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
    bool areIdentical(GraphLocation other) const;

    //MODIFERS
    void moveLocation(int change);


private:
    DeBruijnNode * m_node;
    int m_position;
    void moveForward(int change);
    void moveBackward(int change);
};

#endif // GRAPHLOCATION_H
