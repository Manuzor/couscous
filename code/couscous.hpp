#include "mtb.hpp"
using namespace mtb;

// More descriptive for translation-unit-local functions.
#define internal static

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
  union
  {
    struct
    {
      argument Arg0;
      argument Arg1;
      argument Arg2;
    };
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
