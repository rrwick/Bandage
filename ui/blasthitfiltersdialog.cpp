#include "blasthitfiltersdialog.h"
#include "ui_blasthitfiltersdialog.h"

#include "../program/globals.h"
#include "../program/settings.h"

BlastHitFiltersDialog::BlastHitFiltersDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::BlastHitFiltersDialog)
{
    ui->setupUi(this);

    setInfoTexts();

    connect(ui->alignmentLengthCheckBox, SIGNAL(toggled(bool)), this, SLOT(checkBoxesChanged()));
    connect(ui->queryCoverageCheckBox, SIGNAL(toggled(bool)), this, SLOT(checkBoxesChanged()));
    connect(ui->identityCheckBox, SIGNAL(toggled(bool)), this, SLOT(checkBoxesChanged()));
    connect(ui->eValueCheckBox, SIGNAL(toggled(bool)), this, SLOT(checkBoxesChanged()));
    connect(ui->bitScoreCheckBox, SIGNAL(toggled(bool)), this, SLOT(checkBoxesChanged()));
}

BlastHitFiltersDialog::~BlastHitFiltersDialog()
{
    delete ui;
}


void BlastHitFiltersDialog::checkBoxesChanged()
{
    ui->alignmentLengthSpinBox->setEnabled(ui->alignmentLengthCheckBox->isChecked());

    ui->queryCoverageSpinBox->setEnabled(ui->queryCoverageCheckBox->isChecked());

    ui->identitySpinBox->setEnabled(ui->identityCheckBox->isChecked());

    ui->eValueCoefficientSpinBox->setEnabled(ui->eValueCheckBox->isChecked());
    ui->eValueExponentSpinBox->setEnabled(ui->eValueCheckBox->isChecked());

    ui->bitScoreSpinBox->setEnabled(ui->bitScoreCheckBox->isChecked());
}


void setSpinBoxValMinAndMax(QSpinBox * spinBox, IntSetting setting)
{
    spinBox->setValue(setting.val);
    spinBox->setMinimum(setting.min);
    spinBox->setMaximum(setting.max);
}

void setSpinBoxValMinAndMax(QDoubleSpinBox * spinBox, FloatSetting setting)
{
    spinBox->setValue(setting.val);
    spinBox->setMinimum(setting.min);
    spinBox->setMaximum(setting.max);
}

void BlastHitFiltersDialog::setWidgetsFromSettings()
{
    ui->alignmentLengthCheckBox->setChecked(g_settings->blastAlignmentLengthFilter.on);
    ui->queryCoverageCheckBox->setChecked(g_settings->blastQueryCoverageFilter.on);
    ui->identityCheckBox->setChecked(g_settings->blastIdentityFilter.on);
    ui->eValueCheckBox->setChecked(g_settings->blastEValueFilter.on);
    ui->bitScoreCheckBox->setChecked(g_settings->blastBitScoreFilter.on);

    setSpinBoxValMinAndMax(ui->alignmentLengthSpinBox, g_settings->blastAlignmentLengthFilter);
    setSpinBoxValMinAndMax(ui->queryCoverageSpinBox, g_settings->blastQueryCoverageFilter);
    setSpinBoxValMinAndMax(ui->identitySpinBox, g_settings->blastIdentityFilter);
    setSpinBoxValMinAndMax(ui->bitScoreSpinBox, g_settings->blastBitScoreFilter);

    ui->eValueCoefficientSpinBox->setValue(g_settings->blastEValueFilter.val.getCoefficient());
    ui->eValueExponentSpinBox->setValue(g_settings->blastEValueFilter.val.getExponent());

    checkBoxesChanged();
}


void BlastHitFiltersDialog::setSettingsFromWidgets()
{
    g_settings->blastAlignmentLengthFilter.on = ui->alignmentLengthCheckBox->isChecked();
    g_settings->blastQueryCoverageFilter.on = ui->queryCoverageCheckBox->isChecked();
    g_settings->blastIdentityFilter.on = ui->identityCheckBox->isChecked();
    g_settings->blastEValueFilter.on = ui->eValueCheckBox->isChecked();
    g_settings->blastBitScoreFilter.on = ui->bitScoreCheckBox->isChecked();

    g_settings->blastAlignmentLengthFilter.val = ui->alignmentLengthSpinBox->value();
    g_settings->blastQueryCoverageFilter.val = ui->queryCoverageSpinBox->value();
    g_settings->blastIdentityFilter.val = ui->identitySpinBox->value();
    g_settings->blastEValueFilter.val = SciNot(ui->eValueCoefficientSpinBox->value(), ui->eValueExponentSpinBox->value());
    g_settings->blastBitScoreFilter.val = ui->bitScoreSpinBox->value();
}


QString BlastHitFiltersDialog::getFilterText()
{
    QString filterText;
    if (g_settings->blastAlignmentLengthFilter.on)
    {
        if (filterText.length() > 0)
            filterText += ", ";
        filterText += "alignment length: " + QString::number(g_settings->blastAlignmentLengthFilter);
    }
    if (g_settings->blastQueryCoverageFilter.on)
    {
        if (filterText.length() > 0)
            filterText += ", ";
        filterText += "query coverage: " + QString::number(g_settings->blastQueryCoverageFilter) + "%";
    }
    if (g_settings->blastIdentityFilter.on)
    {
        if (filterText.length() > 0)
            filterText += ", ";
        filterText += "identity: " + QString::number(g_settings->blastIdentityFilter) + "%";
    }
    if (g_settings->blastEValueFilter.on)
    {
        if (filterText.length() > 0)
            filterText += ", ";
        filterText += "e-value: " + g_settings->blastEValueFilter.val.asString(true);
    }
    if (g_settings->blastBitScoreFilter.on)
    {
        if (filterText.length() > 0)
            filterText += ", ";
        filterText += "bit score: " + QString::number(g_settings->blastBitScoreFilter);
    }

    if (filterText == "")
        filterText = "none";

    return filterText;
}


void BlastHitFiltersDialog::setInfoTexts()
{
    ui->alignmentLengthInfoText->setInfoText("This filter will exclude any BLAST hits with an alignment length shorter than the "
                                             "specified value.  I.e. only hits with an alignment length greater than or equal to "
                                             "the value will be included in BLAST results.");
    ui->queryCoverageInfoText->setInfoText("This filter will exclude any BLAST hits with a query coverage less than the "
                                             "specified value.  I.e. only hits with a query coverage greater than or equal to "
                                             "the value will be included in BLAST results.");
    ui->identityInfoText->setInfoText("This filter will exclude any BLAST hits with an identity less than the "
                                             "specified value.  I.e. only hits with an identity greater than or equal to "
                                             "the value will be included in BLAST results.");
    ui->eValueInfoText->setInfoText("This filter will exclude any BLAST hits with an e-value greater than the "
                                             "specified value.  I.e. only hits with an e-value less than or equal to "
                                             "the value will be included in BLAST results.");
    ui->bitScoreInfoText->setInfoText("This filter will exclude any BLAST hits with a bit score length less than the "
                                             "specified value.  I.e. only hits with a bit score greater than or equal to "
                                             "the value will be included in BLAST results.");
}
