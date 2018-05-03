#!/bin/sh +x

export PXS_ROOT=$(dirname "$0")
export PXSHARED_ROOT_DIR=$PWD

# Run packman to ensure dependencies are present and run cmake generation script afterwards
echo "Running packman in preparation for cmake ..."

$PXS_ROOT/buildtools/packman/packman pull "$PXS_ROOT/dependencies.xml" --platform mac --postscript "$PXS_ROOT/buildtools/cmake_projects_mac.sh"
if [ "$?" -ne "0" ]; then
 echo "Error $?"
 exit 1
fi

echo "Success!"