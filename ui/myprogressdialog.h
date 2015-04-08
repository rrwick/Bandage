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


#ifndef MYPROGRESSDIALOG_H
#define MYPROGRESSDIALOG_H

#include <QDialog>

namespace Ui {
class MyProgressDialog;
}

class MyProgressDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MyProgressDialog(QWidget * parent, QString message, bool showCancelButton);
    ~MyProgressDialog();

private:
    Ui::MyProgressDialog *ui;

private slots:
    void cancelLayout();

signals:
    void haltLayout();
};

#endif // MYPROGRESSDIALOG_H
