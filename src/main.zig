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

const max_frame_time = 1.0 / 30.0;
// #TODO What frequency do we want to run the interpreter at?
// const tick_duration = 1.0 / 500.0;
// const tick_duration = 1.0 / 60.0;
const tick_duration = 1.0 / 120.0;
const tick_epsilon = 0.001;
const cpu_timer_interval = 1.0 / 60.0;
const history_len = 16;

const W = 64;
const H = 32;

var global_state: struct {
    allocator: std.mem.Allocator = undefined,

    initialized: bool = false,
    debug: bool = false,
    display_test_pattern: bool = false,
    rom_path: ?[]const u8 = null,

    pause: bool = false,
    remaining_steps: u64 = std.math.maxInt(u64),
    frame_counter: u64 = 0,
    // laptime_store: u64 = 0,
    frame_timer: std.time.Timer = undefined,

    // #TODO random seed?
    rng: std.rand.DefaultPrng = std.rand.DefaultPrng.init(0),

    tick_counter: u64 = 0,
    tick_time_remaining: f64 = 0.0,
    cpu_timer_remaining: f64 = 0.0,
    cpu: chip8.Cpu = .{},
    memory_buf: [4096]u8 = [_]u8{0} ** 4096,
    display_buf: [H * W]u1 = [_]u1{0} ** (H * W),
    keyboard: chip8.Keyboard = .{},
    keyboard_layout: u8 = 0,

    exec_counter: u64 = 0,
    pc_history: [history_len]u16 = [_]u16{0} ** history_len,
    opcode_history: [history_len]u16 = [_]u16{0} ** history_len,

    gfx: struct {
        bindings: sg.Bindings = .{},
        pipeline: sg.Pipeline = .{},
        pass_action: sg.PassAction = .{},
    } = .{},

    fn setPause(state: *@This(), pause: bool) void {
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
            state.remaining_steps = std.math.maxInt(u64);
        }
    }

    fn nextOpcode(state: *@This()) void {
        var cpu = &state.cpu;
        var memory = &state.memory_buf;

        cpu.opcode = std.mem.readIntSliceBig(u16, memory[cpu.pc..]);
        if (!state.looping()) {
            state.exec_counter +%= 1;
            state.pc_history[state.exec_counter & (history_len - 1)] = cpu.pc;
            state.opcode_history[state.exec_counter & (history_len - 1)] = cpu.opcode;
        }
    }

    fn looping(state: @This()) bool {
        const cpu = &state.cpu;
        const index = (state.exec_counter -| 1);
        const last_pc = state.pc_history[index & (history_len - 1)];
        const last_opcode = state.opcode_history[index & (history_len - 1)];
        return last_pc == cpu.pc and last_opcode == cpu.opcode and cpu.step == 0;
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
    _ = state;

    state.frame_timer = std.time.Timer.start() catch |err| {
        log.err("{} - Unable to start timer", .{err});
        return;
    };

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

    initializeCpu(&state.cpu);

    if (state.display_test_pattern) {
        // Initialize display with test pattern.
        loadDisplayTestPattern(&state.display_buf, W);
    } else {
        // Clear screen.
        std.mem.set(u1, &state.display_buf, 0);
    }

    loadCharmap(state.memory_buf[chip8.charmap_base_address..]);

    if (state.rom_path) |rom_path| {
        const bytes_read = loadRomFromFile(state.memory_buf[chip8.user_base_address..], rom_path) catch {
            return;
        };
        log.debug("loaded ROM with {} bytes: '{s}'", .{ bytes_read, rom_path });
        _ = state.nextOpcode();
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

    var cpu = &state.cpu;
    var memory = @as([]u8, &state.memory_buf);
    var display = chip8.Display{
        .width = W,
        .height = H,
        .data = @as([]u1, &state.display_buf),
    };
    var keyboard = &state.keyboard;

    state.frame_counter += 1;

    // run at a fixed tick rate regardless of frame rate
    // #NOTE clamp frame time so debug breakpoints don't screw up timing.
    const delta_time = std.math.min(@intToFloat(f64, state.frame_timer.lap()) * std.time.ns_per_s, max_frame_time);

    const canvas_width = sapp.widthf();
    const canvas_height = sapp.heightf();
    const render_scale: f32 = 1.0;
    const viewport = aspectRatioFit(canvas_width, canvas_height, render_scale * W, render_scale * H).?;
    const render_width = viewport.max_x - viewport.min_x;
    const render_height = viewport.max_y - viewport.min_y;

    if (state.debug) {
        sdtx.setContext(sdtx.defaultContext());
        const debug_font_scale = 1.0 / 2.0;
        sdtx.canvas(debug_font_scale * render_width, debug_font_scale * render_height);
        sdtx.origin(3, 3);
        sdtx.font(0);
    } else {
        sdtx.setContext(.{});
    }

    // #TODO make use of state.debug
    sdtx.print("tick #{}\n", .{state.tick_counter});
    if (state.pause) {
        sdtx.print("=== PAUSED ===\n", .{});
        sdtx.print("press SPACE to step {} times\n", .{state.remaining_steps});
        sdtx.print("press 1|2|3|4 to increase step by 1|10|100|{}\n", .{(H * W) / 8});
        sdtx.print("    hold SHIFT to decrement, hold ALT to set\n", .{});
        sdtx.print("press ENTER to unpause\n", .{});
        sdtx.print("\n", .{});
        sdtx.print("CPU pc={x:0>4} i={x:0>4} sp={x:0>4} dt={x:0>2} st={x:0>2}\n", .{ cpu.pc, cpu.i, cpu.sp, cpu.dt, cpu.st });
        sdtx.print("       0 1 2 3 4 5 6 7 8 9 A B C D E F\n", .{});
        sdtx.print("    v={x}\n", .{std.fmt.fmtSliceHexLower(&cpu.v)});
        // #TODO Don't need to print the opcode here anymore because we have the history now.
        sdtx.print("    opcode={x:0>4} step={}\n", .{ cpu.opcode, cpu.step });
    }
    if (keyboard.block) {
        sdtx.print("!!! waiting for input\n", .{});
    }

    if (!state.pause and state.remaining_steps > 0) {
        state.tick_time_remaining += delta_time;

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
            cpu.tick(memory, display, keyboard, state.rng.random());
            if (cpu.step == 0) {
                state.nextOpcode();
            }

            state.tick_counter +%= 1;

            state.remaining_steps -= 1;
            if (state.remaining_steps == 0) {
                state.setPause(true);
                break;
            }
        }
    }

    if (state.looping()) {
        sdtx.print("loop\n", .{});
    }

    {
        sdtx.print("\n#Exec PC   OPCODE", .{});
        var offset: usize = 0;
        while (offset < history_len) : (offset += 1) {
            const exec_index = state.exec_counter -| offset;
            const index = exec_index & (history_len - 1);
            var buf0: [64]u8 = undefined;
            var buf1: [64]u8 = undefined;
            const opcode = state.opcode_history[index];
            const opcode_str = chip8_asm.opcodePrint(&buf1, opcode);
            const opcode_desc = chip8_asm.disassemble(&buf0, opcode) orelse "???";
            sdtx.print("\n{:>5} {x:0>4} {s} - {s}", .{ exec_index, state.pc_history[index], opcode_str, opcode_desc });
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

        switch (key_code) { // Handle host input first, regardless of state.
            .ESCAPE => {
                if (key_pressed and !key_repeat) {
                    log.debug("exiting - ESCAPE was pressed", .{});
                    sapp.quit();
                }
            },
            .ENTER => {
                if (key_pressed and !key_repeat) {
                    state.setPause(false);
                }
            },
            .SPACE => {
                if (key_pressed) {
                    if (state.pause) {
                        log.debug("single step", .{});
                        state.pause = false;
                    } else if (!key_repeat and !state.pause) {
                        state.setPause(true);
                    }
                }
            },
            else => {},
        }

        // Handle pause input.
        if (state.pause) {
            var step_input: u64 = 0;
            if (key_pressed) {
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
                    else => {},
                }
            }
            if (step_input != 0) {
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
        }

        // Handle regular CHIP-8 input.
        if (!state.pause) {
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
        }
    }
}

// }

fn initializeCpu(cpu: *chip8.Cpu) void {
    cpu.setPc(chip8.user_base_address);
    cpu.sp = chip8.stack_base_address;
}

fn loadCharmap(dest: []u8) void {
    std.mem.copy(u8, dest, &chip8_charmap.charmap);
}

fn loadRomFromFile(dest: []u8, path: []const u8) !usize {
    const file = std.fs.cwd().openFile(path, .{}) catch |err| {
        log.err("{}: Failed to open ROM file '{s}'", .{ err, path });
        return error.RomLoad;
    };
    defer file.close();
    const bytes_read = file.readAll(dest) catch |err| {
        log.err("{}: Failed to read ROM file '{s}'", .{ err, path });
        return error.RomLoad;
    };
    return bytes_read;
}

fn loadDisplayTestPattern(display_data: []u1, stride: usize) void {
    var index: usize = 0;
    while (index < (H * W)) : (index += 1) {
        display_data[index] = @truncate(u1, (index & 1) ^ ((index / stride) & 1));
    }
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
            clap.parseParam("-h, --help    Display this help and exit") catch unreachable,
            clap.parseParam("-p, --pause   Start in pause mode") catch unreachable,
            clap.parseParam("--nocls       Do not start with a clear display.") catch unreachable,
            clap.parseParam("--debug       Show debug information") catch unreachable,
            clap.parseParam("--dis <OUT>   Disassemble the input ROM and write to <OUT>. Use - to print to stdout.") catch unreachable,
            clap.parseParam("--asm <OUT>   Assemble the input file and write to <OUT>. Use - to print to stdout.") catch unreachable,
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
            state.setPause(true);
        }

        state.debug = args.flag("--debug");
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
            if (env.get("COUSCOUS_DEBUG")) |val| {
                if (std.mem.eql(u8, val, "1")) {
                    state.debug = true;
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
            const rom_len = try loadRomFromFile(&rom_buf, rom_path);
            const rom = rom_buf[0..rom_len];

            var out_file = std.fs.cwd().createFile(out_path, .{}) catch |err| {
                log.err("failed to open output file for writing '{s}'", .{out_path});
                return err;
            };
            defer out_file.close();
            var index: usize = 0;
            while (index < rom.len) : (index += 2) {
                const opcode = std.mem.readIntSliceBig(u16, rom[index..]);
                const addr = chip8.user_base_address + index;

                var line_buf: [128]u8 = undefined;
                var line_index: usize = 0;
                line_index += (try std.fmt.bufPrint(line_buf[line_index..], "{X:0>4}  ", .{addr})).len;

                if (chip8_asm.disassemble(line_buf[line_index..], opcode)) |dis| {
                    line_index += dis.len;

                    const min_width = 20;
                    const pad = min_width -| line_index;
                    if (pad > 0) {
                        std.mem.set(u8, line_buf[line_index .. line_index + pad], ' ');
                        line_index += pad;
                    }
                    line_index += (try std.fmt.bufPrint(line_buf[line_index..], " # ", .{})).len;
                    line_index += (try chip8_asm.opcodePrint(line_buf[line_index..], opcode)).len;
                } else {
                    line_index += (try chip8_asm.opcodePrint(line_buf[line_index..], opcode)).len;
                }
                line_index += (try std.fmt.bufPrint(line_buf[line_index..], "\n", .{})).len;
                try out_file.writeAll(line_buf[0..line_index]);
            }
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

            var line_iter = std.mem.tokenize(u8, code, "\r\n");
            while (line_iter.next()) |line| {
                var opstr = if (std.mem.indexOf(u8, std.mem.trimRight(u8, line, " "), " ") orelse 0 == 4) std.mem.trimLeft(u8, line[4..], " ") else line;
                if (chip8_asm.assemble(opstr)) |opcode| {
                    var opcode_buf: [2]u8 = undefined;
                    std.mem.writeIntSliceBig(u16, &opcode_buf, opcode);
                    log.debug("line='{s}' parsed={X:0>4} write={X:0>4}", .{ line, opcode, std.fmt.fmtSliceHexUpper(&opcode_buf) });
                    try out_file.writeAll(&opcode_buf);
                } else {
                    std.debug.assert(opstr.len == 4);
                    try out_file.writeAll(opstr);
                }
            }
            return;
        }

        if (args.option("--asm")) |out_path| {
            _ = out_path;
            unreachable; // not implemented
        }
    }

    sapp.run(.{
        .init_cb = init,
        .frame_cb = frame,
        .event_cb = input,
        .cleanup_cb = cleanup,
        .width = 20 * W,
        .height = 20 * H,
        .window_title = "couscous // CHIP-8 interpreter // written in zig // powered by sokol",
    });
}

test "basic test" {
    try std.testing.expectEqual(10, 3 + 7);
}
