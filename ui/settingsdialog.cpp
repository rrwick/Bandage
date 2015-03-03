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


#include "settingsdialog.h"
#include "ui_settingsdialog.h"
#include <QColorDialog>

SettingsDialog::SettingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);

    connect(ui->restoreDefaultsButton, SIGNAL(clicked()), this, SLOT(restoreDefaults()));
    connect(ui->uniformPositiveNodeColourButton, SIGNAL(clicked()), this, SLOT(uniformPositiveNodeColourClicked()));
    connect(ui->uniformNegativeNodeColourButton, SIGNAL(clicked()), this, SLOT(uniformNegativeNodeColourClicked()));
    connect(ui->uniformNodeSpecialColourButton, SIGNAL(clicked()), this, SLOT(uniformNodeSpecialColourClicked()));
    connect(ui->lowCoverageColourButton, SIGNAL(clicked()), this, SLOT(lowCoverageColourClicked()));
    connect(ui->highCoverageColourButton, SIGNAL(clicked()), this, SLOT(highCoverageColourClicked()));
    connect(ui->selectionColourButton, SIGNAL(clicked()), this, SLOT(selectionColourClicked()));
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}


//These functions either set a spin box to a value or set the value to the spin box.  Pointers to
//these functions will be passed to loadOrSaveSettingsToOrFromWidgets, so that one function can
//take care of both save and load functionality.
void setOneSettingFromWidget(double * setting, QDoubleSpinBox * widget, bool percentage) {*setting = widget->value() / (percentage ? 100.0 : 1.0);}
void setOneSettingFromWidget(int * setting, QSpinBox * widget) {*setting = widget->value();}
void setOneSettingFromWidget(QColor * setting, QColor * widget) {*setting = *widget;}
void setOneWidgetFromSetting(double * setting, QDoubleSpinBox * widget, bool percentage) {widget->setValue(*setting * (percentage ? 100.0 : 1.0));}
void setOneWidgetFromSetting(int * setting, QSpinBox * widget) {widget->setValue(*setting);}
void setOneWidgetFromSetting(QColor * setting, QColor * widget) {*widget = *setting;}



void SettingsDialog::setWidgetsFromSettings()
{
    loadOrSaveSettingsToOrFromWidgets(true, g_settings);
}


void SettingsDialog::setSettingsFromWidgets()
{
    loadOrSaveSettingsToOrFromWidgets(false, g_settings);

    //The highlighted outline thickness should always be a bit
    //bigger than the outlineThickness.
    g_settings->selectionThickness = g_settings->outlineThickness + 1.0;
}



void SettingsDialog::loadOrSaveSettingsToOrFromWidgets(bool setWidgets, Settings * settings)
{
    void (*doubleFunctionPointer)(double *, QDoubleSpinBox *, bool);
    void (*intFunctionPointer)(int *, QSpinBox *);
    void (*colourFunctionPointer)(QColor *, QColor *);

    if (setWidgets)
    {
        doubleFunctionPointer = setOneWidgetFromSetting;
        intFunctionPointer = setOneWidgetFromSetting;
        colourFunctionPointer = setOneWidgetFromSetting;
    }
    else
    {
        doubleFunctionPointer = setOneSettingFromWidget;
        intFunctionPointer = setOneSettingFromWidget;
        colourFunctionPointer = setOneSettingFromWidget;
    }

    intFunctionPointer(&settings->basePairsPerSegment, ui->basePairsPerSegmentSpinBox);
    doubleFunctionPointer(&settings->minimumContigWidth, ui->minimumContigWidthSpinBox, false);
    doubleFunctionPointer(&settings->coverageContigWidth, ui->coverageContigWidthSpinBox, false);
    doubleFunctionPointer(&settings->maxContigWidth, ui->maximumContigWidthSpinBox, false);
    doubleFunctionPointer(&settings->edgeWidth, ui->edgeWidthSpinBox, false);
    doubleFunctionPointer(&settings->outlineThickness, ui->outlineThicknessSpinBox, false);
    doubleFunctionPointer(&settings->textOutlineThickness, ui->textOutlineThicknessSpinBox, false);
    colourFunctionPointer(&settings->uniformPositiveNodeColour, &m_uniformPositiveNodeColour);
    colourFunctionPointer(&settings->uniformNegativeNodeColour, &m_uniformNegativeNodeColour);
    colourFunctionPointer(&settings->uniformNodeSpecialColour, &m_uniformNodeSpecialColour);
    colourFunctionPointer(&settings->lowCoverageColour, &m_lowCoverageColour);
    colourFunctionPointer(&settings->highCoverageColour, &m_highCoverageColour);
    colourFunctionPointer(&settings->selectionColour, &m_selectionColour);


    //A couple of settings are not in a spin box, so they
    //have to be done manually, not with those function pointers.
    if (setWidgets)
    {
        ui->graphLayoutQualitySlider->setValue(settings->graphLayoutQuality);
        ui->antialiasingOnRadioButton->setChecked(settings->antialiasing);
        ui->antialiasingOffRadioButton->setChecked(!settings->antialiasing);

        setButtonColours();
    }
    else
    {
        settings->graphLayoutQuality = ui->graphLayoutQualitySlider->value();
        settings->antialiasing = ui->antialiasingOnRadioButton->isChecked();
    }
}


void SettingsDialog::restoreDefaults()
{
    Settings defaultSettings;
    loadOrSaveSettingsToOrFromWidgets(true, &defaultSettings);
}

void SettingsDialog::uniformPositiveNodeColourClicked()
{
    QColor chosenColor = QColorDialog::getColor(m_uniformPositiveNodeColour, 0, "Uniform positive node colour");
    if (chosenColor.isValid())
    {
        m_uniformPositiveNodeColour = chosenColor.rgb();
        setButtonColours();
    }
}
void SettingsDialog::uniformNegativeNodeColourClicked()
{
    QColor chosenColor = QColorDialog::getColor(m_uniformNegativeNodeColour, 0, "Uniform negative node colour");
    if (chosenColor.isValid())
    {
        m_uniformNegativeNodeColour = chosenColor.rgb();
        setButtonColours();
    }
}
void SettingsDialog::uniformNodeSpecialColourClicked()
{
    QColor chosenColor = QColorDialog::getColor(m_uniformNodeSpecialColour, 0, "Uniform node special colour");
    if (chosenColor.isValid())
    {
        m_uniformNodeSpecialColour = chosenColor.rgb();
        setButtonColours();
    }
}
void SettingsDialog::lowCoverageColourClicked()
{
    QColor chosenColor = QColorDialog::getColor(m_lowCoverageColour, 0, "Low coverage colour");
    if (chosenColor.isValid())
    {
        m_lowCoverageColour = chosenColor.rgb();
        setButtonColours();
    }
}
void SettingsDialog::highCoverageColourClicked()
{
    QColor chosenColor = QColorDialog::getColor(m_highCoverageColour, 0, "High coverage colour");
    if (chosenColor.isValid())
    {
        m_highCoverageColour = chosenColor.rgb();
        setButtonColours();
    }
}
void SettingsDialog::selectionColourClicked()
{
    QColor chosenColor = QColorDialog::getColor(m_selectionColour, 0, "Selection colour");
    if (chosenColor.isValid())
    {
        m_selectionColour = chosenColor.rgb();
        setButtonColours();
    }
}


void SettingsDialog::setButtonColours()
{
    const QString COLOR_STYLE("QPushButton { background-color : %1 }");
    ui->uniformPositiveNodeColourButton->setStyleSheet(COLOR_STYLE.arg(m_uniformPositiveNodeColour.name()));
    ui->uniformNegativeNodeColourButton->setStyleSheet(COLOR_STYLE.arg(m_uniformNegativeNodeColour.name()));
    ui->uniformNodeSpecialColourButton->setStyleSheet(COLOR_STYLE.arg(m_uniformNodeSpecialColour.name()));
    ui->lowCoverageColourButton->setStyleSheet(COLOR_STYLE.arg(m_lowCoverageColour.name()));
    ui->highCoverageColourButton->setStyleSheet(COLOR_STYLE.arg(m_highCoverageColour.name()));
    ui->selectionColourButton->setStyleSheet(COLOR_STYLE.arg(m_selectionColour.name()));
}
