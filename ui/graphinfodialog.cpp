#include "graphinfodialog.h"
#include "ui_graphinfodialog.h"

#include "../program/globals.h"
#include "../graph/assemblygraph.h"
#include <QPair>

GraphInfoDialog::GraphInfoDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GraphInfoDialog)
{
    ui->setupUi(this);

    setLabels();
    setInfoTexts();
}


GraphInfoDialog::~GraphInfoDialog()
{
    delete ui;
}



void GraphInfoDialog::setLabels()
{
    ui->filenameLabel->setText(g_assemblyGraph->m_filename);

    int nodeCount = g_assemblyGraph->m_nodeCount;

    ui->nodeCountLabel->setText(formatIntForDisplay(nodeCount));
    ui->edgeCountLabel->setText(formatIntForDisplay(g_assemblyGraph->m_edgeCount));

    if (g_assemblyGraph->m_edgeCount == 0)
        ui->edgeOverlapRangeLabel->setText("n/a");
    else
    {
        QPair<int, int> overlapRange = g_assemblyGraph->getOverlapRange();
        int smallestOverlap = overlapRange.first;
        int largestOverlap = overlapRange.second;
        if (smallestOverlap == largestOverlap)
            ui->edgeOverlapRangeLabel->setText(formatIntForDisplay(smallestOverlap) + " bp");
        else
            ui->edgeOverlapRangeLabel->setText(formatIntForDisplay(smallestOverlap) + " to " + formatIntForDisplay(largestOverlap) + " bp");
    }

    ui->totalLengthLabel->setText(formatIntForDisplay(g_assemblyGraph->m_totalLength) + " bp");
    ui->totalLengthNoOverlapsLabel->setText(formatIntForDisplay(g_assemblyGraph->getTotalLengthMinusEdgeOverlaps()) + " bp");

    int deadEnds = g_assemblyGraph->getDeadEndCount();
    double percentageDeadEnds = 100.0 * double(deadEnds) / (2 * nodeCount);

    ui->deadEndsLabel->setText(formatIntForDisplay(deadEnds));
    ui->percentageDeadEndsLabel->setText(formatDoubleForDisplay(percentageDeadEnds, 2) + "%");


    int componentCount = 0;
    int largestComponentLength = 0;
    g_assemblyGraph->getGraphComponentCountAndLargestComponentSize(&componentCount, &largestComponentLength);

    ui->connectedComponentsLabel->setText(formatIntForDisplay(componentCount));
    ui->largestComponentLabel->setText(formatIntForDisplay(largestComponentLength) + " bp");
    ui->orphanedLengthLabel->setText(formatIntForDisplay(g_assemblyGraph->getTotalLengthOrphanedNodes()) + " bp");

    int n50 = 0;
    int shortestNode = 0;
    int firstQuartile = 0;
    int median = 0;
    int thirdQuartile = 0;
    int longestNode = 0;
    g_assemblyGraph->getNodeStats(&n50, &shortestNode, &firstQuartile, &median, &thirdQuartile, &longestNode);

    ui->n50Label->setText(formatIntForDisplay(n50) + " bp");
    ui->shortestNodeLabel->setText(formatIntForDisplay(shortestNode) + " bp");
    ui->lowerQuartileNodeLabel->setText(formatIntForDisplay(firstQuartile) + " bp");
    ui->medianNodeLabel->setText(formatIntForDisplay(median) + " bp");
    ui->upperQuartileNodeLabel->setText(formatIntForDisplay(thirdQuartile) + " bp");
    ui->longestNodeLabel->setText(formatIntForDisplay(longestNode) + " bp");

    double medianDepthByBase = g_assemblyGraph->getMedianDepthByBase();
    long long estimatedSequenceLength = g_assemblyGraph->getEstimatedSequenceLength(medianDepthByBase);

    ui->medianDepthLabel->setText(formatDepthForDisplay(medianDepthByBase));
    if (medianDepthByBase == 0.0)
        ui->estimatedSequenceLengthLabel->setText("unavailable");
    else
        ui->estimatedSequenceLengthLabel->setText(formatIntForDisplay(estimatedSequenceLength) + " bp");
}





void GraphInfoDialog::setInfoTexts()
{
    ui->nodeCountInfoText->setInfoText("The number of positive nodes in the graph.<br><br>"
                                       "Since only positive nodes are counted, each complementary node pair counts "
                                       "as one.");
    ui->edgeCountInfoText->setInfoText("The number of positive edges in the graph.<br><br>"
                                       "Since only positive edges are counted, each complementary edge pair counts "
                                       "as one.");
    ui->edgeOverlapRangeInfoText->setInfoText("The sequence overlap size for edges in the graph<br><br>"
                                              "For most graphs, this will either be zero or a single value, but it is "
                                              "also possible for a graph to have edges with different overlaps, "
                                              "in which case this will show the range.");
    ui->totalLengthInfoText->setInfoText("The total length of all sequences in positive nodes.<br><br>"
                                         "This value is a simple sum of node sequence lengths and does not take "
                                         "node overlaps into account.");
    ui->totalLengthNoOverlapsInfoText->setInfoText("The total length of all sequences in positive nodes, with the edge "
                                                   "overlaps removed.<br><br>"
                                                   "This value is the above 'Total length' minus the total overlaps in "
                                                   "positive edges. If the edges in the graph do not contain "
                                                   "overlaps, then this value will be the same as 'Total length'.");
    ui->deadEndsInfoText->setInfoText("The number of dead ends (node ends without any connected edges) in all positive "
                                      "nodes.<br><br>"
                                      "This value will range from 0 (in a graph where all nodes have connected edges "
                                      "on both ends) to two times the node count (in a graph without any edges).");
    ui->percentageDeadEndsInfoText->setInfoText("The fraction of possible dead ends in the graph.<br><br>"
                                                "This value is equal to 'Dead ends' divided by twice the node count. "
                                                "It ranges from 0% (in a graph where all nodes have connected edges "
                                                "on both ends) to 100% (in a graph without any edges).");
    ui->connectedComponentsInfoText->setInfoText("The number of separate connected components in the graph. A "
                                                 "connected component is a subgraph which is connected by edges to "
                                                 "other nodes in the subgraph but disconnected from the rest of the "
                                                 "graph.");
    ui->largestComponentInfoText->setInfoText("The total length of nodes in the largest connected component in the "
                                              "graph.");
    ui->orphanedLengthInfoText->setInfoText("The total length of all nodes which have no edges (i.e. a dead end on "
                                            "both sides).");
    ui->n50InfoText->setInfoText("The N50 node length.<br><br>"
                                 "The sum of lengths for nodes this size and larger is at least 50% of the total "
                                 "length.");
    ui->shortestNodeInfoText->setInfoText("The length of the shortest node in the graph.");
    ui->lowerQuartileNodeInfoText->setInfoText("The first quartile node length in the graph.<br><br>"
                                               "One quarter of the nodes are shorter than this length and three quarters "
                                               "are longer.");
    ui->medianNodeInfoText->setInfoText("The median node length in the graph.<br><br>"
                                        "Half of the nodes are shorter than this length and half are longer.");
    ui->upperQuartileNodeInfoText->setInfoText("The third quartile node length in the graph.<br><br>"
                                               "Three quarters of the nodes are shorter than this length and one quarter "
                                               "are longer.");
    ui->longestNodeInfoText->setInfoText("The length of the longest node in the graph.");
    ui->medianDepthInfoText->setInfoText("The median depth of the graph nodes, by base.<br><br>"
                                             "For most assemblies (assuming the majority of sequences are not repeats) "
                                             "this value will indicate the approximate depth of nodes for "
                                             "sequences that occur once.<br><br>"
                                             "E.g. if this value is 14.5x, then:<ul>"
                                             "<li>a node with depth 14.2x probably represents a sequence which "
                                             "occurs once</li>"
                                             "<li>a node with depth 29.3x probably represents a sequence which "
                                             "occurs twice</li>"
                                             "<li>a node with depth 43.4x probably represents a sequence which "
                                             "occurs twice</li>"
                                             "<li>etc.</li></ul>");
    ui->estimatedSequenceLengthInfoText->setInfoText("An estimate of the underlying sequence length, accounting for "
                                                     "repeated sequences.<br><br>"
                                                     "Repeated sequences can result in the graph's total length being "
                                                     "less than the total length of the underlying sequence. This is "
                                                     "because a sequence which occurs multiple times may assemble into "
                                                     "a single node.<br><br>"
                                                     "To account for this, Bandage estimates sequence length by using "
                                                     "median depth to assign a copy number count to each node. "
                                                     "The node lengths are then multipled by this copy number and "
                                                     "totalled (with any overlap sequences removed).<br><br>"
                                                     "Note that this value represents an estimate of the number of "
                                                     "base pairs per cell. So if a bacterial cell contained 20 copies "
                                                     "of a 10 kb plasmid, then that plasmid would contribute 200 kb to "
                                                     "this value.");
}
