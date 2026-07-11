#ifndef TREELAYOUTWORKER_H
#define TREELAYOUTWORKER_H

#include <QObject>
#include "../ogdf/tree/TreeLayout.h"
#include "../ogdf/basic/GraphAttributes.h"

using namespace ogdf;

class TreeLayoutWorker : public QObject
{
    Q_OBJECT

public:
    TreeLayoutWorker(TreeLayout* fmmm, ogdf::GraphAttributes* graphAttributes,
        ogdf::EdgeArray<double>* edgeArray);

    ogdf::TreeLayout* m_treeLayout;
    ogdf::GraphAttributes* m_graphAttributes;
    ogdf::EdgeArray<double>* m_edgeArray;

public slots:
    void layoutGraph();

signals:
    void finishedLayout();
};

#endif // TREELAYOUTWORKER_H
