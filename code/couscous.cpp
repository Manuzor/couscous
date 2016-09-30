#include "charmap.cpp"

internal
void
InitMachine(machine* M)
{
  // TODO: Ensure charmap size is ok.
  u8* CharMemory = (u8*)M->Memory + CHAR_MEMORY_OFFSET;
  MemCopy(sizeof(GlobalCharMap), CharMemory, (u8*)GlobalCharMap);
}

internal
sprite
GetCharacterSprite(machine* M, char Character)
{
  sprite Result{};
  Result.Length = 5;

  int Offset{};

  if(Character >= '0' && Character <= '9')
  {
    Offset = (int)Character - (int)'0';
  }
  else if(Character >= 'A' && Character <= 'Z')
  {
    Offset = 9 + (int)Character - (int)'A';
  }
  else
  {
    MTB_ReportError("Invalid character.");
  }

  // Every character sprite has a length of 5.
  Offset *= 5;

  u8* CharMemory = (u8*)M->Memory + CHAR_MEMORY_OFFSET;
  Result.Bytes = CharMemory + Offset;

  return Result;
}

void
DrawSprite(machine* M, int StartX, int StartY, sprite Sprite)
{
  MTB_DebugAssert(Sprite.Length <= 15); // As per 2.4 "Chip-8 sprites may be up to 15 bytes, [...]"

  int Pitch = M->Screen.Width / 8;
  int X = StartX / 8;
  for(int Y = StartY; Y < Sprite.Length; ++Y)
  {
    int DestOffset = (Y * Pitch) + X;
    u8* DestPixel = M->Screen.Pixels + DestOffset;
    u8* SourcePixel = Sprite.Bytes + Y;

    // Check for pixel collision.
    if(*DestPixel & *SourcePixel)
      *M->VF = 1;

    *DestPixel = *DestPixel ^ *SourcePixel;
  }
}

internal
void
Tick(machine* M)
{
  static int Test{};

  DrawSprite(M, 8 * Test, 0, GetCharacterSprite(M, '0' + Test));

  if(Test < 3)
    ++Test;
  else
    Test = 0;
}
