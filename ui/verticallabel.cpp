// http://stackoverflow.com/questions/9183050/vertical-qlabel-or-the-equivalent

#include "verticallabel.h"
#include <QPainter>

VerticalLabel::VerticalLabel(QWidget *parent)
    : QLabel(parent)
{
}

VerticalLabel::VerticalLabel(const QString &text, QWidget *parent)
: QLabel(text, parent)
{
}

void VerticalLabel::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    painter.setPen(Qt::black);
    painter.setBrush(Qt::Dense1Pattern);

    double shiftForCentering = (height() - sizeHint().height()) / 2.0;
    painter.translate(sizeHint().width(), sizeHint().height() + shiftForCentering);
    painter.rotate(270);

    painter.drawText(0,0, text());
}

QSize VerticalLabel::minimumSizeHint() const
{
    QSize s = QLabel::minimumSizeHint();
    return QSize(s.height(), s.width());
}

QSize VerticalLabel::sizeHint() const
{
    QSize s = QLabel::sizeHint();
    return QSize(s.height(), s.width());
}
