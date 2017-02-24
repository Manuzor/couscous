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
  if exist workspace (
    if not exist workspace\fastbuild (
      mkdir workspace\fastbuild
    )

    pushd workspace\fastbuild
      ..\..\tools\FASTBuild\FBuild.exe -config ..\..\fbuild.bff %*
    popd
  ) else (
    echo WARNING: No `workspace` folder found in repo-root. Did you forget to run the `init`-script?
  )
popd
