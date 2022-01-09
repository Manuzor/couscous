//! #NOTE All time values are in seconds if not noted otherwise.

const std = @import("std");
const log = std.log;

const sg = @import("sokol").gfx;
const sapp = @import("sokol").app;
const sgapp = @import("sokol").app_gfx_glue;
const saudio = @import("sokol").audio;
const sdtx = @import("sokol").debugtext;

const clap = @import("clap");

const shader = @import("shader.zig");
const chip8 = @import("chip8.zig");
const chip8_asm = @import("chip8_asm.zig");
const chip8_charmap = @import("chip8_charmap.zig");

const window_title_fmt = "couscous {s} // rom={}B dt=Ø{d:.3}s sim={}Hz";
const max_frame_time = 1.0 / 30.0;
// #TODO What frequency do we want to run the interpreter at by default?
// const default_hz = 700;
// const default_hz = 120;
// const default_hz = 60;
const default_hz = 120;
const cpu_timer_hz = 60;
const cooldown_tolerance = 0.001;
const history_len = 32;
const norom_file_path = "NOROM.ch8";

const W = 64;
const H = 32;

const Snapshot = struct {
    frame_index: u64,
    tick_index: u64,
    dt: u8,
    pc: u16,
    opcode: u16,
};

var global_state: struct {
    allocator: std.mem.Allocator = undefined,

    initialized: bool = false,
    display_test_pattern: bool = false,
    rom_path: ?[]const u8 = null,
    rom_size: usize = undefined,

    pause: bool = false,
    remaining_steps: u64 = std.math.maxInt(u64),
    frame_counter: u64 = 0,
    // laptime_store: u64 = 0,
    frame_timer: std.time.Timer = undefined,
    total_time_elapsed: f64 = 0.0,
    step_button_down: bool = false,

    rng_seed: ?u64 = null,
    rng: std.rand.DefaultPrng = undefined,

    delta_time_history: [60]f64 = [_]f64{0} ** 60,

    tick_counter: u64 = 0,
    tick_snapshots: [history_len]Snapshot = [_]Snapshot{undefined} ** history_len,
    tick_hz_config: u64 = default_hz,
    tick_hz: u64 = default_hz,
    last_hz: u64 = 0,
    tick_timer_cooldown: f64 = 0.0,
    cpu_timer_cooldown: f64 = 0.0,

    cpu: chip8.Cpu = .{},
    memory_buf: [4096]u8 = [_]u8{0} ** 4096,
    display_buf: [H * W]u1 = [_]u1{0} ** (H * W),
    keyboard: chip8.Keyboard = .{},
    keyboard_layout: u8 = 0,

    gfx: struct {
        bindings: sg.Bindings = .{},
        pipeline: sg.Pipeline = .{},
        pass_action: sg.PassAction = .{},
    } = .{},

    audio: struct {
        samples: [32]f32 = undefined,
        sample_index: usize = 0,
    } = .{},

    fn setPause(state: *@This(), pause: bool) void {
        if (pause) {
            state.pause = true;
            state.remaining_steps = 1;
        } else {
            state.pause = false;
            state.remaining_steps = std.math.maxInt(u64);
        }
    }

    fn takeSnapshot(state: @This()) Snapshot {
        return .{
            .frame_index = state.frame_counter,
            .tick_index = state.tick_counter,
            .dt = state.cpu.dt,
            .pc = state.cpu.pc,
            .opcode = chip8.readOpcode(&state.memory_buf, state.cpu.pc),
        };
    }
} = .{};

const keyboard_layouts = [_][16]sapp.Keycode{
    [_]sapp.Keycode{
        // 1 2 3 4  ->  1 2 3 C
        // Q W E R  ->  4 5 6 D
        // A S D F  ->  7 8 9 E
        // Z X C V  ->  A 0 B F
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

    state.frame_timer = std.time.Timer.start() catch |err| {
        log.err("{} - Unable to start timer", .{err});
        return;
    };

    state.rng = std.rand.DefaultPrng.init(state.rng_seed orelse std.crypto.random.int(u64));

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

    if (state.display_test_pattern) {
        // Initialize display with test pattern.
        var index: usize = 0;
        while (index < (H * W)) : (index += 1) {
            state.display_buf[index] = @truncate(u1, (index & 1) ^ ((index / W) & 1));
        }
    }

    std.mem.copy(u8, state.memory_buf[chip8.charmap_base_address .. chip8.charmap_base_address + chip8.charmap_size], &chip8_charmap.charmap);

    state.rom_size = if (state.rom_path) |rom_path| blk: {
        const rom = loadRomFromFile(state.memory_buf[chip8.user_base_address..], rom_path) catch {
            return;
        };
        log.debug("loaded ROM with {} bytes: '{s}'", .{ rom.len, rom_path });
        break :blk rom.len;
    } else blk: {
        log.warn("no ROM file specified", .{});
        const rom = @embedFile(norom_file_path);
        std.mem.copy(u8, state.memory_buf[chip8.user_base_address..], rom);
        break :blk rom.len;
    };

    state.initialized = true;
}

export fn cleanup() void {
    var state = &global_state;

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

    var cpu = &state.cpu;
    var memory = @as([]u8, &state.memory_buf);
    var display = chip8.Display{
        .width = W,
        .height = H,
        .data = @as([]u1, &state.display_buf),
    };
    var keyboard = &state.keyboard;

    defer state.frame_counter += 1;

    // run at a fixed tick rate regardless of frame rate
    // #NOTE clamp frame time so debug breakpoints don't screw up timing.
    const delta_time = std.math.min(@intToFloat(f64, state.frame_timer.lap()) / std.time.ns_per_s, max_frame_time);
    state.total_time_elapsed += delta_time;

    const delta_time_history_index = (state.frame_counter) % state.delta_time_history.len;
    state.delta_time_history[delta_time_history_index] = delta_time;

    const canvas_width = sapp.widthf();
    const canvas_height = sapp.heightf();
    const render_scale: f32 = 1.0;
    const viewport = aspectRatioFit(canvas_width, canvas_height, render_scale * W, render_scale * H).?;
    const render_width = viewport.max_x - viewport.min_x;
    const render_height = viewport.max_y - viewport.min_y;

    if (state.pause) {
        sdtx.setContext(sdtx.defaultContext());
        const debug_font_scale = 1.0 / 2.0;
        sdtx.canvas(debug_font_scale * canvas_width, debug_font_scale * canvas_height);
        sdtx.origin(3, 3);
        sdtx.font(0);
    } else {
        sdtx.setContext(.{});
    }

    if (state.pause) {
        sdtx.print("=== PAUSED ===\n", .{});
        sdtx.print("Executing at {}Hz\n", .{state.tick_hz});
        sdtx.print("  press 0 to reset to {}Hz\n", .{state.tick_hz_config});
        sdtx.print("  press 7|8|9 to increase by 1|10|60Hz\n", .{});
        sdtx.print("  hold SHIFT to decrease, hold ALT to multiply the addend by 10\n", .{});
        sdtx.print("press SPACE to step {} times\n", .{state.remaining_steps});
        sdtx.print("  press 1|2|3|4 to increase step by 1|10|100|{}\n", .{(H * W) / 8});
        sdtx.print("  hold SHIFT to decrement, hold ALT to set\n", .{});
        sdtx.print("hold J to run until released\n", .{});
        sdtx.print("press ENTER to unpause\n", .{});
        sdtx.print("\n", .{});
        sdtx.print("CPU pc={x:0>4} i={x:0>4} sp={x:0>4} dt={x:0>2} st={x:0>2}\n", .{ cpu.pc, cpu.i, cpu.sp, cpu.dt, cpu.st });
        sdtx.print("       0 1 2 3 4 5 6 7 8 9 A B C D E F\n", .{});
        sdtx.print("    v={x}\n", .{std.fmt.fmtSliceHexLower(&cpu.v)});
    }

    var allow_tick = !state.pause and state.remaining_steps > 0;
    if (state.pause and state.step_button_down) {
        allow_tick = true;
    }

    if (allow_tick) {
        state.cpu_timer_cooldown -= delta_time;
        const cpu_timer_interval = 1.0 / @intToFloat(f64, cpu_timer_hz);
        while (state.cpu_timer_cooldown < cooldown_tolerance) {
            state.cpu_timer_cooldown += cpu_timer_interval;
            cpu.dt -|= 1;
            cpu.st -|= 1;
        }

        state.tick_timer_cooldown -= delta_time;
        const tick_interval = 1 / @intToFloat(f64, state.tick_hz);
        while (state.tick_timer_cooldown < cooldown_tolerance) {
            state.tick_timer_cooldown += tick_interval;

            if (keyboard.block) {
                // Wait for input before ticking again.
                continue;
            }

            if (!cpu.isLooping(memory)) { // There's no point in ticking anymore if we're stuck in an endless loop.
                const snapshot = state.takeSnapshot();
                var save_snapshot = true;

                const was_waiting_for_input = cpu.waiting_for_input;
                cpu.tick(memory, display, keyboard, state.rng.random());

                if (!was_waiting_for_input and cpu.waiting_for_input) {
                    save_snapshot = false;
                }

                if (save_snapshot) {
                    const snapshot_index = (state.tick_counter) % history_len;
                    state.tick_snapshots[snapshot_index] = snapshot;
                    state.tick_counter +%= 1;
                }
            }

            state.remaining_steps -|= 1;
            if (state.remaining_steps == 0) {
                state.setPause(true);
                if (!state.step_button_down) {
                    break;
                }
            }
        }
    }

    if (keyboard.block) {
        sdtx.print("waiting for input\n", .{});
    } else if (cpu.isLooping(memory)) {
        sdtx.print("loop\n", .{});
    }

    if (state.pause) {
        sdtx.print("\nFrame  Tick DT PC   OPCODE", .{});
        const util = struct {
            fn printSnapshot(snapshot: Snapshot) void {
                sdtx.print("\n{:>5} {:>5} {X:0>2} {X:0>4} {X:0>4} - ", .{ snapshot.frame_index, snapshot.tick_index, snapshot.dt, snapshot.pc, snapshot.opcode });
                var buf: [64]u8 = undefined;
                var buf_stream = std.io.fixedBufferStream(&buf);
                if (chip8_asm.disassembleOpcode(snapshot.opcode, buf_stream.writer()) catch unreachable) {
                    sdtx.print("{s}", .{buf_stream.getWritten()});
                } else {
                    sdtx.print("?", .{});
                }
            }
        };
        util.printSnapshot(state.takeSnapshot());
        const sample_count = std.math.min(state.tick_counter, state.tick_snapshots.len);
        var sample_index = state.tick_counter % state.tick_snapshots.len;
        var iter: usize = 0;
        while (iter < sample_count) : (iter += 1) {
            if (sample_index == 0) {
                sample_index = sample_count;
            }
            sample_index -= 1;

            const snapshot = state.tick_snapshots[sample_index];
            util.printSnapshot(snapshot);
        }
    }

    var tex_data = sg.ImageData{};
    tex_data.subimage[0][0] = sg.asRange(display.data);
    sg.updateImage(state.gfx.bindings.fs_images[shader.SLOT_tex], tex_data);

    sg.beginDefaultPassf(state.gfx.pass_action, canvas_width, canvas_height);
    sg.applyViewportf(viewport.min_x, viewport.min_y, render_width, render_height, false);
    sg.applyPipeline(state.gfx.pipeline);
    sg.applyBindings(state.gfx.bindings);
    sg.draw(0, 6, 1);
    sdtx.draw();
    sg.endPass();
    sg.commit();

    const num_samples = saudio.expect();
    var sample_gen_index: i32 = 0;
    var audio = &state.audio;
    while (sample_gen_index < num_samples) : (sample_gen_index += 1) {
        if (audio.sample_index == audio.samples.len) {
            audio.sample_index = 0;
            _ = saudio.push(&audio.samples[0], @intCast(i32, audio.samples.len));
        }
        audio.samples[audio.sample_index] = blk: {
            if (cpu.st > 0) {
                const mag = 0.1;
                break :blk mag * @sin((@intToFloat(f32, audio.sample_index) / @intToFloat(f32, audio.samples.len)) * std.math.tau);
            } else {
                break :blk 0;
            }
        };
        audio.sample_index += 1;
    }

    if (delta_time_history_index == 0 or state.last_hz != state.tick_hz) blk: {
        const sample_count = std.math.min(state.frame_counter, state.delta_time_history.len);
        const tail_count = std.math.min(sample_count, state.delta_time_history.len - delta_time_history_index);
        const head_count = sample_count - tail_count;
        var sum: f64 = 0;
        for (state.delta_time_history[delta_time_history_index .. delta_time_history_index + tail_count]) |value| {
            sum += value;
        }
        for (state.delta_time_history[0..head_count]) |value| {
            sum += value;
        }
        const delta_time_avg = sum / @intToFloat(f64, sample_count);

        var buf: [128]u8 = undefined;
        const rom_path = std.fs.path.basename(state.rom_path orelse norom_file_path);
        var new_title = std.fmt.bufPrintZ(&buf, window_title_fmt, .{ rom_path, state.rom_size, delta_time_avg, state.tick_hz }) catch break :blk;
        sapp.setWindowTitle(new_title);
        state.last_hz = state.tick_hz;
    }
}

export fn input(ev: ?*const sapp.Event) void {
    var state = &global_state;

    const event = ev.?;
    if ((event.type == .KEY_DOWN) or (event.type == .KEY_UP)) {
        const alt = event.modifiers & sapp.modifier_alt != 0;
        const shift = event.modifiers & sapp.modifier_shift != 0;
        const key_down = event.type == .KEY_DOWN;
        const key_repeat = event.key_repeat;
        const key_code = event.key_code;

        switch (key_code) { // Handle host input first, regardless of state.
            .ESCAPE => {
                if (key_down and !key_repeat) {
                    log.debug("exiting - ESCAPE was pressed", .{});
                    sapp.quit();
                }
            },
            .ENTER => {
                if (key_down and !key_repeat) {
                    state.setPause(false);
                }
            },
            .SPACE => {
                if (key_down) {
                    if (state.pause) {
                        log.debug("single step", .{});
                        state.pause = false;
                    } else if (!key_repeat and !state.pause) {
                        state.setPause(true);
                    }
                }
            },
            .J => {
                state.step_button_down = key_down;
            },
            .T => {
                if (key_down and !key_repeat) {
                    var buf: [4096]u8 = undefined;
                    var buf_index: usize = 0;
                    const sample_count = std.math.min(state.frame_counter, state.delta_time_history.len);
                    var sample_index = state.frame_counter % state.delta_time_history.len;
                    var iter: usize = 0;
                    while (iter < sample_count) : (iter += 1) {
                        if (sample_index == 0) {
                            sample_index = sample_count;
                        }
                        sample_index -= 1;
                        buf_index += (std.fmt.bufPrint(buf[buf_index..], "\n  frame {:>5}: {d:>6.3}", .{ state.frame_counter - iter -| 1, state.delta_time_history[sample_index] }) catch unreachable).len;
                    }
                    log.debug("delta time history:{s}", .{buf[0..buf_index]});
                }
            },
            else => {},
        }

        // Debug input
        var step_input: u64 = 0;
        var hz_input: u64 = 0;
        if (key_down) {
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

                ._0 => {
                    state.tick_hz = state.tick_hz_config;
                },
                ._7 => {
                    hz_input += 1;
                },
                ._8 => {
                    hz_input += 10;
                },
                ._9 => {
                    hz_input += default_hz;
                },
                else => {},
            }
        }
        if (state.pause and step_input != 0) {
            if (alt) {
                state.remaining_steps = step_input;
            } else {
                if (shift) {
                    state.remaining_steps -|= step_input;
                    if (state.remaining_steps == 0) {
                        state.remaining_steps = 1;
                    }
                } else {
                    state.remaining_steps += step_input;
                }
            }
        }
        if (hz_input != 0) {
            if (alt) {
                hz_input *= 10;
            }
            if (shift) {
                state.tick_hz -|= hz_input;
                if (state.tick_hz == 0) {
                    state.tick_hz = 1;
                }
            } else {
                state.tick_hz += hz_input;
            }
        }

        // Handle regular CHIP-8 input.
        if (!state.pause) {
            if (!key_repeat) {
                const layout = &keyboard_layouts[state.keyboard_layout];
                var key_index: u8 = 0x0;
                while (key_index <= 0xF) : (key_index += 1) {
                    if (key_code == layout[key_index]) {
                        state.keyboard.state[key_index] = key_down;
                        state.keyboard.last = key_index;
                        if (key_down) {
                            state.keyboard.block = false;
                        }
                        break;
                    }
                }
            }
        }
    }
}

// }

fn loadRomFromFile(dest: []u8, path: []const u8) ![]u8 {
    const file = std.fs.cwd().openFile(path, .{}) catch |err| {
        log.err("{}: Failed to open ROM file '{s}'", .{ err, path });
        return error.RomLoad;
    };
    defer file.close();
    const bytes_read = file.readAll(dest) catch |err| {
        log.err("{}: Failed to read ROM file '{s}'", .{ err, path });
        return error.RomLoad;
    };
    return dest[0..bytes_read];
}

pub fn main() anyerror!void {
    var state = &global_state;
    var gpa = std.heap.GeneralPurposeAllocator(.{}){};
    defer if (gpa.deinit()) unreachable;
    var arena = std.heap.ArenaAllocator.init(gpa.allocator());
    state.allocator = arena.allocator();
    defer arena.deinit();

    {
        const params = comptime [_]clap.Param(clap.Help){
            clap.parseParam("-p, --pause                  Start in pause mode") catch unreachable,
            clap.parseParam("--hz <HZ>                    Number of instructions executed per second, i.e. the simulation speed. Default: " ++ std.fmt.comptimePrint("{}", .{default_hz})) catch unreachable,
            clap.parseParam("--shift-src <SRC>            Behavior of the shift instructions (8xy6/8xy7). Valid values are 'x' or 'y'. Default: 'y'") catch unreachable,
            clap.parseParam("--jump-offset-src <MODE>     Behavior of the jump instructions with register offset (Bnnn). Valid values are '0' or 'x'. Default: '0'") catch unreachable,
            clap.parseParam("--register-dump-mode <MODE>  Behavior of the register load and store instructions (Fx55/Fx65). Valid values are 'immutable' or 'increment'. Default: 'immutable'") catch unreachable,
            clap.parseParam("--dis <OUT_FILE>             Disassemble the <ROM_FILE> and write to <OUT_FILE>.") catch unreachable,
            clap.parseParam("--asm <IN_FILE>              Assemble <IN_FILE> and write to <ROM_FILE>.") catch unreachable,
            clap.parseParam("--seed <SEED>                Unsigned 64-bit integer seed value used to initialize the PRNG.") catch unreachable,
            clap.parseParam("--nocls                      Do not start with a cleared display.") catch unreachable,
            clap.parseParam("-h, --help                   Display this help and exit") catch unreachable,
            clap.parseParam("<ROM_FILE>                Path to a ROM file to load") catch unreachable,
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
            state.setPause(true);
        }

        if (args.flag("--nocls")) {
            state.display_test_pattern = true;
        }

        if (std.process.getEnvMap(state.allocator) catch null) |*env| {
            defer env.deinit();
            if (env.get("COUSCOUS_PAUSE")) |val| {
                if (std.mem.eql(u8, val, "1")) {
                    state.setPause(true);
                }
            }
            if (env.get("COUSCOUS_NOCLS")) |val| {
                if (std.mem.eql(u8, val, "1")) {
                    state.display_test_pattern = true;
                }
            }
        }

        for (args.positionals()) |rom_path| {
            if (state.rom_path != null) {
                log.warn("ignore loading of additional ROMs.", .{});
                break;
            }
            state.rom_path = try state.allocator.dupe(u8, rom_path);
        }

        if (args.option("--dis")) |out_path| {
            const rom_path = state.rom_path orelse {
                log.err("--dis needs a valid rom to load.", .{});
                return error.InvalidOperation;
            };

            var rom_buf: [4096]u8 = undefined;
            const rom = try loadRomFromFile(&rom_buf, rom_path);

            var out_file = std.fs.cwd().createFile(out_path, .{}) catch |err| {
                log.err("failed to open output file for writing '{s}'", .{out_path});
                return err;
            };
            defer out_file.close();
            try chip8_asm.disassemble(rom, chip8.user_base_address, out_file.writer(), .{});
            return;
        }

        if (args.option("--asm")) |code_path| {
            const out_path = state.rom_path orelse {
                log.err("--asm needs a rom path to write to.", .{});
                return error.InvalidOperation;
            };

            var code_buf: [4096]u8 = undefined;
            const code = blk: {
                const code_file = std.fs.cwd().openFile(code_path, .{}) catch |err| {
                    log.err("{}: Failed to open code file '{s}'", .{ err, code_path });
                    return error.CodeLoad;
                };
                defer code_file.close();
                const bytes_read = code_file.readAll(&code_buf) catch |err| {
                    log.err("{}: Failed to read code file '{s}'", .{ err, code_path });
                    return error.CodeLoad;
                };
                break :blk code_buf[0..bytes_read];
            };

            var out_file = std.fs.cwd().createFile(out_path, .{}) catch |err| {
                log.err("failed to open output file for writing '{s}'", .{out_path});
                return err;
            };
            defer out_file.close();

            try chip8_asm.assemble(arena.allocator(), code, chip8.user_base_address, out_file.writer(), .{
                .file_name = code_path,
            }, log);
            return;
        }

        if (args.option("--shift-src")) |shift_src| {
            var found = false;
            inline for (@typeInfo(chip8.ShiftSrc).Enum.fields) |enum_field| {
                if (std.mem.eql(u8, enum_field.name, shift_src)) {
                    state.cpu.shift_src = @intToEnum(chip8.ShiftSrc, enum_field.value);
                    found = true;
                }
            }
            if (found) {
                log.debug("cpu.shift_src is '{}'", .{state.cpu.shift_src});
            } else {
                log.err("invalid value for --shift-src '{s}'", .{shift_src});
                return error.InvalidCommandLine;
            }
        }

        if (args.option("--jump-offset-src")) |jump_offset_src| {
            var found = false;
            inline for (@typeInfo(chip8.JumpOffsetSrc).Enum.fields) |enum_field| {
                if (std.mem.eql(u8, enum_field.name, jump_offset_src)) {
                    state.cpu.jump_offset_src = @intToEnum(chip8.JumpOffsetSrc, enum_field.value);
                    found = true;
                }
            }
            if (found) {
                log.debug("cpu.jump_offset_src is '{}'", .{state.cpu.jump_offset_src});
            } else {
                log.err("invalid value for --jump-offset-src '{s}'", .{jump_offset_src});
                return error.InvalidCommandLine;
            }
        }

        if (args.option("--register-dump-mode")) |register_dump_behavior| {
            var found = false;
            inline for (@typeInfo(chip8.RegisterDumpBehavior).Enum.fields) |enum_field| {
                if (std.mem.eql(u8, enum_field.name, register_dump_behavior)) {
                    state.cpu.register_dump_behavior = @intToEnum(chip8.RegisterDumpBehavior, enum_field.value);
                    found = true;
                }
            }
            if (found) {
                log.debug("cpu.register_dump_behavior is '{}'", .{state.cpu.register_dump_behavior});
            } else {
                log.err("invalid value for --register-dump-mode '{s}'", .{register_dump_behavior});
                return error.InvalidCommandLine;
            }
        }

        if (args.option("--hz")) |hz_option| {
            state.tick_hz_config = std.fmt.parseInt(u64, hz_option, 0) catch |err| {
                log.err("invalid value --hz '{s}' ({}).", .{ hz_option, err });
                return err;
            };
            if (state.tick_hz_config == 0) {
                log.err("zero is not a valid value for --hz.", .{});
                return error.InvalidCommandLine;
            }
            state.tick_hz = state.tick_hz_config;
        }

        if (args.option("--seed")) |seed_str| {
            state.rng_seed = std.fmt.parseInt(u64, seed_str, 0) catch |err| {
                log.err("invalid value --seed '{s}' ({}).", .{ seed_str, err });
                return err;
            };
        }
    }

    sapp.run(.{
        .init_cb = init,
        .frame_cb = frame,
        .event_cb = input,
        .cleanup_cb = cleanup,
        .width = 20 * W,
        .height = 20 * H,
    });
}

test "basic test" {
    try std.testing.expectEqual(10, 3 + 7);
}
