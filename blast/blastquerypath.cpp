#include "blastquerypath.h"

#include "blastquery.h"
#include "../graph/debruijnnode.h"

BlastQueryPath::BlastQueryPath(Path path, BlastQuery * query) :
    m_path(path), m_query(query)
{

    //This function follows the path, returning the BLAST hits it finds for the
    //query.  It requires that the hits occur in order, i.e. that each hit in
    //the path begins later in the query than the previous hit.

    BlastHit * previousHit = 0;
    QList<DeBruijnNode *> pathNodes = m_path.getNodes();
    for (int i = 0; i < pathNodes.size(); ++i)
    {
        DeBruijnNode * node = pathNodes[i];

        QList<BlastHit *> hitsThisNode;
        QList< QSharedPointer<BlastHit> > queryHits = query->getHits();
        for (int j = 0; j < queryHits.size(); ++j)
        {
            BlastHit * hit = queryHits[j].data();
            if (hit->m_node->m_name == node->m_name)
                hitsThisNode.push_back(hit);
        }

        std::sort(hitsThisNode.begin(), hitsThisNode.end(),
                  BlastHit::compareTwoBlastHitPointers);

        for (int j = 0; j < hitsThisNode.size(); ++j)
        {
            BlastHit * hit = hitsThisNode[j];

            //First check to make sure the hits are within the path.  This means
            //if we are in the first or last nodes of the path, we need to make
            //sure that our hit is contained within the start/end positions.
            if ( (i != 0 || hit->m_nodeStart >= m_path.getStartLocation().getPosition()) &&
                    (i != pathNodes.size()-1 || hit->m_nodeEnd <= m_path.getEndLocation().getPosition()))
            {
                //Now make sure that the hit follows the previous hit in the
                //query.
                if (previousHit == 0 ||
                        hit->m_queryStart > previousHit->m_queryStart)
                {
                    m_hits.push_back(hit);
                    previousHit = hit;
                }
            }
        }
    }
}





double BlastQueryPath::getPercIdentity() const
{
    return 4.321; //TEMP
}




//This function looks at all of the hits in the path for this query and
//multiplies the evalues together.
long double BlastQueryPath::getEvalueProduct() const
{
    long double eValueProduct = 1.0;
    for (int i = 0; i < m_hits.size(); ++i)
        eValueProduct *= m_hits[i]->m_eValue;

    return eValueProduct;
}


//This function looks at the length of the given path and compares it to how
//long the path should be for the hits it contains (i.e. if the path perfectly
//matched up the query).
double BlastQueryPath::getRelativeLengthDiscrepancy() const
{
    if (m_hits.empty())
        return std::numeric_limits<double>::max();

    int queryStart = m_hits.front()->m_queryStart;
    int queryEnd = m_hits.back()->m_queryEnd;
    int hitQueryLength = queryEnd - queryStart;
    if (m_query->getSequenceType() == PROTEIN)
        hitQueryLength *= 3;

    int discrepancy = abs(m_path.getLength() - hitQueryLength);
    return double(discrepancy) / hitQueryLength;
}




bool BlastQueryPath::operator<(BlastQueryPath const &other) const
{
    long double aEValueProduct = getEvalueProduct();
    long double bEValueProduct = other.getEvalueProduct();

    if (aEValueProduct != bEValueProduct)
        return aEValueProduct < bEValueProduct;

    //If the code got here, then the two paths have the same evalue product,
    //probably because they contain the same hits.  In this case, we use their
    //length discrepency.

    return getRelativeLengthDiscrepancy() < other.getRelativeLengthDiscrepancy();
}
