#include "blastqueries.h"
#include "../program/globals.h"
#include <QTextStream>

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

    m_tempFile.open(QIODevice::Append | QIODevice::Text);
    QTextStream out(&m_tempFile);
    for (size_t i = 0; i < m_queries.size(); ++i)
    {
        out << ">" << m_queries[i].m_name << "\n";
        out << m_queries[i].m_sequence;

        if (i + 1 != m_queries.size())
            out << "\n";
    }

    m_tempFile.close();
}


void BlastQueries::searchOccurred()
{
    for (size_t i = 0; i < m_queries.size(); ++i)
        m_queries[i].m_searchedFor = true;
}


void BlastQueries::clearSearchResults()
{
    for (size_t i = 0; i < m_queries.size(); ++i)
    {
        m_queries[i].m_searchedFor = false;
        m_queries[i].m_hits = 0;
    }
}
