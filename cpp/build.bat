@echo off
setlocal EnableDelayedExpansion

call :color-codes
set ROOT_DIR=%~dp0
set ROOT_DIR=%ROOT_DIR:~0,-1%

rem -------------------------------------------------------------------
rem --- Settings ------------------------------------------------------
rem -------------------------------------------------------------------
set OUT_DIR=%ROOT_DIR%\out
set COMMON_FLAGS=-std=c++17 -ferror-limit=6 -DCOUSCOUS_TESTS -luser32 -lgdi32
set DEBUG_FLAGS=-DDEBUG %COMMON_FLAGS%
set RELEASE_FLAGS=-DNDEBUG %COMMON_FLAGS%

rem -------------------------------------------------------------------
rem --- Command Line Parsing ------------------------------------------
rem -------------------------------------------------------------------
set HELP=%OFF%
set BUILD=
set RUN=%OFF%
set RELEASE=%OFF%
set SLN=%OFF%
set RUN_ARGS=
set PARSING_RUN_ARGS=0
for %%a in (%*) do (
    set ARG=%%a
    if "!PARSING_RUN_ARGS!" == "1" (
        set RUN_ARGS=!RUN_ARGS! !ARG!
    ) else if /I "!ARG!" == "--help" (
        set HELP=%ON%
    ) else if /I "!ARG!" == "-help" (
        set HELP=%ON%
    ) else if /I "!ARG!" == "help" (
        set HELP=%ON%
    ) else if /I "!ARG!" == "/?" (
        set HELP=%ON%
    ) else if /I "!ARG!" == "--" (
        set PARSING_RUN_ARGS=1
    ) else if /I "!ARG!" == "build" (
        set BUILD=%ON%
    ) else if /I "!ARG!" == "run" (
        set RUN=%ON%
    ) else if /I "!ARG!" == "-release" (
        set RELEASE=%ON%
    ) else if /I "!ARG!" == "sln" (
        set SLN=%ON%
    ) else (
        echo ERROR: Don't know what to do with '!ARG!'
        set HELP=%ON%
    )
)
if "!BUILD!" == "" (
    set BUILD=%OFF%
    if "!SLN!" == "%OFF%" (
        set BUILD=%ON%
    )
    if "!RUN!" == "%ON%" (
        set BUILD=%ON%
    )
)

rem -------------------------------------------------------------------
rem --- Help ----------------------------------------------------------
rem -------------------------------------------------------------------
if "!HELP!" == "%ON%" (
    call :show-help
    exit /b 1
)

rem -------------------------------------------------------------------
rem --- Setup ---------------------------------------------------------
rem -------------------------------------------------------------------
set COUSCOUS_OUTNAME=%OUT_DIR%\bin\couscous-d.exe
set COUSCOUSC_OUTNAME=%OUT_DIR%\bin\couscousc-d.exe
set COMPILER_FLAGS=%DEBUG_FLAGS%
if "!RELEASE!"=="%ON%" (
    set COUSCOUS_OUTNAME=%OUT_DIR%\bin\couscous.exe
    set COUSCOUSC_OUTNAME=%OUT_DIR%\bin\couscousc.exe
    set COMPILER_FLAGS=%RELEASE_FLAGS%
)

md "%OUT_DIR%" 2> NUL

rem -------------------------------------------------------------------
rem --- Build ---------------------------------------------------------
rem -------------------------------------------------------------------
if "!BUILD!" == "%ON%" (
    pushd %OUT_DIR%
    zig c++ "%ROOT_DIR%\src\win32_main.cpp" !COMPILER_FLAGS! -o !COUSCOUS_OUTNAME!
    zig c++ "%ROOT_DIR%\src\couscousc.cpp"  !COMPILER_FLAGS! -o !COUSCOUSC_OUTNAME!
    popd
)

rem -------------------------------------------------------------------
rem --- Run -----------------------------------------------------------
rem -------------------------------------------------------------------
if "!RUN!" == "%ON%" (
    echo running '!COUSCOUS_OUTNAME!'!RUN_ARGS!
    start "" "!COUSCOUS_OUTNAME!" !RUN_ARGS!
)

rem -------------------------------------------------------------------
rem --- Solution Generation -------------------------------------------
rem -------------------------------------------------------------------
if "!SLN!" == "%ON%" (
    call :bff-template > "%OUT_DIR%\sln.bff"
    where /q fbuild && (
        pushd %OUT_DIR%
        fbuild -ide -config "%OUT_DIR%\sln.bff"
        popd
    ) || (
        echo ERROR: "FBuild.exe" not found - VS solution not generated. If you want this, pleast make sure FastBuild is installed and in your PATH. https://fastbuild.org
    )
)

rem -------------------------------------------------------------------
rem --- End of File and Subroutines -----------------------------------
rem -------------------------------------------------------------------
goto :eof

:color-codes
rem ANSI escape sequences.
rem <ESC>[91m   Strong Foreground Red
rem <ESC>[92m   Strong Foreground Green
rem See also https://gist.github.com/mlocati/fdabcaeb8071d5c75a2d51712db24011?permalink_comment_id=1802144#gistcomment-1802144
set ESC=
set ON=%ESC%[92mON%ESC%[0m
set OFF=%ESC%[91mOFF%ESC%[0m
goto :eof

:show-help
echo Usage: %~n0%~x0 [build][run][sln] [-release] [[-[-]]help] [/?] -- ^<RUN_ARGS^>
echo.
echo Commands:
echo     build  Force building. This is the default command.
echo     run    Run after successful build.
echo     sln    Generate a .sln file. Required FBuild.exe to be on the PATH.
echo.
echo Options:
echo     -help     Show this message and exit
echo     -release  Build in release mode, debug mode otherwise.
echo.
echo Notes:
echo     Argument names are not case sensitive.
echo.
echo Commandline: %*
echo.
echo Parsed:
echo     BUILD .... !BUILD!
echo     RUN ...... !RUN!
echo     SLN ...... !SLN!
echo     RELEASE .. !RELEASE!
goto :eof

:bff-template
echo VCXProject("couscous-proj")
echo {
echo     .ProjectOutput = "%OUT_DIR%\couscous.vcxproj"
echo     .ProjectBuildCommand = "%ROOT_DIR%\build.bat"
echo     .Output = "%OUT_DIR%\bin\couscous-d.exe"
echo     .ProjectBasePath = "%ROOT_DIR%"
echo     .ProjectInputPaths = "%ROOT_DIR%\src"
echo     .PreprocessorDefinitions = "DEBUG;COUSCOUSC=1"
echo     .AdditionalOptions = "%DEBUG_FLAGS%"
echo }
echo VSSolution("all")
echo {
echo     .SolutionOutput = "%ROOT_DIR%\couscous.sln"
echo     .SolutionProjects =
echo     {
echo         "couscous-proj"
echo     }
echo }
goto :eof
