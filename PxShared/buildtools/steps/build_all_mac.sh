#!/bin/bash +x

# exit script on first error
set -e

# get number of CPU cores
CPUS=`sysctl -n hw.ncpu`

# Stackoverflow suggests jobs count of (CPU cores + 1) as a respctively good number!
JOBS=`expr $CPUS + 1`

build_config()
{
    CONFIG=$1
    echo "*** Building: $CONFIG ***"
    pushd "$(dirname "$0")/../../compiler/$CONFIG"
    make -j$JOBS install
    popd
}

# run make for all configs
build_config "osx32-checked"
build_config "osx32-debug"
build_config "osx32-profile"
build_config "osx32-release"
build_config "osx64-checked"
build_config "osx64-debug"
build_config "osx64-profile"
build_config "osx64-release"
