//This class was written by Jason Dolan and acquired here:
//http://qt-project.org/forums/viewthread/13728


#ifndef VERTICALSCROLLAREA_H
#define VERTICALSCROLLAREA_H

#include <QObject>
#include <QWidget>
#include <QEvent>
#include <QScrollArea>

class VerticalScrollArea : public QScrollArea
{
    Q_OBJECT
public:
    explicit VerticalScrollArea(QWidget *parent = 0);
    virtual bool eventFilter(QObject *o, QEvent *e);
};

#endif // VERTICALSCROLLAREA_H
