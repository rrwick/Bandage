#ifndef TAXINFODIALOG_H
#define TAXINFODIALOG_H

#include <QDialog>

namespace Ui {
class TaxInfoDialog;
}

class TaxInfoDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TaxInfoDialog(QWidget *parent = 0);
    TaxInfoDialog(QWidget *parent, int taxId);
    ~TaxInfoDialog();

private:
    Ui::TaxInfoDialog *ui;

    void setInfoTexts();
    void setSpecialTaxInfoTexts(int taxId);
    void setErrorText();
    void setErrorText(int taxId);
};

#endif // TAXINFODIALOG_H
