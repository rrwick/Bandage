#!/bin/bash

# This is the script I use on my Ubuntu 14.04 virtual machine to build the public release of
# Bandage. It produces an app bundle ready for deployment. It requires that the files
# sample_LastGraph and dependencies are in the current directory.

# This script needs two copies of Qt: a dynamically-linked and a statically-linked.
DYNAMIC_QMAKE="$HOME/Qt/5.6/gcc_64/bin/qmake"
STATIC_QMAKE="$HOME/Desktop/qt/build/5.6.0/qtbase/bin/qmake"

# If a 'Bandage' directory already exists, then this script will use assume it is the Bandage
# source code and use it. If not, it will clone the master branch from GitHub.
if [ ! -d "Bandage" ]; then
  git clone https://github.com/rrwick/Bandage
fi

# Get the Bandage version number from the main.cpp file and replace its dots with underscores.
VERSION_LINE=$(grep "setApplicationVersion" Bandage/program/main.cpp)
IFS='"' read -ra ADDR <<< "$VERSION_LINE"
VERSION="${ADDR[1]}"
VERSION=${VERSION//\./_}

# Dynamic and static Bandage will be built in separate folders.
mv Bandage/ Bandage-static/
cp -r Bandage-static/ Bandage-dynamic/

# Build the dynamically-linked version of Bandage.
cd Bandage-dynamic/
$DYNAMIC_QMAKE Bandage.pro
make
chmod +x Bandage
cd ..

# Build the statically-linked version of Bandage.
cd Bandage-static/
$STATIC_QMAKE Bandage.pro
make
chmod +x Bandage
cd ..

# Zip static Bandage
STATIC_ZIP="Bandage_CentOS_static_v$VERSION.zip"
zip -j $STATIC_ZIP Bandage-static/Bandage
zip $STATIC_ZIP sample_LastGraph
rm -rf Bandage-static/

# Zip dynamic Bandage
DYNAMIC_ZIP="Bandage_CentOS_dynamic_v$VERSION.zip"
zip -j $DYNAMIC_ZIP Bandage-dynamic/Bandage
zip $DYNAMIC_ZIP sample_LastGraph
zip $DYNAMIC_ZIP dependencies
rm -rf Bandage-dynamic/
