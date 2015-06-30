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

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Bandage
TEMPLATE = app

# Delete the following line if building on OS X 10.7
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
    command_line/load.cpp

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
    command_line/load.h

FORMS    += \
    ui/mainwindow.ui \
    ui/settingsdialog.ui \
    ui/aboutdialog.ui \
    ui/enteroneblastquerydialog.ui \
    ui/blastsearchdialog.ui \
    ui/myprogressdialog.ui


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

RESOURCES += \
    images/images.qrc
