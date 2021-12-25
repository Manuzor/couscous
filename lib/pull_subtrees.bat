@echo off
git -C "%~dp0.." subtree pull --prefix lib/sokol-zig https://github.com/floooh/sokol-zig master --squash %*
git -C "%~dp0.." subtree pull --prefix lib/sokol-tools-bin https://github.com/floooh/sokol-tools-bin master --squash %*
