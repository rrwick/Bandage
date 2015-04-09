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
    QColor m_edgeColour;
    QColor m_outlineColour;
    QColor m_selectionColour;
    QColor m_textColour;
    QColor m_uniformPositiveNodeColour;
    QColor m_uniformNegativeNodeColour;
    QColor m_uniformNodeSpecialColour;
    QColor m_lowCoverageColour;
    QColor m_highCoverageColour;
    QColor m_contiguousStrandSpecificColour;
    QColor m_contiguousEitherStrandColour;
    QColor m_maybeContiguousColour;
    QColor m_notContiguousColour;
    QColor m_contiguityStartingColour;

    void loadOrSaveSettingsToOrFromWidgets(bool setWidgets, Settings * settings);
    void setButtonColours();
    void setInfoTexts();

private slots:
    void restoreDefaults();
    void edgeColourClicked();
    void outlineColourClicked();
    void selectionColourClicked();
    void textColourClicked();
    void uniformPositiveNodeColourClicked();
    void uniformNegativeNodeColourClicked();
    void uniformNodeSpecialColourClicked();
    void lowCoverageColourClicked();
    void highCoverageColourClicked();
    void contiguousStrandSpecificColourClicked();
    void contiguousEitherStrandColourClicked();
    void maybeContiguousColourClicked();
    void notContiguousColourClicked();
    void contiguityStartingColourClicked();
    void enableDisableCoverageValueSpinBoxes();

public slots:
    void accept();
};

#endif // SETTINGSDIALOG_H
