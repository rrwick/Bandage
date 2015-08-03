#include "tablewidgetitemdouble.h"

TableWidgetItemDouble::TableWidgetItemDouble(QString text) :
    QTableWidgetItem(text)
{
    QString withoutCommas = text.replace(',', "");
    QString withoutPercent = withoutCommas.replace('%', "");
    m_double = withoutPercent.toDouble();
}



bool TableWidgetItemDouble::operator<(QTableWidgetItem const &other) const
{
    TableWidgetItemDouble const *item = dynamic_cast<TableWidgetItemDouble const*>(&other);
    if(item)
        return m_double < item->m_double;
    else
        return QTableWidgetItem::operator<(other);
}
