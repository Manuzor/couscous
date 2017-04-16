#include "mtb.h"

using u8  = mtb_u08;
using u16 = mtb_u16;
using u32 = mtb_u32;
using u64 = mtb_u64;
using s8  = mtb_s08;
using s16 = mtb_s16;
using s32 = mtb_s32;
using s64 = mtb_s64;

using f32 = mtb_f32;
using f64 = mtb_f64;

using bool32 = int;


enum
{
  CHAR_MEMORY_OFFSET = 0,
  SCREEN_WIDTH = 64,
  SCREEN_HEIGHT = 32,
};

struct sprite
{
  int Length;
  u8* Pixels;
};

struct machine
{
  u8 V[16];

  u16 I;

  u8 DT;
  u8 ST;

  // RAM
  union
  {
    u8 Memory[4096];

    struct
    {
      u8 InterpreterMemory[0x200];
      u8 ProgramMemory[4096 - 0x200];
    };
  };

  u8 StackPointer;
  u16 Stack[16];

  u16 ProgramCounter;

  bool Input[16];

  bool32 Screen[SCREEN_HEIGHT * SCREEN_WIDTH];
};

enum struct argument_type
{
  NONE,

  V,
  I,
  DT,
  ST,

  K,
  F,
  B,
  ATI, // [I], "at I"

  ADDRESS,
  BYTE,
  NIBBLE,
};

struct argument
{
  argument_type Type;
  u16 Value; // Interpretation depends on Type.
};

enum struct instruction_type
{
  INVALID,

  CLS,
  RET,
  SYS,
  JP,
  CALL,
  SE,
  SNE,
  LD,
  ADD,
  OR,
  AND,
  XOR,
  SUB,
  SHR,
  SUBN,
  SHL,
  RND,
  DRW,
  SKP,
  SKNP,
};

struct instruction
{
  instruction_type Type;
  argument Args[3];
};

union instruction_decoder
{
  u16 Data; // xxxx

  struct
  {
    u16 Address : 12; // 0xxx
    u16 Group   : 4;  // x000
  };

  struct
  {
    u16 LSN : 4; // 000x (Least significant nibble)
    u16 Y   : 4; // 00x0
    u16 X   : 4; // 0x00
    u16 MSN : 4; // x000 (Most significant nibble)
  };

  struct
  {
    u16 LSB : 8; // 00xx (Least significant byte)
    u16 MSB : 8; // xx00 (Most significant byte)
  };
};

static_assert(sizeof(instruction_decoder) == sizeof(u16), "Invalid size for `instruction`.");

// TODO: Should the machine load the rom?
static void
InitMachine(machine* M);

static void
ClearScreen(machine* M);

static u16
GetDigitSpriteAddress(machine* M, u8 Digit);

static sprite
GetCharacterSprite(machine* M, char Character);

static void
DrawSprite(machine* M, int X, int Y, sprite Sprite);

static bool
LoadRom(machine* M, size_t RomSize, u8* RomPtr);

static u8
ReadByte(machine* M, u16 Address);

static u16
ReadWord(machine* M, u16 Address);

static void
WriteByte(machine* M, u16 Address, u8 Byte);

static void
WriteWord(machine* M, u16 Address, u16 Word);

struct tick_result
{
  bool Continue;
};

// Return value of `true` means continue ticking.
static tick_result
Tick(machine* M);

static u16
FetchInstruction(machine* M);

static instruction
DecodeInstruction(instruction_decoder Decoder);

static u16
EncodeInstruction(instruction Instruction);

static void
ExecuteInstruction(machine* M, instruction Instruction);

static instruction
AssembleInstruction(size_t Size, char* Code);

struct disassembled_instruction
{
  size_t Size;
  #define MAX_ASSEMBLER_INSTRUCTION_SIZE 32
  char Code[MAX_ASSEMBLER_INSTRUCTION_SIZE];
};
static disassembled_instruction
DisassembleInstruction(instruction Instruction);
