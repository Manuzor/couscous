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

union instruction
{
  u16 Data;

  struct
  {
    u16 Args  : 12;
    u16 Group : 4;
  };

  struct
  {
    u16 Nibble3 : 4;
    u16 Nibble2 : 4;
    u16 Nibble1 : 4;
    u16 Nibble0 : 4;
  };

  struct
  {
    u16 Byte1 : 8;
    u16 Byte0 : 8;
  };
};

static_assert(sizeof(instruction) == sizeof(u16), "Invalid size for `instruction`.");

// TODO: Should the machine load the rom?
void
InitMachine(machine* M);

void
ClearScreen(machine* M);

sprite
GetCharacterSprite(machine* M, char Character);

void
DrawSprite(machine* M, int X, int Y, sprite Sprite);

// `true` means continue ticking.
bool
Tick(machine* M);
