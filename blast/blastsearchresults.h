#ifndef BLASTSEARCHRESULTS_H
#define BLASTSEARCHRESULTS_H

#include "blasttarget.h"
#include "blastresult.h"
#include <vector>

class BlastSearchResults
{
public:
    BlastSearchResults();
    ~BlastSearchResults();

    std::vector<BlastTarget> m_targets;
    std::vector<BlastResult> m_results;
};

#endif // BLASTSEARCHRESULTS_H
