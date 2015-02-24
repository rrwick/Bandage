#ifndef LOADBLASTRESULTSDIALOG_H
#define LOADBLASTRESULTSDIALOG_H

#include <QDialog>

namespace Ui {
class LoadBlastResultsDialog;
}

class LoadBlastResultsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LoadBlastResultsDialog(QWidget *parent = 0);
    ~LoadBlastResultsDialog();

private:
    Ui::LoadBlastResultsDialog *ui;
};

#endif // LOADBLASTRESULTSDIALOG_H
