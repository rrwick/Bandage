#include "commoncommandlinefunctions.h"
#include "../graph/assemblygraph.h"
#include <ogdf/energybased/FMMMLayout.h>
#include "../program/graphlayoutworker.h"
#include "../program/settings.h"

bool loadAssemblyGraph(QString filename)
{
    g_assemblyGraph = new AssemblyGraph();
    return g_assemblyGraph->loadGraphFromFile(filename);
}



//Unlike the equivalent function in MainWindow, this does the graph layout in the main thread.
void layoutGraph()
{
    ogdf::FMMMLayout fmmm;
    GraphLayoutWorker * graphLayoutWorker = new GraphLayoutWorker(&fmmm, g_assemblyGraph->m_graphAttributes,
                                                                  g_settings->graphLayoutQuality, g_settings->segmentLength);
    graphLayoutWorker->layoutGraph();
}



//Returns empty string if everything is okay and an error
//message if there's a problem.  If everything is okay, it
//also removes the option and its value from arguments.
QString checkOptionForInt(QString option, QStringList * arguments, int min, int max)
{
    int optionIndex = arguments->indexOf(option);

    //If the option isn't found, that's fine.
    if (optionIndex == -1)
        return "";

    int integerIndex = optionIndex + 1;

    //If nothing follows the option, that's a problem.
    if (integerIndex >= arguments->size())
        return option + " must be followed by an integer";

    //If the thing following the option isn't an integer, that's a problem.
    bool optionIsInt;
    int optionInt = arguments->at(integerIndex).toInt(&optionIsInt);
    if (!optionIsInt)
        return option + " must be followed by an integer";

    //Check the range of the option.
    if (optionInt < min || optionInt > max)
        return "Value of " + option + " must be between "
                + QString::number(min) + " and " + QString::number(max) +
                " (inclusive)";

    //If the code got here, the option and its integer are okay.
    //Remove them from the arguments.
    arguments->removeAt(integerIndex);
    arguments->removeAt(optionIndex);

    return "";
}

//This function simply removes an option from arguments if it is found.
void checkOptionWithoutValue(QString option, QStringList * arguments)
{
    int optionIndex = arguments->indexOf(option);

    //If the option isn't found, that's fine.
    if (optionIndex == -1)
        return ;

    //If the code got here, the option is okay.
    //Remove it from the arguments.
    arguments->removeAt(optionIndex);
}


//This function generates an error if excess arguments are left after
//parsing.
QString checkForExcessArguments(QStringList arguments)
{
    if (arguments.size() == 0)
        return "";

    QString invalidOptionText = "Invalid option";
    if (arguments.size() > 1)
        invalidOptionText += "s";
    invalidOptionText += ": ";
    for (int i = 0; i < arguments.size(); ++i)
    {
        invalidOptionText += arguments.at(i);
        if (i < arguments.size() - 1)
            invalidOptionText += ", ";
    }

    return invalidOptionText;
}
