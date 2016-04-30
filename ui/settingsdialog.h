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


#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QCloseEvent>

class Settings;

namespace Ui {
class SettingsDialog;
}

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = 0);
    ~SettingsDialog();

    void setWidgetsFromSettings();
    void setSettingsFromWidgets();

private:
    Ui::SettingsDialog *ui;

    void loadOrSaveSettingsToOrFromWidgets(bool setWidgets, Settings * settings);
    void setInfoTexts();

private slots:
    void restoreDefaults();
    void enableDisableDepthWidgets();
    void nodeLengthPerMegabaseManualChanged();
    void updateNodeWidthVisualAid();
    void colourSliderChanged();
    void colourSpinBoxChanged();
    void checkBoxesChanged();
    void lengthDiscrepancySpinBoxChanged();

public slots:
    void accept();
};

#endif // SETTINGSDIALOG_H
