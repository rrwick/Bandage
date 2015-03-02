#ifndef ENTERONEBLASTQUERYDIALOG_H
#define ENTERONEBLASTQUERYDIALOG_H

#include <QDialog>
#include <QString>

namespace Ui {
class EnterOneBlastQueryDialog;
}

class EnterOneBlastQueryDialog : public QDialog
{
    Q_OBJECT

public:
    explicit EnterOneBlastQueryDialog(QWidget *parent = 0);
    ~EnterOneBlastQueryDialog();

    QString getName();
    QString getSequence();

private:
    Ui::EnterOneBlastQueryDialog *ui;
};

#endif // ENTERONEBLASTQUERYDIALOG_H
