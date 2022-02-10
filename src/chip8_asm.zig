const mem = @import("std").mem;
const fmt = @import("std").fmt;
const io = @import("std").io;
const assert = @import("std").debug.assert;
const chip8 = @import("chip8.zig");

const Allocator = @import("std").mem.Allocator;
const ArenaAllocator = @import("std").heap.ArenaAllocator;
const FixedBufferAllocator = @import("std").heap.FixedBufferAllocator;
const ArrayList = @import("std").ArrayList;
const StringHashMap = @import("std").StringHashMap;

pub fn opcodePrint(buffer: []u8, opcode: u16) ![]u8 {
    return fmt.bufPrint(buffer, "{X:0>4}", .{opcode});
}

pub const DisassembleOptions = struct {
    print_address: bool = true,
    print_hex: bool = true,
    print_bin: bool = true,
    newline: []const u8 = "\n",
};

pub fn disassemble(input: []const u8, base_address: usize, writer: anytype, options: DisassembleOptions) !void {
    // #TODO Analyze each opcode and determine which bytes are part of an opcode and which are just data. We need this because some ROMs freely mix data and code.
    var index: usize = 0;
    while (index < input.len) {
        if (options.print_address) {
            const addr = base_address + index;
            try fmt.format(writer, "{X:0>4} ", .{addr});
        }
        var opcode_printed = false;
        if (index + 1 < input.len) {
            var opcode_buf: [128]u8 = undefined;
            var opcode_stream = io.fixedBufferStream(&opcode_buf);
            var opcode_writer = opcode_stream.writer();
            const opcode = mem.readIntSliceBig(u16, input[index .. index + 2]);
            if (try disassembleOpcode(opcode, opcode_writer)) {
                if (options.print_hex) {
                    try fmt.format(writer, "{X:0>4} ", .{opcode});
                }
                if (options.print_bin) {
                    try fmt.format(writer, "{b:0>16} ", .{opcode});
                }
                try writer.writeAll(opcode_stream.getWritten());
                try fmt.format(writer, "{s}", .{options.newline});

                opcode_printed = true;
                index += 2;
            }
        }
        if (!opcode_printed) {
            const byte = input[index];
            if (options.print_hex) {
                try fmt.format(writer, "  {X:0>2} ", .{byte});
            }
            if (options.print_bin) {
                try fmt.format(writer, "        {b:0>8} ", .{byte});
            }
            try fmt.format(writer, "{s}", .{options.newline});
            index += 1;
        }
    }
}

pub fn disassembleOpcode(opcode: u16, writer: anytype) !bool {
    const h = @truncate(u4, opcode >> 12);
    const x = @truncate(u4, opcode >> 8);
    const y = @truncate(u4, opcode >> 4);
    const n = @truncate(u4, opcode);
    const kk = @truncate(u8, opcode);
    const nnn = @truncate(u12, opcode);
    const mask = chip8.opcode_mask_table[h];
    switch (opcode & mask) {
        0x00E0 => try fmt.format(writer, "CLS", .{}),
        0x00EE => try fmt.format(writer, "RET", .{}),
        0x1000 => try fmt.format(writer, "JP {X}", .{nnn}),
        0x2000 => try fmt.format(writer, "CALL {X}", .{nnn}),
        0x3000 => try fmt.format(writer, "SE V{X} {X:0>2}", .{ x, kk }),
        0x4000 => try fmt.format(writer, "SNE V{X} {X:0>2}", .{ x, kk }),
        0x5000 => try fmt.format(writer, "SE V{X} V{X}", .{ x, y }),
        0x6000 => try fmt.format(writer, "LD V{X} {X:0>2}", .{ x, kk }),
        0x7000 => try fmt.format(writer, "ADD V{X} {X:0>2}", .{ x, kk }),
        0x8000 => try fmt.format(writer, "LD V{X} V{X}", .{ x, y }),
        0x8001 => try fmt.format(writer, "OR V{X} V{X}", .{ x, y }),
        0x8002 => try fmt.format(writer, "AND V{X} V{X}", .{ x, y }),
        0x8003 => try fmt.format(writer, "XOR V{X} V{X}", .{ x, y }),
        0x8004 => try fmt.format(writer, "ADD V{X} V{X}", .{ x, y }),
        0x8005 => try fmt.format(writer, "SUB V{X} V{X}", .{ x, y }),
        0x8006 => try fmt.format(writer, "SHR V{X} V{X}", .{ x, y }),
        0x8007 => try fmt.format(writer, "SUBN V{X} V{X}", .{ x, y }),
        0x800E => try fmt.format(writer, "SHL V{X} V{X}", .{ x, y }),
        0x9000 => try fmt.format(writer, "SNE V{X} V{X}", .{ x, y }),
        0xA000 => try fmt.format(writer, "LD I {X}", .{nnn}),
        0xB000 => try fmt.format(writer, "JP V0 {X}", .{nnn}),
        0xC000 => try fmt.format(writer, "RND V{X} {X:0>2}", .{ x, kk }),
        0xD000 => try fmt.format(writer, "DRW V{X} V{X} {X}", .{ x, y, n }),
        0xE09E => try fmt.format(writer, "SKP V{X}", .{x}),
        0xE0A1 => try fmt.format(writer, "SKNP V{X}", .{x}),
        0xF007 => try fmt.format(writer, "LD V{X} DT", .{x}),
        0xF00A => try fmt.format(writer, "LD V{X} K", .{x}),
        0xF015 => try fmt.format(writer, "LD DT V{X}", .{x}),
        0xF018 => try fmt.format(writer, "LD ST V{X}", .{x}),
        0xF01E => try fmt.format(writer, "ADD I V{X}", .{x}),
        0xF029 => try fmt.format(writer, "LD F V{X}", .{x}),
        0xF033 => try fmt.format(writer, "LD B V{X}", .{x}),
        0xF055 => try fmt.format(writer, "LD [I] V{X}", .{x}),
        0xF065 => try fmt.format(writer, "LD V{X} [I]", .{x}),
        else => return false,
    }
    return true;
}

pub const AssembleOptions = struct {
    skip_address_prefix: bool = false,
    skip_opcode_prefix: bool = false,
    file_name: ?[]const u8 = null,
};

pub fn assemble(allocator: Allocator, input: []const u8, base_address: usize, writer: anytype, options: AssembleOptions, log: anytype) !void {
    var subst_map = StringHashMap([]const u8).init(allocator);
    defer subst_map.deinit();
    var string_arena = ArenaAllocator.init(allocator);
    defer string_arena.deinit();
    try assemble_impl.parseCode(string_arena.allocator(), input, base_address, writer, &subst_map, options, log, .label_prepass);
    try assemble_impl.parseCode(string_arena.allocator(), input, base_address, writer, &subst_map, options, log, .write);
}

const assemble_impl = struct {
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
            assert(index < self.len);
            assert(self.types[index] == .imm);
            return @as(u16, @intCast(TCheck, self.values[index]));
        }

        // #TODO Return an error instead of asserting?
        // #NOTE Return u16 so we don't have to cast at the call-site.
        fn reg(self: @This(), index: usize) u16 {
            assert(index < self.len);
            assert(self.types[index] == .reg);
            return @as(u16, @intCast(u4, self.values[index]));
        }
    };
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

    fn parseInstruction(code: []const u8, log: anytype) ?u16 {
        var tokens = mem.tokenize(u8, code, " ,");
        const opname = tokens.next() orelse "";

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
                if (args.len == 0) {
                    parsed_type = .f;
                } else {
                    parsed_type = .imm;
                    imm_parse_offset = 0;
                }
            } else if (mem.eql(u8, token, "B")) {
                if (args.len == 0) {
                    parsed_type = .b;
                } else {
                    parsed_type = .imm;
                    imm_parse_offset = 0;
                }
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
                log.err("unidentifiable token '{s}'", .{token});
                break;
            }
        }

        // log.debug("  args: {s}", .{args});
        var hash = ophash(opname, args.types[0..args.len]);

        return switch (hash) {
            op("CLS", null, null, null) => 0x00E0,
            op("RET", null, null, null) => 0x00EE,
            op("JP", .imm, null, null) => 0x1000 | args.imm(u12, 0),
            op("CALL", .imm, null, null) => 0x2000 | args.imm(u12, 0),
            op("SE", .reg, .imm, null) => 0x3000 | args.reg(0) << 8 | args.imm(u8, 1),
            op("SNE", .reg, .imm, null) => 0x4000 | args.reg(0) << 8 | args.imm(u8, 1),
            op("SE", .reg, .reg, null) => 0x5000 | args.reg(0) << 8 | args.reg(1) << 4,
            op("LD", .reg, .imm, null) => 0x6000 | args.reg(0) << 8 | args.imm(u8, 1),
            op("ADD", .reg, .imm, null) => 0x7000 | args.reg(0) << 8 | args.imm(u8, 1),
            op("LD", .reg, .reg, null) => 0x8000 | args.reg(0) << 8 | args.reg(1) << 4,
            op("OR", .reg, .reg, null) => 0x8001 | args.reg(0) << 8 | args.reg(1) << 4,
            op("AND", .reg, .reg, null) => 0x8002 | args.reg(0) << 8 | args.reg(1) << 4,
            op("XOR", .reg, .reg, null) => 0x8003 | args.reg(0) << 8 | args.reg(1) << 4,
            op("ADD", .reg, .reg, null) => 0x8004 | args.reg(0) << 8 | args.reg(1) << 4,
            op("SUB", .reg, .reg, null) => 0x8005 | args.reg(0) << 8 | args.reg(1) << 4,
            op("SHR", .reg, .reg, null) => 0x8006 | args.reg(0) << 8 | args.reg(1) << 4,
            op("SUBN", .reg, .reg, null) => 0x8007 | args.reg(0) << 8 | args.reg(1) << 4,
            op("SHL", .reg, .reg, null) => 0x800E | args.reg(0) << 8 | args.reg(1) << 4,
            op("SNE", .reg, .reg, null) => 0x9000 | args.reg(0) << 8 | args.reg(1) << 4,
            op("LD", .i, .imm, null) => 0xA000 | args.imm(u12, 1),
            op("JP", .reg, .imm, null) => 0xB000 | args.imm(u12, 1),
            op("RND", .reg, .imm, null) => 0xC000 | args.reg(0) << 8 | args.imm(u8, 1),
            op("DRW", .reg, .reg, .imm) => 0xD000 | args.reg(0) << 8 | args.reg(1) << 4 | args.imm(u4, 2),
            op("SKP", .reg, null, null) => 0xE09E | args.reg(0) << 8,
            op("SKNP", .reg, null, null) => 0xE0A1 | args.reg(0) << 8,
            op("LD", .reg, .dt, null) => 0xF007 | args.reg(0) << 8,
            op("LD", .reg, .k, null) => 0xF00A | args.reg(0) << 8,
            op("LD", .dt, .reg, null) => 0xF015 | args.reg(1) << 8,
            op("LD", .st, .reg, null) => 0xF018 | args.reg(1) << 8,
            op("ADD", .i, .reg, null) => 0xF01E | args.reg(1) << 8,
            op("LD", .f, .reg, null) => 0xF029 | args.reg(1) << 8,
            op("LD", .b, .reg, null) => 0xF033 | args.reg(1) << 8,
            op("LD", .i, .reg, null) => 0xF055 | args.reg(1) << 8,
            op("LD", .reg, .i, null) => 0xF065 | args.reg(0) << 8,
            else => null,
        } orelse {
            return fmt.parseInt(u16, opname, 16) catch null;
        };
    }

    fn parseCode(str_allocator: Allocator, input: []const u8, base_address: usize, writer: anytype, subst_map: *StringHashMap([]const u8), options: AssembleOptions, log: anytype, comptime mode: enum { label_prepass, write }) !void {
        var line_iter = mem.tokenize(u8, input, "\r\n");
        var line_index: usize = 0;
        var address: usize = base_address;
        while (line_iter.next()) |line| : (line_index += 1) {
            var instruction = line;
            // log.debug("line '{s}' | foo '{s}'", .{ line, instruction });
            if (mem.indexOf(u8, instruction, "#")) |offset| {
                instruction = instruction[0..offset];
            }
            // log.debug("line '{s}' | bar '{s}'", .{ line, instruction });
            if (options.skip_address_prefix and instruction.len >= 5) {
                instruction = instruction[0..5];
            }
            // log.debug("line '{s}' | baz '{s}'", .{ line, instruction });
            if (options.skip_opcode_prefix and instruction.len >= 5) {
                instruction = instruction[0..5];
            }
            // log.debug("line '{s}' | qux '{s}'", .{ line, instruction });
            instruction = mem.trim(u8, instruction, " ");
            // log.debug("line '{s}' | drp '{s}'", .{ line, instruction });
            if (instruction.len > 0) {
                switch (mode) {
                    .label_prepass => {
                        if (mem.startsWith(u8, instruction, ":")) {
                            const address_str = try fmt.allocPrint(str_allocator, "{X}", .{address});
                            try subst_map.put(instruction[1..], address_str);
                        } else if (mem.startsWith(u8, instruction, "0b")) {
                            address += @divExact((instruction.len - 2), 8);
                        } else if (mem.startsWith(u8, instruction, "0x")) {
                            address += @divExact((instruction.len - 2), 2);
                        } else {
                            address += 2;
                        }
                    },
                    .write => {
                        if (mem.startsWith(u8, instruction, ":")) {
                            continue;
                        }

                        // log.debug("tokenizing '{s}'", .{instruction});

                        // Run all tokens through the substitution map.
                        const token_max_count = 4;
                        var token_buf: [token_max_count * @sizeOf([]const u8)]u8 = undefined;
                        var token_allocator = FixedBufferAllocator.init(&token_buf);
                        var token_arr = ArrayList([]const u8).init(token_allocator.allocator());
                        defer token_arr.deinit();
                        try token_arr.ensureTotalCapacityPrecise(token_max_count);
                        var token_iterator = mem.tokenize(u8, instruction, " ,");
                        while (token_iterator.next()) |token| {
                            var foo = try token_arr.addOne();
                            foo.* = token;
                            var kv_iterator = subst_map.iterator();
                            while (kv_iterator.next()) |kv| {
                                if (mem.eql(u8, foo.*, kv.key_ptr.*)) {
                                    foo.* = kv.value_ptr.*;
                                    break;
                                }
                            }
                        }
                        var subst_buf: [64]u8 = undefined;
                        var subst_allocator = FixedBufferAllocator.init(&subst_buf);
                        var resolved = try mem.join(subst_allocator.allocator(), " ", token_arr.items);

                        if (mem.startsWith(u8, resolved, "0b")) {
                            // Raw binary number.
                            var value_index: usize = 2;
                            while (value_index < resolved.len) : (value_index += 8) {
                                const value = try fmt.parseInt(u8, resolved[value_index .. value_index + 8], 2);
                                try writer.writeIntBig(u8, value);
                            }
                            assert(value_index == resolved.len);
                        } else if (mem.startsWith(u8, resolved, "0x")) {
                            // Raw hexadecimal number.
                            var value_index: usize = 2;
                            while (value_index < resolved.len) : (value_index += 2) {
                                const value = try fmt.parseInt(u8, resolved[value_index .. value_index + 2], 16);
                                try writer.writeIntBig(u8, value);
                            }
                            assert(value_index == resolved.len);
                        } else {
                            if (parseInstruction(resolved, log)) |opcode| {
                                log.debug("{s}:{}: writing instruction='{s}' opcode={X:0>4}", .{ options.file_name, line_index + 1, resolved, opcode });
                                try writer.writeIntBig(u16, opcode);
                            } else {
                                log.err("{s}:{}: unable to make sense of instruction='{s}', line='{s}'", .{ options.file_name, line_index + 1, resolved, line });
                            }
                        }
                    },
                }
            }
        }
    }
};
