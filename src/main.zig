const std = @import("std");
const log = std.log;

const sg = @import("sokol").gfx;
const sapp = @import("sokol").app;
const sgapp = @import("sokol").app_gfx_glue;
const stm = @import("sokol").time;
const saudio = @import("sokol").audio;
const sdtx = @import("sokol").debugtext;

const shader = @import("shader.zig");
const chip8 = @import("chip8.zig");

pub fn newFrame() void {
    var state = &global_state;
    state.last_frame_print_len = 0;
}

pub fn framePrint(comptime fmt: []const u8, args: anytype) void {
    var state = &global_state;
    state.last_frame_print_len = (std.fmt.bufPrint(state.last_frame_print_buf[state.last_frame_print_len..], fmt, args) catch unreachable).len;
}

const W = 64;
const H = 32;

var global_state: struct {
    is_single_stepping: bool = true,
    frame_counter: u64 = 0,
    laptime_store: u64 = 0,

    // #TODO random seed?
    rng: std.rand.DefaultPrng = std.rand.DefaultPrng.init(0),

    tick_counter: u32 = 0,
    cpu: chip8.Cpu = .{},
    memory_buf: [4096]u8 = [_]u8{0} ** 4096,
    display_buf: [H * W]u1 = [_]u1{0} ** (H * W),

    last_frame_print_buf: [4096]u8 = undefined,
    last_frame_print_len: usize = 0,

    gfx: struct {
        bindings: sg.Bindings = .{},
        pipeline: sg.Pipeline = .{},
        pass_action: sg.PassAction = .{},
    } = .{},
} = .{};

// sokol-app callbacks {

export fn init() void {
    var state = &global_state;
    _ = state;

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

    const tex_pixels = [4 * 4]u8{
        1, 0, 1, 0,
        0, 1, 0, 1,
        1, 0, 1, 0,
        0, 1, 0, 1,
    };
    var tex_desc = sg.ImageDesc{
        .width = 4,
        .height = 4,
        .pixel_format = .R8,
    };
    tex_desc.data.subimage[0][0] = sg.asRange(tex_pixels);
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

    std.mem.writeIntSliceBig(u16, state.memory_buf[0..2], 0x1200);

    framePrint("press ENTER to run, press SPACE to single-step\n", .{});
}

export fn cleanup() void {
    var state = &global_state;
    _ = state;

    sdtx.shutdown();
    saudio.shutdown();
    sg.shutdown();
}

export fn frame() void {
    var state = &global_state;
    state.frame_counter += 1;

    // run at a fixed tick rate regardless of frame rate
    var dt = stm.sec(stm.laptime(&state.laptime_store));
    _ = dt;

    // tickChip8();

    const canvas_width = sapp.widthf();
    const canvas_height = sapp.heightf();

    sdtx.canvas(canvas_width, canvas_height);
    sdtx.origin(3, 3);
    sdtx.font(0);

    if (!state.is_single_stepping) {
        tickChip8();
    }

    sdtx.print("{s}", .{state.last_frame_print_buf[0..state.last_frame_print_len]});

    sg.beginDefaultPassf(state.gfx.pass_action, canvas_width, canvas_height);
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
        const key_pressed = event.type == .KEY_DOWN;
        const key_repeat = event.key_repeat;
        const key_code = event.key_code;
        if (key_pressed and !key_repeat) {
            switch (key_code) {
                .ESCAPE => {
                    log.debug("exiting - ESCAPE was pressed", .{});
                    sapp.quit();
                },
                .ENTER => {
                    if (state.is_single_stepping) {
                        log.debug("exit single stepping mode", .{});
                        state.is_single_stepping = false;
                    }
                },
                .SPACE => {
                    if (state.is_single_stepping) {
                        log.debug("single step", .{});
                        tickChip8();
                    }
                },
                else => {},
            }
        }
    }
}

fn tickChip8() void {
    var state = &global_state;

    newFrame();

    framePrint("Tick #{}\n", .{state.tick_counter});

    var memory = @as([]u8, &state.memory_buf);
    var display = chip8.Display{
        .width = W,
        .height = H,
        .data = @as([]u1, &state.display_buf),
    };
    state.cpu.tick(memory, display, state.rng.random());

    state.tick_counter += 1;
}

// }

pub fn main() anyerror!void {
    sapp.run(.{
        .init_cb = init,
        .frame_cb = frame,
        .event_cb = input,
        .cleanup_cb = cleanup,
        .width = 640,
        .height = 480,
        .window_title = "couscous CHIP-8 in zig powered by sokol",
    });
}

test "basic test" {
    try std.testing.expectEqual(10, 3 + 7);
}
