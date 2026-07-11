#include "BlastFeaturesNodesMatcher.h"
#include "BuildBlastDatabaseWorker.h"
#include "RunBlastSearchWorker.h"
#include "../blast/blastsearch.h"
#include "../blast/blasthit.h"
#include "../blast/blastquery.h"
#include "../program/globals.h"
#include "../program/settings.h"

BlastFeaturesNodesMatcher::BlastFeaturesNodesMatcher() {
    m_makeblastdbCommand = "makeblastdb";
    m_blastnCommand = "blastn";
    m_tblastnCommand = "tblastn";
}

void BlastFeaturesNodesMatcher::matchFeaturesNode(RandomForestNode* selectedNode) {
	std::vector<QString> querySequences = selectedNode->getQuerySequences();
	if ((querySequences.size()!= 0) && (selectedNode->getBlastColourInd() == - 1)) {
        //build blast db
        if (!g_blastSearch->findProgram("makeblastdb", &m_makeblastdbCommand)) {
            return;
        }

        BuildBlastDatabaseWorker buildBlastDatabaseWorker(m_makeblastdbCommand);
        buildBlastDatabaseWorker.buildBlastDatabase();
           
        //add blast query
        //g_blastSearch->cleanUp();
        QString featureNodeName = selectedNode->getName() + "_";
        int indexColour = g_blastSearch->m_blastQueries.m_queries.size();
        for (size_t i = 0; i < querySequences.size(); ++i)
        {
            QString queryName = featureNodeName + QString::number(i);
            g_blastSearch->m_blastQueries.addQuery(new BlastQuery(queryName, querySequences[i]), indexColour, selectedNode->getClassInd());

        }
        //run blast search

        if (!g_blastSearch->findProgram("blastn", &m_blastnCommand))
        {
            return;
        }
        if (!g_blastSearch->findProgram("tblastn", &m_tblastnCommand))
        {
            return;
        }

        g_blastSearch->clearBlastHits();

        RunBlastSearchWorker runBlastSearchWorker(m_blastnCommand, m_tblastnCommand, "");
        runBlastSearchWorker.runBlastSearch();
        selectedNode->setBlastColourInd(indexColour);
	}
}
