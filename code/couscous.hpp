#include "mtb.hpp"
using namespace mtb;

// More descriptive for translation-unit-local functions.
#define internal static

enum
{
  MEMORY_LENGTH = 4096,
  PROGRAM_START_ADDRESS = 0x200,
  MAX_ROM_LENGTH = MEMORY_LENGTH - PROGRAM_START_ADDRESS,
  STACK_LENGTH = 16,
  CHAR_MEMORY_OFFSET = 0,
};

struct screen
{
  bool32* Pixels;

  int Width;
  int Height;
};

struct sprite
{
  int Length;
  u8* Pixels;
};

struct machine
{
  u8 GPR[16];
  u8* V0 = GPR + 0x0;
  u8* V1 = GPR + 0x1;
  u8* V2 = GPR + 0x2;
  u8* V3 = GPR + 0x3;
  u8* V4 = GPR + 0x4;
  u8* V5 = GPR + 0x5;
  u8* V6 = GPR + 0x6;
  u8* V7 = GPR + 0x7;
  u8* V8 = GPR + 0x8;
  u8* V9 = GPR + 0x9;
  u8* VA = GPR + 0xA;
  u8* VB = GPR + 0xB;
  u8* VC = GPR + 0xC;
  u8* VD = GPR + 0xD;
  u8* VE = GPR + 0xE;
  u8* VF = GPR + 0xF;

  u16 I;

  u8 DT;
  u8 ST;

  // RAM
  u8 Memory[MEMORY_LENGTH];

  u8 StackPointer;
  u16 Stack[STACK_LENGTH];

  u16 ProgramCounter;

  bool Input[16];

  screen Screen;
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
  union
  {
    argument Arg0, Arg1, Arg2;
    argument Args[3];
  };
};

union instruction_decoder
{
  u16 Data;

  struct
  {
    u16 Address : 12;
    u16 Group   : 4;
  };

  struct
  {
    u16 LSN : 4; // Least significant nibble
    u16 Y   : 4;
    u16 X   : 4;
    u16 MSN : 4; // Most significant nibble
  };

  struct
  {
    u16 LSB : 8; // Least significant byte
    u16 MSB : 8; // Most significant byte
  };
};

static_assert(sizeof(instruction_decoder) == sizeof(u16), "Invalid size for `instruction`.");

// TODO: Should the machine load the rom?
void
InitMachine(machine* M);

void
ClearScreen(machine* M);

u16
GetDigitSpriteAddress(machine* M, u8 Digit);

sprite
GetCharacterSprite(machine* M, char Character);

void
DrawSprite(machine* M, int X, int Y, sprite Sprite);

bool
LoadRom(machine* M, slice<u8> Rom);

u8
ReadByte(machine* M, u16 Address);

u16
ReadWord(machine* M, u16 Address);

void
WriteByte(machine* M, u16 Address, u8 Byte);

void
WriteWord(machine* M, u16 Address, u16 Word);

// Return value of `true` means continue ticking.
bool
Tick(machine* M);

// NOTE: Provided by the platform layer.
void
Print(char const* String);

u16
FetchInstruction(machine* M);

instruction
DecodeInstruction(u16 OpCode);

void
ExecuteInstruction(machine* M, instruction Instruction);
