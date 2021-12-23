@echo off
git -C "%~dp0.." subtree pull --prefix lib/sokol-zig https://github.com/floooh/sokol-zig master --squash
