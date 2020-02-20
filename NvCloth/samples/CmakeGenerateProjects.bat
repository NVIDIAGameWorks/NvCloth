rem @echo off

CD /D %~dp0
@echo on


REM Install cmake using packman
set PACKMAN=call ../scripts/packman/packman.cmd
%PACKMAN% pull -p windows "../scripts/packman/packages/cmake.packman.xml"
IF %ERRORLEVEL% NEQ 0 (
    set EXIT_CODE=%ERRORLEVEL%
    goto End
)
set CMAKE=%PM_cmake_PATH%/bin/cmake.exe

REM Make sure the various variables that we need are set

call "../scripts/locate_cuda.bat" CUDA_PATH_
echo CUDA_PATH_ = %CUDA_PATH_%


set GW_DEPS_ROOT=%~dp0..\..\


IF NOT DEFINED GW_DEPS_ROOT GOTO GW_DEPS_ROOT_UNDEFINED

set PX_OUTPUT_ROOT=%~dp0

set OUTPUT_ROOT=%~dp0
set SAMPLES_ROOT_DIR=%~dp0

REM handle optional command line arguments
set USE_CUDA=1
set USE_DX11=1
IF %1. NEQ . (set USE_CUDA=%1)
IF %2. NEQ . (set USE_DX11=%2)


REM Generate projects here

rmdir /s /q compiler\vc14win64-cmake\
mkdir compiler\vc14win64-cmake\
pushd compiler\vc14win64-cmake\
%CMAKE% ..\.. -G "Visual Studio 14 2015" -Ax64 -DNV_CLOTH_ENABLE_DX11=%USE_DX11% -DNV_CLOTH_ENABLE_CUDA=%USE_CUDA% -DCUDA_TOOLKIT_ROOT_DIR="%CUDA_PATH_%" -DTARGET_BUILD_PLATFORM=windows -DSTATIC_WINCRT=0 -DBL_DLL_OUTPUT_DIR=%OUTPUT_ROOT%\bin\vc14win64-cmake -DBL_LIB_OUTPUT_DIR=%OUTPUT_ROOT%\lib\vc14win64-cmake -DBL_EXE_OUTPUT_DIR=%OUTPUT_ROOT%\bin\vc14win64-cmake
popd


GOTO :End

:End
pause