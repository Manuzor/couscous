# couscous

CHIP-8 interpreter toy thing.

There are two versions in this repository (C++ and Zig).

# C++ Version

**NOTE: This version is obsolete.**

In this version I tried different naming conventions, generating code ahead of time (to minimize template usage),
creating everything without third-party dependencies, and some other things. You can find this in the `cpp` directory.

## Build

In the `cpp` directory, execute the `configure.bat` which generates the following files:

* `build.bat` to generate binaries.
* `sln.bat` to generate a Visual Studio .sln file for development.

The compiler used by the build script is `zig c++`. Compiled binaries can be found in the `cpp/out/bin` directory.

To create an optimized build, set the environment variable `BUILD_RELEASE` prior to invoking `build.bat`.

# Zig Version

Right now, zig is required to build this because the

## Build

Just use regular `zig build`.

## Shaders

Shaders are defined in `src/*.glsl` files. They're sokol-flavored GLSL files. As such, they're not used by the code at
all and have to be compiled into .zig files using the top-level script `shaders.bat`, which generates corresponding
`src/*.zig` files.

# Dependencies

The only external dependency required is a working zig distribution. Latest tested version is 0.9.0.

Third-party libraries are included via `git subtree`, i.e. a copy of them are included in this repository. Use the
script `lib/pull_subtrees.bat` to upgrade all dependencies to the latest `master` version. Alternatively, you can
inspect that script to learn how to upgrade or add individual dependencies.

# TODO

* Better separation between C++ and Zig sources.
* Cleanup roms directory to only include tested roms. Also try to credit the authors.
* WASM?
