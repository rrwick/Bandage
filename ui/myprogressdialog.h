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


#ifndef MYPROGRESSDIALOG_H
#define MYPROGRESSDIALOG_H

#include <QDialog>
#include <QString>

namespace Ui {
class MyProgressDialog;
}

class MyProgressDialog : public QDialog
{
    Q_OBJECT

public:
    //CREATORS
    explicit MyProgressDialog(QWidget * parent, QString message, bool showCancelButton,
                              QString cancelButtonText = "", QString cancelMessage = "", QString cancelInfoText = "");
    ~MyProgressDialog();

    //ACCESSORS
    bool wasCancelled() const {return m_cancelled;}

public slots:
    void setMaxValue(int max);
    void setValue(int value);

private:
    Ui::MyProgressDialog *ui;
    QString m_cancelMessage;
    bool m_cancelled;

private slots:
    void cancel();

signals:
    void halt();
};

#endif // MYPROGRESSDIALOG_H
