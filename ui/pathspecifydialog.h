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


#ifndef PATHSPECIFYDIALOG_H
#define PATHSPECIFYDIALOG_H

#include <QDialog>
#include "../graph/path.h"
#include "../graph/debruijnnode.h"

namespace Ui {
class PathSpecifyDialog;
}

class PathSpecifyDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PathSpecifyDialog(QWidget *parent = 0);
    ~PathSpecifyDialog();

public slots:
    void addNodeName(DeBruijnNode * node);

private:
    Ui::PathSpecifyDialog *ui;
    void setPathValidityUiElements(bool pathValid);

private slots:
    void checkPathValidity();
    void copyPathToClipboard();
    void savePathToFile();
};

#endif // PATHSPECIFYDIALOG_H
