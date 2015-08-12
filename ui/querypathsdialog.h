#ifndef QUERYPATHSDIALOG_H
#define QUERYPATHSDIALOG_H

#include <QDialog>

class BlastQuery;

namespace Ui {
class QueryPathsDialog;
}

class QueryPathsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit QueryPathsDialog(QWidget *parent, BlastQuery * query);
    ~QueryPathsDialog();

private:
    Ui::QueryPathsDialog *ui;

private slots:
    void hidden();
    void tableSelectionChanged();

signals:
    void selectionChanged();
};

#endif // QUERYPATHSDIALOG_H
