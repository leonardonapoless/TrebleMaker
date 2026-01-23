#!/bin/bash

PROJECT_NAME="TrebleMaker"
IDENTIFIER="com.leocodes.treblemaker" 
VERSION="1.0.0"

VST3_SOURCE="Builds/CMake/TrebleMaker_artefacts/VST3/${PROJECT_NAME}.vst3"
AU_SOURCE="Builds/CMake/TrebleMaker_artefacts/AU/${PROJECT_NAME}.component"
CLAP_SOURCE="Builds/CMake/TrebleMaker_artefacts/CLAP/${PROJECT_NAME}.clap"

mkdir -p staging/Library/Audio/Plug-Ins/VST3
mkdir -p staging/Library/Audio/Plug-Ins/Components
mkdir -p staging/Library/Audio/Plug-Ins/CLAP

echo "Copying plugins to staging..."
cp -R "$VST3_SOURCE" "staging/Library/Audio/Plug-Ins/VST3/"
cp -R "$AU_SOURCE" "staging/Library/Audio/Plug-Ins/Components/"
cp -R "$CLAP_SOURCE" "staging/Library/Audio/Plug-Ins/CLAP/"

echo "Building component package..."
pkgbuild --root staging \
         --identifier "$IDENTIFIER" \
         --version "$VERSION" \
         --install-location "/" \
         "${PROJECT_NAME}_Component.pkg"

echo "Synthesizing distribution..."
productbuild --synthesize \
             --package "${PROJECT_NAME}_Component.pkg" \
             distribution.xml

echo "Building final installer..."
productbuild --distribution distribution.xml \
             --package-path . \
             "${PROJECT_NAME}_Installer.pkg"

rm -rf staging
rm "${PROJECT_NAME}_Component.pkg"
rm distribution.xml

echo "Done! ${PROJECT_NAME}_Installer.pkg is ready."

