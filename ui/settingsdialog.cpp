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
#include "../graph/assemblygraph.h"
#include "../program/scinot.h"

SettingsDialog::SettingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);
    setInfoTexts();

    //On a PC and Linux, this window is called 'Settings', but on the Mac it is
    //called 'Preferences'.
    QString windowTitle = "Settings";
#ifdef Q_OS_MAC
    windowTitle = "Preferences";
#endif
    setWindowTitle(windowTitle);

    ui->edgeColourButton->m_name = "Edge colour";
    ui->outlineColourButton->m_name = "Outline colour";
    ui->selectionColourButton->m_name = "Selection colour";
    ui->textColourButton->m_name = "Text colour";
    ui->textOutlineColourButton->m_name = "Text outline colour";
    ui->uniformPositiveNodeColourButton->m_name = "Uniform positive node colour";
    ui->uniformNegativeNodeColourButton->m_name = "Uniform negative node colour";
    ui->uniformNodeSpecialColourButton->m_name = "Uniform special node colour";
    ui->lowReadDepthColourButton->m_name = "Low read depth colour";
    ui->highReadDepthColourButton->m_name = "High read depth colour";
    ui->noBlastHitsColourButton->m_name = "No BLAST hits colour";
    ui->contiguousStrandSpecificColourButton->m_name = "Contiguous (strand-specific) colour";
    ui->contiguousEitherStrandColourButton->m_name = "Contiguous (either strand) colour";
    ui->maybeContiguousColourButton->m_name = "Maybe contiguous colour";
    ui->notContiguousColourButton->m_name = "Not contiguous colour";
    ui->contiguityStartingColourButton->m_name = "Contiguity starting colour";

    connect(ui->restoreDefaultsButton, SIGNAL(clicked()), this, SLOT(restoreDefaults()));
    connect(ui->readDepthValueManualRadioButton, SIGNAL(toggled(bool)), this, SLOT(enableDisableReadDepthWidgets()));
    connect(ui->basePairsPerSegmentManualRadioButton, SIGNAL(toggled(bool)), this, SLOT(basePairsPerSegmentManualChanged()));
    connect(ui->readDepthPowerSpinBox, SIGNAL(valueChanged(double)), this, SLOT(updateNodeWidthVisualAid()));
    connect(ui->readDepthEffectOnWidthSpinBox, SIGNAL(valueChanged(double)), this, SLOT(updateNodeWidthVisualAid()));
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
    connect(ui->minQueryCoveredByHitsCheckBox, SIGNAL(toggled(bool)), this, SLOT(checkBoxesChanged()));
    connect(ui->minMeanHitIdentityCheckBox, SIGNAL(toggled(bool)), this, SLOT(checkBoxesChanged()));
    connect(ui->maxEValueProductCheckBox, SIGNAL(toggled(bool)), this, SLOT(checkBoxesChanged()));
    connect(ui->minLengthPercentageCheckBox, SIGNAL(toggled(bool)), this, SLOT(checkBoxesChanged()));
    connect(ui->maxLengthPercentageCheckBox, SIGNAL(toggled(bool)), this, SLOT(checkBoxesChanged()));
    connect(ui->minLengthBaseDiscrepancyCheckBox, SIGNAL(toggled(bool)), this, SLOT(checkBoxesChanged()));
    connect(ui->maxLengthBaseDiscrepancyCheckBox, SIGNAL(toggled(bool)), this, SLOT(checkBoxesChanged()));
    connect(ui->minLengthBaseDiscrepancySpinBox, SIGNAL(valueChanged(int)), this, SLOT(lengthDiscrepancySpinBoxChanged()));
    connect(ui->maxLengthBaseDiscrepancySpinBox, SIGNAL(valueChanged(int)), this, SLOT(lengthDiscrepancySpinBoxChanged()));
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}


//These functions either set a widget to a value or set the value to the widget.  Pointers to
//these functions will be passed to loadOrSaveSettingsToOrFromWidgets, so that one function can
//take care of both save and load functionality.
void setOneSettingFromWidget(double * setting, QDoubleSpinBox * spinBox, bool percentage) {*setting = spinBox->value() / (percentage ? 100.0 : 1.0);}
void setOneSettingFromWidget(int * setting, QSpinBox * spinBox) {*setting = spinBox->value();}
void setOneSettingFromWidget(QColor * setting, ColourButton * button) {*setting = button->m_colour;}
void setOneSettingFromWidget(SciNot * setting, QDoubleSpinBox * coefficientSpinBox, QSpinBox * exponentSpinBox) {*setting = SciNot(coefficientSpinBox->value(), exponentSpinBox->value());}
void setOneSettingFromWidget(bool * setting, QCheckBox * checkBox) {*setting = checkBox->isChecked();}
void setOneWidgetFromSetting(double * setting, QDoubleSpinBox * spinBox, bool percentage) {spinBox->setValue(*setting * (percentage ? 100.0 : 1.0));}
void setOneWidgetFromSetting(int * setting, QSpinBox * spinBox) {spinBox->setValue(*setting);}
void setOneWidgetFromSetting(QColor * setting, ColourButton * button) {button->setColour(*setting);}
void setOneWidgetFromSetting(SciNot * setting, QDoubleSpinBox * coefficientSpinBox, QSpinBox * exponentSpinBox) {coefficientSpinBox->setValue(setting->getCoefficient()); exponentSpinBox->setValue(setting->getExponent());}
void setOneWidgetFromSetting(bool * setting, QCheckBox * checkBox) {checkBox->setChecked(*setting);}


void SettingsDialog::setWidgetsFromSettings()
{
    loadOrSaveSettingsToOrFromWidgets(true, g_settings.data());

    enableDisableReadDepthWidgets();
    checkBoxesChanged();
    lengthDiscrepancySpinBoxChanged();
}


void SettingsDialog::setSettingsFromWidgets()
{
    loadOrSaveSettingsToOrFromWidgets(false, g_settings.data());

    //The highlighted outline thickness should always be a bit
    //bigger than the outlineThickness.
    g_settings->selectionThickness = g_settings->outlineThickness + 1.0;
}



void SettingsDialog::loadOrSaveSettingsToOrFromWidgets(bool setWidgets, Settings * settings)
{
    void (*doubleFunctionPointer)(double *, QDoubleSpinBox *, bool);
    void (*intFunctionPointer)(int *, QSpinBox *);
    void (*colourFunctionPointer)(QColor *, ColourButton *);
    void (*sciNotFunctionPointer)(SciNot *, QDoubleSpinBox *, QSpinBox *);
    void (*checkBoxFunctionPointer)(bool *, QCheckBox *);

    if (setWidgets)
    {
        doubleFunctionPointer = setOneWidgetFromSetting;
        intFunctionPointer = setOneWidgetFromSetting;
        colourFunctionPointer = setOneWidgetFromSetting;
        sciNotFunctionPointer = setOneWidgetFromSetting;
        checkBoxFunctionPointer = setOneWidgetFromSetting;
    }
    else
    {
        doubleFunctionPointer = setOneSettingFromWidget;
        intFunctionPointer = setOneSettingFromWidget;
        colourFunctionPointer = setOneSettingFromWidget;
        sciNotFunctionPointer = setOneSettingFromWidget;
        checkBoxFunctionPointer = setOneSettingFromWidget;
    }

    intFunctionPointer(&settings->manualBasePairsPerSegment, ui->basePairsPerSegmentSpinBox);
    doubleFunctionPointer(&settings->readDepthEffectOnWidth, ui->readDepthEffectOnWidthSpinBox, true);
    doubleFunctionPointer(&settings->readDepthPower, ui->readDepthPowerSpinBox, false);
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
    colourFunctionPointer(&settings->lowReadDepthColour, ui->lowReadDepthColourButton);
    colourFunctionPointer(&settings->highReadDepthColour, ui->highReadDepthColourButton);
    doubleFunctionPointer(&settings->lowReadDepthValue, ui->lowReadDepthValueSpinBox, false);
    doubleFunctionPointer(&settings->highReadDepthValue, ui->highReadDepthValueSpinBox, false);
    colourFunctionPointer(&settings->noBlastHitsColour, ui->noBlastHitsColourButton);
    intFunctionPointer(&settings->contiguitySearchSteps, ui->contiguitySearchDepthSpinBox);
    colourFunctionPointer(&settings->contiguousStrandSpecificColour, ui->contiguousStrandSpecificColourButton);
    colourFunctionPointer(&settings->contiguousEitherStrandColour, ui->contiguousEitherStrandColourButton);
    colourFunctionPointer(&settings->maybeContiguousColour, ui->maybeContiguousColourButton);
    colourFunctionPointer(&settings->notContiguousColour, ui->notContiguousColourButton);
    colourFunctionPointer(&settings->contiguityStartingColour, ui->contiguityStartingColourButton);
    intFunctionPointer(&settings->maxQueryPathNodes, ui->maxPathNodesSpinBox);
    doubleFunctionPointer(&settings->minQueryCoveredByPath, ui->minQueryCoveredByPathSpinBox, true);
    checkBoxFunctionPointer(&settings->minQueryCoveredByHitsOn, ui->minQueryCoveredByHitsCheckBox);
    doubleFunctionPointer(&settings->minQueryCoveredByHits, ui->minQueryCoveredByHitsSpinBox, true);
    checkBoxFunctionPointer(&settings->minMeanHitIdentityOn, ui->minMeanHitIdentityCheckBox);
    doubleFunctionPointer(&settings->minMeanHitIdentity, ui->minMeanHitIdentitySpinBox, true);
    checkBoxFunctionPointer(&settings->maxEValueProductOn, ui->maxEValueProductCheckBox);
    sciNotFunctionPointer(&settings->maxEValueProduct, ui->maxEValueCoefficientSpinBox, ui->maxEValueExponentSpinBox);
    checkBoxFunctionPointer(&settings->minLengthPercentageOn, ui->minLengthPercentageCheckBox);
    doubleFunctionPointer(&settings->minLengthPercentage, ui->minLengthPercentageSpinBox, true);
    checkBoxFunctionPointer(&settings->maxLengthPercentageOn, ui->maxLengthPercentageCheckBox);
    doubleFunctionPointer(&settings->maxLengthPercentage, ui->maxLengthPercentageSpinBox, true);
    checkBoxFunctionPointer(&settings->minLengthBaseDiscrepancyOn, ui->minLengthBaseDiscrepancyCheckBox);
    intFunctionPointer(&settings->minLengthBaseDiscrepancy, ui->minLengthBaseDiscrepancySpinBox);
    checkBoxFunctionPointer(&settings->maxLengthBaseDiscrepancyOn, ui->maxLengthBaseDiscrepancyCheckBox);
    intFunctionPointer(&settings->maxLengthBaseDiscrepancy, ui->maxLengthBaseDiscrepancySpinBox);

    //A couple of settings are not in a spin box, check box or colour button, so
    //they have to be done manually, not with those function pointers.
    if (setWidgets)
    {
        ui->graphLayoutQualitySlider->setValue(settings->graphLayoutQuality);
        ui->antialiasingOnRadioButton->setChecked(settings->antialiasing);
        ui->antialiasingOffRadioButton->setChecked(!settings->antialiasing);
        ui->readDepthValueAutoRadioButton->setChecked(settings->autoReadDepthValue);
        ui->readDepthValueManualRadioButton->setChecked(!settings->autoReadDepthValue);
        basePairsPerSegmentManualChanged();
        ui->basePairsPerSegmentAutoLabel->setText(formatIntForDisplay(settings->autoBasePairsPerSegment));
        ui->lowReadDepthAutoValueLabel2->setText(formatDoubleForDisplay(g_assemblyGraph->m_firstQuartileReadDepth, 2));
        ui->highReadDepthAutoValueLabel2->setText(formatDoubleForDisplay(g_assemblyGraph->m_thirdQuartileReadDepth, 2));
        ui->basePairsPerSegmentAutoRadioButton->setChecked(settings->nodeLengthMode == AUTO_NODE_LENGTH);
        ui->basePairsPerSegmentManualRadioButton->setChecked(settings->nodeLengthMode != AUTO_NODE_LENGTH);
        ui->positionVisibleRadioButton->setChecked(!settings->positionTextNodeCentre);
        ui->positionCentreRadioButton->setChecked(settings->positionTextNodeCentre);
    }
    else
    {
        settings->graphLayoutQuality = ui->graphLayoutQualitySlider->value();
        settings->antialiasing = ui->antialiasingOnRadioButton->isChecked();
        settings->autoReadDepthValue = ui->readDepthValueAutoRadioButton->isChecked();
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
                                                "positioning the graph components.<br><br>Low values are faster and "
                                                "recommended for big assembly graphs. Higher values may result in smoother, "
                                                "more pleasing layouts.");
    ui->readDepthPowerInfoText->setInfoText("This is the power used in the function for determining node widths.");
    ui->readDepthEffectOnWidthInfoText->setInfoText("This setting controls the degree to which a node's read depth affects its width.<br><br>"
                                                   "If set to 0%, all nodes will have the same width (equal to the average "
                                                   "node width).");
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

    ui->lowReadDepthColourInfoText->setInfoText("When Bandage is set to the 'Colour by read depth' option, this colour is used for "
                                               "nodes with read depth at or below the low read depth value.<br><br>"
                                               "Nodes with read depth between the low and high read depth values will get an "
                                               "intermediate colour.");
    ui->highReadDepthColourInfoText->setInfoText("When Bandage is set to the 'Colour by read depth' option, this colour is used for "
                                                "nodes with read depth above the high read depth value.<br><br>"
                                                "Nodes with read depth between the low and high read depth values will get an "
                                                "intermediate colour.");
    ui->readDepthAutoValuesInfoText->setInfoText("When set to 'Auto', the low read depth value is set to the first quartile and the high "
                                                 "read depth value is set to the third quartile.");
    ui->readDepthManualValuesInfoText->setInfoText("When set to 'Manual', you can specify the values used for read depth colouring.");
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

    ui->maxPathNodesInfoText->setInfoText("This controls the maximum number of nodes in BLAST query paths.<br><br>"
                                          "A higher value will allow for paths containing more nodes, at a performance "
                                          "cost.");
    ui->minQueryCoveredByPathInfoText->setInfoText("This is a minimum value for the fraction of a BLAST query which "
                                                   "is covered by a BLAST query path. Paths that cover less of the "
                                                   "query than this setting will not be included in the query's "
                                                   "paths.<br><br>"
                                                   "Set to a higher value to make BLAST query paths more stringent.");
    ui->minQueryCoveredByHitsInfoText->setInfoText("This is a minimum value for the fraction of a BLAST query which "
                                                   "is covered by the hits in a BLAST query path. Paths with hits that "
                                                   "cover less of the query than this setting will not be included in "
                                                   "the query's paths.<br><br>"
                                                   "Set to a higher value to make BLAST query paths more stringent.");
    ui->minMeanHitIdentityInfoText->setInfoText("This is a minimum value for the mean of the percent identity for the "
                                                "hits in a BLAST query path, weighted by the hits' lengths. Paths with "
                                                "a mean hit percent identity less than this setting will not be included "
                                                "in the query's paths.<br><br>"
                                                "Set to a higher value to make BLAST query paths more stringent.");
    ui->maxEvalueProductInfoText->setInfoText("This is a maximum value for the product of the e-values of all the "
                                              "hits in a BLAST query path. Paths with an e-value product greater "
                                              "than this setting will not be included in the query's paths.<br><br>"
                                              "Set to a lower value to make BLAST query paths more stringent.");
    ui->minLengthPercentageInfoText->setInfoText("This is a minimum value for the relative length between a BLAST "
                                                 "query and its path in the graph. Paths with a relative length "
                                                 "less than this setting will not be included in the query's "
                                                 "paths.<br><br>"
                                                 "Set to a value closer to 100% to make BLAST query paths more stringent.");
    ui->maxLengthPercentageInfoText->setInfoText("This is a maximum value for the relative length between a BLAST "
                                                 "query and its path in the graph. Paths with a relative length "
                                                 "greater than this setting will not be included in the query's "
                                                 "paths.<br><br>"
                                                 "Set to a value closer to 100% to make BLAST query paths more stringent.");
    ui->minLengthBaseDiscrepancyInfoText->setInfoText("This is the minimum length difference (in bases) between a BLAST "
                                                      "query and its path in the graph.  Paths with a length difference "
                                                      "less than this setting will not be included in the query's "
                                                      "paths.<br><br>"
                                                      "Set to a value closer to 0 to make BLAST query paths more stringent.");
    ui->minLengthBaseDiscrepancyInfoText->setInfoText("This is the maximum length difference (in bases) between a BLAST "
                                                      "query and its path in the graph.  Paths with a length difference "
                                                      "greater than this setting will not be included in the query's "
                                                      "paths.<br><br>"
                                                      "Set to a value closer to 0 to make BLAST query paths more stringent.");
}


void SettingsDialog::enableDisableReadDepthWidgets()
{
    bool manual = ui->readDepthValueManualRadioButton->isChecked();

    ui->readDepthManualWidget->setEnabled(manual);
    ui->readDepthAutoWidget->setEnabled(!manual);
}


void SettingsDialog::accept()
{
    if (ui->lowReadDepthValueSpinBox->value() > ui->highReadDepthValueSpinBox->value())
        QMessageBox::warning(this, "Read depth value error", "The low read depth value cannot be greater than the high read depth value.");
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
    ui->nodeWidthVisualAid->m_readDepthEffectOnWidth = ui->readDepthEffectOnWidthSpinBox->value() / 100.0;
    ui->nodeWidthVisualAid->m_readDepthPower = ui->readDepthPowerSpinBox->value();

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

void SettingsDialog::checkBoxesChanged()
{
    ui->minQueryCoveredByHitsSpinBox->setEnabled(ui->minQueryCoveredByHitsCheckBox->isChecked());
    ui->minMeanHitIdentitySpinBox->setEnabled(ui->minMeanHitIdentityCheckBox->isChecked());
    ui->maxEValueCoefficientSpinBox->setEnabled(ui->maxEValueProductCheckBox->isChecked());
    ui->maxEValueExponentSpinBox->setEnabled(ui->maxEValueProductCheckBox->isChecked());
    ui->minLengthPercentageSpinBox->setEnabled(ui->minLengthPercentageCheckBox->isChecked());
    ui->maxLengthPercentageSpinBox->setEnabled(ui->maxLengthPercentageCheckBox->isChecked());
    ui->minLengthBaseDiscrepancySpinBox->setEnabled(ui->minLengthBaseDiscrepancyCheckBox->isChecked());
    ui->maxLengthBaseDiscrepancySpinBox->setEnabled(ui->maxLengthBaseDiscrepancyCheckBox->isChecked());
}

//This function adds or removes the '+' prefix from the length discrepancy
//spin boxes, based on whether or not they hold a positive value.
void SettingsDialog::lengthDiscrepancySpinBoxChanged()
{
    if (ui->minLengthBaseDiscrepancySpinBox->value() > 0)
        ui->minLengthBaseDiscrepancySpinBox->setPrefix("+");
    else
        ui->minLengthBaseDiscrepancySpinBox->setPrefix("");

    if (ui->maxLengthBaseDiscrepancySpinBox->value() > 0)
        ui->maxLengthBaseDiscrepancySpinBox->setPrefix("+");
    else
        ui->maxLengthBaseDiscrepancySpinBox->setPrefix("");
}
