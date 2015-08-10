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

QT       += core gui svg

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Bandage
TEMPLATE = app

CONFIG += c++11

target.path += /usr/local/bin
INSTALLS += target

INCLUDEPATH += ui

SOURCES += \
    program/main.cpp\
    program/settings.cpp \
    program/globals.cpp \
    program/graphlayoutworker.cpp \
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
    command_line/contiguous.cpp \
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
    ui/distancedialog.cpp \
    graph/graphlocation.cpp \
    ui/tablewidgetitemint.cpp \
    ui/tablewidgetitemdouble.cpp \
    ui/tablewidgetitemshown.cpp \
    program/memory.cpp \
    command_line/distance.cpp \
    graph/querydistance.cpp

HEADERS  += \
    program/settings.h \
    program/globals.h \
    program/graphlayoutworker.h \
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
    command_line/contiguous.h \
    command_line/load.h \
    command_line/image.h \
    command_line/commoncommandlinefunctions.h \
    ui/mytablewidget.h \
    blast/buildblastdatabaseworker.h \
    ui/colourbutton.h \
    blast/runblastsearchworker.h \
    graph/path.h \
    ui/pathspecifydialog.h \
    ui/distancedialog.h \
    graph/graphlocation.h \
    ui/tablewidgetitemint.h \
    ui/tablewidgetitemdouble.h \
    ui/tablewidgetitemshown.h \
    program/memory.h \
    command_line/distance.h \
    graph/querydistance.h

FORMS    += \
    ui/mainwindow.ui \
    ui/settingsdialog.ui \
    ui/aboutdialog.ui \
    ui/enteroneblastquerydialog.ui \
    ui/blastsearchdialog.ui \
    ui/myprogressdialog.ui \
    ui/pathspecifydialog.ui \
    ui/distancedialog.ui


# These includes are configured assuming that the OGDF
# directory is next to the Bandage project directory.
unix:INCLUDEPATH += /usr/include/
unix:LIBS += -L/usr/lib
unix:QMAKE_CXXFLAGS += -isystem ../OGDF #prevents compilers warnings for OGDF
unix:LIBS += -L$$PWD/../OGDF/_release
unix:LIBS += -lOGDF
unix:INCLUDEPATH += $$PWD/../OGDF/


# The following settings are compatible with OGDF being
# built in 64 bit release mode using Visual Studio 2013
win32:LIBS += -lpsapi -logdf
win32:LIBS += -L$$PWD/../OGDF/x64/Release/
win32:INCLUDEPATH += $$PWD/../OGDF/
win32:RC_FILE = images/myapp.rc


macx:ICON = images/application.icns
macx:QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.7

RESOURCES += \
    images/images.qrc
