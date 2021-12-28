const std = @import("std");
const mem = std.mem;
const fmt = std.fmt;
const chip8 = @import("chip8.zig");

pub fn opcodePrint(buffer: []u8, opcode: u16) ![]u8 {
    return fmt.bufPrint(buffer, "{X:0>4}", .{opcode});
}

pub fn disassemble(buffer: []u8, opcode: u16) ?[]u8 {
    const h = @truncate(u4, opcode >> 12);
    const x = @truncate(u4, opcode >> 8);
    const y = @truncate(u4, opcode >> 4);
    const n = @truncate(u4, opcode);
    const kk = @truncate(u8, opcode);
    const nnn = @truncate(u12, opcode);

    const mask = chip8.opcode_mask_table[h];

    return switch (opcode & mask) {
        0x00E0 => fmt.bufPrint(buffer, "CLS", .{}) catch unreachable,
        0x00EE => fmt.bufPrint(buffer, "RET", .{}) catch unreachable,
        0x1000 => fmt.bufPrint(buffer, "JP {X}", .{nnn}) catch unreachable,
        0x2000 => fmt.bufPrint(buffer, "CALL {X}", .{nnn}) catch unreachable,
        0x3000 => fmt.bufPrint(buffer, "SE V{X}, {X:0>2}", .{ x, kk }) catch unreachable,
        0x4000 => fmt.bufPrint(buffer, "SNE V{X}, {X:0>2}", .{ x, kk }) catch unreachable,
        0x5000 => fmt.bufPrint(buffer, "SE V{X}, V{X}", .{ x, y }) catch unreachable,
        0x6000 => fmt.bufPrint(buffer, "LD V{X}, {X:0>2}", .{ x, kk }) catch unreachable,
        0x7000 => fmt.bufPrint(buffer, "ADD V{X}, {X:0>2}", .{ x, kk }) catch unreachable,
        0x8000 => fmt.bufPrint(buffer, "LD V{X}, V{X}", .{ x, y }) catch unreachable,
        0x8001 => fmt.bufPrint(buffer, "OR V{X}, V{X}", .{ x, y }) catch unreachable,
        0x8002 => fmt.bufPrint(buffer, "AND V{X}, V{X}", .{ x, y }) catch unreachable,
        0x8003 => fmt.bufPrint(buffer, "XOR V{X}, V{X}", .{ x, y }) catch unreachable,
        0x8004 => fmt.bufPrint(buffer, "ADD V{X}, V{X}", .{ x, y }) catch unreachable,
        0x8005 => fmt.bufPrint(buffer, "SUB V{X}, V{X}", .{ x, y }) catch unreachable,
        0x8006 => fmt.bufPrint(buffer, "SHR V{X}, V{X}", .{ x, y }) catch unreachable,
        0x8007 => fmt.bufPrint(buffer, "SUBN V{X}, V{X}", .{ x, y }) catch unreachable,
        0x800E => fmt.bufPrint(buffer, "SHL V{X}, V{X}", .{ x, y }) catch unreachable,
        0x9000 => fmt.bufPrint(buffer, "SNE V{X}, V{X}", .{ x, y }) catch unreachable,
        0xA000 => fmt.bufPrint(buffer, "LD I, {X}", .{nnn}) catch unreachable,
        0xB000 => fmt.bufPrint(buffer, "JP V0, {X}", .{nnn}) catch unreachable,
        0xC000 => fmt.bufPrint(buffer, "RND V{X}, {X:0>2}", .{ x, kk }) catch unreachable,
        0xD000 => fmt.bufPrint(buffer, "DRW V{X}, V{X}, {X}", .{ x, y, n }) catch unreachable,
        0xE09E => fmt.bufPrint(buffer, "SKP V{X}", .{x}) catch unreachable,
        0xE0A1 => fmt.bufPrint(buffer, "SKNP V{X}", .{x}) catch unreachable,
        0xF007 => fmt.bufPrint(buffer, "LD V{X}, DT", .{x}) catch unreachable,
        0xF00A => fmt.bufPrint(buffer, "LD V{X}, K", .{x}) catch unreachable,
        0xF015 => fmt.bufPrint(buffer, "LD DT, V{X}", .{x}) catch unreachable,
        0xF018 => fmt.bufPrint(buffer, "LD ST, V{X}", .{x}) catch unreachable,
        0xF01E => fmt.bufPrint(buffer, "ADD I, V{X}", .{x}) catch unreachable,
        0xF029 => fmt.bufPrint(buffer, "LD F, V{X}", .{x}) catch unreachable,
        0xF033 => fmt.bufPrint(buffer, "LD B, V{X}", .{x}) catch unreachable,
        0xF055 => fmt.bufPrint(buffer, "LD [I], V{X}", .{x}) catch unreachable,
        0xF065 => fmt.bufPrint(buffer, "LD V{X}, [I]", .{x}) catch unreachable,
        else => null,
    };
}

pub fn assemble(code: []const u8) ?u16 {
    var tokens = mem.tokenize(u8, code, " ,");
    const opname = tokens.next() orelse "";

    const ArgType = enum(u8) {
        reg, // V0, ..., VF
        imm, // x, y, n, kk, nnn
        i, // I, [I]
        dt, // DT
        st, // ST
        f, // F
        b, // B
        k, // K
    };
    const Args = struct {
        types: [3]ArgType = [_]ArgType{undefined} ** 3,
        values: [3]u16 = [_]u16{undefined} ** 3,
        len: usize = 0,

        // #TODO Return an error instead of asserting?
        // #NOTE Return u16 so we don't have to cast at the call-site.
        fn imm(self: @This(), comptime TCheck: type, index: usize) u16 {
            std.debug.assert(index < self.len);
            std.debug.assert(self.types[index] == .imm);
            return @as(u16, @intCast(TCheck, self.values[index]));
        }

        // #TODO Return an error instead of asserting?
        // #NOTE Return u16 so we don't have to cast at the call-site.
        fn reg(self: @This(), index: usize) u16 {
            std.debug.assert(index < self.len);
            std.debug.assert(self.types[index] == .reg);
            return @as(u16, @intCast(u4, self.values[index]));
        }
    };

    var args = Args{};
    while (tokens.next()) |token| {
        if (mem.startsWith(u8, token, "#")) {
            break;
        }

        var parsed_type: ?ArgType = null;
        var imm_parse_offset: ?usize = null;
        if (token[0] == 'V') {
            parsed_type = .reg;
            imm_parse_offset = 1;
        } else if (mem.eql(u8, token, "I") or mem.eql(u8, token, "[I]")) {
            parsed_type = .i;
        } else if (mem.eql(u8, token, "DT")) {
            parsed_type = .dt;
        } else if (mem.eql(u8, token, "ST")) {
            parsed_type = .st;
        } else if (mem.eql(u8, token, "F")) {
            parsed_type = .f;
        } else if (mem.eql(u8, token, "B")) {
            parsed_type = .b;
        } else if (mem.eql(u8, token, "K")) {
            parsed_type = .k;
        } else {
            parsed_type = .imm;
            imm_parse_offset = 0;
        }

        if (imm_parse_offset) |parse_offset| blk: {
            args.values[args.len] = fmt.parseInt(u12, token[parse_offset..], 16) catch {
                parsed_type = null;
                break :blk;
            };
        }

        if (parsed_type) |arg_type| {
            args.types[args.len] = arg_type;
            args.len += 1;
        } else {
            // #NOTE We hit an unidentifiable token.
            break;
        }
    }

    const util = struct {
        fn op(name: []const u8, arg0: ?ArgType, arg1: ?ArgType, arg2: ?ArgType) u64 {
            var buf = [_]ArgType{undefined} ** 3;
            var buf_len: usize = 0;
            if (arg0) |a0| {
                buf[buf_len] = a0;
                buf_len += 1;
                if (arg1) |a1| {
                    buf[buf_len] = a1;
                    buf_len += 1;
                    if (arg2) |a2| {
                        buf[buf_len] = a2;
                        buf_len += 1;
                    }
                }
            }
            return ophash(name, buf[0..buf_len]);
        }

        fn ophash(name: []const u8, arg_types: []const ArgType) u64 {
            var bytes = [_]u8{0} ** 8;
            mem.copy(u8, bytes[0..5], name);
            for (arg_types) |arg, i| {
                bytes[bytes.len - 3 + i] = @enumToInt(arg) + 1;
            }
            return mem.readIntBig(u64, &bytes);
        }
    };

    var ophash = util.ophash(opname, args.types[0..args.len]);

    return switch (ophash) {
        util.op("CLS", null, null, null) => 0x00E0,
        util.op("RET", null, null, null) => 0x00EE,
        util.op("JP", .imm, null, null) => 0x1000 | args.imm(u12, 0),
        util.op("CALL", .imm, null, null) => 0x2000 | args.imm(u12, 0),
        util.op("SE", .reg, .imm, null) => 0x3000 | args.reg(0) << 8 | args.imm(u8, 1),
        util.op("SNE", .reg, .imm, null) => 0x4000 | args.reg(0) << 8 | args.imm(u8, 1),
        util.op("SE", .reg, .reg, null) => 0x5000 | args.reg(0) << 8 | args.reg(1) << 4,
        util.op("LD", .reg, .imm, null) => 0x6000 | args.reg(0) << 8 | args.imm(u8, 1),
        util.op("ADD", .reg, .imm, null) => 0x7000 | args.reg(0) << 8 | args.imm(u8, 1),
        util.op("LD", .reg, .reg, null) => 0x8000 | args.reg(0) << 8 | args.reg(1) << 4,
        util.op("OR", .reg, .reg, null) => 0x8001 | args.reg(0) << 8 | args.reg(1) << 4,
        util.op("AND", .reg, .reg, null) => 0x8002 | args.reg(0) << 8 | args.reg(1) << 4,
        util.op("XOR", .reg, .reg, null) => 0x8003 | args.reg(0) << 8 | args.reg(1) << 4,
        util.op("ADD", .reg, .reg, null) => 0x8004 | args.reg(0) << 8 | args.reg(1) << 4,
        util.op("SUB", .reg, .reg, null) => 0x8005 | args.reg(0) << 8 | args.reg(1) << 4,
        util.op("SHR", .reg, .reg, null) => 0x8006 | args.reg(0) << 8 | args.reg(1) << 4,
        util.op("SUBN", .reg, .reg, null) => 0x8007 | args.reg(0) << 8 | args.reg(1) << 4,
        util.op("SHL", .reg, .reg, null) => 0x800E | args.reg(0) << 8 | args.reg(1) << 4,
        util.op("SNE", .reg, .reg, null) => 0x9000 | args.reg(0) << 8 | args.reg(1) << 4,
        util.op("LD", .i, .imm, null) => 0xA000 | args.imm(u12, 1),
        util.op("JP", .reg, .imm, null) => 0xB000 | args.imm(u12, 1),
        util.op("RND", .reg, .imm, null) => 0xC000 | args.reg(0) << 8 | args.imm(u8, 1),
        util.op("DRW", .reg, .reg, .imm) => 0xD000 | args.reg(0) << 8 | args.reg(1) << 4 | args.imm(u4, 2),
        util.op("SKP", .reg, null, null) => 0xE09E | args.reg(0) << 8,
        util.op("SKNP", .reg, null, null) => 0xE0A1 | args.reg(0) << 8,
        util.op("LD", .reg, .dt, null) => 0xF007 | args.reg(0) << 8,
        util.op("LD", .reg, .k, null) => 0xF00A | args.reg(0) << 8,
        util.op("LD", .dt, .reg, null) => 0xF015 | args.reg(1) << 8,
        util.op("LD", .st, .reg, null) => 0xF018 | args.reg(1) << 8,
        util.op("ADD", .i, .reg, null) => 0xF01E | args.reg(1) << 8,
        util.op("LD", .f, .reg, null) => 0xF029 | args.reg(1) << 8,
        util.op("LD", .b, .reg, null) => 0xF033 | args.reg(1) << 8,
        util.op("LD", .i, .reg, null) => 0xF055 | args.reg(1) << 8,
        util.op("LD", .reg, .i, null) => 0xF065 | args.reg(0) << 8,
        else => null,
    } orelse {
        return fmt.parseInt(u16, opname, 16) catch null;
    };
}
