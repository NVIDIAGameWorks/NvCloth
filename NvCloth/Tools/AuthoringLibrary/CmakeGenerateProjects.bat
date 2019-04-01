CD /D %~dp0
set GW_DEPS_ROOT=%~dp0..\..\..\
echo "Note: You need to run this with admin rights for the first time to set GW_DEPS_ROOT globally."
REM single " is not a mistake
setx GW_DEPS_ROOT "%GW_DEPS_ROOT%
REM this one is for local use
set GW_DEPS_ROOT=%GW_DEPS_ROOT%
echo GW_DEPS_ROOT = %GW_DEPS_ROOT%

REM Install cmake using packman
set PACKMAN=call ../../scripts/packman/packman.cmd
%PACKMAN% pull -p windows "../../scripts/packman/packages/cmake.packman.xml"
IF %ERRORLEVEL% NEQ 0 (
    set EXIT_CODE=%ERRORLEVEL%
    goto End
)
set CMAKE=%PM_cmake_PATH%/bin/cmake.exe

set PX_OUTPUT_ROOT=%~dp0

set OUTPUT_ROOT=%~dp0
set AUTHORINGLIBRARY_ROOT_DIR=%~dp0


rmdir /s /q compiler\vc14win64-cmake\
mkdir compiler\vc14win64-cmake\
pushd compiler\vc14win64-cmake\
%CMAKE% ..\.. -G "Visual Studio 14 2015" -Ax64 -DTARGET_BUILD_PLATFORM=windows -DSTATIC_WINCRT=0 -DBL_DLL_OUTPUT_DIR=%OUTPUT_ROOT%\bin\vc14win64-cmake -DBL_LIB_OUTPUT_DIR=%OUTPUT_ROOT%\lib\vc14win64-cmake -DBL_EXE_OUTPUT_DIR=%OUTPUT_ROOT%\bin\vc14win64-cmake

popd

pause
