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
#include "myprogressdialog.h"

DistanceDialog::DistanceDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DistanceDialog)
{
    ui->setupUi(this);
    loadSettings();
    setInfoTexts();

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

    //Load the previously used query choices.
    bool rememberedQueriesLoaded = false;
    int indexOfQuery1 = comboBoxItems.indexOf(g_memory->distancePathSearchQuery1);
    int indexOfQuery2 = comboBoxItems.indexOf(g_memory->distancePathSearchQuery2);
    if (indexOfQuery1 != -1 && indexOfQuery2 != -1)
    {
        ui->query1ComboBox->setCurrentIndex(indexOfQuery1);
        ui->query2ComboBox->setCurrentIndex(indexOfQuery2);
        rememberedQueriesLoaded = true;
    }

    //If no queries were successfully loaded, then we try to set the first query
    //to index 0 and the second to index 1, if possible.
    else
    {
        if (ui->query1ComboBox->count() > 0)
            ui->query1ComboBox->setCurrentIndex(0);
        if (ui->query2ComboBox->count() > 1)
            ui->query2ComboBox->setCurrentIndex(1);
    }
    query1Changed();
    query2Changed();

    //If remembered queries were loaded, then load the previously used path
    //choices.
    bool rememberedPathsLoaded = false;
    if (rememberedQueriesLoaded)
    {
        QStringList query1Paths;
        for (int i = 0; i < ui->query1PathComboBox->count(); ++i)
            query1Paths.push_back(ui->query1PathComboBox->itemText(i));
        int indexOfQuery1Path = query1Paths.indexOf(g_memory->distancePathSearchQuery1Path);
        QStringList query2Paths;
        for (int i = 0; i < ui->query2PathComboBox->count(); ++i)
            query2Paths.push_back(ui->query2PathComboBox->itemText(i));
        int indexOfQuery2Path = query2Paths.indexOf(g_memory->distancePathSearchQuery2Path);
        if (indexOfQuery1Path != -1 && indexOfQuery2Path != -1)
        {
            ui->query1PathComboBox->setCurrentIndex(indexOfQuery1Path);
            ui->query2PathComboBox->setCurrentIndex(indexOfQuery2Path);
            rememberedPathsLoaded = true;
        }
    }

    //If the previously used queries and paths were successfully loaded and
    //there are results, display them now.  If not, clear any results that might
    //exist.
    if (rememberedQueriesLoaded && rememberedPathsLoaded &&
            !g_memory->distanceSearchPaths.empty())
        fillResultsTable();
    else
    {
        g_memory->distanceSearchOrientations.clear();
        g_memory->distanceSearchDistances.clear();
        g_memory->distanceSearchPaths.clear();
    }

    connect(ui->findPathsButton, SIGNAL(clicked(bool)), this, SLOT(findPaths()));
    connect(ui->query1ComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(query1Changed()));
    connect(ui->query2ComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(query2Changed()));
    connect(ui->maxNodesSpinBox, SIGNAL(valueChanged(int)), this, SLOT(saveSettings()));
    connect(ui->maxPathDistanceSpinBox, SIGNAL(valueChanged(int)), this, SLOT(saveSettings()));
    connect(ui->minPathDistanceSpinBox, SIGNAL(valueChanged(int)), this, SLOT(saveSettings()));
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

    //Remember which queries and paths were used for this search.
    g_memory->distancePathSearchQuery1 = ui->query1ComboBox->currentText();
    g_memory->distancePathSearchQuery2 = ui->query2ComboBox->currentText();
    g_memory->distancePathSearchQuery1Path = ui->query1PathComboBox->currentText();
    g_memory->distancePathSearchQuery2Path = ui->query2PathComboBox->currentText();

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

    //Run the path search.  This is in a separate code block so the progress
    //dialog is destroyed when the search is finished.
    {
        //Display a progress dialog.
        MyProgressDialog progress(this, "Finding paths between queries...", false);
        progress.setWindowModality(Qt::WindowModal);
        progress.show();

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
    }

    int pathCount = g_memory->distanceSearchPaths.size();
    for (int i = 0; i < pathCount; ++i)
        g_memory->distanceSearchDistances.push_back(g_memory->distanceSearchPaths[i].getLength());

    fillResultsTable();

    if (pathCount == 0)
        QMessageBox::information(this, "No paths", "No paths were found between the two given queries.");
}


//These functions populate the path combo boxes for each query.
void DistanceDialog::query1Changed()
{
    BlastQuery * query1 = g_blastSearch->m_blastQueries.getQueryFromName(ui->query1ComboBox->currentText());
    if (query1 == 0)
        return;

    fillPathComboBox(query1, ui->query1PathComboBox);
}
void DistanceDialog::query2Changed()
{
    BlastQuery * query2 = g_blastSearch->m_blastQueries.getQueryFromName(ui->query2ComboBox->currentText());
    if (query2 == 0)
        return;

    fillPathComboBox(query2, ui->query2PathComboBox);
}
void DistanceDialog::fillPathComboBox(BlastQuery * query, QComboBox * comboBox)
{
    comboBox->clear();

    QStringList comboBoxItems;
    QList<Path> paths = query->getPaths();

    if (paths.size() == 0)
        return;
    else if (paths.size() == 1)
        comboBoxItems.push_back(paths[0].getString(true));
    else
    {
        comboBoxItems.push_back("all");
        for (int i = 0; i < paths.size(); ++i)
            comboBoxItems.push_back(QString::number(i+1) + ": "+ paths[i].getString(true));
    }

    comboBox->addItems(comboBoxItems);
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


void DistanceDialog::setInfoTexts()
{
    ui->queriesInfoText->setInfoText("Select two different BLAST queries here for which to find "
                                     "the distance between. Only queries that have at least one "
                                     "graph path may be used.<br><br>"
                                     "If a query has more than one graph path, then you can use "
                                     "all of the paths in the search or you can select only one.");

    ui->maxNodesInfoText->setInfoText("This is the maximum number of nodes that can be in a path "
                                      "between the two queries.<br><br>"
                                      "Larger values can allow the search to find more complex "
                                      "paths in the graph but at a performance cost.");

    ui->minPathDistanceInfoText->setInfoText("Paths shorter than this length (measured in base "
                                             "pairs) will not be included in the search results.");

    ui->maxPathDistanceInfoText->setInfoText("Paths longer than this length (measured in base "
                                             "pairs) will not be included in the search results.");

    ui->orientationsInfoText->setInfoText("Each possible query orientation can be included or "
                                          "excluded from the search using these tick boxes:"
                                          "<ul>"
                                          "<li>1-&#62; 2-&#62; The two queries are on the same strand "
                                          "of DNA, with query 1 occurring upstream of query 2.</li>"
                                          "<li>2-&#62; 1-&#62; The two queries are on the same strand "
                                          "of DNA, with query 1 occurring downstream of query 2.</li>"
                                          "<li>1-&#62; &#60;-2 The two queries are on different strands "
                                          "of DNA, with their 3' ends closer than their 5' ends.</li>"
                                          "<li>&#60;-1 2-&#62; The two queries are on different strands "
                                          "of DNA, with their 5' ends closer than their 3' ends.</li>"
                                          "</ul>");
}
