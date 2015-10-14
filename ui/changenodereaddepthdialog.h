#ifndef CHANGENODEREADDEPTHDIALOG_H
#define CHANGENODEREADDEPTHDIALOG_H

#include <QDialog>

class DeBruijnNode;

namespace Ui {
class ChangeNodeReadDepthDialog;
}

class ChangeNodeReadDepthDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ChangeNodeReadDepthDialog(QWidget * parent,
                                       std::vector<DeBruijnNode *> *nodes,
                                       double oldDepth);
    ~ChangeNodeReadDepthDialog();

    double getNewDepth() const;

private:
    Ui::ChangeNodeReadDepthDialog *ui;
};

#endif // CHANGENODEREADDEPTHDIALOG_H
