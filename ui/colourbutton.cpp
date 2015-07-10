#include "colourbutton.h"
#include <QColorDialog>

ColourButton::ColourButton(QWidget * parent) :
    QPushButton(parent)
{
    connect(this, SIGNAL(clicked(bool)), this, SLOT(chooseColour()));
}


void ColourButton::setColour(QColor newColour)
{
    m_colour = newColour;

    const QString COLOR_STYLE("QPushButton { background-color : %1 }");
    setStyleSheet(COLOR_STYLE.arg(m_colour.name()));
}



void ColourButton::chooseColour()
{
    QColor chosenColor = QColorDialog::getColor(m_colour, this, m_name, QColorDialog::ShowAlphaChannel);
    if (chosenColor.isValid())
        setColour(chosenColor);
}
