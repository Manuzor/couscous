@echo off
rem
rem Compile sokol-flavored glsl shader source files.
rem
pushd %~dp0
lib\sokol-tools-bin\bin\win32\sokol-shdc.exe --input src\shader.glsl --output src\shader.zig --format sokol_zig --tmpdir _out --slang hlsl5
popd
