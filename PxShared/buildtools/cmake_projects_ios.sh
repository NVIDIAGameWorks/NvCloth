#!/bin/bash +x

DATE=$(date +"%m-%d-%Y")
TIME=$(date +"%T")
echo "#############################################################################################"
echo "Starting ${0##*/} $DATE $TIME "
echo "#############################################################################################"

echo "PM_CMakeModules_VERSION $PM_CMakeModules_VERSION"
echo "PM_CMakeModules_PATH $PM_CMakeModules_PATH"
echo "PM_PATHS $PM_PATHS"

[ -z "$PM_CMakeModules_VERSION" ] && echo "Don't run this batch file directly. Run generate_projects_(platform).bat instead." && exit 1;

[ -z "$PM_PACKAGES_ROOT" ] && echo "PM_PACKAGES_ROOT has to be defined, pointing to the root of the dependency tree." && exit 1;

# Now set up the CMake command from PM_PACKAGES_ROOT

CMAKECMD=$PM_cmake_PATH/bin/cmake

echo "Cmake: $CMAKECMD"

echo "#############################################################################################"
echo "Creating Ios XCode projects"


# Common cmd line params
CMAKE_CMD_LINE_PARAMS="\
-DTARGET_BUILD_PLATFORM=ios \
-DUSE_GAMEWORKS_OUTPUT_DIRS=ON \
-DAPPEND_CONFIG_NAME=OFF \
-DCMAKE_PREFIX_PATH=$PM_PATHS \
-DFORCE_64BIT_SUFFIX=ON \
-DPX_OUTPUT_ARCH=arm \
-DPX_OUTPUT_LIB_DIR=$PXSHARED_ROOT_DIR \
-DPX_OUTPUT_BIN_DIR=$PXSHARED_ROOT_DIR \
-DCMAKE_TOOLCHAIN_FILE=$PM_CMakeModules_PATH/ios/ios.toolchain.cmake \
-DCMAKE_INSTALL_PREFIX=$PXSHARED_ROOT_DIR/install/ios \
-DUSE_DEBUG_WINCRT=OFF"

# Generate projects here	

IOSPLATDIR=ios
mkdir -p compiler
CMAKE_OUTPUT_DIR=compiler/$IOSPLATDIR-xcode/

rm -r -f $CMAKE_OUTPUT_DIR
mkdir -p $CMAKE_OUTPUT_DIR
pushd $CMAKE_OUTPUT_DIR

$CMAKECMD $PXSHARED_ROOT_DIR/src/compiler/cmake -G "Xcode" -DCMAKE_INSTALL_PREFIX=$PXSHARED_ROOT_DIR/install/$IOSPLATDIR/ $CMAKE_CMD_LINE_PARAMS || exit 1

popd

OSXPLATDIR=ios
mkdir -p compiler
for config in "debug" "profile" "checked" "release"; do
CMAKE_OUTPUT_DIR=compiler/$IOSPLATDIR-$config/

rm -r -f $CMAKE_OUTPUT_DIR
mkdir -p $CMAKE_OUTPUT_DIR
pushd $CMAKE_OUTPUT_DIR

$CMAKECMD $PXSHARED_ROOT_DIR/src/compiler/cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=$config -DCMAKE_INSTALL_PREFIX=$PXSHARED_ROOT_DIR/install/$IOSPLATDIR/ $CMAKE_CMD_LINE_PARAMS || exit 1

popd

done

