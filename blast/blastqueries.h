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


#ifndef BLASTQUERIES_H
#define BLASTQUERIES_H

#include <vector>
#include "blastquery.h"
#include <QFile>


class BlastQueries
{
public:
    BlastQueries();
    ~BlastQueries();

    std::vector<BlastQuery *> m_queries;

    BlastQuery * getQueryFromName(QString queryName);

    void addQuery(BlastQuery * newQuery);
    void clearQueries();
    void searchOccurred();
    void clearSearchResults();

    std::vector<QColor> presetColours;

private:
    QFile m_tempFile;

    void deleteTempFile();
    void updateTempFile();
    bool tempFileExists() {return m_tempFile.exists();}
    bool tempFileDoesNotExist() {return !tempFileExists();}
    void createPresetColours();

};

#endif // BLASTQUERIES_H
