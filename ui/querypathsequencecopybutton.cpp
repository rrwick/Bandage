#include "querypathsequencecopybutton.h"

#include <QApplication>
#include <QClipboard>

QueryPathSequenceCopyButton::QueryPathSequenceCopyButton(QByteArray pathSequence, QString pathStart) :
    m_pathSequence(pathSequence)
{
    setText(pathStart);
    connect(this, SIGNAL(clicked(bool)), this, SLOT(copySequenceToClipboard()));
}


void QueryPathSequenceCopyButton::copySequenceToClipboard()
{
    QClipboard * clipboard = QApplication::clipboard();
    clipboard->setText(m_pathSequence);
}
