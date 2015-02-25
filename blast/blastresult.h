#ifndef BLASTRESULT_H
#define BLASTRESULT_H

class DeBruijnNode;
class BlastTarget;

class BlastResult
{
public:
    BlastResult(DeBruijnNode * node, BlastTarget * target,
                int nodeStart, int nodeEnd,
                int targetStart, int targetEnd);
    BlastResult();
    ~BlastResult();

    DeBruijnNode * m_node;
    BlastTarget * m_target;

    int m_nodeStart;
    int m_nodeEnd;
    int m_targetStart;
    int m_targetEnd;

    double m_nodeStartFraction;
    double m_nodeEndFraction;
    double m_targetStartFraction;
    double m_targetEndFraction;
};

#endif // BLASTRESULT_H
