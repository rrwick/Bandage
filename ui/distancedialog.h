#ifndef DISTANCEDIALOG_H
#define DISTANCEDIALOG_H

#include <QDialog>
#include <QComboBox>

class BlastQuery;

namespace Ui {
class DistanceDialog;
}

class DistanceDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DistanceDialog(QWidget *parent = 0);
    ~DistanceDialog();

private:
    Ui::DistanceDialog *ui;

    void loadSettings();
    void fillResultsTable();
    void fillPathComboBox(BlastQuery * query, QComboBox * comboBox);

private slots:
    void findPaths();
    void query1Changed();
    void query2Changed();
    void saveSettings();
};

#endif // DISTANCEDIALOG_H
