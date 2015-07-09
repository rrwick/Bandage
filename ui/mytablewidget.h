#ifndef MYTABLEWIDGET_H
#define MYTABLEWIDGET_H

#include <QTableWidget>


class MyTableWidget : public QTableWidget
{
    Q_OBJECT
public:
    explicit MyTableWidget(QWidget *parent = 0);

protected:
    void resizeEvent(QResizeEvent * event);
};

#endif // MYTABLEWIDGET_H
