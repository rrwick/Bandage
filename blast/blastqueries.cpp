//Copyright 2015 Ryan Wick

//This file is part of Bandage

//Bandage is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//Bandage is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

//You should have received a copy of the GNU General Public License
//along with Bandage.  If not, see <http://www.gnu.org/licenses/>.


#include "blastqueries.h"
#include "../program/globals.h"
#include <QTextStream>
#include "blastsearch.h"

BlastQueries::BlastQueries() :
    m_tempNuclFile(0), m_tempProtFile(0)
{
    createPresetColours();
}


BlastQueries::~BlastQueries()
{
    clearAllQueries();
}


void BlastQueries::createTempQueryFiles()
{
    m_tempNuclFile.reset(new QFile(g_blastSearch->m_tempDirectory + "nucl_queries.fasta"));
    m_tempProtFile.reset(new QFile(g_blastSearch->m_tempDirectory + "prot_queries.fasta"));
}

BlastQuery * BlastQueries::getQueryFromName(QString queryName)
{
    for (size_t i = 0; i < m_queries.size(); ++i)
    {
        if (m_queries[i]->m_name == queryName)
            return m_queries[i];
    }
    return 0;
}



void BlastQueries::addQuery(BlastQuery * newQuery)
{
    //Add a suffix to the new query name if it not unique.  Also make
    //sure it's not "all", as that will conflict with viewing all
    //queries at once.
    QString newQueryName = newQuery->m_name;
    int queryNumber = 2;
    QString finalName = newQueryName;
    while (getQueryFromName(finalName) != 0 || finalName == "all")
        finalName = newQueryName + "_" + QString::number(queryNumber++);
    newQuery->m_name = finalName;

    //Give the new query a colour
    int colourIndex = m_queries.size();
    colourIndex %= presetColours.size();
    newQuery->m_colour = presetColours[colourIndex];

    m_queries.push_back(newQuery);
    updateTempFiles();
}

void BlastQueries::clearAllQueries()
{
    for (size_t i = 0; i < m_queries.size(); ++i)
        delete m_queries[i];
    m_queries.clear();
    deleteTempFiles();
}

void BlastQueries::clearSomeQueries(std::vector<BlastQuery *> queriesToRemove)
{
    for (size_t i = 0; i < queriesToRemove.size(); ++i)
    {
        m_queries.erase(std::remove(m_queries.begin(), m_queries.end(), queriesToRemove[i]), m_queries.end());
        delete queriesToRemove[i];
    }

    updateTempFiles();
}

void BlastQueries::deleteTempFiles()
{
    if (tempNuclFileExists())
        m_tempNuclFile->remove();
    if (tempProtFileExists())
        m_tempProtFile->remove();
}

void BlastQueries::updateTempFiles()
{
    deleteTempFiles();

    if (getQueryCount(NUCLEOTIDE) > 0)
        writeTempFile(m_tempNuclFile, NUCLEOTIDE);

    if (getQueryCount(PROTEIN) > 0)
        writeTempFile(m_tempProtFile, PROTEIN);
}


void BlastQueries::writeTempFile(QSharedPointer<QFile> file, SequenceType sequenceType)
{
    file->open(QIODevice::Append | QIODevice::Text);
    QTextStream out(file.data());
    for (size_t i = 0; i < m_queries.size(); ++i)
    {
        if (m_queries[i]->m_sequenceType == sequenceType)
        {
            out << ">" << m_queries[i]->m_name << "\n";
            out << m_queries[i]->m_sequence;
            out << "\n";
        }
    }
    file->close();
}


void BlastQueries::searchOccurred()
{
    for (size_t i = 0; i < m_queries.size(); ++i)
        m_queries[i]->m_searchedFor = true;
}


void BlastQueries::clearSearchResults()
{
    for (size_t i = 0; i < m_queries.size(); ++i)
        m_queries[i]->clearSearchResults();
}

int BlastQueries::getQueryCount(SequenceType sequenceType)
{
    int count = 0;
    for (size_t i = 0; i < m_queries.size(); ++i)
    {
        if (m_queries[i]->m_sequenceType == sequenceType)
            ++count;
    }
    return count;
}


bool BlastQueries::tempNuclFileExists()
{
    if (m_tempNuclFile.isNull())
        return false;
    return m_tempNuclFile->exists();
}
bool BlastQueries::tempProtFileExists()
{
    if (m_tempProtFile.isNull())
        return false;
    return m_tempProtFile->exists();
}


void BlastQueries::createPresetColours()
{
    presetColours.push_back(QColor("#306FF8"));
    presetColours.push_back(QColor("#86BB18"));
    presetColours.push_back(QColor("#DF123A"));
    presetColours.push_back(QColor("#181E2A"));
    presetColours.push_back(QColor("#F91BBD"));
    presetColours.push_back(QColor("#3CB2A4"));
    presetColours.push_back(QColor("#D29AC1"));
    presetColours.push_back(QColor("#E2922E"));
    presetColours.push_back(QColor("#22501B"));
    presetColours.push_back(QColor("#57297D"));
    presetColours.push_back(QColor("#3FA0E6"));
    presetColours.push_back(QColor("#770739"));
    presetColours.push_back(QColor("#6A390C"));
    presetColours.push_back(QColor("#25AB5D"));
    presetColours.push_back(QColor("#ACAF61"));
    presetColours.push_back(QColor("#F0826F"));
    presetColours.push_back(QColor("#E94A80"));
    presetColours.push_back(QColor("#C187F2"));
    presetColours.push_back(QColor("#7E5764"));
    presetColours.push_back(QColor("#037290"));
    presetColours.push_back(QColor("#D65114"));
    presetColours.push_back(QColor("#08396A"));
    presetColours.push_back(QColor("#99ABBE"));
    presetColours.push_back(QColor("#F270C0"));
    presetColours.push_back(QColor("#F056F9"));
    presetColours.push_back(QColor("#8E8D00"));
    presetColours.push_back(QColor("#70010F"));
    presetColours.push_back(QColor("#9C1E9A"));
    presetColours.push_back(QColor("#471B1F"));
    presetColours.push_back(QColor("#A00B6D"));
    presetColours.push_back(QColor("#38C037"));
    presetColours.push_back(QColor("#282C16"));
    presetColours.push_back(QColor("#15604D"));
    presetColours.push_back(QColor("#2E75D6"));
    presetColours.push_back(QColor("#A09DEB"));
    presetColours.push_back(QColor("#8454D7"));
    presetColours.push_back(QColor("#301745"));
    presetColours.push_back(QColor("#A45704"));
    presetColours.push_back(QColor("#4D8C0E"));
    presetColours.push_back(QColor("#C09860"));
    presetColours.push_back(QColor("#009C73"));
    presetColours.push_back(QColor("#FD6453"));
    presetColours.push_back(QColor("#C11C4B"));
    presetColours.push_back(QColor("#183B8B"));
    presetColours.push_back(QColor("#5E6706"));
    presetColours.push_back(QColor("#E42005"));
    presetColours.push_back(QColor("#4873AF"));
    presetColours.push_back(QColor("#6CA563"));
    presetColours.push_back(QColor("#5E0F54"));
    presetColours.push_back(QColor("#FE2065"));
    presetColours.push_back(QColor("#5BB4D2"));
    presetColours.push_back(QColor("#3F4204"));
    presetColours.push_back(QColor("#521839"));
    presetColours.push_back(QColor("#9A7706"));
    presetColours.push_back(QColor("#77AB8C"));
    presetColours.push_back(QColor("#105E04"));
    presetColours.push_back(QColor("#98290F"));
    presetColours.push_back(QColor("#B849D4"));
    presetColours.push_back(QColor("#FC8426"));
    presetColours.push_back(QColor("#341B03"));
    presetColours.push_back(QColor("#E3278C"));
    presetColours.push_back(QColor("#F28F93"));
    presetColours.push_back(QColor("#D1A21F"));
    presetColours.push_back(QColor("#277E46"));
    presetColours.push_back(QColor("#285C60"));
    presetColours.push_back(QColor("#76B945"));
    presetColours.push_back(QColor("#E75D65"));
    presetColours.push_back(QColor("#84ADDC"));
    presetColours.push_back(QColor("#153C2B"));
    presetColours.push_back(QColor("#FD10D9"));
    presetColours.push_back(QColor("#C095D5"));
    presetColours.push_back(QColor("#052B48"));
    presetColours.push_back(QColor("#B365FC"));
    presetColours.push_back(QColor("#97AA75"));
    presetColours.push_back(QColor("#C78C9C"));
    presetColours.push_back(QColor("#FD4838"));
    presetColours.push_back(QColor("#F181E2"));
    presetColours.push_back(QColor("#815A1A"));
    presetColours.push_back(QColor("#BB2093"));
    presetColours.push_back(QColor("#691822"));
    presetColours.push_back(QColor("#C41A12"));
    presetColours.push_back(QColor("#728A1F"));
    presetColours.push_back(QColor("#375B73"));
    presetColours.push_back(QColor("#97022C"));
    presetColours.push_back(QColor("#95B44D"));
    presetColours.push_back(QColor("#EB8DBB"));
    presetColours.push_back(QColor("#83ACAB"));
    presetColours.push_back(QColor("#E37D51"));
    presetColours.push_back(QColor("#D78A68"));
    presetColours.push_back(QColor("#4A41A2"));
    presetColours.push_back(QColor("#8A0C79"));
    presetColours.push_back(QColor("#133102"));
    presetColours.push_back(QColor("#237A78"));
    presetColours.push_back(QColor("#ADB03B"));
    presetColours.push_back(QColor("#289E26"));
    presetColours.push_back(QColor("#7683EC"));
    presetColours.push_back(QColor("#4E1E04"));
    presetColours.push_back(QColor("#BB17B2"));
    presetColours.push_back(QColor("#EB6A81"));
    presetColours.push_back(QColor("#47B4E8"));
    presetColours.push_back(QColor("#0A6191"));
    presetColours.push_back(QColor("#4EADB2"));
    presetColours.push_back(QColor("#442965"));
    presetColours.push_back(QColor("#FE784B"));
    presetColours.push_back(QColor("#55BD8D"));
    presetColours.push_back(QColor("#742B03"));
    presetColours.push_back(QColor("#8C38AA"));
    presetColours.push_back(QColor("#F758A6"));
    presetColours.push_back(QColor("#A32526"));
    presetColours.push_back(QColor("#442C2E"));
    presetColours.push_back(QColor("#F06A97"));
    presetColours.push_back(QColor("#3A1527"));
    presetColours.push_back(QColor("#503509"));
    presetColours.push_back(QColor("#2A67B4"));
    presetColours.push_back(QColor("#243644"));
    presetColours.push_back(QColor("#A74006"));
    presetColours.push_back(QColor("#335900"));
    presetColours.push_back(QColor("#A07484"));
    presetColours.push_back(QColor("#490216"));
    presetColours.push_back(QColor("#B19BCB"));
    presetColours.push_back(QColor("#75B75A"));
    presetColours.push_back(QColor("#BE71EB"));
    presetColours.push_back(QColor("#024A2E"));
    presetColours.push_back(QColor("#A097AB"));
    presetColours.push_back(QColor("#7A287E"));
    presetColours.push_back(QColor("#6A1444"));
    presetColours.push_back(QColor("#212449"));
    presetColours.push_back(QColor("#B07017"));
    presetColours.push_back(QColor("#227D57"));
    presetColours.push_back(QColor("#1B8CAF"));
    presetColours.push_back(QColor("#016438"));
    presetColours.push_back(QColor("#EA64CF"));
    presetColours.push_back(QColor("#B5310E"));
    presetColours.push_back(QColor("#B00765"));
    presetColours.push_back(QColor("#5F42B3"));
    presetColours.push_back(QColor("#EF9649"));
    presetColours.push_back(QColor("#25717F"));
    presetColours.push_back(QColor("#BCA309"));
    presetColours.push_back(QColor("#FA35A6"));
    presetColours.push_back(QColor("#F63D54"));
    presetColours.push_back(QColor("#E83D6C"));
    presetColours.push_back(QColor("#8362F2"));
    presetColours.push_back(QColor("#33BC4A"));
    presetColours.push_back(QColor("#194A85"));
    presetColours.push_back(QColor("#E24215"));
    presetColours.push_back(QColor("#6D71FE"));
    presetColours.push_back(QColor("#3E52AF"));
    presetColours.push_back(QColor("#1E9E89"));
    presetColours.push_back(QColor("#740860"));
    presetColours.push_back(QColor("#4B7BEE"));
    presetColours.push_back(QColor("#8742C0"));
    presetColours.push_back(QColor("#DD8EC6"));
    presetColours.push_back(QColor("#CD202C"));
    presetColours.push_back(QColor("#FD82C2"));
    presetColours.push_back(QColor("#3C2874"));
    presetColours.push_back(QColor("#F9742B"));
    presetColours.push_back(QColor("#013B10"));
    presetColours.push_back(QColor("#D12867"));
    presetColours.push_back(QColor("#F743C3"));
    presetColours.push_back(QColor("#B98EEC"));
    presetColours.push_back(QColor("#D260EC"));
    presetColours.push_back(QColor("#671C06"));
    presetColours.push_back(QColor("#37A968"));
    presetColours.push_back(QColor("#3B9529"));
    presetColours.push_back(QColor("#2A0E33"));
    presetColours.push_back(QColor("#51B237"));
    presetColours.push_back(QColor("#95B61B"));
    presetColours.push_back(QColor("#B195E2"));
    presetColours.push_back(QColor("#68B49A"));
    presetColours.push_back(QColor("#182339"));
    presetColours.push_back(QColor("#FC4822"));
    presetColours.push_back(QColor("#D79621"));
    presetColours.push_back(QColor("#90761B"));
    presetColours.push_back(QColor("#777315"));
    presetColours.push_back(QColor("#E389E9"));
    presetColours.push_back(QColor("#35BD64"));
    presetColours.push_back(QColor("#C17910"));
    presetColours.push_back(QColor("#3386ED"));
    presetColours.push_back(QColor("#E82C2E"));
    presetColours.push_back(QColor("#AC925F"));
    presetColours.push_back(QColor("#F227C8"));
    presetColours.push_back(QColor("#F43E67"));
    presetColours.push_back(QColor("#55AEEB"));
    presetColours.push_back(QColor("#F518E3"));
    presetColours.push_back(QColor("#AB0643"));
    presetColours.push_back(QColor("#8DA1F3"));
    presetColours.push_back(QColor("#5C9C14"));
    presetColours.push_back(QColor("#381F27"));
    presetColours.push_back(QColor("#6BB7B5"));
    presetColours.push_back(QColor("#9842BE"));
    presetColours.push_back(QColor("#4897D6"));
    presetColours.push_back(QColor("#8958E4"));
    presetColours.push_back(QColor("#8F0065"));
    presetColours.push_back(QColor("#A10A5E"));
    presetColours.push_back(QColor("#076315"));
    presetColours.push_back(QColor("#FA5EF9"));
    presetColours.push_back(QColor("#A33402"));
    presetColours.push_back(QColor("#A0ABC4"));
    presetColours.push_back(QColor("#2B6EFE"));
    presetColours.push_back(QColor("#9A9EE7"));
}
