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
#include <QMessageBox>
#include "../program/settings.h"

SettingsDialog::SettingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);
    setInfoTexts();

    connect(ui->restoreDefaultsButton, SIGNAL(clicked()), this, SLOT(restoreDefaults()));
    connect(ui->uniformPositiveNodeColourButton, SIGNAL(clicked()), this, SLOT(uniformPositiveNodeColourClicked()));
    connect(ui->uniformNegativeNodeColourButton, SIGNAL(clicked()), this, SLOT(uniformNegativeNodeColourClicked()));
    connect(ui->uniformNodeSpecialColourButton, SIGNAL(clicked()), this, SLOT(uniformNodeSpecialColourClicked()));
    connect(ui->lowCoverageColourButton, SIGNAL(clicked()), this, SLOT(lowCoverageColourClicked()));
    connect(ui->highCoverageColourButton, SIGNAL(clicked()), this, SLOT(highCoverageColourClicked()));
    connect(ui->selectionColourButton, SIGNAL(clicked()), this, SLOT(selectionColourClicked()));
    connect(ui->coverageValueManualRadioButton, SIGNAL(toggled(bool)), this, SLOT(enableDisableCoverageValueSpinBoxes()));
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

    setButtonColours();
    enableDisableCoverageValueSpinBoxes();
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
    doubleFunctionPointer(&settings->lowCoverageValue, ui->lowCoverageValueSpinBox, false);
    doubleFunctionPointer(&settings->highCoverageValue, ui->highCoverageValueSpinBox, false);

    //A couple of settings are not in a spin box, so they
    //have to be done manually, not with those function pointers.
    if (setWidgets)
    {
        ui->graphLayoutQualitySlider->setValue(settings->graphLayoutQuality);
        ui->antialiasingOnRadioButton->setChecked(settings->antialiasing);
        ui->antialiasingOffRadioButton->setChecked(!settings->antialiasing);
        ui->coverageValueAutoRadioButton->setChecked(settings->autoCoverageValue);
        ui->coverageValueManualRadioButton->setChecked(!settings->autoCoverageValue);
    }
    else
    {
        settings->graphLayoutQuality = ui->graphLayoutQualitySlider->value();
        settings->antialiasing = ui->antialiasingOnRadioButton->isChecked();
        settings->autoCoverageValue = ui->coverageValueAutoRadioButton->isChecked();
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


void SettingsDialog::setInfoTexts()
{
    ui->basePairsPerSegmentInfoText->setInfoText("This controls the length of the drawn nodes. The number of line segments "
                                                 "that make up a drawn node is determined by dividing the sequence length by "
                                                 "this value and rounding up. Any node with a sequence length of less than or "
                                                 "equal to this value will be drawn with a single line segment.<br><br>"
                                                 "Guidelines for this setting:<ul>"
                                                 "<li>Large values will result in shorter nodes. Very large values will result "
                                                 "in all nodes being a similiar size (one line segment).  This can make the graph "
                                                 "layout faster for large assembly graphs.</li>"
                                                 "<li>Small values will result in longer nodes and a stronger correlation between "
                                                 "sequence length and node length.  Longer nodes take longer to lay out, so this "
                                                 "can slow the graph layout process.</li></ul>");
    ui->graphLayoutQualityInfoText->setInfoText("This setting controls how much time the graph layout algorithm spends on "
                                                "positioning the graph components. Low settings are faster and "
                                                "recommended for big assembly graphs. Higher settings may result in smoother, "
                                                "more pleasing layouts.");
    ui->minimumContigWidthInfoText->setInfoText("This is the minimum width for each node, regardless of the node's coverage.");
    ui->coverageContigWidthInfoText->setInfoText("This setting controls the additional width added to nodes based on their "
                                                 "coverage. If set to zero, all nodes will have the same width.");
    ui->maximumContigWidthInfoText->setInfoText("This setting limits the node width so very high coverge nodes do not appear "
                                                "excessively wide.");
    ui->edgeWidthInfoText->setInfoText("This is the width of the edges that connect nodes.");
    ui->outlineThicknessInfoText->setInfoText("This is the thickness of the black outline drawn around each node.");
    ui->textOutlineThicknessInfoText->setInfoText("This is the thickness of the white outline drawn around node labels "
                                                  "when the 'Text outline' option is ticked.<br><br>"
                                                  "Large values can make the text easier to read, but may "
                                                  "obscure more of the graph.");
    ui->antialiasingInfoText->setInfoText("Antialiasing makes the display smoother and more pleasing. Disable antialiasing "
                                          "if you are experiencing slow performance when viewing large graphs.");
    ui->uniformPositiveNodeColourInfoText->setInfoText("This is the colour of all positive nodes when Bandage is set to the "
                                                       "'Uniform colour' option.");
    ui->uniformNegativeNodeColourInfoText->setInfoText("This is the colour of all negative nodes when Bandage is set to the "
                                                       "'Uniform colour' option. Negative nodes are only displayed when the "
                                                       "graph is drawn in 'Double' mode.");
    ui->uniformNodeSpecialColourInfoText->setInfoText("When Bandage is set to the 'Uniform colour' option, this colour is "
                                                      "used for limited graph scopes:<ul>"
                                                      "<li>When the graph scope is set to 'Around node(s)', this colour is "
                                                      "used for the user-specified nodes.</li>"
                                                      "<li>When the graph scope is set to 'Around BLAST hit(s)', this colour is "
                                                      "used for nodes that contain at least one BLAST hit.</li></ul>");
    ui->selectionColourInfoText->setInfoText("This colour is used to outline nodes that are currently selected by the user. "
                                             "Selected edges will also be displayed in this colour.");
    ui->lowCoverageColourInfoText->setInfoText("When Bandage is set to the 'Colour by coverage' option, this colour is used for "
                                               "nodes with a coverage at or below the low coverage value.<br><br>"
                                               "Nodes with a coverage between the low and high coverage values will get an "
                                               "intermediate colour.");
    ui->highCoverageColourInfoText->setInfoText("When Bandage is set to the 'Colour by coverage' option, this colour is used for "
                                                "nodes with a coverage above the high coverage value.<br><br>"
                                                "Nodes with a coverage between the low and high coverage values will get an "
                                                "intermediate colour.");
    ui->coverageValuesInfoText->setInfoText("When set to 'Auto', the low coverage value is set to the first quartile and the high "
                                            "coverage value is set to the third quartile.<br><br>"
                                            "When set to 'Manual', you can specify the values used for coverage colouring.");
}


void SettingsDialog::enableDisableCoverageValueSpinBoxes()
{
    bool enable = ui->coverageValueManualRadioButton->isChecked();

    ui->lowCoverageValueLabel->setEnabled(enable);
    ui->highCoverageValueLabel->setEnabled(enable);
    ui->lowCoverageValueSpinBox->setEnabled(enable);
    ui->highCoverageValueSpinBox->setEnabled(enable);
}


void SettingsDialog::accept()
{
    if (ui->lowCoverageValueSpinBox->value() > ui->highCoverageValueSpinBox->value())
        QMessageBox::warning(this, "Coverage value error", "The low coverage value cannot be greater than the high coverage value.");
    else
        QDialog::accept();
}

