#include "charmap.cpp"

internal
auto
::InitMachine(machine* M)
  -> void
{
  // TODO: Ensure charmap size is ok.
  u8* CharMemory = (u8*)M->Memory + CHAR_MEMORY_OFFSET;
  CopyBytes(ByteLengthOf(GlobalCharMap), CharMemory, (u8*)GlobalCharMap);

  M->ProgramCounter = PROGRAM_START_ADDRESS;
}

internal
auto
::ClearScreen(machine* M)
  -> void
{
  size_t const ScreenPixelsLength = M->Screen.Width * M->Screen.Height;
  SetBytes(ScreenPixelsLength, M->Screen.Pixels, 0);
}

internal
auto
::GetCharacterSprite(machine* M, char Character)
  -> sprite
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
  Result.Pixels = CharMemory + Offset;

  return Result;
}

internal
auto
::DrawSprite(machine* M, int StartX, int StartY, sprite Sprite)
  -> void
{
  MTB_DebugAssert(Sprite.Length <= 15); // As per 2.4 "Chip-8 sprites may be up to 15 bytes, [...]"

  bool32 HasCollision{};
  for(int Y = 0; Y < Sprite.Length; ++Y)
  {
    u8* SpritePixel = Sprite.Pixels + Y;

    int ScreenY = StartY + Y;
    while(ScreenY > M->Screen.Height)
      ScreenY -= M->Screen.Height;

    int ScreenPitch = ScreenY * M->Screen.Width;
    for(int X = 0; X < 8; ++X)
    {
      int ScreenX = StartX + X;
      while(ScreenY > M->Screen.Height)
        ScreenY -= M->Screen.Height;

      int ScreenOffset = ScreenPitch + ScreenX;
      bool32* ScreenPixel = M->Screen.Pixels + ScreenOffset;
      bool32 SpriteValue = IsBitSet((u32)*SpritePixel, X);
      *ScreenPixel = SpriteValue;
      HasCollision |= *ScreenPixel && SpriteValue;
    }
  }

  *M->VF = !!HasCollision;
}

internal
auto
::Tick(machine* M)
  -> bool
{
  // static int Test{};

  // DrawSprite(M, 8 * Test, 0, GetCharacterSprite(M, '0' + Test));

  // if(Test < 7)
  //   ++Test;
  // else
  //   Test = 0;

  if(M->DT)
  {
    --M->DT;
  }

  if(M->ST)
  {
    --M->ST;
  }

  if(M->ST)
  {
    // TODO: Make some noise!
  }
  else
  {
    // TODO: Stop the noise...
  }

  u8* InstructionLocation = M->Memory + M->ProgramCounter;
  M->ProgramCounter += 2;
  if(M->ProgramCounter >= LengthOf(M->Memory))
  {
    // This should be an error case.
    return false;
  }

  instruction Instruction;
  Instruction.Data = *(u16*)InstructionLocation;
  switch(Instruction.Group)
  {
    case 0x0:
    {
      if(Instruction.Nibble2 == 0xE && Instruction.Nibble3 == 0x0)
      {
        ClearScreen(M);
      }
      else if(Instruction.Nibble2 == 0xE && Instruction.Nibble3 == 0xE)
      {
        M->ProgramCounter = M->Stack[--M->StackPointer];
      }
      else goto UnknownInstruction;
    } break;
    case 0x1:
    {
      M->ProgramCounter = (u16)Instruction.Args;
    } break;
    case 0x2:
    {
      M->Stack[M->StackPointer++] = M->ProgramCounter;
      M->ProgramCounter = Instruction.Args;
    } break;
    case 0x3:
    {
      u8* Register = M->GPR + Instruction.Nibble1;
      u8 Value = Instruction.Byte1;
      if(*Register == Value)
        M->ProgramCounter += 2;
    } break;
    case 0x4:
    {
      u8* Register = M->GPR + Instruction.Nibble1;
      u8 Value = Instruction.Byte1;
      if(*Register != Value)
        M->ProgramCounter += 2;
    } break;
    case 0x5:
    {
      u8* RegisterA = M->GPR + Instruction.Nibble1;
      u8* RegisterB = M->GPR + Instruction.Nibble2;
      if(*RegisterA == *RegisterB)
        M->ProgramCounter += 2;
    } break;
    case 0x6:
    {
      u8* Register = M->GPR + Instruction.Nibble1;
      u8 Value = Instruction.Byte1;
      *Register = Value;
    } break;
    case 0x7:
    {
      u8* Register = M->GPR + Instruction.Nibble1;
      u8 Value = Instruction.Byte1;
      *Register += Value;
    } break;
    case 0x8:
    {
      u8* RegisterA = M->GPR + Instruction.Nibble1;
      u8* RegisterB = M->GPR + Instruction.Nibble2;
      switch(Instruction.Nibble3)
      {
        case 0x0:
        {
          *RegisterA = *RegisterB;
        } break;
        case 0x1:
        {
          *RegisterA |= *RegisterB;
        } break;
        case 0x2:
        {
          *RegisterA &= *RegisterB;
        } break;
        case 0x3:
        {
          *RegisterA ^= *RegisterB;
        } break;
        case 0x4:
        {
          int Result = (int)*RegisterA + (int)*RegisterB;
          *M->VF = !!(Result > 255);
          *RegisterA = (u8)Result;
        } break;
        case 0x5:
        {
          *M->VF = !!(*RegisterA > *RegisterB);
          *RegisterA -= *RegisterB;
        } break;
        case 0x6:
        {
          *M->VF = !!(*RegisterA & 1);
          *RegisterA /= 2;
        } break;
        case 0x7:
        {
          *M->VF = !!(*RegisterB > *RegisterA);
          *RegisterA = *RegisterB - *RegisterA;
        } break;
        case 0xE:
        {
          *M->VF = !!(*RegisterA & 0b1000'0000);
          *RegisterA *= 2;
        } break;
        default: goto UnknownInstruction;
      }
    } break;
    case 0x9:
    {
      u8* RegisterA = M->GPR + Instruction.Nibble1;
      u8* RegisterB = M->GPR + Instruction.Nibble2;
      if(*RegisterA != *RegisterB)
        M->ProgramCounter += 2;
    } break;
    case 0xA:
    {
      M->I = Instruction.Args;
    } break;
    case 0xB:
    {
      M->ProgramCounter = Instruction.Args + *M->V0;
    } break;
    case 0xC:
    {
      u8* Register = M->GPR + Instruction.Nibble1;
      // TODO: Random numbers!
      u8 RandomNumber = 42;
      *Register = (u8)(RandomNumber & Instruction.Byte1);
    } break;
    case 0xD:
    {
      int X = (int)Instruction.Nibble1;
      int Y = (int)Instruction.Nibble2;
      int Num = (int)Instruction.Nibble3;
      u8* Pixels = M->Memory + M->I;
      sprite Sprite{ Num, Pixels };
      DrawSprite(M, X, Y, Sprite);
    } break;
    case 0xE:
    {
      u8* Register = M->GPR + Instruction.Nibble1;
      if(Instruction.Nibble2 == 0x9 && Instruction.Nibble3 == 0xE)
      {
        u8 KeyIndex = *Register;
        if(M->Input[KeyIndex])
          M->ProgramCounter += 2;
      }
      else if(Instruction.Nibble2 == 0xA && Instruction.Nibble3 == 0x1)
      {
        u8 KeyIndex = *Register;
        if(!M->Input[KeyIndex])
          M->ProgramCounter += 2;
      }
      else goto UnknownInstruction;
    } break;
    case 0xF:
    {
      u8* Register = M->GPR + Instruction.Nibble1;
      switch(Instruction.Nibble2)
      {
        case 0x0:
        {
          if(Instruction.Nibble3 == 0x7)
          {
            *Register = M->DT;
          }
          else if(Instruction.Nibble3 == 0xA)
          {
            // TODO: All execution stops until a key is pressed, then the
            // value of that key is stored in `*Register`.
          }
          else goto UnknownInstruction;
        } break;
        case 0x1:
        {
          switch(Instruction.Nibble3)
          {
            case 0x5:
            {
              M->DT = *Register;
            } break;
            case 0x8:
            {
              M->ST = *Register;
            } break;
            case 0xE:
            {
              M->I += *Register;
            } break;
            default: goto UnknownInstruction;
          }
        } break;
        case 0x2:
        {
          if(Instruction.Nibble3 == 0x9)
          {
            // TODO
          }
          else goto UnknownInstruction;
        } break;
        case 0x3:
        {
          if(Instruction.Nibble3 == 0x3)
          {
            u8 Value = *Register;
            u8 Digit100 = Value / 100;
            u8 Digit010 = Value / 10 - Digit100;
            u8 Digit001 = Value / 1  - Digit100 - Digit010;

            u8* Location0 = M->Memory + M->I + 0;
            u8* Location1 = M->Memory + M->I + 1;
            u8* Location2 = M->Memory + M->I + 2;

            *Location0 = Digit100;
            *Location1 = Digit010;
            *Location2 = Digit001;
          }
          else goto UnknownInstruction;
        } break;
        case 0x5:
        {
          if(Instruction.Nibble3 == 0x5)
          {
            u8* Source = Register;
            u8* Dest = M->Memory + M->I;
            u8 Num = Instruction.Nibble1 + 1;
            CopyBytes(Num, Dest, Source);
          }
          else goto UnknownInstruction;
        } break;
        case 0x6:
        {
          if(Instruction.Nibble3 == 0x5)
          {
            u8* Source = M->Memory + M->I;
            u8* Dest = Register;
            u8 Num = Instruction.Nibble1 + 1;
            CopyBytes(Num, Dest, Source);
          }
          else goto UnknownInstruction;
        } break;
        default: goto UnknownInstruction;
      }
    } break;
    default: goto UnknownInstruction;
  }

  return true;

  UnknownInstruction:
    if(Instruction.Data == 0xFFFF || Instruction.Data == 0x0000)
    {
      // Treat these as terminators.
    }
    else
    {
      MTB_ReportError("Unknown instruction.");
    }

    return false;
}
