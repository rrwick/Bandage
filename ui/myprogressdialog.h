#ifndef MYPROGRESSDIALOG_H
#define MYPROGRESSDIALOG_H

#include <QDialog>

namespace Ui {
class MyProgressDialog;
}

class MyProgressDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MyProgressDialog(QWidget * parent, QString message, bool showCancelButton);
    ~MyProgressDialog();

private:
    Ui::MyProgressDialog *ui;
};

#endif // MYPROGRESSDIALOG_H
