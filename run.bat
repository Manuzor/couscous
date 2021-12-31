@echo off
rem
rem Convenient script that I use to clear the console screen, compile all shaders, compile the source code,
rem and finally run the executable passing along all arguments.
rem
cls
call "%~dp0shader.bat" && (
    call "%~dp0build.bat" && (
        "%~dp0zig-out\bin\couscous.exe" %*
    )
)
