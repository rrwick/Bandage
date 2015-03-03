#include "blastqueries.h"
#include "../program/globals.h"

BlastQueries::BlastQueries() :
    m_tempFile(g_tempDirectory + "queries.fasta")
{
}


BlastQueries::~BlastQueries()
{
    deleteTempFile();
}


BlastQuery * BlastQueries::getQueryFromName(QString queryName)
{
    for (size_t i = 0; i < m_queries.size(); ++i)
    {
        if (m_queries[i].m_name == queryName)
            return &(m_queries[i]);
    }
    return 0;
}



void BlastQueries::addQuery(BlastQuery newQuery)
{
    //POSSIBLE CODE HERE TO CHECK FOR DUPLICATE QUERY NAMES?

    m_queries.push_back(newQuery);
    updateTempFile();
}

void BlastQueries::clearQueries()
{
    m_queries.clear();
    deleteTempFile();
}

void BlastQueries::createTempFile()
{
    m_tempFile.open(QIODevice::WriteOnly);
}

void BlastQueries::deleteTempFile()
{
    if (tempFileDoesNotExist())
        return;

    m_tempFile.remove();
}

void BlastQueries::updateTempFile()
{
    deleteTempFile();

    //If there aren't any queries, there's no need for a temp file.
    if (m_queries.size() == 0)
        return;

    if (tempFileDoesNotExist())
        createTempFile();

    //ADD THE QUERIES TO THE FILE HERE!
}
