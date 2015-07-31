#include "distancedialog.h"
#include "ui_distancedialog.h"

#include "../program/globals.h"
#include "../blast/blastsearch.h"
#include <QMessageBox>
#include "../graph/graphlocation.h"
#include "../graph/assemblygraph.h"

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
        if (g_blastSearch->m_blastQueries.m_queries[i]->m_paths.size() > 0)
            comboBoxItems.push_back(g_blastSearch->m_blastQueries.m_queries[i]->m_name);
    }

    ui->query1ComboBox->addItems(comboBoxItems);
    ui->query2ComboBox->addItems(comboBoxItems);

    if (comboBoxItems.size() > 1)
    {
        ui->query1ComboBox->setCurrentIndex(0);
        ui->query2ComboBox->setCurrentIndex(1);
    }

    connect(ui->findPathsButton, SIGNAL(clicked(bool)), this, SLOT(findPaths()));
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






}
