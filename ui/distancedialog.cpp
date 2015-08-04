#include "distancedialog.h"
#include "ui_distancedialog.h"

#include "../program/globals.h"
#include "../blast/blastsearch.h"
#include <QMessageBox>
#include "../graph/graphlocation.h"
#include "../graph/assemblygraph.h"
#include "../graph/path.h"

DistanceDialog::DistanceDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DistanceDialog)
{
    ui->setupUi(this);

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

    if (comboBoxItems.size() > 1)
    {
        ui->query1ComboBox->setCurrentIndex(0);
        ui->query2ComboBox->setCurrentIndex(1);
    }

    query1Changed();
    query2Changed();

    connect(ui->findPathsButton, SIGNAL(clicked(bool)), this, SLOT(findPaths()));
    connect(ui->query1ComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(query1Changed()));
    connect(ui->query2ComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(query2Changed()));
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

    Path query1Path = query1->getPaths()[ui->query1PathComboBox->currentIndex()];
    Path query2Path = query2->getPaths()[ui->query1PathComboBox->currentIndex()];

    int pathSearchDepth = ui->maxNodesSpinBox->value() - 1;
    int minDistance = ui->minPathDistanceSpinBox->value();
    int maxDistance = ui->maxPathDistanceSpinBox->value();

    QStringList orientations;
    QList<int> distances;
    QList<Path> paths;

    //First orientation to check: 1-> 2->
    GraphLocation start = query1Path.getEndLocation();
    GraphLocation end = query2Path.getStartLocation();
    paths.append(Path::getAllPossiblePaths(start, end, pathSearchDepth, minDistance, maxDistance));
    while (orientations.size() < paths.size())
        orientations.push_back("1-> 2->");

    //Second orientation to check: 2-> 1->
    start = query2Path.getEndLocation();
    end = query1Path.getStartLocation();
    paths.append(Path::getAllPossiblePaths(start, end, pathSearchDepth, minDistance, maxDistance));
    while (orientations.size() < paths.size())
        orientations.push_back("2-> 1->");

    //Third orientation to check: 1-> <-2
    start = query1Path.getEndLocation();
    end = query2Path.getEndLocation().reverseComplementLocation();
    paths.append(Path::getAllPossiblePaths(start, end, pathSearchDepth, minDistance, maxDistance));
    while (orientations.size() < paths.size())
        orientations.push_back("1-> <-2");

    //Fourth orientation to check: <-1 2->
    start = query1Path.getStartLocation().reverseComplementLocation();
    end = query2Path.getEndLocation();
    paths.append(Path::getAllPossiblePaths(start, end, pathSearchDepth, minDistance, maxDistance));
    while (orientations.size() < paths.size())
        orientations.push_back("<-1 2->");

    for (int i = 0; i < paths.size(); ++i)
        distances.push_back(paths[i].getLength());
}


//These function populate the path combo boxes for each query.
void DistanceDialog::query1Changed()
{
    ui->query1PathComboBox->clear();

    BlastQuery * query1 = g_blastSearch->m_blastQueries.getQueryFromName(ui->query1ComboBox->currentText());
    if (query1 == 0)
        return;

    QStringList comboBoxItems;
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
    QList<Path> paths = query2->getPaths();
    for (int i = 0; i < paths.size(); ++i)
        comboBoxItems.push_back(QString::number(i+1) + ": "+ paths[i].getString(true));

    ui->query2PathComboBox->addItems(comboBoxItems);
}
