#ifndef TABLEWIDGETITEMINT_H
#define TABLEWIDGETITEMINT_H

#include <QTableWidgetItem>

class TableWidgetItemInt : public QTableWidgetItem
{
public:
    TableWidgetItemInt(QString text);

    int m_int;

    virtual bool operator<(QTableWidgetItem const &other) const;
};

#endif // TABLEWIDGETITEMINT_H
