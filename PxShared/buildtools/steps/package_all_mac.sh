#!/bin/bash +x

# exit script on first error
set -e

#@set ROOT_PATH=%~dp0..\..

#::Remove old builds to keep things clean (important on build agents so we don't fill them up)
#@del /q /f "%ROOT_PATH%\pxshared*.zip"

# run packager
if [ -z $1 ] ; then
    VERSION=0
else
    VERSION=$1
fi

ROOT_PATH=$(dirname ${BASH_SOURCE})/../..
PACKMAN_CMD=$ROOT_PATH/buildtools/packman/packman
echo $PACKMAN_CMD

source "$PACKMAN_CMD" install packman-packager 1.0.3
echo "Launching create_packman_packages"
python "$PM_packman_packager_PATH/create_packman_packages.py" "$ROOT_PATH/install" --output-dir="$ROOT_PATH" osx pxshared $VERSION "$PM_packman_packager_PATH/templates/CMakeConfigWrapperNIX.cmake" "$PM_packman_packager_PATH/templates/CMakeConfigVersionTemplate.cmake"
