#include "blastresult.h"

#include "../graph/debruijnnode.h"
#include "blasttarget.h"

BlastResult::BlastResult(DeBruijnNode * node, BlastTarget * target,
                         int nodeStart, int nodeEnd,
                         int targetStart, int targetEnd) :
    m_node(node), m_target(target), m_nodeStart(nodeStart),
    m_nodeEnd(nodeEnd), m_targetStart(targetStart), m_targetEnd(targetEnd)
{
    m_nodeStartFraction = double(nodeStart) / m_node->m_length;
    m_nodeEndFraction = double(nodeEnd) / m_node->m_length;
    m_targetStartFraction = double(targetStart) / m_target->m_length;
    m_targetEndFraction = double(targetEnd) / m_target->m_length;
}

BlastResult::~BlastResult()
{

}

