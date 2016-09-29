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
};

struct colorRGB8
{
  union
  {
    struct
    {
      u8 R;
      u8 G;
      u8 B;
    };

    u8 Data[3];
  };
};

struct back_buffer
{
  colorRGB8* Memory{};
  int Width;
  int Height;
  int Pitch;
  int BytesPerPixel;
};

struct machine
{
  u8 GPR[16]{};
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

  u16 I{};

  // RAM
  u8 Memory[MEMORY_LENGTH]{};

  u16 Stack[STACK_LENGTH]{};

  bool Input[16]{};

  back_buffer Screen;
};

// NOTE: Implemented in the platform layer.
void
ClearBackBuffer(back_buffer* BackBuffer, colorRGB8 Color);

void
Tick(machine* M);
