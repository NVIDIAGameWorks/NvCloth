CD /D %~dp0
echo "Note: You need to run this with admin rights for the first time to set GW_DEPS_ROOT globally."
call "../../../scripts/locate_gw_root.bat" GW_DEPS_ROOT_F
@echo on
setx GW_DEPS_ROOT "%GW_DEPS_ROOT_F%
echo GW_DEPS_ROOT = %GW_DEPS_ROOT%
call "../../../scripts/locate_cmake.bat" CMAKE_PATH_F
echo CMAKE_PATH_F = %CMAKE_PATH_F%


SET PATH=%PATH%;"%CMAKE_PATH_F%"

rmdir /s /q compiler\vc14win64-cmake\
mkdir compiler\vc14win64-cmake\
pushd compiler\vc14win64-cmake\
cmake ..\.. -G "Visual Studio 14 2015" -Ax64 
popd

rmdir /s /q compiler\vc14win32-cmake\
mkdir compiler\vc14win32-cmake\
pushd compiler\vc14win32-cmake\
cmake ..\.. -G "Visual Studio 14 2015" -AWin32
popd

pause
