//Copyright 2016 Ryan Wick

//This file is part of Bandage.

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


#include "aboutdialog.h"
#include "ui_aboutdialog.h"
#include <QApplication>

AboutDialog::AboutDialog(QWidget *parent) :
    QDialog(parent, Qt::WindowTitleHint | Qt::WindowCloseButtonHint),
    ui(new Ui::AboutDialog)
{
    ui->setupUi(this);

    QPixmap icon = *(ui->iconLabel->pixmap());
    icon.setDevicePixelRatio(devicePixelRatio());
    ui->iconLabel->setPixmap(icon);

    //Make the word 'Bandage' a larger font size.
    QFont font;
    font.setPointSize(font.pointSize() * 2);
    ui->titleLabel->setFont(font);

    QString versionCopyrightText = "<html><head/><body><p>Version: " + QApplication::applicationVersion() + "</p><p>Copyright 2016 Ryan Wick</p><p><a href=\"http://rrwick.github.io/Bandage/\"><span style=\" text-decoration: underline; color:#0000ff;\">http://rrwick.github.io/Bandage/</span></a></p></body></html>";
    ui->versionCopyrightLabel->setText(versionCopyrightText);

    layout()->setSizeConstraint(QLayout::SetFixedSize);
}


AboutDialog::~AboutDialog()
{
    delete ui;
}
