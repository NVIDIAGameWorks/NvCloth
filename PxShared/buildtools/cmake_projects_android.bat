@echo off

echo #############################################################################################
echo Starting %~n0 %date% %time%
echo #############################################################################################

echo PM_CMakeModules_VERSION %PM_CMakeModules_VERSION%
echo PM_CMakeModules_PATH %PM_CMakeModules_PATH%
echo PM_android-ndk_PATH %PM_android-ndk_PATH%
echo PM_android-ndk_VERSION %PM_android-ndk_VERSION%
echo PM_PATHS %PM_PATHS%

if NOT DEFINED PM_CMakeModules_VERSION GOTO DONT_RUN_STEP_2

IF NOT DEFINED PM_PACKAGES_ROOT GOTO PM_PACKAGES_ROOT_UNDEFINED

REM Now set up the CMake command from PM_PACKAGES_ROOT

SET CMAKECMD=%PM_cmake_PATH%\bin\cmake.exe

echo "Cmake: %CMAKECMD%"

REM Generate projects here

echo.
echo #############################################################################################
ECHO "Creating android mingw"

REM Set android compiler variables for cmake
set ANDROID_PLATFORM=android-19
set ANDROID_ABI="armeabi-v7a with NEON"

REM Common cmd line params
set CMAKE_CMD_LINE_PARAMS=-DTARGET_BUILD_PLATFORM=android ^
-DCMAKE_TOOLCHAIN_FILE=%PM_CMakeModules_PATH%/android/android.toolchain.cmake ^
-DANDROID_NATIVE_API_LEVEL=%ANDROID_PLATFORM% ^
-DANDROID_ABI=%ANDROID_ABI% ^
-DANDROID_STL="gnustl_static" ^
-DCM_ANDROID_FP="softfp" ^
-DANDROID_NDK=%PM_AndroidNDK_PATH% ^
-DCM_ANDROID_NDK_VERSION=%PM_AndroidNDK_VERSION% ^
-DCMAKE_MAKE_PROGRAM=%PM_android-ndk_PATH%/prebuilt/windows/bin/make.exe ^
-DUSE_GAMEWORKS_OUTPUT_DIRS=ON ^
-DAPPEND_CONFIG_NAME=OFF ^
-DCMAKE_PREFIX_PATH="%PM_PATHS%" ^
-DPX_OUTPUT_LIB_DIR=%PXSHARED_ROOT_DIR%/ ^
-DPX_OUTPUT_BIN_DIR=%PXSHARED_ROOT_DIR%/ ^
-DCMAKE_INSTALL_PREFIX=%PXSHARED_ROOT_DIR%/install/android19/


REM Compiler dependend params
set CMAKE_MINGW_CMD_LINE_PARAMS=-G "MinGW Makefiles"
set CMAKE_MSYS_CMD_LINE_PARAMS=-G "MSYS Makefiles"

SET CMAKE_OUTPUT_DIR=compiler\%ANDROID_PLATFORM%-debug
IF EXIST %CMAKE_OUTPUT_DIR% rmdir /S /Q %CMAKE_OUTPUT_DIR%
mkdir %CMAKE_OUTPUT_DIR%
pushd %CMAKE_OUTPUT_DIR%

%CMAKECMD% %PXSHARED_ROOT_DIR%\src\compiler\cmake %CMAKE_CMD_LINE_PARAMS% -DCMAKE_BUILD_TYPE=debug %CMAKE_MINGW_CMD_LINE_PARAMS%

popd
if %ERRORLEVEL% NEQ 0 exit /b %ERRORLEVEL%
)


SET CMAKE_OUTPUT_DIR=compiler\%ANDROID_PLATFORM%-profile
IF EXIST %CMAKE_OUTPUT_DIR% rmdir /S /Q %CMAKE_OUTPUT_DIR%
mkdir %CMAKE_OUTPUT_DIR%
pushd %CMAKE_OUTPUT_DIR%

%CMAKECMD% %PXSHARED_ROOT_DIR%\src\compiler\cmake %CMAKE_CMD_LINE_PARAMS% -DCMAKE_BUILD_TYPE=profile %CMAKE_MINGW_CMD_LINE_PARAMS%

popd
if %ERRORLEVEL% NEQ 0 exit /b %ERRORLEVEL%
)

SET CMAKE_OUTPUT_DIR=compiler\%ANDROID_PLATFORM%-checked
IF EXIST %CMAKE_OUTPUT_DIR% rmdir /S /Q %CMAKE_OUTPUT_DIR%
mkdir %CMAKE_OUTPUT_DIR%
pushd %CMAKE_OUTPUT_DIR%

%CMAKECMD% %PXSHARED_ROOT_DIR%\src\compiler\cmake %CMAKE_CMD_LINE_PARAMS% -DCMAKE_BUILD_TYPE=checked %CMAKE_MINGW_CMD_LINE_PARAMS%

popd
if %ERRORLEVEL% NEQ 0 exit /b %ERRORLEVEL%
)

SET CMAKE_OUTPUT_DIR=compiler\%ANDROID_PLATFORM%-release
IF EXIST %CMAKE_OUTPUT_DIR% rmdir /S /Q %CMAKE_OUTPUT_DIR%
mkdir %CMAKE_OUTPUT_DIR%
pushd %CMAKE_OUTPUT_DIR%

%CMAKECMD% %PXSHARED_ROOT_DIR%\src\compiler\cmake %CMAKE_CMD_LINE_PARAMS% -DCMAKE_BUILD_TYPE=release %CMAKE_MINGW_CMD_LINE_PARAMS%

popd
if %ERRORLEVEL% NEQ 0 exit /b %ERRORLEVEL%
)

GOTO :End

:PM_PACKAGES_ROOT_UNDEFINED
ECHO PM_PACKAGES_ROOT has to be defined, pointing to the root of the dependency tree.
PAUSE
GOTO END

:DONT_RUN_STEP_2
ECHO Don't run this batch file directly. Run generate_projects_(platform).bat instead
PAUSE
GOTO END

:End
