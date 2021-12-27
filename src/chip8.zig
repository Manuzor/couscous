const std = @import("std");
const mem = std.mem;
const log = std.log.scoped(.chip8);
const root = @import("root");

pub const stack_base_address: u16 = 0x0000;
pub const charmap_base_address: u16 = 0x0010;
pub const user_base_address: u16 = 0x0200;

pub const Display = struct {
    width: u16 = 64,
    height: u16 = 32,
    data: []u1,
};

pub const Keyboard = struct {
    last: u8 = undefined,
    block: bool = false,
    state: [16]bool = [_]bool{false} ** 16,
};

pub const Cpu = struct {
    v: [16]u8 = [_]u8{0} ** 16,
    dt: u8 = 0,
    st: u8 = 0,

    pc: u16 = undefined,
    sp: u16 = undefined,

    i: u16 = 0,

    opcode: u16 = undefined,
    step: u16 = undefined,

    pub fn setPc(cpu: *Cpu, pc: u16) void {
        cpu.pc = pc;
        cpu.step = 0;
    }

    pub fn loadOpcode(cpu: Cpu, memory: []u8) u16 {
        return mem.readIntSliceBig(u16, memory[cpu.pc..]);
    }

    pub fn tick(cpu: *Cpu, memory: []u8, display: Display, keyboard: *Keyboard, rand: std.rand.Random) void {
        const code = cpu.opcode;

        const h = @truncate(u4, code >> 12);
        const x = @truncate(u4, code >> 8);
        const y = @truncate(u4, code >> 4);
        const n = @truncate(u4, code);
        const kk = @truncate(u8, code);
        const nnn = @truncate(u12, code);

        const mask_table = [16]u16{
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
        const mask = mask_table[h];

        const step = cpu.step;
        cpu.step +%= 1;

        switch (code & mask) {
            0x00E0 => { // 00E0 - CLS
                const end = display.height * display.width / 8;
                if (step < end) {
                    const index = step * 8;
                    mem.set(u1, display.data[index .. index + 8], 0);
                }
                if (step + 1 == end) {
                    cpu.setPc(cpu.pc + 2);
                }
            },
            0x00EE => { // 00EE - RET
                if (cpu.sp > 0) {
                    cpu.sp -= 1;
                } else {
                    log.warn("00EE - unable to return: sp is already 0.", .{});
                }
                cpu.setPc(mem.readIntSliceBig(u16, memory[cpu.sp .. cpu.sp + 2]));
            },
            0x1000 => { // 1nnn - JP addr
                cpu.setPc(nnn);
            },
            0x2000 => { // 2nnn - CALL addr
                std.debug.assert(cpu.sp < 16); // stack overflow
                if (cpu.sp < 16) {
                    mem.writeIntSliceBig(u16, memory[cpu.sp .. cpu.sp + 2], cpu.pc);
                    cpu.sp += 1;
                } else {
                    log.warn("2nnn - stack overflow.", .{});
                }
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
                cpu.v[x] &= cpu.v[y];
                cpu.setPc(cpu.pc + 2);
            },
            0x8004 => { // 8xy4 - ADD Vx, Vy
                const carry = @addWithOverflow(u8, cpu.v[x], cpu.v[y], &cpu.v[x]);
                cpu.v[0xF] = if (carry) 1 else 0;
                cpu.setPc(cpu.pc + 2);
            },
            0x8005 => { // 8xy5 - SUB Vx, Vy
                const carry = @subWithOverflow(u8, cpu.v[x], cpu.v[y], &cpu.v[x]);
                cpu.v[0xF] = if (carry) 1 else 0;
                cpu.setPc(cpu.pc + 2);
            },
            0x8006 => { // 8xy6 - SHR Vx {, Vy}
                cpu.v[0xF] = @truncate(u1, cpu.v[x]);
                cpu.v[x] >>= 1;
                cpu.setPc(cpu.pc + 2);
            },
            0x8007 => { // 8xy7 - SUBN Vx, Vy
                const carry = @subWithOverflow(u8, cpu.v[y], cpu.v[x], &cpu.v[x]);
                cpu.v[0xF] = if (carry) 1 else 0;
                cpu.setPc(cpu.pc + 2);
            },
            0x800E => { // 8xyE - SHL Vx {, Vy}
                cpu.v[0xF] = @truncate(u1, cpu.v[x] >> 7);
                cpu.v[x] <<= 1;
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
                cpu.setPc(@as(u16, nnn) + @as(u16, cpu.v[0]));
            },
            0xC000 => { // Cxkk - RND Vx, byte
                // #TODO get rid of the std.rand.Random dependency by requesting a random number another way.
                cpu.v[x] = rand.int(u8) & kk;
                cpu.setPc(cpu.pc + 2);
            },
            0xD000 => { // Dxyn - DRW Vx, Vy, nibble
                if (cpu.i + n < memory.len) {
                    var carry = false;
                    if (step == 0) {
                        cpu.v[0xF] = 0;
                    }
                    if (step < n) {
                        const display_y = (cpu.v[y] + step) % display.height;
                        const byte_value = memory[cpu.i + step];
                        const display_x0 = cpu.v[x];
                        comptime var bit = 8;
                        inline while (bit > 0) {
                            comptime bit -= 1;
                            const display_x = (display_x0 + bit) % display.width;
                            const value = @truncate(u1, byte_value >> 7 - bit);
                            const display_index = display_y * display.width + display_x;
                            const pixel = display.data[display_index];
                            if (pixel == 1 and value == 1) {
                                cpu.v[0xF] = 1;
                            }
                            display.data[display_index] = pixel ^ value;
                        }
                        if (step + 1 == n) {
                            cpu.v[0xF] = if (carry) 1 else 0;
                            cpu.setPc(cpu.pc + 2);
                        }
                    }
                } else {
                    log.warn("Dxyn - range (0x{x}..0x{x}) is outside the valid memory region.", .{ cpu.i, cpu.i + n });
                    cpu.setPc(cpu.pc + 2);
                }
            },
            0xE09E => { // Ex9E - SKP Vx
                var next_pc = cpu.pc + 2;
                const index = cpu.v[x];
                if (index < keyboard.state.len) {
                    if (keyboard.state[index]) {
                        next_pc += 2;
                    }
                } else {
                    log.warn("Ex9E - index '0x{x}' is out of range.", .{index});
                }
                cpu.setPc(next_pc);
            },
            0xE0A1 => { // ExA1 - SKNP Vx
                var next_pc = cpu.pc + 2;
                const index = cpu.v[x];
                if (index < keyboard.state.len) {
                    if (!keyboard.state[index]) {
                        next_pc += 2;
                    }
                } else {
                    log.warn("ExA1 - index '0x{x}' is out of range.", .{index});
                }
                cpu.setPc(next_pc);
            },
            0xF007 => { // Fx07 - LD Vx, DT
                cpu.v[x] = cpu.dt;
                cpu.setPc(cpu.pc + 2);
            },
            0xF00A => { // Fx0A - LD Vx, K
                if (step == 0) {
                    keyboard.block = true;
                } else if (!keyboard.block) {
                    cpu.v[x] = keyboard.last;
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
                cpu.i += cpu.v[x];
                cpu.setPc(cpu.pc + 2);
            },
            0xF029 => { // Fx29 - LD F, Vx
                cpu.i = charmap_base_address + 5 * cpu.v[x];
                cpu.setPc(cpu.pc + 2);
            },
            0xF033 => { // Fx33 - LD B, Vx
                // #TODO step
                if (cpu.i + 2 < memory.len) {
                    memory[cpu.i + 0] = cpu.v[x] / 100;
                    memory[cpu.i + 1] = (cpu.v[x] / 10) % 10;
                    memory[cpu.i + 2] = cpu.v[x] % 10;
                } else {
                    log.warn("Fx33 - range (0x{x}..0x{x}) is outside the valid memory region.", .{ cpu.i, cpu.i + 2 });
                }
                cpu.setPc(cpu.pc + 2);
            },
            0xF055 => { // Fx55 - LD [I], Vx
                // #TODO step
                if (cpu.i + 16 < memory.len) {
                    mem.copy(u8, memory[cpu.i .. cpu.i + 16], &cpu.v);
                } else {
                    log.warn("Fx55 - range (0x{x}..0x{x}) is outside the valid memory region.", .{ cpu.i, cpu.i + 16 });
                }
                cpu.setPc(cpu.pc + 2);
            },
            0xF065 => { // Fx65 - LD Vx, [I]
                // #TODO step
                if (cpu.i + 16 < memory.len) {
                    mem.copy(u8, &cpu.v, memory[cpu.i .. cpu.i + 16]);
                } else {
                    log.warn("Fx55 - range (0x{x}..0x{x}) is outside the valid memory region.", .{ cpu.i, cpu.i + 16 });
                }
                cpu.setPc(cpu.pc + 2);
            },
            else => {
                log.err("unknown opcode '0x{x}' (mask is '0x{x}')", .{ code, mask });
                cpu.setPc(cpu.pc + 2);
            },
        }
    }
};
