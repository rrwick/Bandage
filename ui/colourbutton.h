#ifndef COLOURBUTTON_H
#define COLOURBUTTON_H

#include <QPushButton>
#include <QColor>
#include <QString>

class ColourButton : public QPushButton
{
    Q_OBJECT

public:
    ColourButton(QWidget * parent = 0);

    QColor m_colour;
    QString m_name;

    void setColour(QColor newColour);

public slots:
    void chooseColour();
};

#endif // COLOURBUTTON_H
