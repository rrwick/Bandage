#include "mytablewidget.h"

#include <vector>
#include <QHeaderView>
#include <QScrollBar>

MyTableWidget::MyTableWidget(QWidget * parent) :
    QTableWidget(parent)
{
    verticalHeader ()->hide();
}




void MyTableWidget::resizeEvent(QResizeEvent * event)
{
    resizeColumns();
    QTableWidget::resizeEvent(event);
}


void MyTableWidget::resizeColumns()
{
    resizeColumnsToContents();

    //Set the first column to a minimum size, as it will hold the colour
    //for both queries and hits.
    int minColumnWidth = horizontalHeader()->minimumSectionSize();
    horizontalHeader()->resizeSection(0, minColumnWidth);

    std::vector<int> columnWidths;
    int oldTotalColumnWidth = 0;
    for (int i = 0; i < columnCount(); ++i)
    {
        int columnWidth = horizontalHeader()->sectionSize(i);
        columnWidths.push_back(columnWidth);
        oldTotalColumnWidth += columnWidth;
    }

    //If the total column width exceeds the size of the table, just leave things alone.
    int tableWidth = viewport()->width();
    if (oldTotalColumnWidth > tableWidth)
        return;

    //If the code got here, then there is width to spare in the table.  Resize each column to
    //take up the whole width, keeping their relative size.

    oldTotalColumnWidth -= minColumnWidth;
    int newTotalColumnWidth = minColumnWidth;

    for (int i = 1; i < columnCount() - 1; ++i)
    {
        int oldColumnWidth = columnWidths[i];
        double fraction = double(oldColumnWidth) / oldTotalColumnWidth;
        int newColumnWidth = fraction * tableWidth;
        newTotalColumnWidth += newColumnWidth;
        horizontalHeader()->resizeSection(i, newColumnWidth);
    }
    int lastColumnWidth = tableWidth - newTotalColumnWidth;
    horizontalHeader()->resizeSection(columnCount() - 1, lastColumnWidth);
}
