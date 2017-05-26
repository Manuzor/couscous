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

using uint = unsigned int;
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

  u16 InputState;
  u8 RequiredInputRegisterIndexPlusOne; // "PlusOne" so it can be 0 by default.

  bool32 Screen[SCREEN_HEIGHT * SCREEN_WIDTH];

  mtb_rng RNG;
};


//
// Argument type stuff
//

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

  CONSTANT,
};

static char const*
GetArgumentTypeAsString(argument_type Value);

static argument_type
MakeArgumentTypeFromString(size_t CodeLen, char const* Code);


//
// argument
//

struct argument
{
  argument_type Type;
  u16 Value; // Interpretation depends on Type.
};

static size_t
GetArgumentAsString(argument Argument, size_t BufferSize, u8* Buffer);

static argument
MakeArgumentFromString(size_t CodeLen, char const* Code);


//
// Instruction stuff
//

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

static char const*
GetInstructionTypeAsString(instruction_type Value);

static instruction_type
MakeInstructionTypeFromString(size_t CodeLen, char const* Code);


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

static u16
GetDigitSpriteAddress(machine* M, u8 Digit);

static sprite
GetCharacterSprite(machine* M, char Character);

static void
DrawSprite(machine* M, int X, int Y, sprite Sprite);

// TODO(Manuzor): Stuff like this could be put to the platform layer.
static u8
ReadByte(void* Ptr);

static u16
ReadWord(void* Ptr);

static void
WriteByte(void* Ptr, u8 Value);

static void
WriteWord(void* Ptr, u16 Value);

struct tick_result
{
  bool Continue;
};

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

static u16
MapCharToKeyIndex(char Key);

static char
MapKeyIndexToChar(u16 KeyIndex);

static bool
IsKeyDown(u16 InputState, u16 KeyIndex);

static u16
SetKeyDown(u16 InputState, u16 KeyIndex, bool32 IsDown);

// Assumed keyboard layout:
//   1 2 3 C
//   4 5 6 D
//   7 8 9 E
//   A 0 B F
//
// Usage: IsKeyDown(M, 'A');
static bool32
IsKeyDown(machine* M, char Key);


#if COUSCOUS_ASSEMBLER
  struct assembler_code
  {
    size_t Size;
    char Data[32];
  };

  static instruction
  AssembleInstruction(assembler_code Code);

  static assembler_code
  DisassembleInstruction(instruction Instruction);
#endif // COUSCOUS_ASSEMBLER
