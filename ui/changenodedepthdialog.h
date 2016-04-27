#ifndef CHANGENODEDEPTHDIALOG_H
#define CHANGENODEDEPTHDIALOG_H

#include <QDialog>

class DeBruijnNode;

namespace Ui {
class ChangeNodeDepthDialog;
}

class ChangeNodeDepthDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ChangeNodeDepthDialog(QWidget * parent,
                                       std::vector<DeBruijnNode *> *nodes,
                                       double oldDepth);
    ~ChangeNodeDepthDialog();

    double getNewDepth() const;

private:
    Ui::ChangeNodeDepthDialog *ui;
};

#endif // CHANGENODEDEPTHDIALOG_H
