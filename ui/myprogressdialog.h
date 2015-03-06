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

private slots:
    void cancelLayout();

signals:
    void haltLayout();
};

#endif // MYPROGRESSDIALOG_H
