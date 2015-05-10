// http://stackoverflow.com/questions/9183050/vertical-qlabel-or-the-equivalent

#ifndef VERTICALLABEL_H
#define VERTICALLABEL_H

#include <QLabel>

class VerticalLabel : public QLabel
{
    Q_OBJECT

public:
    explicit VerticalLabel(QWidget *parent=0);
    explicit VerticalLabel(const QString &text, QWidget *parent=0);

protected:
    void paintEvent(QPaintEvent*);
    QSize sizeHint() const ;
    QSize minimumSizeHint() const;
};

#endif // VERTICALLABEL_H
