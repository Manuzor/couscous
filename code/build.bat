@echo off
setlocal

set SourceDir=%~dp0.

set BuildDir=%~dp0../build/win_msvc_x86-64_debug
if not exist "%BuildDir%" mkdir "%BuildDir%"

pushd "%BuildDir%"
  set CFlags=/nologo /MTd /Zi /DEBUG
  set LFlags=/INCREMENTAL:NO
  set Libs=User32.lib Gdi32.lib
  cl %CFlags% "%SourceDir%/win32_main.cpp" /Fe"couscous.exe" /link %LFlags% %Libs%
popd
