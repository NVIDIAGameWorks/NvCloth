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

export CMAKECMD=$PM_cmake_PATH/bin/cmake

echo "Cmake: $CMAKECMD"

echo "PXshared Root: $PXSHARED_ROOT_DIR"

# Generate projects here

echo "#############################################################################################"
echo "Creating Unix Makefiles"

# Common cmd line params
CMAKE_CMD_LINE_PARAMS="\
-DTARGET_BUILD_PLATFORM=linux \
-DUSE_GAMEWORKS_OUTPUT_DIRS=ON \
-DAPPEND_CONFIG_NAME=OFF \
-DCMAKE_PREFIX_PATH=$PM_PATHS \
-DPX_OUTPUT_ARCH=x86 \
-DPX_OUTPUT_LIB_DIR=$PXSHARED_ROOT_DIR \
-DPX_OUTPUT_BIN_DIR=$PXSHARED_ROOT_DIR \
-DCUDA_TOOLKIT_ROOT_DIR=$PM_CUDA_PATH \
-DCMAKE_INSTALL_PREFIX=$PXSHARED_ROOT_DIR/install/linux/ \
-DGENERATE_STATIC_LIBRARIES=ON \
-DPX_GENERATE_GPU_PROJECTS=ON"

# Compiler dependend params
CMAKE_CLANG_CMD_LINE_PARAMS="-DCMAKE_C_COMPILER=clang-3.8 -DCMAKE_CXX_COMPILER=clang++-3.8"

# Generate projects here
mkdir -p compiler
for config in "debug" "profile" "checked" "release"; do

rm -r -f compiler/linux-$config-clang/
mkdir -p compiler/linux-$config-clang/
pushd compiler/linux-$config-clang/
$CMAKECMD $PXSHARED_ROOT_DIR/src/compiler/cmake -G "Unix Makefiles" $CMAKE_CLANG_CMD_LINE_PARAMS -DCMAKE_BUILD_TYPE=$config $CMAKE_CMD_LINE_PARAMS || exit 1
popd

done
