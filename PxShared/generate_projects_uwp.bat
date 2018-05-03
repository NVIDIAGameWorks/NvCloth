:: Reset errorlevel status so we are not inheriting this state from the calling process:
@call :CLEAN_EXIT

@IF %1.==. GOTO ADDITIONAL_PARAMS_MISSING

@SET PXSHARED_ROOT_DIR=%~dp0
@SET PXSHARED_ROOT_DIR=%PXSHARED_ROOT_DIR:\=/%

:: Run packman to ensure dependencies are present and run cmake generation script afterwards
@echo Running packman in preparation for cmake ...
@echo/

call "%~dp0buildtools\packman\packman.cmd" pull "%~dp0dependencies.xml" --platform %1uwp --postscript "%~dp0buildtools\cmake_projects_%1uwp.bat"
@if %ERRORLEVEL% neq 0 (
    @exit /b %errorlevel%
) else (
    @echo Success!
    @goto CLEAN_EXIT
)

:ADDITIONAL_PARAMS_MISSING
@echo Compiler needs to be specified (vc11, vc12, vc14, vc17 supported). Example: vc14

:CLEAN_EXIT
@exit /b 0