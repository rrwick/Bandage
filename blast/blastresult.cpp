#include "blastresult.h"

#include "../graph/debruijnnode.h"
#include "blasttarget.h"

BlastResult::BlastResult(DeBruijnNode * node, BlastTarget * blastTarget,
                         double nodeStart, double nodeEnd,
                         double targetStart, double targetEnd) :
    m_node(node), m_blastTarget(blastTarget), m_nodeStart(nodeStart),
    m_nodeEnd(nodeEnd), m_targetStart(targetStart), m_targetEnd(targetEnd)
{

}

BlastResult::~BlastResult()
{

}

