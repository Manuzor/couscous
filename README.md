# couscous

CHIP-8 interpreter toy thing.

# Build Instructions
_Note: Right now, only Windows platforms are supported. In fact, only Windows 10 and MSVC 14 (Visual Studio 2015) have been tested._

## TL;DR
~~~
> init.bat
> build.bat
~~~

Generate a Visual Studio project file in `workspace/vsXY` (e.g. `workspace/vs14`):
~~~
> build.bat vs
~~~

## Details
The first thing to do is to execute the init-script (`init.bat`) located in the root of the repository. A `workspace` folder will be created that is used for a number of things (intermediate build stuff, Visual Studio files, etc.). If the `workspace`-folder is deleted, the init-script needs to be run again, otherwise building won't work. It is safe to run the init-script multiple times in a row.

The build-system used by this project is [FASTBuild](https://fastbuild.org). However, you should **prefer to use the top-level `build.bat`**, which forwards all arguments to `FBuild.exe` but also changes the working directory for the duration of the script.
