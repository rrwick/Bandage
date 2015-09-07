#include "blasthitfiltersdialog.h"
#include "ui_blasthitfiltersdialog.h"

#include "../program/globals.h"
#include "../program/settings.h"

BlastHitFiltersDialog::BlastHitFiltersDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::BlastHitFiltersDialog)
{
    ui->setupUi(this);

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
    ui->eValueBaseSpinBox->setValue(g_settings->blastEValueBaseFilterValue);
    ui->eValueExponentSpinBox->setValue(g_settings->blastEValueExponentFilterValue);
    ui->bitScoreSpinBox->setValue(g_settings->blastBitScoreFilterValue);
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
    g_settings->blastEValueBaseFilterValue = ui->eValueBaseSpinBox->value();
    g_settings->blastEValueExponentFilterValue = ui->eValueExponentSpinBox->value();
    g_settings->blastBitScoreFilterValue = ui->bitScoreSpinBox->value();
}


QString BlastHitFiltersDialog::getFilterText() const
{
    QString lessThanOrEqualTo = QChar(0x2264);
    QString greaterThanOrEqualTo = QChar(0x2265);

    QString filterText;

    if (g_settings->blastAlignmentLengthFilterOn)
    {
        if (filterText.length() > 0)
            filterText += ", ";
        filterText += "alignment length " + greaterThanOrEqualTo + QString::number(g_settings->blastAlignmentLengthFilterValue);
    }
    if (g_settings->blastQueryCoverageFilterOn)
    {
        if (filterText.length() > 0)
            filterText += ", ";
        filterText += "query coverage " + greaterThanOrEqualTo + QString::number(g_settings->blastQueryCoverageFilterValue) + "%";
    }

    if (g_settings->blastIdentityFilterOn)
    {
        if (filterText.length() > 0)
            filterText += ", ";
        filterText += "identity " + greaterThanOrEqualTo + QString::number(g_settings->blastIdentityFilterValue) + "%";
    }

    if (g_settings->blastEValueFilterOn)
    {
        if (filterText.length() > 0)
            filterText += ", ";
        filterText += "e-value " + lessThanOrEqualTo + QString::number(g_settings->blastEValueBaseFilterValue) + " x 10^" + QString::number(g_settings->blastEValueExponentFilterValue);
    }

    if (g_settings->blastBitScoreFilterOn)
    {
        if (filterText.length() > 0)
            filterText += ", ";
        filterText += "bit score " + greaterThanOrEqualTo + QString::number(g_settings->blastBitScoreFilterValue);
    }

    if (filterText == "")
        filterText = "none";

    return filterText;
}
