#ifndef BLASTHITFILTERSDIALOG_H
#define BLASTHITFILTERSDIALOG_H

#include <QDialog>

namespace Ui {
class BlastHitFiltersDialog;
}

class BlastHitFiltersDialog : public QDialog
{
    Q_OBJECT

public:
    explicit BlastHitFiltersDialog(QWidget *parent = 0);
    ~BlastHitFiltersDialog();

    void setWidgetsFromSettings();
    void setSettingsFromWidgets();
    QString getFilterText() const;

private:
    Ui::BlastHitFiltersDialog *ui;

    void setInfoTexts();

private slots:
    void checkBoxesChanged();
};

#endif // BLASTHITFILTERSDIALOG_H
