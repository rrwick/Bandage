//Copyright 2016 Ryan Wick

//This file is part of Bandage

//Bandage is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//Bandage is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

//You should have received a copy of the GNU General Public License
//along with Bandage.  If not, see <http://www.gnu.org/licenses/>.


#include "globals.h"
#include <QLocale>
#include <QDir>
#include <QStringList>
#include <QFile>
#include <QTextStream>
#include <QApplication>
#include <QProcess>
#include <math.h>

QSharedPointer<Settings> g_settings;
QSharedPointer<Memory> g_memory;
MyGraphicsView * g_graphicsView;
double g_absoluteZoom;
QSharedPointer<BlastSearch> g_blastSearch;
QString m_tempDirectory;
QSharedPointer<AssemblyGraph> g_assemblyGraph;


QString formatIntForDisplay(int num)
{
    QLocale locale;
    return locale.toString(num);
}

QString formatIntForDisplay(long long num)
{
    QLocale locale;
    return locale.toString(num);
}

QString formatDoubleForDisplay(double num, int decimalPlacesToDisplay)
{
    //Add a bit for rounding
    double addValue = 0.5 / pow(10, decimalPlacesToDisplay);
    num += addValue;

    QLocale locale;
    QString withCommas = locale.toString(num, 'f');

    QString final;
    bool pastDecimalPoint = false;
    int numbersPastDecimalPoint = 0;
    for (int i = 0; i < withCommas.length(); ++i)
    {
        final += withCommas[i];

        if (pastDecimalPoint)
            ++numbersPastDecimalPoint;

        if (numbersPastDecimalPoint >= decimalPlacesToDisplay)
            return final;

        if (withCommas[i] == locale.decimalPoint())
            pastDecimalPoint = true;
    }
    return final;
}


QString formatDepthForDisplay(double depth)
{
    if (depth == 0.0)
        return "0.0x";

    int decimals = 1;
    double multipliedDepth = fabs(depth);
    while (multipliedDepth < 10.0)
    {
        multipliedDepth *= 10.0;
        decimals += 1;
    }
    return formatDoubleForDisplay(depth, decimals) + "x";
}



std::vector<QColor> getPresetColours()
{
    std::vector<QColor> presetColours;

    presetColours.push_back(QColor("#306FF8"));
    presetColours.push_back(QColor("#86BB18"));
    presetColours.push_back(QColor("#DF123A"));
    presetColours.push_back(QColor("#181E2A"));
    presetColours.push_back(QColor("#F91BBD"));
    presetColours.push_back(QColor("#3CB2A4"));
    presetColours.push_back(QColor("#D29AC1"));
    presetColours.push_back(QColor("#E2922E"));
    presetColours.push_back(QColor("#22501B"));
    presetColours.push_back(QColor("#57297D"));
    presetColours.push_back(QColor("#3FA0E6"));
    presetColours.push_back(QColor("#770739"));
    presetColours.push_back(QColor("#6A390C"));
    presetColours.push_back(QColor("#25AB5D"));
    presetColours.push_back(QColor("#ACAF61"));
    presetColours.push_back(QColor("#F0826F"));
    presetColours.push_back(QColor("#E94A80"));
    presetColours.push_back(QColor("#C187F2"));
    presetColours.push_back(QColor("#7E5764"));
    presetColours.push_back(QColor("#037290"));
    presetColours.push_back(QColor("#D65114"));
    presetColours.push_back(QColor("#08396A"));
    presetColours.push_back(QColor("#99ABBE"));
    presetColours.push_back(QColor("#F270C0"));
    presetColours.push_back(QColor("#F056F9"));
    presetColours.push_back(QColor("#8E8D00"));
    presetColours.push_back(QColor("#70010F"));
    presetColours.push_back(QColor("#9C1E9A"));
    presetColours.push_back(QColor("#471B1F"));
    presetColours.push_back(QColor("#A00B6D"));
    presetColours.push_back(QColor("#38C037"));
    presetColours.push_back(QColor("#282C16"));
    presetColours.push_back(QColor("#15604D"));
    presetColours.push_back(QColor("#2E75D6"));
    presetColours.push_back(QColor("#A09DEB"));
    presetColours.push_back(QColor("#8454D7"));
    presetColours.push_back(QColor("#301745"));
    presetColours.push_back(QColor("#A45704"));
    presetColours.push_back(QColor("#4D8C0E"));
    presetColours.push_back(QColor("#C09860"));
    presetColours.push_back(QColor("#009C73"));
    presetColours.push_back(QColor("#FD6453"));
    presetColours.push_back(QColor("#C11C4B"));
    presetColours.push_back(QColor("#183B8B"));
    presetColours.push_back(QColor("#5E6706"));
    presetColours.push_back(QColor("#E42005"));
    presetColours.push_back(QColor("#4873AF"));
    presetColours.push_back(QColor("#6CA563"));
    presetColours.push_back(QColor("#5E0F54"));
    presetColours.push_back(QColor("#FE2065"));
    presetColours.push_back(QColor("#5BB4D2"));
    presetColours.push_back(QColor("#3F4204"));
    presetColours.push_back(QColor("#521839"));
    presetColours.push_back(QColor("#9A7706"));
    presetColours.push_back(QColor("#77AB8C"));
    presetColours.push_back(QColor("#105E04"));
    presetColours.push_back(QColor("#98290F"));
    presetColours.push_back(QColor("#B849D4"));
    presetColours.push_back(QColor("#FC8426"));
    presetColours.push_back(QColor("#341B03"));
    presetColours.push_back(QColor("#E3278C"));
    presetColours.push_back(QColor("#F28F93"));
    presetColours.push_back(QColor("#D1A21F"));
    presetColours.push_back(QColor("#277E46"));
    presetColours.push_back(QColor("#285C60"));
    presetColours.push_back(QColor("#76B945"));
    presetColours.push_back(QColor("#E75D65"));
    presetColours.push_back(QColor("#84ADDC"));
    presetColours.push_back(QColor("#153C2B"));
    presetColours.push_back(QColor("#FD10D9"));
    presetColours.push_back(QColor("#C095D5"));
    presetColours.push_back(QColor("#052B48"));
    presetColours.push_back(QColor("#B365FC"));
    presetColours.push_back(QColor("#97AA75"));
    presetColours.push_back(QColor("#C78C9C"));
    presetColours.push_back(QColor("#FD4838"));
    presetColours.push_back(QColor("#F181E2"));
    presetColours.push_back(QColor("#815A1A"));
    presetColours.push_back(QColor("#BB2093"));
    presetColours.push_back(QColor("#691822"));
    presetColours.push_back(QColor("#C41A12"));
    presetColours.push_back(QColor("#728A1F"));
    presetColours.push_back(QColor("#375B73"));
    presetColours.push_back(QColor("#97022C"));
    presetColours.push_back(QColor("#95B44D"));
    presetColours.push_back(QColor("#EB8DBB"));
    presetColours.push_back(QColor("#83ACAB"));
    presetColours.push_back(QColor("#E37D51"));
    presetColours.push_back(QColor("#D78A68"));
    presetColours.push_back(QColor("#4A41A2"));
    presetColours.push_back(QColor("#8A0C79"));
    presetColours.push_back(QColor("#133102"));
    presetColours.push_back(QColor("#237A78"));
    presetColours.push_back(QColor("#ADB03B"));
    presetColours.push_back(QColor("#289E26"));
    presetColours.push_back(QColor("#7683EC"));
    presetColours.push_back(QColor("#4E1E04"));
    presetColours.push_back(QColor("#BB17B2"));
    presetColours.push_back(QColor("#EB6A81"));
    presetColours.push_back(QColor("#47B4E8"));
    presetColours.push_back(QColor("#0A6191"));
    presetColours.push_back(QColor("#4EADB2"));
    presetColours.push_back(QColor("#442965"));
    presetColours.push_back(QColor("#FE784B"));
    presetColours.push_back(QColor("#55BD8D"));
    presetColours.push_back(QColor("#742B03"));
    presetColours.push_back(QColor("#8C38AA"));
    presetColours.push_back(QColor("#F758A6"));
    presetColours.push_back(QColor("#A32526"));
    presetColours.push_back(QColor("#442C2E"));
    presetColours.push_back(QColor("#F06A97"));
    presetColours.push_back(QColor("#3A1527"));
    presetColours.push_back(QColor("#503509"));
    presetColours.push_back(QColor("#2A67B4"));
    presetColours.push_back(QColor("#243644"));
    presetColours.push_back(QColor("#A74006"));
    presetColours.push_back(QColor("#335900"));
    presetColours.push_back(QColor("#A07484"));
    presetColours.push_back(QColor("#490216"));
    presetColours.push_back(QColor("#B19BCB"));
    presetColours.push_back(QColor("#75B75A"));
    presetColours.push_back(QColor("#BE71EB"));
    presetColours.push_back(QColor("#024A2E"));
    presetColours.push_back(QColor("#A097AB"));
    presetColours.push_back(QColor("#7A287E"));
    presetColours.push_back(QColor("#6A1444"));
    presetColours.push_back(QColor("#212449"));
    presetColours.push_back(QColor("#B07017"));
    presetColours.push_back(QColor("#227D57"));
    presetColours.push_back(QColor("#1B8CAF"));
    presetColours.push_back(QColor("#016438"));
    presetColours.push_back(QColor("#EA64CF"));
    presetColours.push_back(QColor("#B5310E"));
    presetColours.push_back(QColor("#B00765"));
    presetColours.push_back(QColor("#5F42B3"));
    presetColours.push_back(QColor("#EF9649"));
    presetColours.push_back(QColor("#25717F"));
    presetColours.push_back(QColor("#BCA309"));
    presetColours.push_back(QColor("#FA35A6"));
    presetColours.push_back(QColor("#F63D54"));
    presetColours.push_back(QColor("#E83D6C"));
    presetColours.push_back(QColor("#8362F2"));
    presetColours.push_back(QColor("#33BC4A"));
    presetColours.push_back(QColor("#194A85"));
    presetColours.push_back(QColor("#E24215"));
    presetColours.push_back(QColor("#6D71FE"));
    presetColours.push_back(QColor("#3E52AF"));
    presetColours.push_back(QColor("#1E9E89"));
    presetColours.push_back(QColor("#740860"));
    presetColours.push_back(QColor("#4B7BEE"));
    presetColours.push_back(QColor("#8742C0"));
    presetColours.push_back(QColor("#DD8EC6"));
    presetColours.push_back(QColor("#CD202C"));
    presetColours.push_back(QColor("#FD82C2"));
    presetColours.push_back(QColor("#3C2874"));
    presetColours.push_back(QColor("#F9742B"));
    presetColours.push_back(QColor("#013B10"));
    presetColours.push_back(QColor("#D12867"));
    presetColours.push_back(QColor("#F743C3"));
    presetColours.push_back(QColor("#B98EEC"));
    presetColours.push_back(QColor("#D260EC"));
    presetColours.push_back(QColor("#671C06"));
    presetColours.push_back(QColor("#37A968"));
    presetColours.push_back(QColor("#3B9529"));
    presetColours.push_back(QColor("#2A0E33"));
    presetColours.push_back(QColor("#51B237"));
    presetColours.push_back(QColor("#95B61B"));
    presetColours.push_back(QColor("#B195E2"));
    presetColours.push_back(QColor("#68B49A"));
    presetColours.push_back(QColor("#182339"));
    presetColours.push_back(QColor("#FC4822"));
    presetColours.push_back(QColor("#D79621"));
    presetColours.push_back(QColor("#90761B"));
    presetColours.push_back(QColor("#777315"));
    presetColours.push_back(QColor("#E389E9"));
    presetColours.push_back(QColor("#35BD64"));
    presetColours.push_back(QColor("#C17910"));
    presetColours.push_back(QColor("#3386ED"));
    presetColours.push_back(QColor("#E82C2E"));
    presetColours.push_back(QColor("#AC925F"));
    presetColours.push_back(QColor("#F227C8"));
    presetColours.push_back(QColor("#F43E67"));
    presetColours.push_back(QColor("#55AEEB"));
    presetColours.push_back(QColor("#F518E3"));
    presetColours.push_back(QColor("#AB0643"));
    presetColours.push_back(QColor("#8DA1F3"));
    presetColours.push_back(QColor("#5C9C14"));
    presetColours.push_back(QColor("#381F27"));
    presetColours.push_back(QColor("#6BB7B5"));
    presetColours.push_back(QColor("#9842BE"));
    presetColours.push_back(QColor("#4897D6"));
    presetColours.push_back(QColor("#8958E4"));
    presetColours.push_back(QColor("#8F0065"));
    presetColours.push_back(QColor("#A10A5E"));
    presetColours.push_back(QColor("#076315"));
    presetColours.push_back(QColor("#FA5EF9"));
    presetColours.push_back(QColor("#A33402"));
    presetColours.push_back(QColor("#A0ABC4"));
    presetColours.push_back(QColor("#2B6EFE"));
    presetColours.push_back(QColor("#9A9EE7"));

    return presetColours;
}



//This function will convert a colour to its SVG name, if one exists, or the hex value otherwise.
QString getColourName(QColor colour)
{
    if (colour == QColor(240, 248, 255)) return "aliceblue";
    if (colour == QColor(250, 235, 215)) return "antiquewhite";
    if (colour == QColor(  0, 255, 255)) return "aqua";
    if (colour == QColor(127, 255, 212)) return "aquamarine";
    if (colour == QColor(240, 255, 255)) return "azure";
    if (colour == QColor(245, 245, 220)) return "beige";
    if (colour == QColor(255, 228, 196)) return "bisque";
    if (colour == QColor(  0,   0,   0)) return "black";
    if (colour == QColor(255, 235, 205)) return "blanchedalmond";
    if (colour == QColor(  0,   0, 255)) return "blue";
    if (colour == QColor(138,  43, 226)) return "blueviolet";
    if (colour == QColor(165,  42,  42)) return "brown";
    if (colour == QColor(222, 184, 135)) return "burlywood";
    if (colour == QColor( 95, 158, 160)) return "cadetblue";
    if (colour == QColor(127, 255,   0)) return "chartreuse";
    if (colour == QColor(210, 105,  30)) return "chocolate";
    if (colour == QColor(255, 127,  80)) return "coral";
    if (colour == QColor(100, 149, 237)) return "cornflowerblue";
    if (colour == QColor(255, 248, 220)) return "cornsilk";
    if (colour == QColor(220,  20,  60)) return "crimson";
    if (colour == QColor(  0, 255, 255)) return "cyan";
    if (colour == QColor(  0,   0, 139)) return "darkblue";
    if (colour == QColor(  0, 139, 139)) return "darkcyan";
    if (colour == QColor(184, 134,  11)) return "darkgoldenrod";
    if (colour == QColor(  0, 100,   0)) return "darkgreen";
    if (colour == QColor(169, 169, 169)) return "darkgrey";
    if (colour == QColor(189, 183, 107)) return "darkkhaki";
    if (colour == QColor(139,   0, 139)) return "darkmagenta";
    if (colour == QColor( 85, 107,  47)) return "darkolivegreen";
    if (colour == QColor(255, 140,   0)) return "darkorange";
    if (colour == QColor(153,  50, 204)) return "darkorchid";
    if (colour == QColor(139,   0,   0)) return "darkred";
    if (colour == QColor(233, 150, 122)) return "darksalmon";
    if (colour == QColor(143, 188, 143)) return "darkseagreen";
    if (colour == QColor( 72,  61, 139)) return "darkslateblue";
    if (colour == QColor( 47,  79,  79)) return "darkslategrey";
    if (colour == QColor(  0, 206, 209)) return "darkturquoise";
    if (colour == QColor(148,   0, 211)) return "darkviolet";
    if (colour == QColor(255,  20, 147)) return "deeppink";
    if (colour == QColor(  0, 191, 255)) return "deepskyblue";
    if (colour == QColor(105, 105, 105)) return "dimgrey";
    if (colour == QColor( 30, 144, 255)) return "dodgerblue";
    if (colour == QColor(178,  34,  34)) return "firebrick";
    if (colour == QColor(255, 250, 240)) return "floralwhite";
    if (colour == QColor( 34, 139,  34)) return "forestgreen";
    if (colour == QColor(255,   0, 255)) return "fuchsia";
    if (colour == QColor(220, 220, 220)) return "gainsboro";
    if (colour == QColor(248, 248, 255)) return "ghostwhite";
    if (colour == QColor(255, 215,   0)) return "gold";
    if (colour == QColor(218, 165,  32)) return "goldenrod";
    if (colour == QColor(128, 128, 128)) return "grey";
    if (colour == QColor(  0, 128,   0)) return "green";
    if (colour == QColor(173, 255,  47)) return "greenyellow";
    if (colour == QColor(240, 255, 240)) return "honeydew";
    if (colour == QColor(255, 105, 180)) return "hotpink";
    if (colour == QColor(205,  92,  92)) return "indianred";
    if (colour == QColor( 75,   0, 130)) return "indigo";
    if (colour == QColor(255, 255, 240)) return "ivory";
    if (colour == QColor(240, 230, 140)) return "khaki";
    if (colour == QColor(230, 230, 250)) return "lavender";
    if (colour == QColor(255, 240, 245)) return "lavenderblush";
    if (colour == QColor(124, 252,   0)) return "lawngreen";
    if (colour == QColor(255, 250, 205)) return "lemonchiffon";
    if (colour == QColor(173, 216, 230)) return "lightblue";
    if (colour == QColor(240, 128, 128)) return "lightcoral";
    if (colour == QColor(224, 255, 255)) return "lightcyan";
    if (colour == QColor(250, 250, 210)) return "lightgoldenrodyellow";
    if (colour == QColor(144, 238, 144)) return "lightgreen";
    if (colour == QColor(211, 211, 211)) return "lightgrey";
    if (colour == QColor(255, 182, 193)) return "lightpink";
    if (colour == QColor(255, 160, 122)) return "lightsalmon";
    if (colour == QColor( 32, 178, 170)) return "lightseagreen";
    if (colour == QColor(135, 206, 250)) return "lightskyblue";
    if (colour == QColor(119, 136, 153)) return "lightslategrey";
    if (colour == QColor(176, 196, 222)) return "lightsteelblue";
    if (colour == QColor(255, 255, 224)) return "lightyellow";
    if (colour == QColor(  0, 255,   0)) return "lime";
    if (colour == QColor( 50, 205,  50)) return "limegreen";
    if (colour == QColor(250, 240, 230)) return "linen";
    if (colour == QColor(255,   0, 255)) return "magenta";
    if (colour == QColor(128,   0,   0)) return "maroon";
    if (colour == QColor(102, 205, 170)) return "mediumaquamarine";
    if (colour == QColor(  0,   0, 205)) return "mediumblue";
    if (colour == QColor(186,  85, 211)) return "mediumorchid";
    if (colour == QColor(147, 112, 219)) return "mediumpurple";
    if (colour == QColor( 60, 179, 113)) return "mediumseagreen";
    if (colour == QColor(123, 104, 238)) return "mediumslateblue";
    if (colour == QColor(  0, 250, 154)) return "mediumspringgreen";
    if (colour == QColor( 72, 209, 204)) return "mediumturquoise";
    if (colour == QColor(199,  21, 133)) return "mediumvioletred";
    if (colour == QColor( 25,  25, 112)) return "midnightblue";
    if (colour == QColor(245, 255, 250)) return "mintcream";
    if (colour == QColor(255, 228, 225)) return "mistyrose";
    if (colour == QColor(255, 228, 181)) return "moccasin";
    if (colour == QColor(255, 222, 173)) return "navajowhite";
    if (colour == QColor(  0,   0, 128)) return "navy";
    if (colour == QColor(253, 245, 230)) return "oldlace";
    if (colour == QColor(128, 128,   0)) return "olive";
    if (colour == QColor(107, 142,  35)) return "olivedrab";
    if (colour == QColor(255, 165,   0)) return "orange";
    if (colour == QColor(255,  69,   0)) return "orangered";
    if (colour == QColor(218, 112, 214)) return "orchid";
    if (colour == QColor(238, 232, 170)) return "palegoldenrod";
    if (colour == QColor(152, 251, 152)) return "palegreen";
    if (colour == QColor(175, 238, 238)) return "paleturquoise";
    if (colour == QColor(219, 112, 147)) return "palevioletred";
    if (colour == QColor(255, 239, 213)) return "papayawhip";
    if (colour == QColor(255, 218, 185)) return "peachpuff";
    if (colour == QColor(205, 133,  63)) return "peru";
    if (colour == QColor(255, 192, 203)) return "pink";
    if (colour == QColor(221, 160, 221)) return "plum";
    if (colour == QColor(176, 224, 230)) return "powderblue";
    if (colour == QColor(128,   0, 128)) return "purple";
    if (colour == QColor(255,   0,   0)) return "red";
    if (colour == QColor(188, 143, 143)) return "rosybrown";
    if (colour == QColor( 65, 105, 225)) return "royalblue";
    if (colour == QColor(139,  69,  19)) return "saddlebrown";
    if (colour == QColor(250, 128, 114)) return "salmon";
    if (colour == QColor(244, 164,  96)) return "sandybrown";
    if (colour == QColor( 46, 139,  87)) return "seagreen";
    if (colour == QColor(255, 245, 238)) return "seashell";
    if (colour == QColor(160,  82,  45)) return "sienna";
    if (colour == QColor(192, 192, 192)) return "silver";
    if (colour == QColor(135, 206, 235)) return "skyblue";
    if (colour == QColor(106,  90, 205)) return "slateblue";
    if (colour == QColor(112, 128, 144)) return "slategrey";
    if (colour == QColor(255, 250, 250)) return "snow";
    if (colour == QColor(  0, 255, 127)) return "springgreen";
    if (colour == QColor( 70, 130, 180)) return "steelblue";
    if (colour == QColor(210, 180, 140)) return "tan";
    if (colour == QColor(  0, 128, 128)) return "teal";
    if (colour == QColor(216, 191, 216)) return "thistle";
    if (colour == QColor(255,  99,  71)) return "tomato";
    if (colour == QColor( 64, 224, 208)) return "turquoise";
    if (colour == QColor(238, 130, 238)) return "violet";
    if (colour == QColor(245, 222, 179)) return "wheat";
    if (colour == QColor(255, 255, 255)) return "white";
    if (colour == QColor(245, 245, 245)) return "whitesmoke";
    if (colour == QColor(255, 255,   0)) return "yellow";
    if (colour == QColor(154, 205,  50)) return "yellowgreen";

    return colour.name();
}


