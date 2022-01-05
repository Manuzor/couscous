const std = @import("std");
const mem = std.mem;
const log = std.log.scoped(.chip8);
const root = @import("root");

pub const charmap_base_address = 0x0000;
pub const charmap_size = 0x0050;
pub const stack_base_address = 0x0050;
pub const stack_size = 0x0010;
pub const user_base_address = 0x0200;
pub const mem_size = 0x1000;

pub const opcode_mask_table = [16]u16{
    0xFFFF, // h: 0
    0xF000, // h: 1
    0xF000, // h: 2
    0xF000, // h: 3
    0xF000, // h: 4
    0xF00F, // h: 5
    0xF000, // h: 6
    0xF000, // h: 7
    0xF00F, // h: 8
    0xF00F, // h: 9
    0xF000, // h: A
    0xF000, // h: B
    0xF000, // h: C
    0xF000, // h: D
    0xF0FF, // h: E
    0xF0FF, // h: F
};

pub const Display = struct {
    width: usize = 64,
    height: usize = 32,
    data: []u1,
};

pub const Keyboard = struct {
    last: u8 = undefined,
    block: bool = false,
    state: [16]bool = [_]bool{false} ** 16,
};

// #NOTE Some ROMs seem to have different expectations of how the SHL/SHR instructions work.
pub const ShiftSrc = enum {
    x, // Read from Vx, perform the shift, store the result in Vx.
    y, // Read from Vy, perform the shift, store the result in Vx.
};

pub const JumpOffsetSrc = enum {
    @"0", // Offset is read from V0.
    x, // Offset is read from Vx.
};

pub const RegisterDumpBehavior = enum {
    immutable, // Do not modify I at all.
    increment, // Increment I while dumping registers.
};

pub const Cpu = struct {
    v: [16]u8 = [_]u8{0} ** 16,
    dt: u8 = 0,
    st: u8 = 0,

    pc: u16 = user_base_address,
    sp: u4 = 0,
    stack: [16]u16 = undefined,

    i: u16 = 0,

    opcode: u16 = undefined,

    waiting_for_input: bool = false,

    shift_src: ShiftSrc = .y,
    jump_offset_src: JumpOffsetSrc = .@"0",
    register_dump_behavior: RegisterDumpBehavior = .immutable,

    pub fn setPc(cpu: *Cpu, pc: u16) void {
        cpu.pc = pc;
        cpu.pc = wrapAddress(pc, null);
    }

    pub fn tick(cpu: *Cpu, memory: []u8, display: Display, keyboard: *Keyboard, rand: std.rand.Random) void {
        const opcode = cpu.opcode;

        const h = @truncate(u4, opcode >> 12);
        const x = @truncate(u4, opcode >> 8);
        const y = @truncate(u4, opcode >> 4);
        const n = @truncate(u4, opcode);
        const kk = @truncate(u8, opcode);
        const nnn = @truncate(u12, opcode);

        const opcode_mask = opcode_mask_table[h];

        switch (opcode & opcode_mask) {
            0x00E0 => { // 00E0 - CLS
                mem.set(u1, display.data, 0);
                cpu.setPc(cpu.pc + 2);
            },
            0x00EE => { // 00EE - RET
                // #NOTE Handle stack underflow by wrapping.
                cpu.sp -%= 1;
                cpu.setPc(cpu.stack[cpu.sp] + 2);
            },
            0x1000 => { // 1nnn - JP addr
                cpu.setPc(nnn);
            },
            0x2000 => { // 2nnn - CALL addr
                cpu.stack[cpu.sp] = cpu.pc;
                // #NOTE Handle stack overflow by wrapping.
                cpu.sp +%= 1;
                cpu.setPc(nnn);
            },
            0x3000 => { // 3xkk - SE Vx, byte
                if (cpu.v[x] == kk) {
                    cpu.setPc(cpu.pc + 4);
                } else {
                    cpu.setPc(cpu.pc + 2);
                }
            },
            0x4000 => { // 4xkk - SNE Vx, byte
                if (cpu.v[x] != kk) {
                    cpu.setPc(cpu.pc + 4);
                } else {
                    cpu.setPc(cpu.pc + 2);
                }
            },
            0x5000 => { // 5xy0 - SE Vx, Vy
                if (cpu.v[x] == cpu.v[y]) {
                    cpu.setPc(cpu.pc + 4);
                } else {
                    cpu.setPc(cpu.pc + 2);
                }
            },
            0x6000 => { // 6xkk - LD Vx, byte
                cpu.v[x] = kk;
                cpu.setPc(cpu.pc + 2);
            },
            0x7000 => { // 7xkk - ADD Vx, byte
                cpu.v[x] +%= kk;
                cpu.setPc(cpu.pc + 2);
            },
            0x8000 => { // 8xy0 - LD Vx, Vy
                cpu.v[x] = cpu.v[y];
                cpu.setPc(cpu.pc + 2);
            },
            0x8001 => { // 8xy1 - OR Vx, Vy
                cpu.v[x] |= cpu.v[y];
                cpu.setPc(cpu.pc + 2);
            },
            0x8002 => { // 8xy2 - AND Vx, Vy
                cpu.v[x] &= cpu.v[y];
                cpu.setPc(cpu.pc + 2);
            },
            0x8003 => { // 8xy3 - XOR Vx, Vy
                cpu.v[x] ^= cpu.v[y];
                cpu.setPc(cpu.pc + 2);
            },
            0x8004 => { // 8xy4 - ADD Vx, Vy
                const carry = @addWithOverflow(u8, cpu.v[x], cpu.v[y], &cpu.v[x]);
                cpu.v[0xF] = if (carry) 1 else 0;
                cpu.setPc(cpu.pc + 2);
            },
            0x8005 => { // 8xy5 - SUB Vx, Vy
                cpu.v[0xF] = if (cpu.v[x] > cpu.v[y]) 1 else 0;
                cpu.v[x] -%= cpu.v[y];
                cpu.setPc(cpu.pc + 2);
            },
            0x8006 => { // 8xy6 - SHR Vx {, Vy}
                const src = switch (cpu.shift_src) {
                    .x => x,
                    .y => y,
                };
                cpu.v[0xF] = @truncate(u1, cpu.v[src]);
                cpu.v[x] = cpu.v[src] >> 1;
                cpu.setPc(cpu.pc + 2);
            },
            0x8007 => { // 8xy7 - SUBN Vx, Vy
                cpu.v[0xF] = if (cpu.v[y] > cpu.v[x]) 1 else 0;
                cpu.v[x] = cpu.v[y] -% cpu.v[x];
                cpu.setPc(cpu.pc + 2);
            },
            0x800E => { // 8xyE - SHL Vx {, Vy}
                const src = switch (cpu.shift_src) {
                    .x => x,
                    .y => y,
                };
                cpu.v[0xF] = @truncate(u1, cpu.v[src] >> 7);
                cpu.v[x] = cpu.v[src] << 1;
                cpu.setPc(cpu.pc + 2);
            },
            0x9000 => { // 9xy0 - SNE Vx, Vy
                if (cpu.v[x] == cpu.v[y]) {
                    cpu.setPc(cpu.pc + 4);
                } else {
                    cpu.setPc(cpu.pc + 2);
                }
            },
            0xA000 => { // Annn - LD I, addr
                cpu.i = nnn;
                cpu.setPc(cpu.pc + 2);
            },
            0xB000 => { // Bnnn - JP V0, addr
                const src = switch(cpu.jump_offset_src) {
                    .@"0" => 0,
                    .x => x,
                };
                const pc = wrapAddress(@as(u16, nnn) + @as(u16, cpu.v[src]), null);
                cpu.setPc(pc);
            },
            0xC000 => { // Cxkk - RND Vx, byte
                cpu.v[x] = rand.int(u8) & kk;
                cpu.setPc(cpu.pc + 2);
            },
            0xD000 => { // Dxyn - DRW Vx, Vy, nibble
                cpu.v[0xF] = 0;
                var sprite_y: u16 = 0;
                while (sprite_y < n) : (sprite_y += 1) {
                    const sprite_value = memory[wrapAddress(cpu.i + sprite_y, null)];
                    var sprite_x: u16 = 0;
                    while (sprite_x < 8) : (sprite_x += 1) {
                        const mask = @as(u8, 0b1000_0000) >> @intCast(u3, sprite_x);
                        if (sprite_value & mask != 0) {
                            const display_x = (@as(usize, cpu.v[x]) + @as(usize, sprite_x)) % display.width;
                            const display_y = (cpu.v[y] + sprite_y) % display.height;
                            const display_index = display_y * display.width + display_x;
                            var pixel = &display.data[display_index];
                            if (pixel.* == 1) {
                                cpu.v[0xF] = 1;
                            }
                            pixel.* ^= 1;
                        }
                    }
                }
                cpu.setPc(cpu.pc + 2);
            },
            0xE09E => { // Ex9E - SKP Vx
                // #NOTE Only consider the lower 4 bits, i.e. values from 0 to 15 (inclusive) because we only have that
                // many keys to check.
                const index = cpu.v[x] & 0xF;
                if (keyboard.state[index]) {
                    cpu.setPc(cpu.pc + 4);
                } else {
                    cpu.setPc(cpu.pc + 2);
                }
            },
            0xE0A1 => { // ExA1 - SKNP Vx
                // #NOTE Only consider the lower 4 bits, i.e. values from 0 to 15 (inclusive) because we only have that
                // many keys to check.
                const index = cpu.v[x] & 0xF;
                if (!keyboard.state[index]) {
                    cpu.setPc(cpu.pc + 4);
                } else {
                    cpu.setPc(cpu.pc + 2);
                }
            },
            0xF007 => { // Fx07 - LD Vx, DT
                cpu.v[x] = cpu.dt;
                cpu.setPc(cpu.pc + 2);
            },
            0xF00A => { // Fx0A - LD Vx, K
                if (!cpu.waiting_for_input) {
                    cpu.waiting_for_input = true;
                    keyboard.block = true;
                } else if (!keyboard.block) {
                    cpu.v[x] = keyboard.last;
                    cpu.waiting_for_input = false;
                    cpu.setPc(cpu.pc + 2);
                }
            },
            0xF015 => { // Fx15 - LD DT, Vx
                cpu.dt = cpu.v[x];
                cpu.setPc(cpu.pc + 2);
            },
            0xF018 => { // Fx18 - LD ST, Vx
                cpu.st = cpu.v[x];
                cpu.setPc(cpu.pc + 2);
            },
            0xF01E => { // Fx1E - ADD I, Vx
                cpu.i = wrapAddress(cpu.i + cpu.v[x], &cpu.v[0xF]);
                cpu.setPc(cpu.pc + 2);
            },
            0xF029 => { // Fx29 - LD F, Vx
                // #NOTE We're only taking the lower 4 bits of VX, so if someone wanted the address of sprite 0xA3,
                // they'll get the address of sprite 0x03.
                cpu.i = charmap_base_address + 5 * (cpu.v[x] & 0xF);
                cpu.setPc(cpu.pc + 2);
            },
            0xF033 => { // Fx33 - LD B, Vx
                // #NOTE Wrap the address back the beginning of user-addressable memory on overflow.
                const i = [_]usize{
                    if (cpu.i + 0 < memory.len) cpu.i + 0 else user_base_address + (cpu.i + 0) - memory.len,
                    if (cpu.i + 1 < memory.len) cpu.i + 1 else user_base_address + (cpu.i + 1) - memory.len,
                    if (cpu.i + 2 < memory.len) cpu.i + 2 else user_base_address + (cpu.i + 2) - memory.len,
                };
                memory[i[0]] = cpu.v[x] / 100;
                memory[i[1]] = (cpu.v[x] / 10) % 10;
                memory[i[2]] = cpu.v[x] % 10;
                cpu.setPc(cpu.pc + 2);
            },
            0xF055 => { // Fx55 - LD [I], Vx
                // #NOTE Wrap back around to user addressable memory if attempting to read/write past the end of RAM.
                if (cpu.i >= memory.len) unreachable;
                const count = @as(u16, x) + 1;
                const left = std.math.min(@as(usize, count), memory.len - cpu.i);
                const right = count - left;
                mem.copy(u8, memory[cpu.i .. cpu.i + left], cpu.v[0..left]);
                if (right > 0) {
                    mem.copy(u8, memory[user_base_address .. user_base_address + right], cpu.v[left..count]);
                }
                switch (cpu.register_dump_behavior) {
                    .immutable => {},
                    .increment => {
                        cpu.i = wrapAddress(cpu.i + count, &cpu.v[0xF]);
                    },
                }
                cpu.setPc(cpu.pc + 2);
            },
            0xF065 => { // Fx65 - LD Vx, [I]
                // #NOTE Wrap back around to user addressable memory if attempting to read/write past the end of RAM.
                if (cpu.i >= memory.len) unreachable;
                const count = @as(u16, x) + 1;
                const left = std.math.min(@as(usize, count), memory.len - cpu.i);
                const right = count - left;
                mem.copy(u8, cpu.v[0..left], memory[cpu.i .. cpu.i + left]);
                if (right > 0) {
                    mem.copy(u8, cpu.v[left..count], memory[user_base_address .. user_base_address + right]);
                }
                switch (cpu.register_dump_behavior) {
                    .immutable => {},
                    .increment => {
                        cpu.i = wrapAddress(cpu.i + count, &cpu.v[0xF]);
                    },
                }
                cpu.setPc(cpu.pc + 2);
            },
            else => {
                log.err("unknown opcode '0x{x}' (mask is '0x{x}')", .{ opcode, opcode_mask });
                cpu.setPc(cpu.pc + 2);
            },
        }
    }
};

fn wrapAddress(value: u16, carry_flag: ?*u8) u16 {
    const min = user_base_address;
    const max = mem_size;
    const user_range = max - min;
    var carry = false;
    var result = value;
    if (result < min) {
        result += user_range;
        carry = true;
    }
    if (result >= max) {
        result -= user_range;
        carry = true;
    }
    if (carry_flag) |flag| {
        flag.* = if (carry) 1 else 0;
    }
    return result;
}
