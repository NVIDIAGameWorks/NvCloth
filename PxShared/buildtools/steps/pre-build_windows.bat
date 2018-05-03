@pushd "%~dp0..\.."
@call generate_projects_win.bat vc12
@call generate_projects_win.bat vc14
@popd
@if %errorlevel% NEQ 0 exit /b 1

