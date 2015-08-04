#include "distancedialog.h"
#include "ui_distancedialog.h"

#include "../program/globals.h"
#include "../blast/blastsearch.h"
#include <QMessageBox>
#include "../graph/graphlocation.h"
#include "../graph/assemblygraph.h"
#include "../graph/path.h"
#include "tablewidgetitemint.h"
#include "../program/memory.h"
#include "../program/settings.h"

DistanceDialog::DistanceDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DistanceDialog)
{
    ui->setupUi(this);
    loadSettings();

    //Fill the query combo boxes with any BLAST queries that have paths.
    ui->query1ComboBox->clear();
    ui->query2ComboBox->clear();
    QStringList comboBoxItems;
    for (size_t i = 0; i < g_blastSearch->m_blastQueries.m_queries.size(); ++i)
    {
        if (g_blastSearch->m_blastQueries.m_queries[i]->getPathCount() > 0)
            comboBoxItems.push_back(g_blastSearch->m_blastQueries.m_queries[i]->getName());
    }
    ui->query1ComboBox->addItems(comboBoxItems);
    ui->query2ComboBox->addItems(comboBoxItems);

    //Load the previously used combobox choices.
    if (ui->query1ComboBox->count() > g_memory->distancePathSearchQuery1)
        ui->query1ComboBox->setCurrentIndex(g_memory->distancePathSearchQuery1);
    if (ui->query2ComboBox->count() > g_memory->distancePathSearchQuery2)
        ui->query2ComboBox->setCurrentIndex(g_memory->distancePathSearchQuery2);

    query1Changed();
    query2Changed();

    if (!g_memory->distanceSearchPaths.empty())
        fillResultsTable();

    connect(ui->findPathsButton, SIGNAL(clicked(bool)), this, SLOT(findPaths()));
    connect(ui->query1ComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(query1Changed()));
    connect(ui->query2ComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(query2Changed()));
    connect(ui->maxNodesSpinBox, SIGNAL(valueChanged(int)), this, SLOT(settingsChanged()));
    connect(ui->maxPathDistanceSpinBox, SIGNAL(valueChanged(int)), this, SLOT(settingsChanged()));
    connect(ui->minPathDistanceSpinBox, SIGNAL(valueChanged(int)), this, SLOT(settingsChanged()));
}

DistanceDialog::~DistanceDialog()
{
    delete ui;
}

void DistanceDialog::findPaths()
{
    if (ui->query1ComboBox->count() < 2)
    {
        QMessageBox::information(this, "Two queries required", "To find paths between queries, you must first conduct a BLAST search "
                                                               "for at least two different queries.");
        return;
    }

    BlastQuery * query1 = g_blastSearch->m_blastQueries.getQueryFromName(ui->query1ComboBox->currentText());
    BlastQuery * query2 = g_blastSearch->m_blastQueries.getQueryFromName(ui->query2ComboBox->currentText());

    if (query1 == 0 || query2 == 0)
        return;

    if (query1 == query2)
    {
        QMessageBox::information(this, "Same query", "The two selected queries are the same. To find paths between queries, you must select two different queries.");
        return;
    }

    int pathSearchDepth = g_settings->distancePathSearchDepth;
    int minDistance = g_settings->minDistancePathLength;
    int maxDistance = g_settings->maxDistancePathLength;

    if (minDistance > maxDistance)
    {
        QMessageBox::information(this, "Min greater than max", "The minimum path distance must be less than or equal to the maximum path distance.");
        return;
    }

    QList<Path> query1Paths;
    if (ui->query1PathComboBox->currentIndex() == 0)
        query1Paths = query1->getPaths();
    else
        query1Paths.push_back(query1->getPaths()[ui->query1PathComboBox->currentIndex() - 1]);

    QList<Path> query2Paths;
    if (ui->query2PathComboBox->currentIndex() == 0)
        query2Paths = query2->getPaths();
    else
        query2Paths.push_back(query2->getPaths()[ui->query2PathComboBox->currentIndex() - 1]);

    g_memory->distanceSearchOrientations.clear();
    g_memory->distanceSearchDistances.clear();
    g_memory->distanceSearchPaths.clear();

    for (int i = 0; i < query1Paths.size(); ++i)
    {
        Path query1Path = query1Paths[i];

        for (int j = 0; j < query2Paths.size(); ++j)
        {
            Path query2Path = query2Paths[j];

            //First orientation to check: 1-> 2->
            if (ui->orientation1CheckBox->isChecked())
            {
                GraphLocation start = query1Path.getEndLocation();
                GraphLocation end = query2Path.getStartLocation();
                g_memory->distanceSearchPaths.append(Path::getAllPossiblePaths(start, end, pathSearchDepth, minDistance, maxDistance));
                while (g_memory->distanceSearchOrientations.size() < g_memory->distanceSearchPaths.size())
                    g_memory->distanceSearchOrientations.push_back("1-> 2->");
            }

            //Second orientation to check: 2-> 1->
            if (ui->orientation2CheckBox->isChecked())
            {
                GraphLocation start = query2Path.getEndLocation();
                GraphLocation end = query1Path.getStartLocation();
                g_memory->distanceSearchPaths.append(Path::getAllPossiblePaths(start, end, pathSearchDepth, minDistance, maxDistance));
                while (g_memory->distanceSearchOrientations.size() < g_memory->distanceSearchPaths.size())
                    g_memory->distanceSearchOrientations.push_back("2-> 1->");
            }

            //Third orientation to check: 1-> <-2
            if (ui->orientation3CheckBox->isChecked())
            {
                GraphLocation start = query1Path.getEndLocation();
                GraphLocation end = query2Path.getEndLocation().reverseComplementLocation();
                g_memory->distanceSearchPaths.append(Path::getAllPossiblePaths(start, end, pathSearchDepth, minDistance, maxDistance));
                while (g_memory->distanceSearchOrientations.size() < g_memory->distanceSearchPaths.size())
                    g_memory->distanceSearchOrientations.push_back("1-> <-2");
            }

            //Fourth orientation to check: <-1 2->
            if (ui->orientation4CheckBox->isChecked())
            {
                GraphLocation start = query1Path.getStartLocation().reverseComplementLocation();
                GraphLocation end = query2Path.getStartLocation();
                g_memory->distanceSearchPaths.append(Path::getAllPossiblePaths(start, end, pathSearchDepth, minDistance, maxDistance));
                while (g_memory->distanceSearchOrientations.size() < g_memory->distanceSearchPaths.size())
                    g_memory->distanceSearchOrientations.push_back("<-1 2->");
            }
        }
    }

    int pathCount = g_memory->distanceSearchPaths.size();
    for (int i = 0; i < pathCount; ++i)
        g_memory->distanceSearchDistances.push_back(g_memory->distanceSearchPaths[i].getLength());

    fillResultsTable();

    if (pathCount == 0)
        QMessageBox::information(this, "No paths", "No paths were found between the two given queries.");
}


//These function populate the path combo boxes for each query.
void DistanceDialog::query1Changed()
{
    ui->query1PathComboBox->clear();

    BlastQuery * query1 = g_blastSearch->m_blastQueries.getQueryFromName(ui->query1ComboBox->currentText());
    if (query1 == 0)
        return;

    QStringList comboBoxItems;
    comboBoxItems.push_back("all");
    QList<Path> paths = query1->getPaths();
    for (int i = 0; i < paths.size(); ++i)
        comboBoxItems.push_back(QString::number(i+1) + ": "+ paths[i].getString(true));

    ui->query1PathComboBox->addItems(comboBoxItems);
}

void DistanceDialog::query2Changed()
{
    ui->query2PathComboBox->clear();

    BlastQuery * query2 = g_blastSearch->m_blastQueries.getQueryFromName(ui->query2ComboBox->currentText());
    if (query2 == 0)
        return;

    QStringList comboBoxItems;
    comboBoxItems.push_back("all");
    QList<Path> paths = query2->getPaths();
    for (int i = 0; i < paths.size(); ++i)
        comboBoxItems.push_back(QString::number(i+1) + ": "+ paths[i].getString(true));

    ui->query2PathComboBox->addItems(comboBoxItems);
}



void DistanceDialog::fillResultsTable()
{
    int pathCount = g_memory->distanceSearchPaths.size();

    ui->resultsTableWidget->clearContents();
    ui->resultsTableWidget->setSortingEnabled(false);
    ui->resultsTableWidget->setRowCount(pathCount);

    for (int i = 0; i < pathCount; ++i)
    {
        QTableWidgetItem * orientation = new QTableWidgetItem(g_memory->distanceSearchOrientations[i]);
        orientation->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);

        TableWidgetItemInt * distance = new TableWidgetItemInt(formatIntForDisplay(g_memory->distanceSearchDistances[i]));
        distance->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);

        QTableWidgetItem * path = new QTableWidgetItem(g_memory->distanceSearchPaths[i].getString(true));
        orientation->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);

        ui->resultsTableWidget->setItem(i, 0, orientation);
        ui->resultsTableWidget->setItem(i, 1, distance);
        ui->resultsTableWidget->setItem(i, 2, path);
    }

    ui->resultsTableWidget->resizeColumns();
    ui->resultsTableWidget->setSortingEnabled(true);
}



void DistanceDialog::loadSettings()
{
    ui->maxNodesSpinBox->setValue(g_settings->distancePathSearchDepth + 1);
    ui->minPathDistanceSpinBox->setValue(g_settings->minDistancePathLength);
    ui->maxPathDistanceSpinBox->setValue(g_settings->maxDistancePathLength);
}

void DistanceDialog::saveSettings()
{
    g_settings->distancePathSearchDepth = ui->maxNodesSpinBox->value() - 1;
    g_settings->minDistancePathLength = ui->minPathDistanceSpinBox->value();
    g_settings->maxDistancePathLength = ui->maxPathDistanceSpinBox->value();
}
