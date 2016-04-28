#!/bin/bash

# This is the script I use on my OS X 10.8 virtual machine to build the public release of Bandage.
# It produces an app bundle ready for deployment. It requires that sample_LastGraph is in the
# current directory.

# Find the Qt path automatically. This assumes it is installed in ~/Qt/ and is a 5.* version.
QT_PATH=$(find $HOME/Qt -name "5*" -maxdepth 1 | tail -n 1)

# Clone and build Bandage from the master branch.
git clone https://github.com/rrwick/Bandage
cd Bandage/
$QT_PATH/clang_64/bin/qmake
make

# Add the necessary libraries so Bandage can be deployed.
$QT_PATH/clang_64/bin/macdeployqt Bandage.app
cp $QT_PATH/clang_64/plugins/platforms/libqminimal.dylib Bandage.app/Contents/PlugIns/platforms/libqminimal.dylib

# Get the Bandage version number from the main.cpp file and replace its dots with underscores.
cd ..
VERSION_LINE=$(grep "setApplicationVersion" Bandage/program/main.cpp)
IFS='"' read -ra ADDR <<< "$VERSION_LINE"
VERSION="${ADDR[1]}"
VERSION=${VERSION//\./_}

# Zip Bandage, add the sample graph and clean up.
ZIP="Bandage_Mac_v$VERSION.zip"
ditto -c -k --sequesterRsrc --keepParent Bandage/Bandage.app $ZIP
zip $ZIP sample_LastGraph 
rm -rf Bandage/
