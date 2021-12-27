const std = @import("std");
const mem = std.mem;
const fmt = std.fmt;
const chip8 = @import("chip8.zig");

pub fn disassemble(opcode: u16, buffer: []u8) []u8 {
    const h = @truncate(u4, opcode >> 12);
    const x = @truncate(u4, opcode >> 8);
    const y = @truncate(u4, opcode >> 4);
    const n = @truncate(u4, opcode);
    const kk = @truncate(u8, opcode);
    const nnn = @truncate(u12, opcode);

    const mask = chip8.opcode_mask_table[h];

    return switch (opcode & mask) {
        0x00E0 => fmt.bufPrint(buffer, "{x:0>4} - CLS", .{opcode}) catch unreachable,
        0x00EE => fmt.bufPrint(buffer, "{x:0>4} - RET", .{opcode}) catch unreachable,
        0x1000 => fmt.bufPrint(buffer, "{x:0>4} - JP {x}", .{ opcode, nnn }) catch unreachable,
        0x2000 => fmt.bufPrint(buffer, "{x:0>4} - CALL {x}", .{ opcode, nnn }) catch unreachable,
        0x3000 => fmt.bufPrint(buffer, "{x:0>4} - SE V{x}, {x:0>2}", .{ opcode, x, kk }) catch unreachable,
        0x4000 => fmt.bufPrint(buffer, "{x:0>4} - SNE V{x}, {x:0>2}", .{ opcode, x, kk }) catch unreachable,
        0x5000 => fmt.bufPrint(buffer, "{x:0>4} - SE V{x}, V{x}", .{ opcode, x, y }) catch unreachable,
        0x6000 => fmt.bufPrint(buffer, "{x:0>4} - LD V{x}, {x:0>2}", .{ opcode, x, kk }) catch unreachable,
        0x7000 => fmt.bufPrint(buffer, "{x:0>4} - ADD V{x}, {x:0>2}", .{ opcode, x, kk }) catch unreachable,
        0x8000 => fmt.bufPrint(buffer, "{x:0>4} - LD V{x}, V{x}", .{ opcode, x, y }) catch unreachable,
        0x8001 => fmt.bufPrint(buffer, "{x:0>4} - OR V{x}, V{x}", .{ opcode, x, y }) catch unreachable,
        0x8002 => fmt.bufPrint(buffer, "{x:0>4} - AND V{x}, V{x}", .{ opcode, x, y }) catch unreachable,
        0x8003 => fmt.bufPrint(buffer, "{x:0>4} - XOR V{x}, V{x}", .{ opcode, x, y }) catch unreachable,
        0x8004 => fmt.bufPrint(buffer, "{x:0>4} - ADD V{x}, V{x}", .{ opcode, x, y }) catch unreachable,
        0x8005 => fmt.bufPrint(buffer, "{x:0>4} - SUB V{x}, V{x}", .{ opcode, x, y }) catch unreachable,
        0x8006 => fmt.bufPrint(buffer, "{x:0>4} - SHR V{x}, V{x}", .{ opcode, x, y }) catch unreachable,
        0x8007 => fmt.bufPrint(buffer, "{x:0>4} - SUBN V{x}, V{x}", .{ opcode, x, y }) catch unreachable,
        0x800E => fmt.bufPrint(buffer, "{x:0>4} - SHL V{x}, V{x}", .{ opcode, x, y }) catch unreachable,
        0x9000 => fmt.bufPrint(buffer, "{x:0>4} - SNE V{x}, V{x}", .{ opcode, x, y }) catch unreachable,
        0xA000 => fmt.bufPrint(buffer, "{x:0>4} - LD I, {x}", .{ opcode, nnn }) catch unreachable,
        0xB000 => fmt.bufPrint(buffer, "{x:0>4} - JP V0, {x}", .{ opcode, nnn }) catch unreachable,
        0xC000 => fmt.bufPrint(buffer, "{x:0>4} - RND V{x}, {x:0>2}", .{ opcode, x, kk }) catch unreachable,
        0xD000 => fmt.bufPrint(buffer, "{x:0>4} - DRW V{x}, V{x}, {x}", .{ opcode, x, y, n }) catch unreachable,
        0xE09E => fmt.bufPrint(buffer, "{x:0>4} - SKP V{x}", .{ opcode, x }) catch unreachable,
        0xE0A1 => fmt.bufPrint(buffer, "{x:0>4} - SKNP V{x}", .{ opcode, x }) catch unreachable,
        0xF007 => fmt.bufPrint(buffer, "{x:0>4} - LD V{x}, DT", .{ opcode, x }) catch unreachable,
        0xF00A => fmt.bufPrint(buffer, "{x:0>4} - LD V{x}, K", .{ opcode, x }) catch unreachable,
        0xF015 => fmt.bufPrint(buffer, "{x:0>4} - LD DT, V{x}", .{ opcode, x }) catch unreachable,
        0xF018 => fmt.bufPrint(buffer, "{x:0>4} - LD ST, V{x}", .{ opcode, x }) catch unreachable,
        0xF01E => fmt.bufPrint(buffer, "{x:0>4} - ADD I, V{x}", .{ opcode, x }) catch unreachable,
        0xF029 => fmt.bufPrint(buffer, "{x:0>4} - LD F, V{x}", .{ opcode, x }) catch unreachable,
        0xF033 => fmt.bufPrint(buffer, "{x:0>4} - LD B, V{x}", .{ opcode, x }) catch unreachable,
        0xF055 => fmt.bufPrint(buffer, "{x:0>4} - LD [I], V{x}", .{ opcode, x }) catch unreachable,
        0xF065 => fmt.bufPrint(buffer, "{x:0>4} - LD V{x}, [I]", .{ opcode, x }) catch unreachable,
        else => fmt.bufPrint(buffer, "{x:0>4} - ???", .{opcode}) catch unreachable,
    };
}

pub fn assemble(code: []const u8) ?u16 {
    const c = code[0];
    const x = fmt.parseInt(u4, code[1], 16) catch null;
    const y = fmt.parseInt(u4, code[2], 16) catch null;
    const n = fmt.parseInt(u4, code[3], 16) catch null;
    const kk = fmt.parseInt(u8, code[2..], 16) catch null;
    const nnn = fmt.parseInt(u12, code[1..], 16) catch null;
    switch (c) {
        'A' => {
            if (mem.startsWith(u8, code, "ADD I")) return 0x00E0;
            if (mem.startsWith(u8, code, "ADD V")) {
                switch (mem.count(u8, code, "V")) {
                    1 => return 0x7000 | @as(u16, x) << 8 | @as(u16, kk),
                    2 => return 0x8004 | @as(u16, x) << 8 | @as(u16, y) << 4,
                    else => {},
                }
            }
        },
        'C' => {
            if (mem.eql(u8, code, "CLS")) return 0x00E0;
            if (mem.startsWith(u8, code, "CALL ")) return 0x2000 | @as(u16, nnn);
        },
        'D' => {
            if (mem.startsWith(u8, code, "DRW ")) return 0xD000 | @as(u16, x) << 8 | @as(u16, y) << 4 | @as(u16, n);
        },
        'J' => {
            if (mem.startsWith(u8, code, "JP V")) return 0xB000 | @as(u16, nnn);
            if (mem.startsWith(u8, code, "JP ")) return 0x1000 | @as(u16, nnn);
        },
        'L' => {
            //
            if (mem.startsWith(u8, code, "LD [I]")) return 0xF055 | @as(u16, x) << 8;
            if (mem.startsWith(u8, code, "LD I")) return 0xA000 | @as(u16, nnn);
            if (mem.startsWith(u8, code, "LD F")) return 0xF029 | @as(u16, x) << 8;
            if (mem.startsWith(u8, code, "LD B")) return 0xF033 | @as(u16, x) << 8;
            if (mem.startsWith(u8, code, "LD DT")) return 0xF015 | @as(u16, x) << 8;
            if (mem.startsWith(u8, code, "LD ST")) return 0xF018 | @as(u16, x) << 8;
            if (mem.startsWith(u8, code, "LD V")) {
                if (mem.lastIndexOf(u8, code, "[I]") != null) return 0xF065 | @as(u16, x) << 8;
                if (mem.lastIndexOf(u8, code, " DT") != null) return 0xF007 | @as(u16, x) << 8 | @as(u16, y) << 4;
                if (mem.lastIndexOf(u8, code, " K") != null) return 0xF00A | @as(u16, x) << 8 | @as(u16, y) << 4;
                switch (mem.count(u8, code, "V")) {
                    1 => return 0x6000 | @as(u16, nnn),
                    2 => return 0x8000 | @as(u16, nnn),
                    else => {},
                }
            }
        },
        'O' => {
            if (mem.startsWith(u8, code, "OR ")) return 0x8001 | @as(u16, x) << 8 | @as(u16, y) << 4;
        },
        'R' => {
            if (mem.eql(u8, code, "RET")) return 0x00EE;
            if (mem.startsWith(u8, code, "RND")) return 0xC000 | @as(u16, x) << 8 | @as(u16, kk);
        },
        'S' => {
            if (mem.startsWith(u8, code, "SUB ")) return 0x8005 | @as(u16, x) << 8 | @as(u16, y) << 4;
            if (mem.startsWith(u8, code, "SHR ")) return 0x8006 | @as(u16, x) << 8 | @as(u16, y) << 4;
            if (mem.startsWith(u8, code, "SUBN ")) return 0x8007 | @as(u16, x) << 8 | @as(u16, y) << 4;
            if (mem.startsWith(u8, code, "SHL ")) return 0x800E | @as(u16, x) << 8 | @as(u16, y) << 4;
            if (mem.startsWith(u8, code, "SKP ")) return 0xE09E | @as(u16, x) << 8;
            if (mem.startsWith(u8, code, "SKNP ")) return 0xE0A1 | @as(u16, x) << 8;
            if (mem.startsWith(u8, code, "SE ")) {
                switch (mem.count(u8, code, "V")) {
                    1 => return 0x3000 | @as(u16, nnn),
                    2 => return 0x5000 | @as(u16, nnn),
                    else => {},
                }
            }
            if (mem.startsWith(u8, code, "SNE ")) {
                switch (mem.count(u8, code, "V")) {
                    1 => return 0x4000 | @as(u16, nnn),
                    2 => return 0x9000 | @as(u16, nnn),
                    else => {},
                }
            }
        },
        'X' => {
            if (mem.startsWith(u8, code, "XOR ")) return 0x8001 | @as(u16, x) << 8 | @as(u16, y) << 4;
        },
        else => {},
    }
    return null;
}
