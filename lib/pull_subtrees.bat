@echo off
pushd %~dp0..
git subtree pull --prefix lib/sokol-zig https://github.com/floooh/sokol-zig master --squash %*
git subtree pull --prefix lib/sokol-tools-bin https://github.com/floooh/sokol-tools-bin master --squash %*
git subtree pull --prefix lib/zig-clap https://github.com/Hejsil/zig-clap master --squash %*
popd
