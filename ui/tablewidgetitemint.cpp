#include "tablewidgetitemint.h"

TableWidgetItemInt::TableWidgetItemInt(QString text) :
    QTableWidgetItem(text)
{
    QString withoutCommas = text.replace(',', "");
    m_int = withoutCommas.toInt();
}



bool TableWidgetItemInt::operator<(QTableWidgetItem const &other) const
{
    TableWidgetItemInt const *item = dynamic_cast<TableWidgetItemInt const*>(&other);
    if(item)
        return m_int < item->m_int;
    else
        return QTableWidgetItem::operator<(other);
}
