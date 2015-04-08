//This class was written by Jason Dolan and acquired here:
//http://qt-project.org/forums/viewthread/13728


#include "verticalscrollarea.h"

#include <QEvent>
#include <QScrollBar>


VerticalScrollArea::VerticalScrollArea(QWidget *parent) :
    QScrollArea(parent)
{
    setWidgetResizable(true);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
}

bool VerticalScrollArea::eventFilter(QObject *o, QEvent *e)
{
    // This works because QScrollArea::setWidget installs an eventFilter on the widget
    if(o && o == widget() && e->type() == QEvent::Resize)
        setMinimumWidth(widget()->minimumSizeHint().width() + verticalScrollBar()->width());

    return QScrollArea::eventFilter(o, e);
}
