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

Build artifacts can be found in the `_out` directory.

Requirements:
* **Windows 10** or higher
  * Needs zig 0.9 or higher (https://ziglang.org/)
  * Needs some flavor of **Visual Studio** (until zig eliminates that dependency). The latest Build Tools are probably fine.

Other platforms may or may not be supported in the future.

## TL;DR
~~~
> ./configure.bat
~~~

Compile executables:
~~~
> ./build.bat
~~~

Generate a top-level Visual Studio Solution file:
~~~
> ./sln.bat
~~~
