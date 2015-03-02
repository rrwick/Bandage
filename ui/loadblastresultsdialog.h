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


#ifndef LOADBLASTRESULTSDIALOG_H
#define LOADBLASTRESULTSDIALOG_H

#include <QDialog>
#include "../blast/blastsearchresults.h"
#include <QMap>

class DeBruijnNode;
class BlastTarget;

namespace Ui {
class LoadBlastResultsDialog;
}

class LoadBlastResultsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LoadBlastResultsDialog(QMap<int, DeBruijnNode*> * deBruijnGraphNodes,
                                    QWidget *parent = 0);
    ~LoadBlastResultsDialog();

private:
    Ui::LoadBlastResultsDialog *ui;
    QMap<int, DeBruijnNode*> * m_deBruijnGraphNodes;
    QString m_tempDirectory;

    int getNodeNumberFromString(QString nodeString);
    BlastTarget * getTargetFromString(QString targetName);
    void quitBlastHitLoading(QString error);

private slots:
    void buildBlastDatabase1();
    void buildBlastDatabase2();



    void loadBlastTargets();
    void loadBlastHits();
    void fillTargetsTable();
    void fillHitsTable();

signals:
    void createAllNodesFasta(QString path);
};

#endif // LOADBLASTRESULTSDIALOG_H
