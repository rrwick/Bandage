#ifndef LOADBLASTRESULTSDIALOG_H
#define LOADBLASTRESULTSDIALOG_H

#include <QDialog>
#include "../blast/blastsearchresults.h"
#include <QMap>

class DeBruijnNode;
class BlastTarget;

namespace Ui {
class LoadBlastResultsDialog;
}

class LoadBlastResultsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LoadBlastResultsDialog(QMap<int, DeBruijnNode*> * deBruijnGraphNodes,
                                    QWidget *parent = 0);
    ~LoadBlastResultsDialog();

private:
    Ui::LoadBlastResultsDialog *ui;
    BlastSearchResults * m_blastSearchResults;
    QMap<int, DeBruijnNode*> * m_deBruijnGraphNodes;

    int getNodeNumberFromString(QString nodeString);
    BlastTarget * getTargetFromString(QString targetName);

private slots:
    void loadBlastDatabase();
    void loadBlastOutput();
};

#endif // LOADBLASTRESULTSDIALOG_H
