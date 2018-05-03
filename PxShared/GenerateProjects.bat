@echo off

REM Make sure the various variables that we need are set

IF NOT DEFINED GW_DEPS_ROOT GOTO GW_DEPS_ROOT_UNDEFINED

IF NOT DEFINED BOOST_ROOT GOTO BOOST_ROOT_UNDEFINED

IF NOT DEFINED CUDA_BIN_PATH GOTO CUDA_ROOT_UNDEFINED

IF EXIST GW_DEPS_ROOT\CMakeModules (
	set CMAKE_MODULE_PATH=%GW_DEPS_ROOT%\CMakeModules
) ELSE (
	set CMAKE_MODULE_PATH=%GW_DEPS_ROOT%\sw\physx\tools\CMakeModules
)

REM Generate projects here

rmdir /s /q compiler\vc12win32-cmake\
mkdir compiler\vc12win32-cmake\
pushd compiler\vc12win32-cmake\
cmake ..\cmake\windows -G "Visual Studio 12 2013" -AWin32 -DTARGET_BUILD_PLATFORM=windows
popd

rmdir /s /q compiler\vc12win64-cmake\
mkdir compiler\vc12win64-cmake\
pushd compiler\vc12win64-cmake\
cmake ..\cmake\windows -G "Visual Studio 12 2013" -Ax64 -DTARGET_BUILD_PLATFORM=windows
popd

popd

GOTO :End

:GW_DEPS_ROOT_UNDEFINED
ECHO GW_DEPS_ROOT has to be defined, pointing to the root of the dependency tree.
PAUSE
GOTO END

:BOOST_ROOT_UNDEFINED
ECHO BOOST_ROOT has to be defined, pointing to the root of your local Boost install.
PAUSE
GOTO END

:CUDA_ROOT_UNDEFINED
ECHO CUDA_BIN_PATH has to be defined, pointing to the bin folder of your local CUDA install.
PAUSE

:End
