#ifndef QUERYPATHSPUSHBUTTON_H
#define QUERYPATHSPUSHBUTTON_H

#include <QPushButton>

class BlastQuery;

class QueryPathsPushButton : public QPushButton
{
    Q_OBJECT

public:
    QueryPathsPushButton(int pathCount, BlastQuery * query);

private:
    int m_pathCount;
    BlastQuery * m_query;

private slots:
    void clickedSlot();

signals:
    void showPathsDialog(BlastQuery *);


};

#endif // QUERYPATHSPUSHBUTTON_H
