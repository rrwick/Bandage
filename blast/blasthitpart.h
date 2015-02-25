#ifndef BLASTHITPART_H
#define BLASTHITPART_H

#include <QColor>

class BlastHitPart
{
public:
    BlastHitPart() {}
    BlastHitPart(QColor colour, double nodeFractionStart, double nodeFractionEnd) :
        m_colour(colour), m_nodeFractionStart(nodeFractionStart), m_nodeFractionEnd(nodeFractionEnd) {}
    ~BlastHitPart() {}

    QColor m_colour;
    double m_nodeFractionStart;
    double m_nodeFractionEnd;
};

#endif // BLASTHITPART_H
