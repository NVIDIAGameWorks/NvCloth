@set ROOT_PATH=%~dp0..\..

::Remove old builds to keep things clean (important on build agents so we don't fill them up)
@del /q /f "%ROOT_PATH%\pxshared*.zip"

@set VERSION=%1
@set OPTIONS=
@if ["%VERSION%"] EQU [""] set VERSION=0

@call "%ROOT_PATH%\buildtools\packman\packman" install python 2.7.6-windows-x86
@if %ERRORLEVEL% NEQ 0 goto :ERROR

@call "%ROOT_PATH%\buildtools\packman\packman" install packman-packager 1.0
@if %ERRORLEVEL% NEQ 0 goto :ERROR

%PM_PYTHON% "%PM_packman_packager_PATH%/create_packman_packages.py" "%ROOT_PATH%\install" --output-dir="%ROOT_PATH%" vc14win pxshared %VERSION% "%PM_packman_packager_PATH%/templates/CMakeConfigWrapperWindows.cmake" "%PM_packman_packager_PATH%/templates/CMakeConfigVersionTemplate.cmake"
@if %ERRORLEVEL% NEQ 0 goto :ERROR

%PM_PYTHON% "%PM_packman_packager_PATH%/create_packman_packages.py" "%ROOT_PATH%\install" --output-dir="%ROOT_PATH%" vc12win pxshared %VERSION% "%PM_packman_packager_PATH%/templates/CMakeConfigWrapperWindows.cmake" "%PM_packman_packager_PATH%/templates/CMakeConfigVersionTemplate.cmake"
@if %ERRORLEVEL% NEQ 0 goto :ERROR

@goto :eof

:ERROR
@echo Failure during packaging for Windows!!!
@exit /b 1
