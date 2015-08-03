#ifndef TABLEWIDGETITEMDOUBLE_H
#define TABLEWIDGETITEMDOUBLE_H

#include <QTableWidgetItem>

class TableWidgetItemDouble : public QTableWidgetItem
{
public:
    TableWidgetItemDouble(QString text);

    double m_double;

    virtual bool operator<(QTableWidgetItem const &other) const;
};

#endif // TABLEWIDGETITEMDOUBLE_H
