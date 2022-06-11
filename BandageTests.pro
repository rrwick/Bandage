# Copyright 2015 Ryan Wick

# This file is part of Bandage

# Bandage is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.

# Bandage is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with Bandage.  If not, see <http://www.gnu.org/licenses/>.

QT       += core gui svg testlib

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = BandageTests
TEMPLATE = app

CONFIG += c++11

target.path += /usr/local/bin
INSTALLS += target

INCLUDEPATH += ui

SOURCES += \
    program/settings.cpp \
    program/globals.cpp \
    program/graphlayoutworker.cpp \
		program/dotplot.cpp \
    graph/debruijnnode.cpp \
    graph/debruijnedge.cpp \
    graph/graphicsitemnode.cpp \
    graph/graphicsitemedge.cpp \
    ui/mainwindow.cpp \
    ui/graphicsviewzoom.cpp \
    ui/settingsdialog.cpp \
    ui/mygraphicsview.cpp \
    ui/mygraphicsscene.cpp \
    ui/aboutdialog.cpp \
    ui/enteroneblastquerydialog.cpp \
    blast/blasthit.cpp \
    blast/blastqueries.cpp \
    ui/blastsearchdialog.cpp \
    ui/infotextwidget.cpp \
    graph/assemblygraph.cpp \
    ui/verticalscrollarea.cpp \
    ui/myprogressdialog.cpp \
    ui/nodewidthvisualaid.cpp \
    ui/verticallabel.cpp \
    command_line/load.cpp \
    command_line/image.cpp \
    command_line/commoncommandlinefunctions.cpp \
    ui/mytablewidget.cpp \
    blast/buildblastdatabaseworker.cpp \
    ui/colourbutton.cpp \
    blast/blastquery.cpp \
    blast/runblastsearchworker.cpp \
    blast/blastsearch.cpp \
    graph/path.cpp \
    ui/pathspecifydialog.cpp \
    graph/graphlocation.cpp \
    ui/tablewidgetitemint.cpp \
    ui/tablewidgetitemdouble.cpp \
    ui/tablewidgetitemshown.cpp \
    program/memory.cpp \
    ui/querypathspushbutton.cpp \
    ui/querypathsdialog.cpp \
    blast/blastquerypath.cpp \
    tests/bandagetests.cpp \
    ui/blasthitfiltersdialog.cpp \
    program/scinot.cpp \
    ui/changenodenamedialog.cpp \
    ui/changenodedepthdialog.cpp \
    ui/querypathsequencecopybutton.cpp \
    command_line/querypaths.cpp \
    command_line/info.cpp \
    command_line/reduce.cpp \
    ogdf/basic/Graph.cpp \
    ogdf/basic/GraphAttributes.cpp \
    ogdf/energybased/FMMMLayout.cpp \
    ogdf/basic/geometry.cpp \
    ogdf/cluster/ClusterGraphAttributes.cpp \
    ogdf/internal/energybased/FruchtermanReingold.cpp \
    ogdf/internal/energybased/NMM.cpp \
    ogdf/fileformats/GmlParser.cpp \
    ogdf/basic/simple_graph_alg.cpp \
    ogdf/basic/basic.cpp \
    ogdf/fileformats/XmlParser.cpp \
    ogdf/basic/String.cpp \
    ogdf/basic/Hashing.cpp \
    ogdf/internal/basic/PoolMemoryAllocator.cpp \
    ogdf/basic/GraphCopy.cpp \
    ogdf/basic/CombinatorialEmbedding.cpp \
    ogdf/fileformats/OgmlParser.cpp \
    ogdf/cluster/ClusterGraph.cpp \
    ogdf/basic/Math.cpp \
    ogdf/internal/energybased/EdgeAttributes.cpp \
    ogdf/internal/energybased/NodeAttributes.cpp \
    ogdf/energybased/MAARPacking.cpp \
    ogdf/energybased/Multilevel.cpp \
    ogdf/energybased/numexcept.cpp \
    ogdf/energybased/Set.cpp \
    ogdf/fileformats/Ogml.cpp \
    ogdf/fileformats/DinoXmlParser.cpp \
    ogdf/fileformats/DinoXmlScanner.cpp \
    ogdf/fileformats/DinoTools.cpp \
    ogdf/fileformats/DinoLineBuffer.cpp \
    ogdf/basic/System.cpp \
    ogdf/internal/energybased/QuadTreeNM.cpp \
    ogdf/internal/energybased/QuadTreeNodeNM.cpp \
    ogdf/basic/Constraint.cpp \
    ogdf/internal/energybased/MultilevelGraph.cpp \
    ui/graphinfodialog.cpp \
    ui/tablewidgetitemname.cpp

HEADERS  += \
    program/settings.h \
    program/globals.h \
    program/graphlayoutworker.h \
		program/dotplot.h \
    graph/debruijnnode.h \
    graph/debruijnedge.h \
    graph/graphicsitemnode.h \
    graph/graphicsitemedge.h \
    graph/ogdfnode.h \
    ui/mainwindow.h \
    ui/graphicsviewzoom.h \
    ui/settingsdialog.h \
    ui/mygraphicsview.h \
    ui/mygraphicsscene.h \
    ui/aboutdialog.h \
    ui/enteroneblastquerydialog.h \
    blast/blasthitpart.h \
    blast/blasthit.h \
    blast/blastquery.h \
    blast/blastqueries.h \
    blast/blastsearch.h \
    ui/blastsearchdialog.h \
    ui/infotextwidget.h \
    graph/assemblygraph.h \
    ui/verticalscrollarea.h \
    ui/myprogressdialog.h \
    ui/nodewidthvisualaid.h \
    ui/verticallabel.h \
    command_line/load.h \
    command_line/image.h \
    command_line/commoncommandlinefunctions.h \
    ui/mytablewidget.h \
    blast/buildblastdatabaseworker.h \
    ui/colourbutton.h \
    blast/runblastsearchworker.h \
    graph/path.h \
    ui/pathspecifydialog.h \
    graph/graphlocation.h \
    ui/tablewidgetitemint.h \
    ui/tablewidgetitemdouble.h \
    ui/tablewidgetitemshown.h \
    program/memory.h \
    graph/querydistance.h \
    ui/querypathspushbutton.h \
    ui/querypathsdialog.h \
    blast/blastquerypath.h \
    ui/blasthitfiltersdialog.h \
    program/scinot.h \
    ui/changenodenamedialog.h \
    ui/changenodedepthdialog.h \
    ui/querypathsequencecopybutton.h \
    command_line/querypaths.h \
    command_line/info.h \
    command_line/reduce.h \
    ogdf/basic/Graph.h \
    ogdf/basic/GraphAttributes.h \
    ogdf/energybased/FMMMLayout.h \
    ogdf/basic/geometry.h \
    ogdf/cluster/ClusterGraphAttributes.h \
    ogdf/module/LayoutModule.h \
    ogdf/internal/energybased/FruchtermanReingold.h \
    ogdf/internal/energybased/NMM.h \
    ogdf/basic/AdjEntryArray.h \
    ogdf/basic/Array.h \
    ogdf/fileformats/GmlParser.h \
    ogdf/basic/GraphObserver.h \
    ogdf/basic/simple_graph_alg.h \
    ogdf/basic/basic.h \
    ogdf/basic/EdgeArray.h \
    ogdf/basic/List.h \
    ogdf/basic/NodeArray.h \
    ogdf/basic/Graph_d.h \
    ogdf/fileformats/XmlParser.h \
    ogdf/basic/String.h \
    ogdf/internal/basic/list_templates.h \
    ogdf/basic/Hashing.h \
    ogdf/basic/comparer.h \
    ogdf/basic/exceptions.h \
    ogdf/basic/memory.h \
    ogdf/internal/basic/MallocMemoryAllocator.h \
    ogdf/internal/basic/PoolMemoryAllocator.h \
    ogdf/basic/BoundedStack.h \
    ogdf/basic/GraphCopy.h \
    ogdf/basic/SList.h \
    ogdf/basic/Stack.h \
    ogdf/basic/tuples.h \
    ogdf/basic/FaceSet.h \
    ogdf/basic/FaceArray.h \
    ogdf/basic/CombinatorialEmbedding.h \
    ogdf/cluster/ClusterArray.h \
    ogdf/fileformats/OgmlParser.h \
    ogdf/cluster/ClusterGraph.h \
    ogdf/cluster/ClusterGraphObserver.h \
    ogdf/basic/HashArray.h \
    ogdf/basic/Math.h \
    ogdf/internal/energybased/EdgeAttributes.h \
    ogdf/internal/energybased/NodeAttributes.h \
    ogdf/energybased/Edge.h \
    ogdf/energybased/MAARPacking.h \
    ogdf/energybased/Multilevel.h \
    ogdf/energybased/numexcept.h \
    ogdf/energybased/Rectangle.h \
    ogdf/energybased/PackingRowInfo.h \
    ogdf/energybased/PQueue.h \
    ogdf/energybased/Set.h \
    ogdf/energybased/Node.h \
    ogdf/fileformats/Ogml.h \
    ogdf/fileformats/DinoXmlParser.h \
    ogdf/fileformats/DinoXmlScanner.h \
    ogdf/fileformats/DinoTools.h \
    ogdf/fileformats/DinoLineBuffer.h \
    ogdf/fileformats/XmlObject.h \
    ogdf/basic/CriticalSection.h \
    ogdf/basic/System.h \
    ogdf/basic/Array2D.h \
    ogdf/internal/energybased/ParticleInfo.h \
    ogdf/internal/energybased/QuadTreeNM.h \
    ogdf/internal/energybased/QuadTreeNodeNM.h \
    ogdf/basic/Constraints.h \
    ogdf/internal/energybased/MultilevelGraph.h \
    ui/graphinfodialog.h \
    ui/tablewidgetitemname.h

FORMS    += \
    ui/mainwindow.ui \
    ui/settingsdialog.ui \
    ui/aboutdialog.ui \
    ui/enteroneblastquerydialog.ui \
    ui/blastsearchdialog.ui \
    ui/myprogressdialog.ui \
    ui/pathspecifydialog.ui \
    ui/querypathsdialog.ui \
    ui/blasthitfiltersdialog.ui \
    ui/changenodenamedialog.ui \
    ui/changenodedepthdialog.ui \
    ui/graphinfodialog.ui

RESOURCES += \
    images/images.qrc


unix:INCLUDEPATH += /usr/include/
unix:LIBS += -L/usr/lib

# The following settings are compatible with OGDF being built in 64 bit release mode using Visual Studio 2013
win32:LIBS += -lpsapi
win32:RC_FILE = images/myapp.rc

macx:ICON = images/application.icns
macx:QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.7

# Each target platform needs the native platform as well as Qt's minimal platform.
win32: QTPLUGIN.platforms += qwindows qminimal
unix:!macx: QTPLUGIN.platforms += qxcb qminimal
macx: QTPLUGIN.platforms += qcocoa qminimal
