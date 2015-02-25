#ifndef BLASTRESULT_H
#define BLASTRESULT_H

class DeBruijnNode;
class BlastTarget;

#include <QString>
#include "blasthitpart.h"

class BlastResult
{
public:
    BlastResult(DeBruijnNode * node, int nodeStart, int nodeEnd,
                BlastTarget * target, int targetStart, int targetEnd);
    BlastResult();
    ~BlastResult();

    DeBruijnNode * m_node;
    int m_nodeStart;
    int m_nodeEnd;

    BlastTarget * m_target;
    int m_targetStart;
    int m_targetEnd;

    double m_nodeStartFraction;
    double m_nodeEndFraction;
    double m_targetStartFraction;
    double m_targetEndFraction;

    std::vector<BlastHitPart> getBlastHitParts(bool reverse);
};

#endif // BLASTRESULT_H
