#ifndef BLASTDOT_H
#define BLASTDOT_H

#include <QColor>

class BlastDot
{
public:
    BlastDot() {}
    BlastDot(QColor colour, double nodeFraction) : m_colour(colour), m_nodeFraction(nodeFraction) {}
    ~BlastDot() {}

    QColor m_colour;
    double m_nodeFraction;
};

#endif // BLASTDOT_H
