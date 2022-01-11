const std = @import("std");
const buildSokol = @import("lib/sokol-zig/build.zig").buildSokol;
const sokol_package_path = "lib/sokol-zig/src/sokol/sokol.zig";

pub fn build(b: *std.build.Builder) void {
    // Standard target options allows the person running `zig build` to choose
    // what target to build for. Here we do not override the defaults, which
    // means any target is allowed, and the default is native. Other options
    // for restricting supported target set are available.
    const target = b.standardTargetOptions(.{});

    // Standard release options allow the person running `zig build` to select
    // between Debug, ReleaseSafe, ReleaseFast, and ReleaseSmall.
    const mode = b.standardReleaseOptions();

    const sokol = buildSokol(b, target, mode, "lib/sokol-zig/");

    const shdc_exe: []const u8 = switch (@import("builtin").os.tag) {
        .windows => "lib/sokol-tools-bin/bin/win32/sokol-shdc.exe",
        else => unreachable, // not implemented yet.
    };
    const shdc = b.addSystemCommand(&[_][]const u8{
        shdc_exe,
        "--input",
        "src/shader.glsl",
        "--output",
        "src/shader.zig",
        "--format",
        "sokol_zig",
        "--tmpdir",
        "_out",
        "--slang",
        "hlsl5",
    });
    if (b.args) |args| {
        shdc.addArgs(args);
    }
    const shdc_run = b.step("shader", "Compile shaders");
    shdc_run.dependOn(&shdc.step);

    const couscous = b.addExecutable("couscous", "src/main.zig");
    couscous.setTarget(target);
    couscous.setBuildMode(mode);
    couscous.linkLibrary(sokol);
    couscous.addPackagePath("sokol", sokol_package_path);
    couscous.addPackagePath("clap", "lib/zig-clap/clap.zig");
    couscous.install();

    const couscous_run = couscous.run();
    couscous_run.step.dependOn(b.getInstallStep());
    if (b.args) |args| {
        couscous_run.addArgs(args);
    }

    const run_step = b.step("run", "Run the interpreter");
    run_step.dependOn(&couscous_run.step);

    const exe_tests = b.addTest("src/main.zig");
    exe_tests.setBuildMode(mode);

    const test_step = b.step("test", "Run unit tests");
    test_step.dependOn(&exe_tests.step);
}
