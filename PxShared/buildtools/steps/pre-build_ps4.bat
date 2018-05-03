@pushd "%~dp0..\.."
@call generate_projects_vc14ps4.bat
@popd
@if %errorlevel% NEQ 0 exit /b 1

