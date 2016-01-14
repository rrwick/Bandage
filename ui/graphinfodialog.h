#ifndef GRAPHINFODIALOG_H
#define GRAPHINFODIALOG_H

#include <QDialog>

namespace Ui {
class GraphInfoDialog;
}

class GraphInfoDialog : public QDialog
{
    Q_OBJECT

public:
    explicit GraphInfoDialog(QWidget *parent = 0);
    ~GraphInfoDialog();

private:
    Ui::GraphInfoDialog *ui;
};

#endif // GRAPHINFODIALOG_H
