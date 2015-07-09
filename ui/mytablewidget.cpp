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
    QTableWidget::resizeEvent(event);
    resizeColumns();
}


void MyTableWidget::resizeColumns()
{
    resizeColumnsToContents();

    std::vector<int> columnWidths;
    int totalColumnWidth = 0;
    for (int i = 0; i < columnCount(); ++i)
    {
        int columnWidth = horizontalHeader()->sectionSize(i);
        columnWidths.push_back(columnWidth);
        totalColumnWidth += columnWidth;
    }

    //If the total column width exceeds the size of the table, just leave things alone.
    int tableWidth = width() - 2;  //Subtract 2 as a bit of a buffer
    if (verticalScrollBar()->isVisible())
        tableWidth -= verticalScrollBar()->width();

    if (totalColumnWidth > tableWidth)
        return;

    //If the code got here, then there is width to spare in the table.  Resize each column to
    //take up the whole width, keeping their relative size.
    for (size_t i = 0; i < columnWidths.size(); ++i)
    {
        int oldColumnWidth = columnWidths[i];
        double fraction = double(oldColumnWidth) / totalColumnWidth;
        int newColumnWidth = fraction * tableWidth;
        horizontalHeader()->resizeSection(i, newColumnWidth);
    }
}
