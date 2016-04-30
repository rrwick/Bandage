//Copyright 2016 Ryan Wick

//This file is part of Bandage

//Bandage is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//Bandage is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

//You should have received a copy of the GNU General Public License
//along with Bandage.  If not, see <http://www.gnu.org/licenses/>.


#include "mytablewidget.h"

#include <vector>
#include <QHeaderView>
#include <QScrollBar>
#include <QKeyEvent>
#include <QApplication>
#include <QClipboard>

MyTableWidget::MyTableWidget(QWidget * parent) :
    QTableWidget(parent), m_smallFirstColumn(false), m_smallSecondColumn(false)
{
    verticalHeader()->hide();
    horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);
}


void MyTableWidget::resizeColumns()
{
    //If sorting is enabled, the columns are given way too much space.
    setSortingEnabled(false);
    resizeColumnsToContents();
    setSortingEnabled(true);

    //For some table widgets, we set the first column to a minimum size, as it
    //will hold the colour for both queries and hits.
    int minColumnWidth = horizontalHeader()->minimumSectionSize();
    if (m_smallFirstColumn)
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

    //If the code got here, then there is width to spare in the table.  Resize each column
    //(except for the first) to take up the whole width, keeping their relative size.

    int newTotalColumnWidth = 0.0;
    int startingColumn = 0;
    if (m_smallFirstColumn)
    {
        startingColumn = 1;
        newTotalColumnWidth += horizontalHeader()->sectionSize(0);
    }
    if (m_smallSecondColumn)
    {
        startingColumn = 2;
        newTotalColumnWidth += horizontalHeader()->sectionSize(1);
    }

    for (int i = startingColumn; i < columnCount() - 1; ++i)
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


void MyTableWidget::showEvent(QShowEvent * event)
{
    QTableWidget::showEvent(event);
    resizeColumns();
}



//http://stackoverflow.com/questions/3135737/copying-part-of-qtableview
void MyTableWidget::keyPressEvent(QKeyEvent* event)
{
    //If Ctrl-C typed (Command-C on Mac)
    if (event->key() == Qt::Key_C && (event->modifiers() & Qt::ControlModifier))
    {
        QModelIndexList cells = selectedIndexes();
        qSort(cells); // Necessary, otherwise they are in column order

        QString text;
        int currentRow = 0; // To determine when to insert newlines
        foreach (const QModelIndex& cell, cells) {
            if (text.length() == 0) {
                // First item
            } else if (cell.row() != currentRow) {
                // New row
                text += '\n';
            } else {
                // Next cell
                text += '\t';
            }
            currentRow = cell.row();
            text += cell.data().toString();
        }

        QApplication::clipboard()->setText(text);
    }
}
