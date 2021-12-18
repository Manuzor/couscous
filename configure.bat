@echo off

where /q pwsh || (
  echo ERROR: "pwsh" not found - please make sure powershell core is installed and in your PATH. https://github.com/PowerShell/powershell/releases
  exit /b 1
)

set ROOT_DIR=%~dp0
set ROOT_DIR=%ROOT_DIR:~0,-1%
set OUT_DIR=%ROOT_DIR%\_out

set COMMON_FLAGS=-DCOUSCOUS_TESTS -luser32 -lgdi32
set DEBUG_FLAGS=-DDEBUG %COMMON_FLAGS%
set RELEASE_FLAGS=-DNDEBUG %COMMON_FLAGS%

pushd %ROOT_DIR%
    call :build-script-template > build.bat

    where /q fbuild && (
        call :sln-script-template > sln.bat
    ) || (
        echo WARNING: "fbuild" not found - sln.bat not generated. If you want this, pleast make sure FastBuild is installed and in your PATH. https://fastbuild.org
    )
popd

goto :eof

:build-script-template
echo @echo off
echo pushd %ROOT_DIR%
echo.
echo rem move into out dir
echo md %OUT_DIR% 2^> NUL
echo cd %OUT_DIR%
echo.
echo rem code generation
echo pwsh "%ROOT_DIR%\tools\generate_code.ps1"
echo.
echo if "%%BUILD_RELEASE%%"=="" (
echo    rem debug build
echo    zig c++ "%ROOT_DIR%\src\win32_main.cpp" %DEBUG_FLAGS% -o "%OUT_DIR%\bin\couscous-d.exe" -g
echo    zig c++ "%ROOT_DIR%\src\couscousc.cpp" %DEBUG_FLAGS% -o "%OUT_DIR%\bin\couscousc-d.exe" -g
echo ) else (
echo    rem release build
echo    zig c++ "%ROOT_DIR%\src\win32_main.cpp" %RELEASE_FLAGS% -o "%OUT_DIR%\bin\couscous.exe" -Ofast
echo    zig c++ "%ROOT_DIR%\src\couscousc.cpp" %RELEASE_FLAGS% -o "%OUT_DIR%\bin\couscousc.exe" -Ofast
echo )
echo.
echo popd
goto :eof

:sln-script-template
echo @echo off
echo call :bff-template ^> %OUT_DIR%\sln.bff
echo fbuild -ide -config %OUT_DIR%\sln.bff
echo goto :eof
echo :bff-template
echo echo VCXProject("couscous-proj")
echo echo {
echo echo     .ProjectOutput = "%OUT_DIR%\couscous.vcxproj"
echo echo     .ProjectBuildCommand = "%ROOT_DIR%\build.bat"
echo echo     .Output = "%OUT_DIR%\bin\couscous.exe"
echo echo     .ProjectBasePath = "%ROOT_DIR%"
echo echo     .ProjectInputPaths = "%ROOT_DIR%\src"
echo echo     .PreprocessorDefinitions = "DEBUG;COUSCOUSC=1"
echo echo }
echo echo VSSolution("all")
echo echo {
echo echo     .SolutionOutput = "%ROOT_DIR%\couscous.sln"
echo echo     .SolutionProjects =
echo echo     {
echo echo         "couscous-proj"
echo echo     }
echo echo }
goto :eof
