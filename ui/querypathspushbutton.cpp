#include "querypathspushbutton.h"

#include "../program/globals.h"

QueryPathsPushButton::QueryPathsPushButton(int pathCount, BlastQuery * query) :
    m_pathCount(pathCount), m_query(query)
{
    setText(formatIntForDisplay(pathCount));
    if (pathCount == 0)
        setEnabled(false);

    connect(this, SIGNAL(clicked(bool)), this, SLOT(clickedSlot()));
}





void QueryPathsPushButton::clickedSlot()
{
    emit showPathsDialog(m_query);
}
