@echo off
cls
call "%~dp0build.bat" && (
    "%~dp0zig-out\bin\couscous.exe" %*
)
