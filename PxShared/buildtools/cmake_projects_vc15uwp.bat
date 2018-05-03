@echo off

echo #############################################################################################
echo Starting %~n0 %date% %time%
echo #############################################################################################

echo PM_CMakeModules_VERSION %PM_CMakeModules_VERSION%
echo PM_CMakeModules_PATH %PM_CMakeModules_PATH%
echo PM_PATHS %PM_PATHS%

if NOT DEFINED PM_CMakeModules_VERSION GOTO DONT_RUN_STEP_2

IF NOT DEFINED PM_PACKAGES_ROOT GOTO PM_PACKAGES_ROOT_UNDEFINED

REM Now set up the CMake command from PM_PACKAGES_ROOT

SET CMAKECMD=%PM_cmake_PATH%\bin\cmake.exe

echo "Cmake: %CMAKECMD%"

REM Common cmd line params
set CMAKE_CMD_LINE_PARAMS=-DTARGET_BUILD_PLATFORM=uwp ^
-DUSE_GAMEWORKS_OUTPUT_DIRS=ON ^
-DAPPEND_CONFIG_NAME=OFF ^
-DCMAKE_PREFIX_PATH="%PM_PATHS%" ^
-DPX_OUTPUT_LIB_DIR=%PXSHARED_ROOT_DIR%\ ^
-DPX_OUTPUT_BIN_DIR=%PXSHARED_ROOT_DIR%\ ^
-DUSE_DEBUG_WINCRT=OFF ^
-DCMAKE_SYSTEM_NAME=WindowsStore ^
-DCMAKE_SYSTEM_VERSION=10.0

REM Compiler dependend params	
set CMAKE_VC15WIN32_CMD_LINE_PARAMS= ^
-G "Visual Studio 15 2017" -AWin32 ^
-DPX_OUTPUT_ARCH=x86 ^
-DPX_GENERATE_GPU_PROJECTS=OFF


set CMAKE_VC15WIN64_CMD_LINE_PARAMS= ^
-G "Visual Studio 15 2017" -Ax64 ^
-DPX_OUTPUT_ARCH=x86 ^
-DPX_GENERATE_GPU_PROJECTS=OFF

set CMAKE_VC15ARM_CMD_LINE_PARAMS= ^
-G "Visual Studio 15 2017" -Aarm ^
-DPX_OUTPUT_ARCH=arm ^
-DPX_GENERATE_GPU_PROJECTS=OFF

echo.
echo #############################################################################################
ECHO "Creating VC15 VS2017"

SET VCPLATDIR=vc15uwp64
SET CMAKE_OUTPUT_DIR=compiler\%VCPLATDIR%\
IF EXIST %CMAKE_OUTPUT_DIR% rmdir /S /Q %CMAKE_OUTPUT_DIR%
mkdir %CMAKE_OUTPUT_DIR%
pushd %CMAKE_OUTPUT_DIR%

%CMAKECMD% %PXSHARED_ROOT_DIR%\src\compiler\cmake -DCMAKE_INSTALL_PREFIX=%PXSHARED_ROOT_DIR%\install\%VCPLATDIR%\ %CMAKE_CMD_LINE_PARAMS% %CMAKE_VC15WIN64_CMD_LINE_PARAMS%

popd
if %ERRORLEVEL% NEQ 0 exit /b %ERRORLEVEL%

SET VCPLATDIR=vc15uwp32
SET CMAKE_OUTPUT_DIR=compiler\%VCPLATDIR%\
IF EXIST %CMAKE_OUTPUT_DIR% rmdir /S /Q %CMAKE_OUTPUT_DIR%
mkdir %CMAKE_OUTPUT_DIR%
pushd %CMAKE_OUTPUT_DIR%

%CMAKECMD% %PXSHARED_ROOT_DIR%\src\compiler\cmake -DCMAKE_INSTALL_PREFIX=%PXSHARED_ROOT_DIR%\install\%VCPLATDIR%\ %CMAKE_CMD_LINE_PARAMS% %CMAKE_VC15WIN32_CMD_LINE_PARAMS%

popd
if %ERRORLEVEL% NEQ 0 exit /b %ERRORLEVEL%

GOTO :End

SET VCPLATDIR=vc15uwparm
SET CMAKE_OUTPUT_DIR=compiler\%VCPLATDIR%\
IF EXIST %CMAKE_OUTPUT_DIR% rmdir /S /Q %CMAKE_OUTPUT_DIR%
mkdir %CMAKE_OUTPUT_DIR%
pushd %CMAKE_OUTPUT_DIR%

%CMAKECMD% %PXSHARED_ROOT_DIR%\src\compiler\cmake -DCMAKE_INSTALL_PREFIX=%PXSHARED_ROOT_DIR%\install\%VCPLATDIR%\ %CMAKE_CMD_LINE_PARAMS% %CMAKE_VC15ARM_CMD_LINE_PARAMS%

popd
if %ERRORLEVEL% NEQ 0 exit /b %ERRORLEVEL%


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
