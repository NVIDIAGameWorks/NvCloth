#!/bin/bash +x

export PXSHARED_ROOT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

# Run packman to ensure dependencies are present and run cmake generation script afterwards
echo "Running packman in preparation for cmake ..."

$PXSHARED_ROOT_DIR/buildtools/packman/packman pull "$PXSHARED_ROOT_DIR/dependencies.xml" --platform linux --postscript "$PXSHARED_ROOT_DIR/buildtools/cmake_projects_linux.sh"
if [ "$?" -ne "0" ]; then
 echo "Error $?"
 exit 1
fi

echo "Success!"

