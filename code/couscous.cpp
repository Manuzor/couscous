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
  SetElements(ScreenPixelsLength, M->Screen.Pixels);
}

internal
auto
::GetDigitSpriteAddress(machine* M, u8 Digit)
  -> u16
{
  u16 Result = (u16)CHAR_MEMORY_OFFSET + (u16)Digit;
  return Result;
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

  int const SpriteWidth = 8; // Always 1 byte.
  int const SpriteHeight = Sprite.Length;

  bool32 HasCollision{};
  for (int SpriteY = 0; SpriteY < SpriteHeight; ++SpriteY)
  {
    u8* SpritePixel = Sprite.Pixels + SpriteY;

    int ScreenY = StartY + SpriteY;
    while(ScreenY > M->Screen.Height) ScreenY -= M->Screen.Height;
    for (int SpriteX = 0; SpriteX < SpriteWidth; ++SpriteX)
    {
      int ScreenX = StartX + SpriteX;
      while(ScreenX > M->Screen.Width) ScreenX -= M->Screen.Width;
      int const ScreenOffset = (ScreenY * M->Screen.Width) + ScreenX;
      bool32* ScreenPixel = M->Screen.Pixels + ScreenOffset;

      bool32 SpriteColor = IsBitSet((u32)*SpritePixel, 7 - SpriteX);
      HasCollision |= *ScreenPixel & SpriteColor;
      *ScreenPixel ^= SpriteColor;
    }
  }

  *M->VF = !!HasCollision;
}

internal
auto
::ReadByte(machine* M, u16 Address)
  -> u8
{
  u8* Ptr = M->Memory + Address;
  u8 Result = *Ptr;
  return Result;
}

internal
auto
::ReadWord(machine* M, u16 Address)
  -> u16
{
  u8* Ptr = M->Memory + Address;
  u16 Result = *(u16*)Ptr;
  #if MTB_IsOn(MTB_LittleEndian)
    Result = (u16)((Result << 8) | (Result >> 8));
  #endif
  return Result;
}

internal
auto
::WriteByte(machine* M, u16 Address, u8 Byte)
  -> void
{
  u8* Ptr = M->Memory + Address;
  *Ptr = Byte;
}

internal
auto
::WriteWord(machine* M, u16 Address, u16 Word)
  -> void
{
  u8* Ptr = M->Memory + Address;
  #if MTB_IsOn(MTB_LittleEndian)
    Word = (u16)((Word << 8) | (Word >> 8));
  #endif
    *(u16*)Ptr = Word;
}

internal
void
PrintInstruction(instruction Instruction)
{
  switch(Instruction.Type)
  {
    case instruction_type::CLS:  Print("CLS");  break;
    case instruction_type::RET:  Print("RET");  break;
    case instruction_type::SYS:  Print("SYS");  break;
    case instruction_type::JP:   Print("JP");   break;
    case instruction_type::CALL: Print("CALL"); break;
    case instruction_type::SE:   Print("SE");   break;
    case instruction_type::SNE:  Print("SNE");  break;
    case instruction_type::LD:   Print("LD");   break;
    case instruction_type::ADD:  Print("ADD");  break;
    case instruction_type::OR:   Print("OR");   break;
    case instruction_type::AND:  Print("AND");  break;
    case instruction_type::XOR:  Print("XOR");  break;
    case instruction_type::SUB:  Print("SUB");  break;
    case instruction_type::SHR:  Print("SHR");  break;
    case instruction_type::SUBN: Print("SUBN"); break;
    case instruction_type::SHL:  Print("SHL");  break;
    case instruction_type::RND:  Print("RND");  break;
    case instruction_type::DRW:  Print("DRW");  break;
    case instruction_type::SKP:  Print("SKP");  break;
    case instruction_type::SKNP: Print("SKNP"); break;
    default:
      Print("<INVALID INSTRUCTION TYPE>");
      return;
  }

  char const* Prefix = " ";
  for(auto Arg : Instruction.Args)
  {
    if(Arg.Type == argument_type::NONE)
      break;

    Print(Prefix);
    char ConversionBuffer[8]{};
    switch(Arg.Type)
    {
      case argument_type::V:
      {
        Print("V");
        Convert<slice<char>>(Arg.Value, Slice(ConversionBuffer));
        Print(ConversionBuffer);
        break;
      }
      case argument_type::I:
      {
        Print("I");
        break;
      }
      case argument_type::DT:
      {
        Print("DT");
        break;
      }
      case argument_type::ST:
      {
        Print("ST");
        break;
      }
      case argument_type::K:
      {
        Print("K");
        break;
      }
      case argument_type::F:
      {
        Print("F");
        break;
      }
      case argument_type::B:
      {
        Print("B");
        break;
      }
      case argument_type::ATI:
      {
        Print("[I]");
        break;
      }
      case argument_type::ADDRESS:
      {
        Convert<slice<char>>(Arg.Value, Slice(ConversionBuffer));
        Print(ConversionBuffer);
        break;
      }
      case argument_type::BYTE:
      {
        Convert<slice<char>>(Arg.Value, Slice(ConversionBuffer));
        Print(ConversionBuffer);
        break;
      }
      case argument_type::NIBBLE:
      {
        Convert<slice<char>>(Arg.Value, Slice(ConversionBuffer));
        Print(ConversionBuffer);
        break;
      }
    }

    Prefix = ", ";
  }

  Print("\n");
}

internal
auto
::Tick(machine* M)
  -> bool
{
  // Fetch new instruction.
  u16 EncodedInstruction = ReadWord(M, M->ProgramCounter);
  instruction Instruction = DecodeInstruction(EncodedInstruction);

  // Update the timer slots.
  if(M->DT > 0)
    --M->DT;
  if(M->ST > 0)
    --M->ST;

  if(M->ST)
  {
    // TODO: Make some noise!
  }
  else
  {
    // TODO: Stop the noise...
  }

  PrintInstruction(Instruction);

  if(Instruction.Type == instruction_type::INVALID)
    return false;

  // Execute the fetched instruction.
  ExecuteInstruction(M, Instruction);

  // Advance the program counter.
  M->ProgramCounter += 2;

  return true;
}

internal
auto
::DecodeInstruction(u16 OpCode)
  -> instruction
{
  using inst = instruction_type;
  using arg = argument_type;

  instruction_decoder Decoder{ OpCode };
  instruction Result{};
  switch(Decoder.Group)
  {
    case 0x0:
    {
      switch(OpCode)
      {
        case 0x00E0: // 00E0 - CLS
          Result.Type = inst::CLS;
          break;
        case 0x00EE: // 00EE - RET
          Result.Type = inst::RET;
          break;
        default: // 0nnn - SYS addr
          Result.Type = inst::SYS;
          Result.Args[0].Type = arg::ADDRESS;
          Result.Args[0].Value = Decoder.Address;
          break;
      }
      break;
    }
    case 0x1: // 1nnn - JP addr
    {
      Result.Type = inst::JP;
      Result.Args[0].Type = arg::ADDRESS;
      Result.Args[0].Value = Decoder.Address;
      break;
    }
    case 0x2: // 2nnn - CALL addr
    {
      Result.Type = inst::CALL;
      Result.Args[0].Type = arg::ADDRESS;
      Result.Args[0].Value = Decoder.Address;
      break;
    }
    case 0x3: // 3xkk - SE Vx, byte
    {
      Result.Type = inst::SE;
      Result.Args[0].Type = arg::V;
      Result.Args[0].Value = Decoder.X;
      Result.Args[1].Type = arg::BYTE;
      Result.Args[1].Value = Decoder.LSB;
      break;
    }
    case 0x4: // 4xkk - SNE Vx, byte
    {
      Result.Type = inst::SNE;
      Result.Args[0].Type = arg::V;
      Result.Args[0].Value = Decoder.X;
      Result.Args[1].Type = arg::BYTE;
      Result.Args[1].Value = Decoder.LSB;
      break;
    }
    case 0x5: // 5xy0 - SE Vx, Vy
    {
      Result.Type = inst::SE;
      Result.Args[0].Type = arg::V;
      Result.Args[0].Value = Decoder.X;
      Result.Args[1].Type = arg::V;
      Result.Args[1].Value = Decoder.Y;
      break;
    }
    case 0x6: // 6xkk - LD Vx, byte
    {
      Result.Type = inst::LD;
      Result.Args[0].Type = arg::V;
      Result.Args[0].Value = Decoder.X;
      Result.Args[1].Type = arg::BYTE;
      Result.Args[1].Value = Decoder.LSB;
      break;
    }
    case 0x7: // 7xkk - ADD Vx, byte
    {
      Result.Type = inst::ADD;
      Result.Args[0].Type = arg::V;
      Result.Args[0].Value = Decoder.X;
      Result.Args[1].Type = arg::BYTE;
      Result.Args[1].Value = Decoder.LSB;
      break;
    }
    case 0x8:
    {
      Result.Args[0].Type = arg::V;
      Result.Args[0].Value = Decoder.X;
      Result.Args[1].Type = arg::V;
      Result.Args[1].Value = Decoder.Y;
      switch(Decoder.LSN)
      {
        case 0x0: Result.Type = inst::LD; break;   // 8xy0 - LD   Vx, Vy
        case 0x1: Result.Type = inst::OR; break;   // 8xy1 - OR   Vx, Vy
        case 0x2: Result.Type = inst::AND; break;  // 8xy2 - AND  Vx, Vy
        case 0x3: Result.Type = inst::XOR; break;  // 8xy3 - XOR  Vx, Vy
        case 0x4: Result.Type = inst::ADD; break;  // 8xy4 - ADD  Vx, Vy
        case 0x5: Result.Type = inst::SUB; break;  // 8xy5 - SUB  Vx, Vy
        case 0x6: Result.Type = inst::SHR; break;  // 8xy6 - SHR  Vx {, Vy}
        case 0x7: Result.Type = inst::SUBN; break; // 8xy7 - SUBN Vx, Vy
        case 0xE: Result.Type = inst::SHL; break;  // 8xyE - SHL  Vx {, Vy}
      }
      break;
    }
    case 0x9: // 9xy0 - SNE Vx, Vy
    {
      Result.Type = inst::SNE;
      Result.Args[0].Type = arg::V;
      Result.Args[0].Value = Decoder.X;
      Result.Args[1].Type = arg::V;
      Result.Args[1].Value = Decoder.Y;
      break;
    }
    case 0xA: // Annn - LD I, addr
    {
      Result.Type = inst::LD;
      Result.Args[0].Type = arg::I;
      Result.Args[1].Type = arg::ADDRESS;
      Result.Args[1].Value = Decoder.Address;
      break;
    }
    case 0xB: // Bnnn - JP V0, addr
    {
      Result.Type = inst::LD;
      Result.Args[0].Type = arg::V;
      Result.Args[0].Value = 0;
      Result.Args[1].Type = arg::ADDRESS;
      Result.Args[1].Value = Decoder.Address;
      break;
    }
    case 0xC: // Cxkk - RND Vx, byte
    {
      Result.Type = inst::RND;
      Result.Args[0].Type = arg::V;
      Result.Args[0].Value = Decoder.X;
      Result.Args[1].Type = arg::BYTE;
      Result.Args[1].Value = Decoder.LSB;
      break;
    }
    case 0xD: // Dxyn - DRW Vx, Vy, nibble
    {
      Result.Type = inst::DRW;
      Result.Args[0].Type = arg::V;
      Result.Args[0].Value = Decoder.X;
      Result.Args[1].Type = arg::V;
      Result.Args[1].Value = Decoder.Y;
      Result.Args[2].Type = arg::NIBBLE;
      Result.Args[2].Value = Decoder.LSN;
      break;
    }
    case 0xE:
    {
      Result.Args[0].Type = arg::V;
      Result.Args[0].Value = Decoder.X;
      switch(Decoder.LSB)
      {
        case 0x9E: Result.Type = inst::SKP;  break; // Ex9E - SKP Vx
        case 0xA1: Result.Type = inst::SKNP; break; // ExA1 - SKNP Vx
      }
      break;
    }
    case 0xF:
    {
      switch(Decoder.LSB)
      {
        case 0x07: // Fx07 - LD Vx, DT
        {
          Result.Type = inst::LD;
          Result.Args[0].Type = arg::V;
          Result.Args[0].Value = Decoder.X;
          Result.Args[1].Type = arg::DT;
          break;
        }
        case 0x0A: // Fx0A - LD Vx, K
        {
          Result.Type = inst::LD;
          Result.Args[0].Type = arg::V;
          Result.Args[0].Value = Decoder.X;
          Result.Args[1].Type = arg::K;
          break;
        }
        case 0x15: // Fx15 - LD DT, Vx
        {
          Result.Type = inst::LD;
          Result.Args[0].Type = arg::DT;
          Result.Args[1].Type = arg::V;
          Result.Args[1].Value = Decoder.X;
          break;
        }
        case 0x18: // Fx18 - LD ST, Vx
        {
          Result.Type = inst::LD;
          Result.Args[0].Type = arg::ST;
          Result.Args[1].Type = arg::V;
          Result.Args[1].Value = Decoder.X;
          break;
        }
        case 0x1E: // Fx1E - ADD I, Vx
        {
          Result.Type = inst::ADD;
          Result.Args[0].Type = arg::I;
          Result.Args[1].Type = arg::V;
          Result.Args[1].Value = Decoder.X;
          break;
        }
        case 0x29: // Fx29 - LD F, Vx
        {
          Result.Type = inst::LD;
          Result.Args[0].Type = arg::F;
          Result.Args[1].Type = arg::V;
          Result.Args[1].Value = Decoder.X;
          break;
        }
        case 0x33: // Fx33 - LD B, Vx
        {
          Result.Type = inst::LD;
          Result.Args[0].Type = arg::B;
          Result.Args[1].Type = arg::V;
          Result.Args[1].Value = Decoder.X;
          break;
        }
        case 0x55: // Fx55 - LD [I], Vx
        {
          Result.Type = inst::LD;
          Result.Args[0].Type = arg::ATI;
          Result.Args[1].Type = arg::V;
          Result.Args[1].Value = Decoder.X;
          break;
        }
        case 0x65: // Fx65 - LD Vx, [I]
        {
          Result.Type = inst::LD;
          Result.Args[0].Type = arg::V;
          Result.Args[0].Value = Decoder.X;
          Result.Args[1].Type = arg::ATI;
          break;
        }
      }
    }
  }

  return Result;
}

internal
auto
::ExecuteInstruction(machine* M, instruction Instruction)
  -> void
{
  switch(Instruction.Type)
  {
    case instruction_type::CLS:
    {
      ClearScreen(M);
      return;
    }
    case instruction_type::RET:
    {
      M->ProgramCounter = M->Stack[--M->StackPointer];
      return;
    }
    case instruction_type::SYS:
    {
      MTB_ReportError("Not implemented.");
      return;
    }
    case instruction_type::JP:
    {
      switch(Instruction.Args[0].Type)
      {
        case argument_type::V:
        {
          u8* Reg = M->GPR + Instruction.Args[0].Value;
          M->ProgramCounter = Instruction.Args[1].Value + *Reg;
          return;
        }
        case argument_type::ADDRESS:
        {
          M->ProgramCounter = Instruction.Args[0].Value;
          return;
        }
      }
      break;
    }
    case instruction_type::CALL:
    {
      switch(Instruction.Args[0].Type)
      {
        case argument_type::ADDRESS:
        {
          M->Stack[M->StackPointer++] = M->ProgramCounter;
          M->ProgramCounter = Instruction.Args[0].Value;
          return;
        }
      }
      break;
    }
    case instruction_type::SE:
    {
      switch(Instruction.Args[0].Type)
      {
        case argument_type::V:
        {
          u8 Lhs = M->GPR[Instruction.Args[0].Value];
          u8 Rhs = M->GPR[Instruction.Args[1].Value];
          if(Lhs == Rhs)
            M->ProgramCounter += 2;
          return;
        }
        case argument_type::BYTE:
        {
          u8 Lhs = M->GPR[Instruction.Args[0].Value];
          u8 Rhs = (u8)Instruction.Args[1].Value;
          if(Lhs == Rhs)
            M->ProgramCounter += 2;
          return;
        }
      }
      break;
    }
    case instruction_type::SNE:
    {
      switch(Instruction.Args[0].Type)
      {
        case argument_type::V:
        {
          u8 Lhs = M->GPR[Instruction.Args[0].Value];
          u8 Rhs = M->GPR[Instruction.Args[1].Value];
          if(Lhs != Rhs)
            M->ProgramCounter += 2;
          return;
        }
        case argument_type::BYTE:
        {
          u8 Lhs = M->GPR[Instruction.Args[0].Value];
          u8 Rhs = (u8)Instruction.Args[1].Value;
          if(Lhs != Rhs)
            M->ProgramCounter += 2;
          return;
        }
      }
      break;
    }
    case instruction_type::LD:
    {
      switch(Instruction.Args[0].Type)
      {
        case argument_type::ATI:
        {
          switch(Instruction.Args[1].Type)
          {
            case argument_type::V:
            {
              u8 Num = (u8)Instruction.Args[1].Value;
              u8* Dest = M->Memory + M->I;
              u8* Source = M->GPR;
              CopyBytes(Num, Dest, Source);
              return;
            }
          }
          break;
        }
        case argument_type::B:
        {
          switch(Instruction.Args[1].Type)
          {
            case argument_type::V:
            {
              u8* Reg = M->GPR + Instruction.Args[1].Value;
              u8* DDD = M->Memory + (M->I + 0);
              u8* DD  = M->Memory + (M->I + 1);
              u8* D   = M->Memory + (M->I + 2);

              *DDD = *Reg / 100;
              *DD  = *Reg / 10 - *DDD;
              *DD  = *Reg / 1  - *DDD - *DD;
              // TODO: Test this instruction!
              return;
            }
          }
          break;
        }
        case argument_type::DT:
        {
          switch(Instruction.Args[1].Type)
          {
            case argument_type::V:
            {
              u8* Reg = M->GPR + Instruction.Args[1].Value;
              M->DT = *Reg;
              return;
            }
          }
          break;
        }
        case argument_type::F:
        {
          switch(Instruction.Args[1].Type)
          {
            case argument_type::V:
            {
              u8* Reg = M->GPR + Instruction.Args[1].Value;
              M->I = GetDigitSpriteAddress(M, *Reg);
              return;
            }
          }
          break;
        }
        case argument_type::I:
        {
          switch(Instruction.Args[1].Type)
          {
            case argument_type::ADDRESS:
            {
              M->I = Instruction.Args[1].Value;
              return;
            }
          }
          break;
        }
        case argument_type::ST:
        {
          switch(Instruction.Args[1].Type)
          {
            case argument_type::V:
            {
              u8* Reg = M->GPR + Instruction.Args[1].Value;
              M->ST = *Reg;
              return;
            }
          }
          break;
        }
        case argument_type::V:
        {
          switch(Instruction.Args[1].Type)
          {
            case argument_type::ATI:
            {
              u8 Num = (u8)Instruction.Args[0].Value;
              u8* Dest = M->GPR;
              u8* Source = M->Memory + M->I;
              CopyBytes(Num, Dest, Source);
              return;
            }
            case argument_type::BYTE:
            {
              u8* Reg = M->GPR + Instruction.Args[0].Value;
              *Reg = (u8)Instruction.Args[1].Value;
              return;
            }
            case argument_type::DT:
            {
              u8* Reg = M->GPR + Instruction.Args[0].Value;
              *Reg = M->DT;
              return;
            }
            case argument_type::K:
            {
              u8* Reg = M->GPR + Instruction.Args[0].Value;
              // TODO: All execution stops until a key is pressed, then the
              // value of that key is stored in Vx.
              return;
            }
            case argument_type::V:
            {
              u8* RegA = M->GPR + Instruction.Args[0].Value;
              u8* RegB = M->GPR + Instruction.Args[1].Value;
              *RegA = *RegB;
              return;
            }
          }
          break;
        }
      }
      break;
    }
    case instruction_type::ADD:
    {
      switch(Instruction.Args[0].Type)
      {
        case argument_type::I:
        {
          switch(Instruction.Args[1].Type)
          {
            case argument_type:: V:
            {
              u8* Reg = M->GPR + Instruction.Args[1].Value;
              M->I += *Reg;
              return;
            }
          }
          break;
        }
        case argument_type::V:
        {
          u8* Reg = M->GPR + Instruction.Args[0].Value;
          switch(Instruction.Args[1].Type)
          {
            case argument_type:: BYTE:
            {
              *Reg += (u8)Instruction.Args[1].Value;
              return;
            }
            case argument_type:: V:
            {
              u8* OtherReg = M->GPR + Instruction.Args[1].Value;
              u16 Result = (u16)*Reg + (u16)*OtherReg;
              *Reg += *OtherReg;
              *M->VF = (u8)(Result > 255);
              *Reg = (u8)(Result & 0xFF);
              return;
            }
          }
          break;
        }
      }
      break;
    }
    case instruction_type::OR:
    {
      // TODO
      break;
    }
    case instruction_type::AND:
    {
      // TODO
      break;
    }
    case instruction_type::XOR:
    {
      // TODO
      break;
    }
    case instruction_type::SUB:
    {
      // TODO
      break;
    }
    case instruction_type::SHR:
    {
      // TODO
      break;
    }
    case instruction_type::SUBN:
    {
      // TODO
      break;
    }
    case instruction_type::SHL:
    {
      // TODO
      break;
    }
    case instruction_type::RND:
    {
      // TODO
      break;
    }
    case instruction_type::DRW:
    {
      switch(Instruction.Args[0].Type)
      {
        case argument_type::V:
        {
          switch(Instruction.Args[1].Type)
          {
            case argument_type::V:
            {
              switch(Instruction.Args[2].Type)
              {
                case argument_type::NIBBLE:
                {
                  u8* RegA = M->GPR + Instruction.Args[0].Value;
                  u8* RegB = M->GPR + Instruction.Args[1].Value;
                  sprite Sprite;
                  Sprite.Length = (int)Instruction.Args[2].Value;
                  Sprite.Pixels = (u8*)(M->Memory + M->I);
                  DrawSprite(M, *RegA, *RegB, Sprite);
                  return;
                }
              }
            }
          }
        }
      }
      break;
    }
    case instruction_type::SKP:
    {
      // TODO
      break;
    }
    case instruction_type::SKNP:
    {
      // TODO
      break;
    }
    default: break;
  }
  Print("Invalid instruction to execute.");
}

#if defined(COUSCOUS_TESTS)
  #include "couscous_tests.cpp"
#endif
