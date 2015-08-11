#ifndef BLASTQUERYPATH_H
#define BLASTQUERYPATH_H

#include "../graph/path.h"
#include <QList>
#include "blasthit.h"

class BlastQuery;

class BlastQueryPath
{
public:

    //CREATORS
    BlastQueryPath(Path path, BlastQuery * query);

    //ACCESSORS
    Path getPath() const {return m_path;}
    QList<BlastHit *> getHits() const {return m_hits;}
    long double getEvalueProduct() const;
    double getMeanHitPercIdentity() const;
    double getRelativeLengthDiscrepancy() const;
    double getPathQueryCoverage() const;
    double getHitsQueryCoverage() const;
    int getTotalHitMismatches() const;
    int getTotalHitGapOpens() const;

    bool operator<(BlastQueryPath const &other) const;

private:
    Path m_path;
    BlastQuery * m_query;
    QList<BlastHit *> m_hits;

    int getHitQueryLength() const;
};

#endif // BLASTQUERYPATH_H
