@cmd /c "%~dp0build_all_windows_vc12.bat"
@if %ERRORLEVEL% neq 0 goto ERROR

@cmd /c "%~dp0build_all_windows_vc14.bat"
@if %ERRORLEVEL% neq 0 goto ERROR

:: Success
@exit /B 0

:ERROR
@echo !!! Failure while building for Windows!!!
@exit /B 1
