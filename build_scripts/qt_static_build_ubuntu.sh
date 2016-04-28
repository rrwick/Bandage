#!/bin/bash

# This script builds a static version of Qt. It was originally writen by Lennart Rolland and taken
# from here: https://stackoverflow.com/questions/24275551/
# I modified it to move around some QtSvg stuff so Bandage (which uses QtSvg) can build.

# Change this to the desired version.
VER="5.6.0"

# Dependencies:
sudo apt-get install build-essential perl python git "^libxcb.*" libx11-xcb-dev libglu1-mesa-dev libxrender-dev libasound2-dev libgstreamer0.10-dev libgstreamer-plugins-base0.10-dev libfontconfig1-dev

VER2="${VER%.*}"
WSRC="http://download.qt.io/official_releases/qt/$VER2/$VER/single/qt-everywhere-opensource-src-$VER.tar.xz"

# Current dir ( allows this script to be called from another dir)
B=$(pwd)
# Base folder for the whole operation
Q="$B/qt"
# The uncompressed source
SRC="$Q/src/$VER"
# The actual shadow dir
O="$Q/build/$VER"
# The tar.xz archive
XZ="$Q/xz/qt-$VER.tar.xz"
# Paralelle make, number of cores
J=$(grep -c ^processor /proc/cpuinfo)
# Build log file
LOG="$O/log.txt"

# My configuration options for qt change to your hearts content, but make sure to clean out your current build before using it.
OPTS=""
OPTS+=" -release"
OPTS+=" -opensource"
OPTS+=" -static" 
OPTS+=" -confirm-license"
#OPTS+=" -fully-process" #Breaks my build
OPTS+=" -c++11"
OPTS+=" -platform linux-g++"
OPTS+=" -largefile" 
#OPTS+=" -continue"
OPTS+=" -silent"
#OPTS+=" -optimized-qmake" 
#OPTS+=" -reduce-relocations"
OPTS+=" -qpa xcb"
#OPTS+=" -declarative"
OPTS+=" -opengl desktop"
#OPTS+=" -svg"
OPTS+=" -qt-zlib" # ........... Use the zlib bundled with Qt.
OPTS+=" -qt-libpng" # ......... Use the libpng bundled with Qt.
OPTS+=" -qt-libjpeg" # ........ Use the libjpeg bundled with Qt.
OPTS+=" -qt-freetype" # ........ Use the freetype bundled with Qt.
OPTS+=" -qt-harfbuzz" # ........ Use the freetype bundled with Qt.
OPTS+=" -qt-pcre" # ........... Use the PCRE library bundled with Qt.
OPTS+=" -qt-xcb" # ............ Use xcb- libraries bundled with Qt.
OPTS+=" -qt-xkbcommon" # ...... 
OPTS+=" -no-gtkstyle"
OPTS+=" -no-sql-db2" 
OPTS+=" -no-sql-ibase" 
OPTS+=" -no-sql-mysql" 
OPTS+=" -no-sql-oci" 
OPTS+=" -no-sql-odbc" 
OPTS+=" -no-sql-psql" 
OPTS+=" -no-sql-sqlite" 
OPTS+=" -no-sql-sqlite2" 
OPTS+=" -no-sql-tds"
OPTS+=" -no-gif"
OPTS+=" -no-nis"
OPTS+=" -no-cups" 
OPTS+=" -no-iconv"
OPTS+=" -no-dbus"
OPTS+=" -no-eglfs"
OPTS+=" -no-directfb" 
OPTS+=" -no-linuxfb"
OPTS+=" -no-glib"
OPTS+=" -no-kms"
OPTS+=" -nomake examples" 
#OPTS+=" -nomake demos" NOT AVAILABLE ANYMORE
OPTS+=" -nomake tests"
OPTS+=" -skip qtwebkit"
OPTS+=" -skip qtwebkit-examples"
#OPTS+=" -no-openssl"

# The modules that are relevant for me. Please observe that THE ORDER MATTERS! I would add one module at the time and see how it complains when you try to build it.
MODS="qtx11extras qtimageformats qtscript qtquick1 qtdeclarative qtquickcontrols qtsvg qtmultimedia"

# Just echo out the current state before starting the configuration and make
echo "B: $B"
echo "MODS: $MODS"
echo "OPTS: $OPTS"
echo "Q: $Q"
echo "O: $O"
echo "XZ: $XZ"
echo "SRC: $SRC"
echo "J: $J"
echo "LOG: $LOG"

# Create dirs
mkdir -p "$Q"
mkdir -p "$Q/xz"
mkdir -p "$SRC"
mkdir -p "$O"
# Start log
date > $LOG
# Download source archive
[ ! -f $XZ ] && wget "$WSRC" -c -O "$XZ"
# Unpack source archive
[ ! -x $SRC/configure ] && tar pxf "$XZ" --strip=1 -C "$SRC" "qt-everywhere-opensource-src-$VER" 

# Set some variables
export QMAKESPEC=linux-g++
export QTDIR=$O/qtbase
export PATH=$O/qtbase/bin:$PATH

# Configure qt build
cd "$O"
MAKEFLAGS=-j$J "$SRC/configure" $OPTS

# Build qtbase with new config (results in the basic qt libs plus a new qmake that you can use for building the rest of the modules and your own projects).
# TIP: Don't put make all here
make -j$J >> $LOG

#build your modules with the new qmake, keeping the resulting static libs in each module's shadow build folder where they will be located by qmke during compilation of your projects
for M in $MODS
do
    echo "----------------------------------------- MODULE: $M"
    echo "----------------------------------------- MODULE: $M" >> $LOG
    # Make module dir
    D=$O/$M
    mkdir -p $D
    cd $D
    # Use new qmake to create module makefile
    $O/qtbase/bin/qmake $SRC/$M/
    # Build module
    make -j$J >> $LOG
done

# Copy qtsvg stuff into qtbase folder
cp -r -n "$O/qtsvg/*" "$O/qtbase/"

echo "DONE"
