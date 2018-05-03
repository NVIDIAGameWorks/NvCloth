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
echo "Creating Mac XCode projects"


# Common cmd line params
CMAKE_CMD_LINE_PARAMS="\
-DTARGET_BUILD_PLATFORM=mac \
-DUSE_GAMEWORKS_OUTPUT_DIRS=ON \
-DAPPEND_CONFIG_NAME=OFF \
-DCMAKE_PREFIX_PATH=$PM_PATHS \
-DPX_OUTPUT_ARCH=x86 \
-DPX_OUTPUT_LIB_DIR=$PXSHARED_ROOT_DIR \
-DPX_OUTPUT_BIN_DIR=$PXSHARED_ROOT_DIR \
-DGENERATE_STATIC_LIBRARIES=ON \
-DUSE_DEBUG_WINCRT=OFF"

# Compiler dependend params	
CMAKE_MAC32_CMD_LINE_PARAMS="-DFORCE_32BIT_SUFFIX=ON"

CMAKE_MAC64_CMD_LINE_PARAMS="-DFORCE_64BIT_SUFFIX=ON"	

# Generate projects here	

MACPLATDIR=mac32
mkdir -p compiler
for config in "debug" "profile" "checked" "release"; do
CMAKE_OUTPUT_DIR=compiler/$MACPLATDIR-$config/

rm -r -f $CMAKE_OUTPUT_DIR
mkdir -p $CMAKE_OUTPUT_DIR
pushd $CMAKE_OUTPUT_DIR

$CMAKECMD $PXSHARED_ROOT_DIR/src/compiler/cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=$config -DCMAKE_INSTALL_PREFIX=$PXSHARED_ROOT_DIR/install/$MACPLATDIR/ $CMAKE_CMD_LINE_PARAMS $CMAKE_MAC32_CMD_LINE_PARAMS || exit 1

popd

done

MACPLATDIR=mac64
mkdir -p compiler
for config in "debug" "profile" "checked" "release"; do
CMAKE_OUTPUT_DIR=compiler/$MACPLATDIR-$config/

rm -r -f $CMAKE_OUTPUT_DIR
mkdir -p $CMAKE_OUTPUT_DIR
pushd $CMAKE_OUTPUT_DIR

$CMAKECMD $PXSHARED_ROOT_DIR/src/compiler/cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=$config -DCMAKE_INSTALL_PREFIX=$PXSHARED_ROOT_DIR/install/$MACPLATDIR/ $CMAKE_CMD_LINE_PARAMS $CMAKE_MAC64_CMD_LINE_PARAMS || exit 1

popd

done

MACPLATDIR=mac32
mkdir -p compiler
CMAKE_OUTPUT_DIR=compiler/$MACPLATDIR-xcode/

rm -r -f $CMAKE_OUTPUT_DIR
mkdir -p $CMAKE_OUTPUT_DIR
pushd $CMAKE_OUTPUT_DIR

$CMAKECMD $PXSHARED_ROOT_DIR/src/compiler/cmake -G "Xcode" -DCMAKE_BUILD_TYPE=$config -DCMAKE_INSTALL_PREFIX=$PXSHARED_ROOT_DIR/install/$MACPLATDIR/ $CMAKE_CMD_LINE_PARAMS $CMAKE_MAC32_CMD_LINE_PARAMS || exit 1

popd

MACPLATDIR=mac64
mkdir -p compiler
CMAKE_OUTPUT_DIR=compiler/$MACPLATDIR-xcode/

rm -r -f $CMAKE_OUTPUT_DIR
mkdir -p $CMAKE_OUTPUT_DIR
pushd $CMAKE_OUTPUT_DIR

$CMAKECMD $PXSHARED_ROOT_DIR/src/compiler/cmake -G "Xcode" -DCMAKE_BUILD_TYPE=$config -DCMAKE_INSTALL_PREFIX=$PXSHARED_ROOT_DIR/install/$MACPLATDIR/ $CMAKE_CMD_LINE_PARAMS $CMAKE_MAC64_CMD_LINE_PARAMS || exit 1

popd
