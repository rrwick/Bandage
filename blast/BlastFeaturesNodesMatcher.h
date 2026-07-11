#ifndef BLASTFEATURESNODESMATCHER_H
#define BLASTFEATURESNODESMATCHER_H

#include <vector>
#include "../random_forest/RandomForestNode.h"

class BlastFeaturesNodesMatcher
{
public:
	BlastFeaturesNodesMatcher();
	void matchFeaturesNode(RandomForestNode* selectedNode);
private:
	QString m_makeblastdbCommand;
	QString m_blastnCommand;
	QString m_tblastnCommand;
};

#endif //BLASTFEATURESNODESMATCHER_H