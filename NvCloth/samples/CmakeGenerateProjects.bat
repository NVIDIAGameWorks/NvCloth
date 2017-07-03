rem @echo off

CD /D %~dp0
echo "Note: You need to run this with admin rights for the first time to set GW_DEPS_ROOT globally."
call "../scripts/locate_gw_root.bat" GW_DEPS_ROOT_F
@echo on
setx GW_DEPS_ROOT "%GW_DEPS_ROOT_F%
echo GW_DEPS_ROOT = %GW_DEPS_ROOT%
call "../scripts/locate_cmake.bat" CMAKE_PATH_F
echo CMAKE_PATH_F = %CMAKE_PATH_F%


SET PATH=%PATH%;"%CMAKE_PATH_F%"

REM Make sure the various variables that we need are set

call "../scripts/locate_cuda.bat" CUDA_PATH_
echo CUDA_PATH_ = %CUDA_PATH_%

IF EXIST %~dp0..\Externals\CMakeModules (
	set GW_DEPS_ROOT=%~dp0..\
)

IF NOT DEFINED GW_DEPS_ROOT GOTO GW_DEPS_ROOT_UNDEFINED

set PX_OUTPUT_ROOT=%~dp0

set OUTPUT_ROOT=%~dp0
set SAMPLES_ROOT_DIR=%~dp0

REM Generate projects here

rmdir /s /q compiler\vc14win32-cmake\
mkdir compiler\vc14win32-cmake\
pushd compiler\vc14win32-cmake\
cmake ..\.. -G "Visual Studio 14 2015" -AWin32 -DCUDA_TOOLKIT_ROOT_DIR="%CUDA_PATH_%" -DTARGET_BUILD_PLATFORM=windows -DSTATIC_WINCRT=0 -DBL_DLL_OUTPUT_DIR=%OUTPUT_ROOT%\bin\vc14win32-cmake -DBL_LIB_OUTPUT_DIR=%OUTPUT_ROOT%\lib\vc14win32-cmake -DBL_EXE_OUTPUT_DIR=%OUTPUT_ROOT%\bin\vc14win32-cmake
popd

rmdir /s /q compiler\vc14win64-cmake\
mkdir compiler\vc14win64-cmake\
pushd compiler\vc14win64-cmake\
cmake ..\.. -G "Visual Studio 14 2015" -Ax64 -DCUDA_TOOLKIT_ROOT_DIR="%CUDA_PATH_%" -DTARGET_BUILD_PLATFORM=windows -DSTATIC_WINCRT=0 -DBL_DLL_OUTPUT_DIR=%OUTPUT_ROOT%\bin\vc14win64-cmake -DBL_LIB_OUTPUT_DIR=%OUTPUT_ROOT%\lib\vc14win64-cmake -DBL_EXE_OUTPUT_DIR=%OUTPUT_ROOT%\bin\vc14win64-cmake
popd


GOTO :End

:End
pause