#!/bin/bash

PROJECT_NAME="TrebleMaker"
IDENTIFIER="com.leocodes.treblemaker" 
VERSION="1.0.0"

VST3_SOURCE="Builds/CMake/TrebleMaker_artefacts/VST3/${PROJECT_NAME}.vst3"
AU_SOURCE="Builds/CMake/TrebleMaker_artefacts/AU/${PROJECT_NAME}.component"

# this mimics the folder structure on the user's computer
mkdir -p staging/Library/Audio/Plug-Ins/VST3
mkdir -p staging/Library/Audio/Plug-Ins/Components

echo "Copying plugins to staging area..."
cp -R "$VST3_SOURCE" "staging/Library/Audio/Plug-Ins/VST3/"
cp -R "$AU_SOURCE" "staging/Library/Audio/Plug-Ins/Components/"

# build the .pkg
# the --install-location / tells the installer to put the files at the root of the drive (so they land in /Library/...)
echo "Building package..."
pkgbuild --root staging \
         --identifier "$IDENTIFIER" \
         --version "$VERSION" \
         --install-location "/" \
         "${PROJECT_NAME}_Installer.pkg"

# cleanup
rm -rf staging

echo "Done! ${PROJECT_NAME}_Installer.pkg is ready."
