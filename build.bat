@echo off
pushd %~dp0
zig build --prominent-compile-errors %*
popd
