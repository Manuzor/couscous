@echo off
setlocal

set SourceDir=%~dp0.

set BuildDir=%~dp0../build/win_msvc_x86-64_debug
if not exist "%BuildDir%" mkdir "%BuildDir%"

pushd "%BuildDir%"
  set CFlags=/nologo /MTd /Z7
  cl %CFlags% "%SourceDir%/main.cpp" /Fe"main.exe"
popd
