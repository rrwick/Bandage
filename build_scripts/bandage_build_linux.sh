#!/bin/bash

# This is the script I use on my Ubuntu and CentOS virtual machines to build the public release of
# Bandage. It produces an app bundle ready for deployment. It requires that the 'sample_LastGraph'
# and 'dependencies' files are in the current directory.

# This script will by default build Bandage twice: once as a statically-linked executable and once
# as a dynamically-linked executable. If the user passes -s or -d, then it will build only the
# static or dynamic variety, respectively. The two different varieties use two separate versions of
# Qt. The dynamic version uses an older Qt version, as this will be compatible with more Linux
# systems. The static version uses a current version of Qt.

DYNAMIC_QMAKE="$HOME/Qt/5.2.1/gcc_64/bin/qmake"
STATIC_QMAKE="$HOME/Desktop/qt/build/5.6.0/qtbase/bin/qmake"

# Parse the arguments. If there are none, build both static and dynamic.
DYNAMIC_BUILD=false
STATIC_BUILD=false
while getopts "ds" opt; do
  case $opt in
    d)
      DYNAMIC_BUILD=true
      ;;
    s)
      STATIC_BUILD=true
      ;;
  esac
done
if ! $DYNAMIC_BUILD && ! $STATIC_BUILD; then
  STATIC_BUILD=true
  DYNAMIC_BUILD=true
fi

# If a 'Bandage' directory already exists, then this script will assume it is the Bandage source
# code and use it. If not, it will clone the master branch from GitHub.
if [ ! -d "Bandage" ]; then
  git clone https://github.com/rrwick/Bandage
fi

# Get the Bandage version number from the main.cpp file and replace its dots with underscores.
VERSION_LINE=$(grep "setApplicationVersion" Bandage/program/main.cpp)
IFS='"' read -ra ADDR <<< "$VERSION_LINE"
VERSION="${ADDR[1]}"
VERSION=${VERSION//\./_}

# Read the Linux release info (just used for naming the final zip files)
. /etc/*-release
BASE_ZIP_NAME="Bandage_$DISTRIB_ID"

if $STATIC_BUILD; then
  # Build the statically-linked version of Bandage.
  cp -r Bandage/ Bandage-static/
  cd Bandage-static/
  $STATIC_QMAKE Bandage.pro
  make
  chmod +x Bandage
  cd ..

  # Zip static Bandage
  STATIC_ZIP=$BASE_ZIP_NAME
  STATIC_ZIP+="_static_v$VERSION.zip"
  zip -j $STATIC_ZIP Bandage-static/Bandage
  zip $STATIC_ZIP sample_LastGraph
  rm -rf Bandage-static/
fi

if $DYNAMIC_BUILD; then
  # Build the dynamically-linked version of Bandage.
  cp -r Bandage/ Bandage-dynamic/
  cd Bandage-dynamic/
  $DYNAMIC_QMAKE Bandage.pro
  make
  chmod +x Bandage
  cd ..

  # Zip dynamic Bandage
  DYNAMIC_ZIP=$BASE_ZIP_NAME
  DYNAMIC_ZIP+="_dynamic_v$VERSION.zip"
  zip -j $DYNAMIC_ZIP Bandage-dynamic/Bandage
  zip $DYNAMIC_ZIP sample_LastGraph
  zip $DYNAMIC_ZIP dependencies
  rm -rf Bandage-dynamic/
fi

rm -rf Bandage/
