#!/bin/bash

# Run this script as root (using su)

# This script builds a static version of Qt. It was originally writen by Lennart Rolland and taken
# from here: https://stackoverflow.com/questions/24275551/
# I modified it to work with CentOS and Qt 5.6 and move around some QtSvg stuff for Bandage.

# Change this to the desired version.
VER="5.6.0"

# Dependencies:
yum install libxcb libxcb-devel libXrender libXrender-devel xcb-util-wm xcb-util-wm-devel xcb-util xcb-util-devel xcb-util-image xcb-util-image-devel xcb-util-keysyms xcb-util-keysyms-devel fontconfig-devel freetype-devel libX11-devel libXext-devel libXfixes-devel libXi-devel mesa-libGLw gtkglext-devel

VER2="${VER%.*}"
WSRC="http://download.qt.io/official_releases/qt/$VER2/$VER/single/qt-everywhere-opensource-src-$VER.tar.xz"
B=$(pwd) # Current dir (allows this script to be called from another dir)
Q="$B/qt" # Base folder for the whole operation
SRC="$Q/src/$VER" # The uncompressed source
O="$Q/build/$VER" # The actual shadow dir
XZ="$Q/xz/qt-$VER.tar.xz" # The tar.xz archive
J=$(grep -c ^processor /proc/cpuinfo) # Parallel make, number of cores
LOG="$O/log.txt" # Build log file

# My configuration options for qt change to your hearts content, but make sure to clean out your current build before using it.
OPTS=""
OPTS+=" -release"
OPTS+=" -optimized-tools" 
OPTS+=" -opensource"
OPTS+=" -confirm-license"
OPTS+=" -c++std c++11"
OPTS+=" -static"
OPTS+=" -largefile"
OPTS+=" -no-sql-db2"
OPTS+=" -no-sql-ibase"
OPTS+=" -no-sql-mysql"
OPTS+=" -no-sql-oci"
OPTS+=" -no-sql-odbc"
OPTS+=" -no-sql-psql"
OPTS+=" -no-sql-sqlite"
OPTS+=" -no-sql-sqlite2"
OPTS+=" -no-sql-tds"
OPTS+=" -platform linux-g++"
OPTS+=" -qt-zlib"
OPTS+=" -no-gif"
OPTS+=" -qt-libpng"
OPTS+=" -qt-libjpeg"
OPTS+=" -qt-freetype"
OPTS+=" -qt-harfbuzz"
OPTS+=" -qt-pcre"
OPTS+=" -qt-xcb"
OPTS+=" -qt-xkbcommon-x11" 
OPTS+=" -no-glib"
OPTS+=" -no-gtkstyle"
OPTS+=" -nomake examples"
OPTS+=" -nomake tests"
OPTS+=" -no-compile-examples"
OPTS+=" -silent"
OPTS+=" -no-nis"
OPTS+=" -no-cups"
OPTS+=" -no-iconv"
OPTS+=" -no-icu"
OPTS+=" -no-dbus"
OPTS+=" -no-eglfs"
OPTS+=" -no-kms"
OPTS+=" -no-directfb"
OPTS+=" -no-linuxfb"
OPTS+=" -qpa xcb"
OPTS+=" -opengl desktop"

# The modules that are relevant for me. Please observe that THE ORDER MATTERS! I would add one module at the time and see how it complains when you try to build it.
MODS="qtx11extras qtimageformats qtsvg qtmultimedia"

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

# Copy qtsvg stuff into qtbase folder. This is specifically for building Bandage.
cp -r -n $O/qtsvg/* $O/qtbase/

echo "DONE"
