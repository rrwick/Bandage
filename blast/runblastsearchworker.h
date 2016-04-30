//Copyright 2016 Ryan Wick

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


#ifndef RUNBLASTSEARCHWORKER_H
#define RUNBLASTSEARCHWORKER_H

#include <QObject>
#include <QProcess>
#include <QString>
#include "../program/globals.h"

//This class carries out the task of running blastn and/or
//tblastn.
//It is a separate class because when run from the GUI, this
//process takes place in a separate thread.

class RunBlastSearchWorker : public QObject
{
    Q_OBJECT

public:
    RunBlastSearchWorker(QString blastnCommand, QString tblastnCommand, QString parameters);
    QString m_error;

private:
    QString m_blastnCommand;
    QString m_tblastnCommand;
    QString m_parameters;
    QString runOneBlastSearch(SequenceType sequenceType, bool * success);

public slots:
    void runBlastSearch();

signals:
    void finishedSearch(QString error);
};

#endif // RUNBLASTSEARCHWORKER_H
