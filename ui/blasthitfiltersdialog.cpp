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

    ui->eValueBaseSpinBox->setEnabled(ui->eValueCheckBox->isChecked());
    ui->eValueExponentSpinBox->setEnabled(ui->eValueCheckBox->isChecked());

    ui->bitScoreSpinBox->setEnabled(ui->bitScoreCheckBox->isChecked());
}


void BlastHitFiltersDialog::setWidgetsFromSettings()
{
    ui->alignmentLengthCheckBox->setChecked(g_settings->blastAlignmentLengthFilterOn);
    ui->queryCoverageCheckBox->setChecked(g_settings->blastQueryCoverageFilterOn);
    ui->identityCheckBox->setChecked(g_settings->blastIdentityFilterOn);
    ui->eValueCheckBox->setChecked(g_settings->blastEValueFilterOn);
    ui->bitScoreCheckBox->setChecked(g_settings->blastBitScoreFilterOn);

    ui->alignmentLengthSpinBox->setValue(g_settings->blastAlignmentLengthFilterValue);
    ui->queryCoverageSpinBox->setValue(g_settings->blastQueryCoverageFilterValue);
    ui->identitySpinBox->setValue(g_settings->blastIdentityFilterValue);
    ui->eValueBaseSpinBox->setValue(g_settings->blastEValueFilterCoefficientValue);
    ui->eValueExponentSpinBox->setValue(g_settings->blastEValueFilterExponentValue);
    ui->bitScoreSpinBox->setValue(g_settings->blastBitScoreFilterValue);

    checkBoxesChanged();
}


void BlastHitFiltersDialog::setSettingsFromWidgets()
{
    g_settings->blastAlignmentLengthFilterOn = ui->alignmentLengthCheckBox->isChecked();
    g_settings->blastQueryCoverageFilterOn = ui->queryCoverageCheckBox->isChecked();
    g_settings->blastIdentityFilterOn = ui->identityCheckBox->isChecked();
    g_settings->blastEValueFilterOn = ui->eValueCheckBox->isChecked();
    g_settings->blastBitScoreFilterOn = ui->bitScoreCheckBox->isChecked();

    g_settings->blastAlignmentLengthFilterValue = ui->alignmentLengthSpinBox->value();
    g_settings->blastQueryCoverageFilterValue = ui->queryCoverageSpinBox->value();
    g_settings->blastIdentityFilterValue = ui->identitySpinBox->value();
    g_settings->blastEValueFilterCoefficientValue = ui->eValueBaseSpinBox->value();
    g_settings->blastEValueFilterExponentValue = ui->eValueExponentSpinBox->value();
    g_settings->blastBitScoreFilterValue = ui->bitScoreSpinBox->value();
}


QString BlastHitFiltersDialog::getFilterText() const
{
    QString filterText;
    if (g_settings->blastAlignmentLengthFilterOn)
    {
        if (filterText.length() > 0)
            filterText += ", ";
        filterText += "alignment length: " + QString::number(g_settings->blastAlignmentLengthFilterValue);
    }
    if (g_settings->blastQueryCoverageFilterOn)
    {
        if (filterText.length() > 0)
            filterText += ", ";
        filterText += "query coverage: " + QString::number(g_settings->blastQueryCoverageFilterValue) + "%";
    }
    if (g_settings->blastIdentityFilterOn)
    {
        if (filterText.length() > 0)
            filterText += ", ";
        filterText += "identity: " + QString::number(g_settings->blastIdentityFilterValue) + "%";
    }
    if (g_settings->blastEValueFilterOn)
    {
        if (filterText.length() > 0)
            filterText += ", ";
        filterText += "e-value: " + QString::number(g_settings->blastEValueFilterCoefficientValue) + "x10^" + QString::number(g_settings->blastEValueFilterExponentValue);
    }
    if (g_settings->blastBitScoreFilterOn)
    {
        if (filterText.length() > 0)
            filterText += ", ";
        filterText += "bit score: " + QString::number(g_settings->blastBitScoreFilterValue);
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
