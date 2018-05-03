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


REM Generate projects here

echo.
echo #############################################################################################
ECHO "Creating VC14 VS2015 XboxOne"

SET CMAKE_OUTPUT_DIR=compiler\vc14xboxone\
IF EXIST %CMAKE_OUTPUT_DIR% rmdir /S /Q %CMAKE_OUTPUT_DIR%
mkdir %CMAKE_OUTPUT_DIR%
pushd %CMAKE_OUTPUT_DIR%


%CMAKECMD% %PXSHARED_ROOT_DIR%\src\compiler\cmake ^
-G "Visual Studio 14 2015"  ^
-DTARGET_BUILD_PLATFORM=xboxone ^
-DCMAKE_TOOLCHAIN_FILE="%PM_CMakeModules_PATH%\xboxone\XboxOneToolchain.txt" ^
-DCMAKE_GENERATOR_PLATFORM=Durango ^
-DPX_OUTPUT_LIB_DIR=%PXSHARED_ROOT_DIR% ^
-DPX_OUTPUT_BIN_DIR=%PXSHARED_ROOT_DIR% ^
-DAPPEND_CONFIG_NAME=OFF ^
-DCMAKE_PREFIX_PATH="%PM_PATHS%" ^
-DUSE_GAMEWORKS_OUTPUT_DIRS=ON ^
-DCMAKE_INSTALL_PREFIX=%PXSHARED_ROOT_DIR%\install\vc14xboxone\

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
