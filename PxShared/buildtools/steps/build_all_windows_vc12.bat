:: Setup VS2013 build environment
@call "%VS120COMNTOOLS%VsdevCmd.bat"

:: Note that we use /build rather than /rebuild because cmake cleans the directories when we
:: generate the projects (making /rebuild redundant since it's basically /clean + /build). 
:: By using /build these bat files can be useful during regular development (to verify changes)
:: Will rename them from rebuild to build at a future point in time.

@set ROOT_PATH=%~dp0..\..\compiler

@set SOLUTION_PATH=vc12win32-mt\PxShared.sln
@call :BUILD
@if %ERRORLEVEL% neq 0 goto ERROR

@set SOLUTION_PATH=vc12win32-md\PxShared.sln
@call :BUILD
@if %ERRORLEVEL% neq 0 goto ERROR

@set SOLUTION_PATH=vc12win64-mt\PxShared.sln
@call :BUILD
@if %ERRORLEVEL% neq 0 goto ERROR

@set SOLUTION_PATH=vc12win64-md\PxShared.sln
@call :BUILD
@if %ERRORLEVEL% neq 0 goto ERROR

:: Success
@exit /B 0

:ERROR
@echo Failure while building *Windows vc12* targets!
@exit /B 1

:BUILD
@echo | set /p dummyName=** Building %SOLUTION_PATH% debug ... **
@devenv "%ROOT_PATH%\%SOLUTION_PATH%" /build "debug" /Project INSTALL
@echo ** End of %SOLUTION_PATH% debug **
@echo.
@if %ERRORLEVEL% neq 0 goto ERROR

@echo | set /p dummyName=** Building %SOLUTION_PATH% profile ... **
@devenv "%ROOT_PATH%\%SOLUTION_PATH%" /build "profile" /Project INSTALL
@echo ** End of %SOLUTION_PATH% profile **
@echo.
@if %ERRORLEVEL% neq 0 goto ERROR

@echo | set /p dummyName=** Building %SOLUTION_PATH% profile ... **
@devenv "%ROOT_PATH%\%SOLUTION_PATH%" /build "checked" /Project INSTALL
@echo ** End of %SOLUTION_PATH% checked **
@echo.
@if %ERRORLEVEL% neq 0 goto ERROR

@echo | set /p dummyName=** Building %SOLUTION_PATH% profile ... **
@devenv "%ROOT_PATH%\%SOLUTION_PATH%" /build "release" /Project INSTALL
@echo ** End of %SOLUTION_PATH% release **
@echo.
@if %ERRORLEVEL% neq 0 goto ERROR


@exit /B