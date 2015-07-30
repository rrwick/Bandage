#ifndef DISTANCEDIALOG_H
#define DISTANCEDIALOG_H

#include <QDialog>

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

private slots:
    void findPaths();
};

#endif // DISTANCEDIALOG_H
