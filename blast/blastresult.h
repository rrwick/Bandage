#ifndef BLASTRESULT_H
#define BLASTRESULT_H

class DeBruijnNode;
class BlastTarget;

class BlastResult
{
public:
    BlastResult(DeBruijnNode * node, BlastTarget * blastTarget,
                double nodeStart, double nodeEnd,
                double targetStart, double targetEnd);
    BlastResult();
    ~BlastResult();

    DeBruijnNode * m_node;
    BlastTarget * m_blastTarget;

    double m_nodeStart;
    double m_nodeEnd;
    double m_targetStart;
    double m_targetEnd;
};

#endif // BLASTRESULT_H
