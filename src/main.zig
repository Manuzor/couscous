//! #NOTE All time values are in seconds if not noted otherwise.

const std = @import("std");
const log = std.log;

const sg = @import("sokol").gfx;
const sapp = @import("sokol").app;
const sgapp = @import("sokol").app_gfx_glue;
const stm = @import("sokol").time;
const saudio = @import("sokol").audio;
const sdtx = @import("sokol").debugtext;

const clap = @import("clap");

const shader = @import("shader.zig");
const chip8 = @import("chip8.zig");

// #CLEANUP
// pub fn framePrint(comptime fmt: []const u8, args: anytype) void {
//     var state = &global_state;
//     state.last_frame_print_len += (std.fmt.bufPrint(state.last_frame_print_buf[state.last_frame_print_len..], fmt, args) catch unreachable).len;
// }

const max_frame_time = 1.0 / 30.0;
const tick_duration = 1.0 / 60.0;
const tick_epsilon = 0.001;
const cpu_timer_interval = 1.0 / 60.0;

const W = 64;
const H = 32;

var global_state: struct {
    initialized: bool = false,
    rom_loaded: bool = false,
    debug: bool = false,

    pause: bool = false,
    remaining_steps: u32 = std.math.maxInt(u32),
    frame_counter: u64 = 0,
    laptime_store: u64 = 0,

    // #TODO random seed?
    rng: std.rand.DefaultPrng = std.rand.DefaultPrng.init(0),

    tick_counter: u32 = 0,
    tick_time_remaining: f64 = 0.0,
    cpu_timer_remaining: f64 = 0.0,
    cpu: chip8.Cpu = .{},
    memory_buf: [4096]u8 = [_]u8{0} ** 4096,
    display_buf: [H * W]u1 = [_]u1{0} ** (H * W),
    keyboard: chip8.Keyboard = .{},
    keyboard_layout: u8 = 0,

    // #CLEANUP
    // last_frame_print_buf: [8192]u8 = undefined,
    // last_frame_print_len: usize = 0,

    gfx: struct {
        bindings: sg.Bindings = .{},
        pipeline: sg.Pipeline = .{},
        pass_action: sg.PassAction = .{},
    } = .{},
} = .{};

const keyboard_layouts = [_][16]sapp.Keycode{
    [_]sapp.Keycode{
        ._0, // 0x0
        ._1, // 0x1
        ._2, // 0x2
        ._3, // 0x3
        ._4, // 0x4
        ._5, // 0x5
        ._6, // 0x6
        ._7, // 0x7
        ._8, // 0x8
        ._9, // 0x9
        .A, // 0xA
        .B, // 0xB
        .C, // 0xC
        .D, // 0xD
        .E, // 0xE
        .F, // 0xF
    },
    [_]sapp.Keycode{
        .X, // 0x0
        ._1, // 0x1
        ._2, // 0x2
        ._3, // 0x3
        .Q, // 0x4
        .W, // 0x5
        .E, // 0x6
        .A, // 0x7
        .S, // 0x8
        .D, // 0x9
        .Z, // 0xA
        .C, // 0xB
        ._4, // 0xC
        .R, // 0xD
        .F, // 0xE
        .V, // 0xF
    },
};

const RenderBox = struct {
    min_x: f32,
    min_y: f32,
    max_x: f32,
    max_y: f32,
};

pub fn aspectRatioFit(window_width: f32, window_height: f32, render_width: f32, render_height: f32) ?RenderBox {
    if (render_width <= 0 or render_height <= 0 or window_width <= 0 or window_height <= 0) {
        return null;
    }

    const optimal_window_width = window_height * (render_width / render_height);
    const optimal_window_height = window_width * (render_height / render_width);

    if (optimal_window_width > window_width) {
        const half_empty = 0.5 * (window_height - optimal_window_height);
        return RenderBox{
            .min_x = 0,
            .min_y = half_empty,
            .max_x = window_width,
            .max_y = half_empty + optimal_window_height,
        };
    } else {
        const half_empty = 0.5 * (window_width - optimal_window_width);
        return RenderBox{
            .min_x = half_empty,
            .min_y = 0,
            .max_x = half_empty + optimal_window_width,
            .max_y = window_height,
        };
    }
}

// sokol-app callbacks {

export fn init() void {
    var state = &global_state;
    _ = state;

    const params = comptime [_]clap.Param(clap.Help){
        clap.parseParam("-h, --help    Display this help and exit") catch unreachable,
        clap.parseParam("-p, --pause   Start in pause mode") catch unreachable,
        clap.parseParam("--debug       Show debug information") catch unreachable,
        clap.parseParam("<POS>...      Path to a ROM file to load") catch unreachable,
    };
    var diag = clap.Diagnostic{};
    var args = clap.parse(clap.Help, &params, .{ .diagnostic = &diag }) catch |err| {
        diag.report(std.io.getStdErr().writer(), err) catch {};
        log.err("error parsing command line.", .{});
        return;
    };
    defer args.deinit();

    if (args.flag("--help")) {
        var writer = std.io.getStdErr().writer();
        writer.print("usage: couscous ", .{}) catch {};
        clap.usage(writer, &params) catch {};
        writer.print("\noptions:\n", .{}) catch {};
        clap.help(writer, &params) catch {};
        return;
    }

    if (args.flag("--pause")) {
        setPause(true);
    }

    state.debug = args.flag("--debug");

    stm.setup();

    sg.setup(.{
        .context = sgapp.context(),
    });

    const quad_verts = [_]f32{
        -1.0, -1.0,
        1.0,  -1.0,
        1.0,  1.0,
        -1.0, 1.0,
    };
    state.gfx.bindings.vertex_buffers[0] = sg.makeBuffer(.{
        .data = sg.asRange(quad_verts),
    });

    const quad_indices = [_]u16{
        0, 1, 2,
        0, 2, 3,
    };
    state.gfx.bindings.index_buffer = sg.makeBuffer(.{
        .type = .INDEXBUFFER,
        .data = sg.asRange(quad_indices),
    });

    var tex_desc = sg.ImageDesc{
        .width = W,
        .height = H,
        .pixel_format = .R8,
        .usage = .STREAM,
    };
    state.gfx.bindings.fs_images[shader.SLOT_tex] = sg.makeImage(tex_desc);

    var pip_desc = sg.PipelineDesc{
        .index_type = .UINT16,
        .shader = sg.makeShader(shader.quadShaderDesc(sg.queryBackend())),
    };

    pip_desc.layout.attrs[shader.ATTR_vs_position].format = .FLOAT2;
    state.gfx.pipeline = sg.makePipeline(pip_desc);

    state.gfx.pass_action.colors[0] = .{
        .action = .CLEAR,
        .value = .{ .r = 0.25, .g = 0.1, .b = 0.3, .a = 1 },
    };

    saudio.setup(.{});

    var sdtx_desc = sdtx.Desc{};
    sdtx_desc.fonts[0] = sdtx.fontOric();
    sdtx.setup(sdtx_desc);

    // initialize display with test pattern.
    var tex_index: usize = 0;
    while (tex_index < (H * W)) : (tex_index += 1) {
        state.display_buf[tex_index] = @truncate(u1, (tex_index & 1) ^ ((tex_index / W) & 1));
    }

    for (args.positionals()) |rom_path| {
        if (state.rom_loaded) {
            log.warn("ignore loading of additional ROMs.", .{});
            break;
        }
        loadRomFromFile(state.memory_buf[0x200..], rom_path) catch {
            return;
        };
    }

    state.initialized = true;
}

export fn cleanup() void {
    var state = &global_state;
    _ = state;

    if (state.initialized) {
        sdtx.shutdown();
        saudio.shutdown();
        sg.shutdown();
    }
}

export fn frame() void {
    var state = &global_state;

    if (!state.initialized) {
        sapp.quit();
        return;
    }

    state.frame_counter += 1;

    // run at a fixed tick rate regardless of frame rate
    // #NOTE clamp frame time so debug breakpoints don't screw up timing.
    const delta_time = std.math.min(stm.sec(stm.laptime(&state.laptime_store)), max_frame_time);

    const canvas_width = sapp.widthf();
    const canvas_height = sapp.heightf();
    const render_scale: f32 = 1.0;
    const viewport = aspectRatioFit(canvas_width, canvas_height, render_scale * W, render_scale * H).?;
    const render_width = viewport.max_x - viewport.min_x;
    const render_height = viewport.max_y - viewport.min_y;

    const debug_font_scale = 1.0 / 2.0;
    sdtx.canvas(debug_font_scale * render_width, debug_font_scale * render_height);
    sdtx.origin(3, 3);
    sdtx.font(0);

    if (state.pause) {
        sdtx.print("=== PAUSED ===\n", .{});
        sdtx.print("Tick #{}\n", .{state.tick_counter});
        sdtx.print("press SPACE to step {} times\n", .{state.remaining_steps});
        sdtx.print("press 1|2|3|4 to increase step by 1|10|100|{}\n", .{(H * W) / 8});
        sdtx.print("    hold SHIFT to decrement, hold ALT to set\n", .{});
        sdtx.print("press ENTER to unpause\n", .{});
        sdtx.print("\n", .{});
        const cpu = &state.cpu;
        sdtx.print("CPU pc={x:0>4} i={x:0>4} sp={x:0>4} dt={x:0>2} st={x:0>2}\n", .{ cpu.pc, cpu.i, cpu.sp, cpu.dt, cpu.st });
        sdtx.print("       0 1 2 3 4 5 6 7 8 9 A B C D E F\n", .{});
        sdtx.print("    v={x}\n", .{std.fmt.fmtSliceHexLower(&cpu.v)});
        sdtx.print("    opcode={x:0>4} step={}\n", .{ cpu.loadOpcode(&state.memory_buf), cpu.step });
    }
    if (state.keyboard.block) {
        sdtx.print("!!! waiting for input\n", .{});
    }

    if (!state.pause and state.remaining_steps > 0) {
        state.tick_time_remaining += delta_time;

        var cpu = &state.cpu;
        var memory = @as([]u8, &state.memory_buf);
        var display = chip8.Display{
            .width = W,
            .height = H,
            .data = @as([]u1, &state.display_buf),
        };
        var keyboard = &state.keyboard;

        // #TODO wrap this in the tick-loop below?
        state.cpu_timer_remaining -= delta_time;
        if (state.cpu_timer_remaining <= 0) {
            state.cpu_timer_remaining += cpu_timer_interval;
            if (cpu.dt > 0) {
                cpu.dt -= 1;
            }
            if (cpu.st > 0) {
                cpu.st -= 1;
            }
        }

        while (state.tick_time_remaining > -tick_epsilon) {
            state.tick_time_remaining -= tick_duration;
            cpu.opcode = cpu.loadOpcode(memory);
            cpu.tick(memory, display, keyboard, state.rng.random());

            state.tick_counter += 1;

            state.remaining_steps -= 1;
            if (state.remaining_steps == 0) {
                setPause(true);
                break;
            }
        }
    }

    var tex_data = sg.ImageData{};
    tex_data.subimage[0][0] = sg.asRange(state.display_buf);
    sg.updateImage(state.gfx.bindings.fs_images[shader.SLOT_tex], tex_data);

    sg.beginDefaultPassf(state.gfx.pass_action, canvas_width, canvas_height);
    sg.applyViewportf(viewport.min_x, viewport.min_y, render_width, render_height, false);
    sg.applyPipeline(state.gfx.pipeline);
    sg.applyBindings(state.gfx.bindings);
    sg.draw(0, 6, 1);
    sdtx.draw();
    sg.endPass();
    sg.commit();
}

export fn input(ev: ?*const sapp.Event) void {
    var state = &global_state;
    _ = state;

    const event = ev.?;
    if ((event.type == .KEY_DOWN) or (event.type == .KEY_UP)) {
        const alt = event.modifiers & sapp.modifier_alt != 0;
        const shift = event.modifiers & sapp.modifier_shift != 0;
        const key_pressed = event.type == .KEY_DOWN;
        const key_repeat = event.key_repeat;
        const key_code = event.key_code;

        if (!key_repeat) {
            const layout = &keyboard_layouts[state.keyboard_layout];
            var key_index: u8 = 0x0;
            while (key_index <= 0xF) : (key_index += 1) {
                if (key_code == layout[key_index]) {
                    state.keyboard.state[key_index] = key_pressed;
                    state.keyboard.last = key_index;
                    state.keyboard.block = false;
                    break;
                }
            }
        }

        if (key_pressed) {
            var step_input: i64 = 0;
            switch (key_code) {
                ._1 => {
                    step_input = 1;
                },
                ._2 => {
                    step_input = 10;
                },
                ._3 => {
                    step_input = 100;
                },
                ._4 => {
                    step_input = H * W / 8;
                },
                .ESCAPE => {
                    if (!key_repeat) {
                        log.debug("exiting - ESCAPE was pressed", .{});
                        sapp.quit();
                    }
                },
                .ENTER => {
                    if (!key_repeat) {
                        setPause(false);
                    }
                },
                .SPACE => {
                    if (state.pause) {
                        log.debug("single step", .{});
                        state.pause = false;
                    } else if (!key_repeat and !state.pause) {
                        setPause(true);
                    }
                },
                else => {},
            }
            if (step_input != 0 and state.pause) {
                if (alt) {
                    state.remaining_steps = @intCast(u32, step_input);
                } else {
                    const sign: i64 = if (shift) -1 else 1;
                    state.remaining_steps = @intCast(u32, std.math.max(1, @as(i64, state.remaining_steps) + sign * step_input));
                }
            }
        }
    }
}

// }

fn loadRomFromFile(dest: []u8, path: []const u8) !void {
    const file = std.fs.cwd().openFile(path, .{}) catch |err| {
        log.err("{}: Failed to open ROM file '{s}'", .{ err, path });
        return error.RomLoad;
    };
    defer file.close();
    const bytes_read = file.readAll(dest) catch |err| {
        log.err("{}: Failed to read ROM file '{s}'", .{ err, path });
        return error.RomLoad;
    };
    log.debug("loaded ROM with {} bytes: '{s}'", .{ bytes_read, path });
}

fn setPause(pause: bool) void {
    var state = &global_state;

    if (state.pause == pause) {
        return;
    }

    if (pause) {
        log.debug("pause", .{});
        state.pause = true;
        state.remaining_steps = 1;
    } else {
        log.debug("unpause", .{});
        state.pause = false;
        state.remaining_steps = std.math.maxInt(u32);
    }
}

pub fn main() anyerror!void {
    sapp.run(.{
        .init_cb = init,
        .frame_cb = frame,
        .event_cb = input,
        .cleanup_cb = cleanup,
        .width = 20 * W,
        .height = 20 * H,
        .window_title = "couscous CHIP-8 in zig powered by sokol",
    });
}

test "basic test" {
    try std.testing.expectEqual(10, 3 + 7);
}
