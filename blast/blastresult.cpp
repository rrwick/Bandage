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


std::vector<BlastHitPart> BlastResult::getBlastHitParts(bool reverse)
{
    //TEST CODE
    if (m_node->m_number == 346)
    {
        int test = 5;
    }


    std::vector<BlastHitPart> returnVector;

    int partCount = ceil(g_settings->blastPartsPerTarget * fabs(m_targetStartFraction - m_targetEndFraction));

    double nodeSpacing = (m_nodeEndFraction - m_nodeStartFraction) / partCount;
    double targetSpacing = (m_targetEndFraction - m_targetStartFraction) / partCount;

    double nodeFraction = m_nodeStartFraction;
    double targetFraction = m_targetStartFraction;
    for (int i = 0; i < partCount; ++i)
    {
        QColor dotColour;
        dotColour.setHsvF(targetFraction * 0.9, 1.0, 1.0);  //times 0.9 to keep the colour from getting too clsoe to red, as that could confuse the end with the start

        double nextFraction = nodeFraction + nodeSpacing;

        if (reverse)
            returnVector.push_back(BlastHitPart(dotColour, 1.0 - nodeFraction, 1.0 - nextFraction));
        else
            returnVector.push_back(BlastHitPart(dotColour, nodeFraction, nextFraction));

        nodeFraction = nextFraction;
        targetFraction += targetSpacing;
    }

    return returnVector;
}
