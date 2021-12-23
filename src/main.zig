const std = @import("std");
const log = std.log;

const sg = @import("sokol").gfx;
const sapp = @import("sokol").app;
const sgapp = @import("sokol").app_gfx_glue;
const stm = @import("sokol").time;
const saudio = @import("sokol").audio;

const shader = @import("shader.zig");

var gstate: struct {
    laptime_store: u64 = 0,

    gfx: struct {
        bindings: sg.Bindings = .{},
        pipeline: sg.Pipeline = .{},
        pass_action: sg.PassAction = .{},
    } = .{},
} = .{};

// sokol-app callbacks {

export fn frame() void {
    // run the game at a fixed tick rate regardless of frame rate
    var frame_time_ns = stm.ns(stm.laptime(&gstate.laptime_store));
    _ = frame_time_ns;

    const canvas_width = sapp.widthf();
    const canvas_height = sapp.heightf();
    sg.beginDefaultPassf(gstate.gfx.pass_action, canvas_width, canvas_height);
    sg.applyPipeline(gstate.gfx.pipeline);
    sg.applyBindings(gstate.gfx.bindings);
    sg.draw(0, 6, 1);
    sg.endPass();
    sg.commit();
}

export fn input(ev: ?*const sapp.Event) void {
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
                else => {},
            }
        }
    }
}

export fn init() void {
    stm.setup();

    sg.setup(.{
        .buffer_pool_size = 2,
        .image_pool_size = 3,
        .shader_pool_size = 2,
        .pipeline_pool_size = 2,
        .pass_pool_size = 1,
        .context = sgapp.context(),
    });

    const quad_verts = [_]f32{
        -1.0, -1.0,
        1.0,  -1.0,
        1.0,  1.0,
        -1.0, 1.0,
    };
    gstate.gfx.bindings.vertex_buffers[0] = sg.makeBuffer(.{
        .data = sg.asRange(quad_verts),
    });

    const quad_indices = [_]u16{
        0, 1, 2,
        0, 2, 3,
    };
    gstate.gfx.bindings.index_buffer = sg.makeBuffer(.{
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
    gstate.gfx.bindings.fs_images[shader.SLOT_tex] = sg.makeImage(tex_desc);

    var pip_desc = sg.PipelineDesc{
        .index_type = .UINT16,
        .shader = sg.makeShader(shader.quadShaderDesc(sg.queryBackend())),
    };

    pip_desc.layout.attrs[shader.ATTR_vs_position].format = .FLOAT2;
    gstate.gfx.pipeline = sg.makePipeline(pip_desc);

    gstate.gfx.pass_action.colors[0] = .{
        .action = .CLEAR,
        .value = .{ .r = 0.25, .g = 0.1, .b = 0.3, .a = 1 },
    };

    saudio.setup(.{});
}

export fn cleanup() void {
    saudio.shutdown();
    sg.shutdown();
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
