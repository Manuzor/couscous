# couscous

CHIP-8 interpreter toy thing.

# Directories

* `code` - Contains all source code
* `tools` - Contains tools to generate a Sublime Text project, for example.
* `roms` - Some roms to quickly try out the interpreter.
* `docs` - Misc documentation.

Generated directories start with an underscore `_` and are ignored by the versioning system.
* `_build` - Contains all build artifacts
* `_workspace` - Intermediate files used by the build system or tools

# Build Instructions

Platform requirements:
* **Windows 10** or higher
  * Needs **Visual Studio 2017** or higher installed
  * An active **internet connection** the first time you execute the build script. See below for details.

Other platforms may or may not be supported in the future.

## TL;DR
~~~
> ./build.ps1
~~~

Generate a Visual Studio project file in `_workspace/vsXY` (e.g. `_workspace/vs15`):
~~~
> ./build.ps1 vs
~~~

## Details
The underlying build system is [FASTBuild](https://fastbuild.org). You do not have to have it installed because the build script downloads an appropriate version automatically when it can't find FASTBuild in `_workspace/fastbuild`. This only needs to be done the first time the build script is executed. But of course, this requires an active internet connection. If you don't have a connection and still want to build, either make sure FASTBuild is in your path or put the binaries into the directory `_workspace/fastbuild`.

Arguments to the build script are forwarded to FASTBuild, i.e. you can use `./build.ps1 -showtargets` to show all defined targets.

Build artifacts can be found in the `_build` directory.
