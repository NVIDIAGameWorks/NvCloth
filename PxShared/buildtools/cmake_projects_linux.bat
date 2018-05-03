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

SET LINUX_ROOT=%PM_ClangCrosscompile_PATH%

echo "Cmake: %CMAKECMD%"
echo "LINUX_ROOT: %LINUX_ROOT%"

echo.
echo #############################################################################################
ECHO "Creating linux crosscompiler makefiles"

REM Common cmd line params
set CMAKE_CMD_LINE_PARAMS= ^
-DTARGET_BUILD_PLATFORM=linux ^
-DUSE_GAMEWORKS_OUTPUT_DIRS=ON ^
-DAPPEND_CONFIG_NAME=OFF ^
-DCMAKE_PREFIX_PATH="%PM_PATHS%" ^
-DPX_OUTPUT_ARCH=x86 ^
-DPX_OUTPUT_LIB_DIR="%PXSHARED_ROOT_DIR%" ^
-DPX_OUTPUT_BIN_DIR="%PXSHARED_ROOT_DIR%" ^
-DCMAKE_INSTALL_PREFIX="%PXSHARED_ROOT_DIR%install/linux/" ^
-DCMAKE_TOOLCHAIN_FILE="%PM_CMakeModules_PATH%\linux\LinuxCrossToolchain.x86_64-unknown-linux-gnu.cmake" ^
-DARCHITECTURE_TRIPLE=x86_64-unknown-linux-gnu ^
-DCMAKE_MAKE_PROGRAM:PATH="%PM_MinGW_PATH%\bin\mingw32-make.exe" ^
-DGENERATE_STATIC_LIBRARIES=ON 


REM Generate projects here
SET CMAKE_OUTPUT_DIR=compiler\linux-crosscompile-debug
IF EXIST %CMAKE_OUTPUT_DIR% rmdir /S /Q %CMAKE_OUTPUT_DIR%
mkdir %CMAKE_OUTPUT_DIR%
pushd %CMAKE_OUTPUT_DIR%

%CMAKECMD% %PXSHARED_ROOT_DIR%\src\compiler\cmake --no-warn-unused-cli %CMAKE_CMD_LINE_PARAMS% -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=debug 

popd
if %ERRORLEVEL% NEQ 0 exit /b %ERRORLEVEL%
)

SET CMAKE_OUTPUT_DIR=compiler\linux-crosscompile-profile
IF EXIST %CMAKE_OUTPUT_DIR% rmdir /S /Q %CMAKE_OUTPUT_DIR%
mkdir %CMAKE_OUTPUT_DIR%
pushd %CMAKE_OUTPUT_DIR%

%CMAKECMD% %PXSHARED_ROOT_DIR%\src\compiler\cmake -G "Unix Makefiles" %CMAKE_CMD_LINE_PARAMS% -DCMAKE_BUILD_TYPE=profile

popd
if %ERRORLEVEL% NEQ 0 exit /b %ERRORLEVEL%
)

SET CMAKE_OUTPUT_DIR=compiler\linux-crosscompile-checked
IF EXIST %CMAKE_OUTPUT_DIR% rmdir /S /Q %CMAKE_OUTPUT_DIR%
mkdir %CMAKE_OUTPUT_DIR%
pushd %CMAKE_OUTPUT_DIR%

%CMAKECMD% %PXSHARED_ROOT_DIR%\src\compiler\cmake -G "Unix Makefiles" %CMAKE_CMD_LINE_PARAMS% -DCMAKE_BUILD_TYPE=checked 

popd
if %ERRORLEVEL% NEQ 0 exit /b %ERRORLEVEL%
)

SET CMAKE_OUTPUT_DIR=compiler\linux-crosscompile-release
IF EXIST %CMAKE_OUTPUT_DIR% rmdir /S /Q %CMAKE_OUTPUT_DIR%
mkdir %CMAKE_OUTPUT_DIR%
pushd %CMAKE_OUTPUT_DIR%

%CMAKECMD% %PXSHARED_ROOT_DIR%\src\compiler\cmake -G "Unix Makefiles" %CMAKE_CMD_LINE_PARAMS% -DCMAKE_BUILD_TYPE=release 

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
