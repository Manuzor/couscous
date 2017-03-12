::
:: This script sets the working directory to `workspace\fastbuild` before
:: executing `FBuild.exe`, passing through all arguments passed to this
:: script.
::
:: The working directory is changed to prevent FASTBuild from dropping
:: unwanted intermediate stuff in the repo-root.
::
@echo off

pushd %~dp0
  if not exist workspace (
    powershell tools\generate_systembff.ps1 %*
  )

  if not exist workspace\fastbuild (
    mkdir workspace\fastbuild
  )

  pushd workspace\fastbuild
    ..\..\tools\FASTBuild\FBuild.exe -config ..\..\fbuild.bff %*
  popd
popd
