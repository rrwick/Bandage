#ifndef CHANGENODENAMEDIALOG_H
#define CHANGENODENAMEDIALOG_H

#include <QDialog>

namespace Ui {
class ChangeNodeNameDialog;
}

class ChangeNodeNameDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ChangeNodeNameDialog(QWidget * parent, QString oldName);
    ~ChangeNodeNameDialog();

    QString getNewName() const;

private:
    Ui::ChangeNodeNameDialog *ui;
    QString m_oldName;

private slots:
    void checkNodeNameValidity();
};

#endif // CHANGENODENAMEDIALOG_H
