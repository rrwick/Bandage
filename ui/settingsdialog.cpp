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
#include "colourbutton.h"

SettingsDialog::SettingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);
    setInfoTexts();

    ui->edgeColourButton->m_name = "Edge colour";
    ui->outlineColourButton->m_name = "Outline colour";
    ui->selectionColourButton->m_name = "Selection colour";
    ui->textColourButton->m_name = "Text colour";
    ui->textOutlineColourButton->m_name = "Text outline colour";
    ui->uniformPositiveNodeColourButton->m_name = "Uniform positive node colour";
    ui->uniformNegativeNodeColourButton->m_name = "Uniform negative node colour";
    ui->uniformNodeSpecialColourButton->m_name = "Uniform special node colour";
    ui->lowCoverageColourButton->m_name = "Low coverage colour";
    ui->highCoverageColourButton->m_name = "High coverage colour";
    ui->noBlastHitsColourButton->m_name = "No BLAST hits colour";
    ui->contiguousStrandSpecificColourButton->m_name = "Contiguous (strand-specific) colour";
    ui->contiguousEitherStrandColourButton->m_name = "Contiguous (either strand) colour";
    ui->maybeContiguousColourButton->m_name = "Maybe contiguous colour";
    ui->notContiguousColourButton->m_name = "Not contiguous colour";
    ui->contiguityStartingColourButton->m_name = "Contiguity starting colour";

    connect(ui->restoreDefaultsButton, SIGNAL(clicked()), this, SLOT(restoreDefaults()));
    connect(ui->coverageValueManualRadioButton, SIGNAL(toggled(bool)), this, SLOT(enableDisableCoverageValueSpinBoxes()));
    connect(ui->basePairsPerSegmentManualRadioButton, SIGNAL(toggled(bool)), this, SLOT(basePairsPerSegmentManualChanged()));
    connect(ui->coveragePowerSpinBox, SIGNAL(valueChanged(double)), this, SLOT(updateNodeWidthVisualAid()));
    connect(ui->coverageEffectOnWidthSpinBox, SIGNAL(valueChanged(double)), this, SLOT(updateNodeWidthVisualAid()));
    connect(ui->averageNodeWidthSpinBox, SIGNAL(valueChanged(double)), this, SLOT(updateNodeWidthVisualAid()));
    connect(ui->randomColourPositiveOpacitySpinBox, SIGNAL(valueChanged(int)), this, SLOT(colourSpinBoxChanged()));
    connect(ui->randomColourNegativeOpacitySpinBox, SIGNAL(valueChanged(int)), this, SLOT(colourSpinBoxChanged()));
    connect(ui->randomColourPositiveSaturationSpinBox, SIGNAL(valueChanged(int)), this, SLOT(colourSpinBoxChanged()));
    connect(ui->randomColourNegativeSaturationSpinBox, SIGNAL(valueChanged(int)), this, SLOT(colourSpinBoxChanged()));
    connect(ui->randomColourPositiveLightnessSpinBox, SIGNAL(valueChanged(int)), this, SLOT(colourSpinBoxChanged()));
    connect(ui->randomColourNegativeLightnessSpinBox, SIGNAL(valueChanged(int)), this, SLOT(colourSpinBoxChanged()));
    connect(ui->randomColourPositiveOpacitySlider, SIGNAL(valueChanged(int)), this, SLOT(colourSliderChanged()));
    connect(ui->randomColourNegativeOpacitySlider, SIGNAL(valueChanged(int)), this, SLOT(colourSliderChanged()));
    connect(ui->randomColourPositiveSaturationSlider, SIGNAL(valueChanged(int)), this, SLOT(colourSliderChanged()));
    connect(ui->randomColourNegativeSaturationSlider, SIGNAL(valueChanged(int)), this, SLOT(colourSliderChanged()));
    connect(ui->randomColourPositiveLightnessSlider, SIGNAL(valueChanged(int)), this, SLOT(colourSliderChanged()));
    connect(ui->randomColourNegativeLightnessSlider, SIGNAL(valueChanged(int)), this, SLOT(colourSliderChanged()));
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}


//These functions either set a widget to a value or set the value to the widget.  Pointers to
//these functions will be passed to loadOrSaveSettingsToOrFromWidgets, so that one function can
//take care of both save and load functionality.
void setOneSettingFromWidget(double * setting, QDoubleSpinBox * widget, bool percentage) {*setting = widget->value() / (percentage ? 100.0 : 1.0);}
void setOneSettingFromWidget(int * setting, QSpinBox * widget) {*setting = widget->value();}
void setOneSettingFromWidget(QColor * setting, ColourButton * widget) {*setting = widget->m_colour;}
void setOneWidgetFromSetting(double * setting, QDoubleSpinBox * widget, bool percentage) {widget->setValue(*setting * (percentage ? 100.0 : 1.0));}
void setOneWidgetFromSetting(int * setting, QSpinBox * widget) {widget->setValue(*setting);}
void setOneWidgetFromSetting(QColor * setting, ColourButton * widget) {widget->setColour(*setting);}



void SettingsDialog::setWidgetsFromSettings()
{
    loadOrSaveSettingsToOrFromWidgets(true, g_settings);

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
    void (*colourFunctionPointer)(QColor *, ColourButton *);

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

    intFunctionPointer(&settings->manualBasePairsPerSegment, ui->basePairsPerSegmentSpinBox);
    doubleFunctionPointer(&settings->averageNodeWidth, ui->averageNodeWidthSpinBox, false);
    doubleFunctionPointer(&settings->coverageEffectOnWidth, ui->coverageEffectOnWidthSpinBox, true);
    doubleFunctionPointer(&settings->coveragePower, ui->coveragePowerSpinBox, false);
    doubleFunctionPointer(&settings->edgeWidth, ui->edgeWidthSpinBox, false);
    doubleFunctionPointer(&settings->outlineThickness, ui->outlineThicknessSpinBox, false);
    doubleFunctionPointer(&settings->textOutlineThickness, ui->textOutlineThicknessSpinBox, false);
    colourFunctionPointer(&settings->edgeColour, ui->edgeColourButton);
    colourFunctionPointer(&settings->outlineColour, ui->outlineColourButton);
    colourFunctionPointer(&settings->selectionColour, ui->selectionColourButton);
    colourFunctionPointer(&settings->textColour, ui->textColourButton);
    colourFunctionPointer(&settings->textOutlineColour, ui->textOutlineColourButton);
    intFunctionPointer(&settings->randomColourPositiveSaturation, ui->randomColourPositiveSaturationSpinBox);
    intFunctionPointer(&settings->randomColourNegativeSaturation, ui->randomColourNegativeSaturationSpinBox);
    intFunctionPointer(&settings->randomColourPositiveLightness, ui->randomColourPositiveLightnessSpinBox);
    intFunctionPointer(&settings->randomColourNegativeLightness, ui->randomColourNegativeLightnessSpinBox);
    intFunctionPointer(&settings->randomColourPositiveOpacity, ui->randomColourPositiveOpacitySpinBox);
    intFunctionPointer(&settings->randomColourNegativeOpacity, ui->randomColourNegativeOpacitySpinBox);
    colourFunctionPointer(&settings->uniformPositiveNodeColour, ui->uniformPositiveNodeColourButton);
    colourFunctionPointer(&settings->uniformNegativeNodeColour, ui->uniformNegativeNodeColourButton);
    colourFunctionPointer(&settings->uniformNodeSpecialColour, ui->uniformNodeSpecialColourButton);
    colourFunctionPointer(&settings->lowCoverageColour, ui->lowCoverageColourButton);
    colourFunctionPointer(&settings->highCoverageColour, ui->highCoverageColourButton);
    doubleFunctionPointer(&settings->lowCoverageValue, ui->lowCoverageValueSpinBox, false);
    doubleFunctionPointer(&settings->highCoverageValue, ui->highCoverageValueSpinBox, false);
    colourFunctionPointer(&settings->noBlastHitsColour, ui->noBlastHitsColourButton);
    intFunctionPointer(&settings->contiguitySearchSteps, ui->contiguitySearchDepthSpinBox);
    colourFunctionPointer(&settings->contiguousStrandSpecificColour, ui->contiguousStrandSpecificColourButton);
    colourFunctionPointer(&settings->contiguousEitherStrandColour, ui->contiguousEitherStrandColourButton);
    colourFunctionPointer(&settings->maybeContiguousColour, ui->maybeContiguousColourButton);
    colourFunctionPointer(&settings->notContiguousColour, ui->notContiguousColourButton);
    colourFunctionPointer(&settings->contiguityStartingColour, ui->contiguityStartingColourButton);

    //A couple of settings are not in a spin box, so they
    //have to be done manually, not with those function pointers.
    if (setWidgets)
    {
        ui->graphLayoutQualitySlider->setValue(settings->graphLayoutQuality);
        ui->antialiasingOnRadioButton->setChecked(settings->antialiasing);
        ui->antialiasingOffRadioButton->setChecked(!settings->antialiasing);
        ui->coverageValueAutoRadioButton->setChecked(settings->autoCoverageValue);
        ui->coverageValueManualRadioButton->setChecked(!settings->autoCoverageValue);
        basePairsPerSegmentManualChanged();
        ui->basePairsPerSegmentAutoLabel->setText(QString::number(settings->autoBasePairsPerSegment));
        ui->basePairsPerSegmentAutoRadioButton->setChecked(settings->nodeLengthMode == AUTO_NODE_LENGTH);
        ui->basePairsPerSegmentManualRadioButton->setChecked(settings->nodeLengthMode != AUTO_NODE_LENGTH);
        ui->positionVisibleRadioButton->setChecked(!settings->positionTextNodeCentre);
        ui->positionCentreRadioButton->setChecked(settings->positionTextNodeCentre);
    }
    else
    {
        settings->graphLayoutQuality = ui->graphLayoutQualitySlider->value();
        settings->antialiasing = ui->antialiasingOnRadioButton->isChecked();
        settings->autoCoverageValue = ui->coverageValueAutoRadioButton->isChecked();
        if (ui->basePairsPerSegmentAutoRadioButton->isChecked())
            settings->nodeLengthMode = AUTO_NODE_LENGTH;
        else
            settings->nodeLengthMode = MANUAL_NODE_LENGTH;
        settings->positionTextNodeCentre = ui->positionCentreRadioButton->isChecked();
    }
}


void SettingsDialog::restoreDefaults()
{
    Settings defaultSettings;

    //The auto base pairs per segment is the only setting we don't want to
    //restore, as it is calculated from the graph.
    defaultSettings.autoBasePairsPerSegment = g_settings->autoBasePairsPerSegment;

    loadOrSaveSettingsToOrFromWidgets(true, &defaultSettings);
}


void SettingsDialog::setInfoTexts()
{
    ui->basePairsPerSegmentInfoText->setInfoText("This controls the length of the drawn nodes. The number of line segments "
                                                 "that make up a drawn node is determined by dividing the sequence length by "
                                                 "this value and rounding up. Any node with a sequence length of less than or "
                                                 "equal to this value will be drawn with a single line segment.<br><br>"
                                                 "Guidelines for this setting:<ul>"
                                                 "<li>Large values will result in shorter nodes. Very large values will result "
                                                 "in all nodes being a similar size (one line segment). This can make the graph "
                                                 "layout faster for large assembly graphs.</li>"
                                                 "<li>Small values will result in longer nodes and a stronger correlation between "
                                                 "sequence length and node length. Longer nodes can slow the graph layout "
                                                 "process.</li></ul><br>"
                                                 "When a graph is loaded, Bandage calculates an appropriate value and uses this "
                                                 "for the 'Auto' option. Switch to 'Manual' if you want to specify this setting "
                                                 "yourself.");
    ui->graphLayoutQualityInfoText->setInfoText("This setting controls how much time the graph layout algorithm spends on "
                                                "positioning the graph components.<br><br>Low settings are faster and "
                                                "recommended for big assembly graphs. Higher settings may result in smoother, "
                                                "more pleasing layouts.");
    ui->averageNodeWidthInfoText->setInfoText("This is the minimum width for each node, regardless of the node's coverage.");
    ui->coveragePowerInfoText->setInfoText("This is the power used in the function for determining node widths.");
    ui->coverageEffectOnWidthInfoText->setInfoText("This setting controls the degree to which a node's coverage affects its width.<br><br>"
                                                   "If set to 0%, all nodes will have the same width (equal to the average "
                                                   "node width).");
    ui->nodeWidthPlotInfoText->setInfoText("This is a plot of the function that will be used to set node widths.<br><br>"
                                           "The centre point on the plot is the average coverage and average node width.");
    ui->edgeWidthInfoText->setInfoText("This is the width of the edges that connect nodes.");
    ui->outlineThicknessInfoText->setInfoText("This is the thickness of the outline drawn around each node.<br><br>"
                                              "Drawing outlines can result in slow performance, so set this to zero "
                                              "to improve performance.");
    ui->textOutlineThicknessInfoText->setInfoText("This is the thickness of the outline drawn around node labels "
                                                  "when the 'Text outline' option is ticked.<br><br>"
                                                  "Large values can make text easier to read, but may "
                                                  "obscure more of the graph.");
    ui->positionTextInfoText->setInfoText("When 'Over visible regions' is selected, node labels will move based on the position "
                                          "of the view to stay visible.<br><br>"
                                          "When 'On node centre' is selected, node labels will always be displayed at the centre "
                                          "of each node, regardless of the view's position.");

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
    ui->edgeColourInfoText->setInfoText("This colour is used for all edges connecting nodes.");
    ui->outlineColourInfoText->setInfoText("This colour is used to outline nodes that are not currently selected by the user.");
    ui->selectionColourInfoText->setInfoText("This colour is used to outline nodes that are currently selected by the user. "
                                             "Selected edges will also be displayed in this colour.");
    ui->textColourInfoText->setInfoText("This colour is used for the text of node labels.");
    ui->textOutlineColourInfoText->setInfoText("If the text outline thickness setting has a nonzero value, then a text outline "
                                               "will be displayed with this colour.");

    ui->randomColourPositiveSaturationInfoText->setInfoText("This controls the colour saturation of the positive nodes when the "
                                                            "'Random colours' option is used.<br><br>"
                                                            "The minimum value will result in colourless (grey) nodes "
                                                            "while high values will result in brightly coloured nodes.");
    ui->randomColourNegativeSaturationInfoText->setInfoText("This controls the colour saturation of the negative nodes when the "
                                                            "'Random colours' option is used.<br><br>"
                                                            "The minimum value will result in colourless (grey) nodes "
                                                            "while high values will result in brightly coloured nodes.<br><br>"
                                                            "Note that negative nodes are only visible when the graph is drawn "
                                                            "in double mode.");
    ui->randomColourPositiveLightnessInfoText->setInfoText("This controls the colour lightness of the positive nodes when the "
                                                           "'Random colours' option is used.<br><br>"
                                                           "Low values will result in dark nodes while high values result in "
                                                           "light nodes");
    ui->randomColourNegativeLightnessInfoText->setInfoText("This controls the colour lightness of the negative nodes when the "
                                                           "'Random colours' option is used.<br><br>"
                                                           "Low values will result in dark nodes while high values result in "
                                                           "light nodes.<br><br>"
                                                           "Note that negative nodes are only visible when the graph is drawn "
                                                           "in double mode.");
    ui->randomColourPositiveOpacityInfoText->setInfoText("This controls how opaque the positive nodes are when the 'Random colours' "
                                                         "option is used.<br><br>"
                                                         "Set to the minimum value for fully transparent nodes. Set to the "
                                                         "maximum value for completely opaque nodes.");
    ui->randomColourNegativeOpacityInfoText->setInfoText("This controls how opaque the negative nodes are when the 'Random colours' "
                                                         "option is used.<br><br>"
                                                         "Set to the minimum value for fully transparent nodes. Set to the "
                                                         "maximum value for completely opaque nodes.<br><br>"
                                                         "Note that negative nodes are only visible when the graph is drawn "
                                                         "in double mode.");

    ui->lowCoverageColourInfoText->setInfoText("When Bandage is set to the 'Colour by coverage' option, this colour is used for "
                                               "nodes with coverage at or below the low coverage value.<br><br>"
                                               "Nodes with coverage between the low and high coverage values will get an "
                                               "intermediate colour.");
    ui->highCoverageColourInfoText->setInfoText("When Bandage is set to the 'Colour by coverage' option, this colour is used for "
                                                "nodes with coverage above the high coverage value.<br><br>"
                                                "Nodes with coverage between the low and high coverage values will get an "
                                                "intermediate colour.");
    ui->coverageValuesInfoText->setInfoText("When set to 'Auto', the low coverage value is set to the first quartile and the high "
                                            "coverage value is set to the third quartile.<br><br>"
                                            "When set to 'Manual', you can specify the values used for coverage colouring.");
    ui->noBlastHitsColourInfoText->setInfoText("When Bandage is set to the 'Colour using BLAST hits' option, this colour is "
                                               "used for nodes that do not have any BLAST hits. It is also used for any region "
                                               "of a node without BLAST hits, even if there are BLAST hits in other regions of "
                                               "that node.");
    ui->contiguitySearchDepthInfoText->setInfoText("This is the number of steps the contiguity search will take. Larger "
                                                   "values will find more distant contiguous nodes, at a performance cost.<br><br>"
                                                   "The time taken to complete the search can grow rapidly as values increase, "
                                                   "so use values above 20 with caution.");
    ui->contiguousStrandSpecificColourInfoText->setInfoText("When a contiguity search is conducted, this is the colour given to "
                                                            "nodes that are determined to be contiguous with the starting "
                                                            "node(s).<br><br>"
                                                            "This colour is used for strand-specific matches. It is only used "
                                                            "for nodes that are determined to be on the same strand as the "
                                                            "starting node");
    ui->contiguousEitherStrandColourInfoText->setInfoText("When a contiguity search is conducted, this is the colour given to "
                                                          "nodes that are determined to be contiguous with the starting "
                                                          "node(s).<br><br>"
                                                          "This colour is used for nodes where either the node or its reverse "
                                                          "complement are contiguous with the starting node, but it cannot be "
                                                          "determined which.");
    ui->maybeContiguousColourInfoText->setInfoText("When a contiguity search is conducted, this is the colour given to nodes that "
                                                   "are determined to be possibly contiguous with the starting node(s).");
    ui->notContiguousColourInfoText->setInfoText("When a contiguity search is conducted, this is the colour given to nodes that "
                                                 "are not determined to be contiguous with the starting node(s).");
    ui->contiguityStartingColourInfoText->setInfoText("When a contiguity search is conducted, this is the colour given to the "
                                                      "starting node(s).");
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


void SettingsDialog::basePairsPerSegmentManualChanged()
{
    bool manual = ui->basePairsPerSegmentManualRadioButton->isChecked();
    ui->basePairsPerSegmentSpinBox->setEnabled(manual);
    ui->basePairsPerSegmentAutoLabel->setEnabled(!manual);
}


void SettingsDialog::updateNodeWidthVisualAid()
{
    ui->nodeWidthVisualAid->m_averageNodeWidth = ui->averageNodeWidthSpinBox->value();
    ui->nodeWidthVisualAid->m_coverageEffectOnWidth = ui->coverageEffectOnWidthSpinBox->value() / 100.0;
    ui->nodeWidthVisualAid->m_coveragePower = ui->coveragePowerSpinBox->value();

    ui->nodeWidthVisualAid->update();
}


void SettingsDialog::colourSliderChanged()
{
    ui->randomColourPositiveOpacitySpinBox->setValue(ui->randomColourPositiveOpacitySlider->value());
    ui->randomColourNegativeOpacitySpinBox->setValue(ui->randomColourNegativeOpacitySlider->value());
    ui->randomColourPositiveSaturationSpinBox->setValue(ui->randomColourPositiveSaturationSlider->value());
    ui->randomColourNegativeSaturationSpinBox->setValue(ui->randomColourNegativeSaturationSlider->value());
    ui->randomColourPositiveLightnessSpinBox->setValue(ui->randomColourPositiveLightnessSlider->value());
    ui->randomColourNegativeLightnessSpinBox->setValue(ui->randomColourNegativeLightnessSlider->value());
}
void SettingsDialog::colourSpinBoxChanged()
{
    ui->randomColourPositiveOpacitySlider->setValue(ui->randomColourPositiveOpacitySpinBox->value());
    ui->randomColourNegativeOpacitySlider->setValue(ui->randomColourNegativeOpacitySpinBox->value());
    ui->randomColourPositiveSaturationSlider->setValue(ui->randomColourPositiveSaturationSpinBox->value());
    ui->randomColourNegativeSaturationSlider->setValue(ui->randomColourNegativeSaturationSpinBox->value());
    ui->randomColourPositiveLightnessSlider->setValue(ui->randomColourPositiveLightnessSpinBox->value());
    ui->randomColourNegativeLightnessSlider->setValue(ui->randomColourNegativeLightnessSpinBox->value());
}
