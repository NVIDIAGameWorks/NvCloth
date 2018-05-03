:: Reset errorlevel status so we are not inheriting this state from the calling process:
@call :CLEAN_EXIT

:: Use the "short" path so that we don't have to quote paths in that calls below. If we don't do that spaces can break us.
@SET PXSHARED_ROOT_DIR=%~dp0
@SET PXSHARED_ROOT_DIR=%PXSHARED_ROOT_DIR:\=/%

:: Run packman to ensure dependencies are present and run cmake generation script afterwards
@echo Running packman in preparation for cmake ...
@echo.
call "%~dp0buildtools\packman\packman.cmd" pull "%~dp0dependencies.xml" --platform android --postscript "%~dp0buildtools\cmake_projects_android.bat"
@if %ERRORLEVEL% neq 0 (
    @exit /b %errorlevel%
) else (
    @echo Success!
)

:CLEAN_EXIT
@exit /b 0