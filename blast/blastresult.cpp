#include "blastresult.h"

#include "../graph/debruijnnode.h"
#include "blasttarget.h"
#include "../program/settings.h"
#include "../program/globals.h"
#include <math.h>

BlastResult::BlastResult(DeBruijnNode * node, int nodeStart, int nodeEnd,
                         BlastTarget * target, int targetStart, int targetEnd) :
    m_node(node), m_nodeStart(nodeStart), m_nodeEnd(nodeEnd),
    m_target(target), m_targetStart(targetStart), m_targetEnd(targetEnd)
{
    m_nodeStartFraction = double(nodeStart) / m_node->m_length;
    m_nodeEndFraction = double(nodeEnd) / m_node->m_length;
    m_targetStartFraction = double(targetStart) / m_target->m_length;
    m_targetEndFraction = double(targetEnd) / m_target->m_length;
}

BlastResult::~BlastResult()
{

}


std::vector<BlastDot> BlastResult::getBlastDots(bool reverse)
{
    std::vector<BlastDot> returnVector;

    int dotCount = g_settings->blastDotsPerTarget * fabs(m_targetStartFraction - m_targetEndFraction);

    double nodeSpacing = (m_nodeEndFraction - m_nodeStartFraction) / (dotCount - 1);
    double targetSpacing = (m_targetEndFraction - m_targetStartFraction) / (dotCount - 1);

    double nodeFraction = m_nodeStartFraction;
    double targetFraction = m_targetStartFraction;
    for (int i = 0; i < dotCount; ++i)
    {
        QColor dotColour;
        dotColour.setHsvF(targetFraction, 1.0, 1.0);

        if (reverse)
            returnVector.push_back(BlastDot(dotColour, 1.0 - nodeFraction));
        else
            returnVector.push_back(BlastDot(dotColour, nodeFraction));

        nodeFraction += nodeSpacing;
        targetFraction += targetSpacing;
    }

    return returnVector;
}
