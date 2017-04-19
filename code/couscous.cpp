#include "charmap.cpp"

void
InitMachine(machine* M)
{
  // TODO: Ensure charmap size is ok.
  u8* CharMemory = (u8*)M->Memory + CHAR_MEMORY_OFFSET;
  mtb_CopyBytes(mtb_ArrayByteSizeOf(GlobalCharMap), CharMemory, (u8*)GlobalCharMap);

  M->ProgramCounter = (u16)((u8*)M->ProgramMemory - (u8*)M->InterpreterMemory);
  MTB_AssertDebug(M->ProgramCounter == 0x200);
}

void
ClearScreen(machine* M)
{
  mtb_SetBytes(mtb_ArrayByteSizeOf(M->Screen), M->Screen, 0);
}

u16
GetDigitSpriteAddress(machine* M, u8 Digit)
{
  u16 Result = (u16)CHAR_MEMORY_OFFSET + (u16)Digit;
  return Result;
}

void
DrawSprite(machine* M, int StartX, int StartY, sprite Sprite)
{
  MTB_AssertDebug(Sprite.Length <= 15); // As per 2.4 "Chip-8 sprites may be up to 15 bytes, [...]"

  int SpriteWidth = 8; // Always 1 byte.
  int SpriteHeight = Sprite.Length;

  bool32 HasCollision = false;
  for (int SpriteY = 0; SpriteY < SpriteHeight; ++SpriteY)
  {
    u8* SpritePixel = Sprite.Pixels + SpriteY;
    int ScreenY = (StartY + SpriteY) % SCREEN_HEIGHT;
    for (int SpriteX = 0; SpriteX < SpriteWidth; ++SpriteX)
    {
      int ScreenX = (StartX + SpriteX) % SCREEN_WIDTH;

      int ScreenOffset = (ScreenY * SCREEN_WIDTH) + ScreenX;
      bool32* ScreenPixel = M->Screen + ScreenOffset;

      bool32 SpriteColor = mtb_IsBitSet((u32)*SpritePixel, 7 - SpriteX);
      bool32 ScreenColor = *ScreenPixel;
      HasCollision |= ScreenColor & SpriteColor;
      *ScreenPixel = ScreenColor ^ SpriteColor;
    }
  }

  M->V[0xF] = !!HasCollision;
}

bool
LoadRom(machine* M,  size_t RomSize, u8* RomPtr)
{
  bool Result = false;

  if(RomSize <= mtb_ArrayLengthOf(M->ProgramMemory))
  {
    mtb_CopyBytes(RomSize, M->ProgramMemory, RomPtr);
    Result = true;
  }

  return Result;
}

u8
ReadByte(void* Ptr)
{
  u8 Result = *(u8*)Ptr;
  return Result;
}

u16
ReadWord(void* Ptr)
{
  u16 Result = *(u16*)Ptr;
  #if MTB_FLAG(LITTLE_ENDIAN)
    Result = (u16)((Result << 8) | (Result >> 8));
  #endif
  return Result;
}

void
WriteByte(void* Ptr, u8 Value)
{
  *(u8*)Ptr = Value;
}

void
WriteWord(void* Ptr, u16 Value)
{
  #if MTB_FLAG(LITTLE_ENDIAN)
    Value = (u16)((Value << 8) | (Value >> 8));
  #endif
  *(u16*)Ptr = Value;
}

tick_result
Tick(machine* M)
{
  tick_result Result{};
  Result.Continue = true;

  // Fetch new instruction.
  instruction_decoder Decoder;
  Decoder.Data = ReadWord(M->Memory + M->ProgramCounter);
  instruction Instruction = DecodeInstruction(Decoder);
  #if 0
  disassembled_instruction DisInst = DisassembleInstruction(Instruction);
  printf("0x%04X => %s\n", Decoder.Data, DisInst.Code);
  instruction AssInst = AssembleInstruction(DisInst.Size, DisInst.Code);
  #endif

  if(Instruction.Type != instruction_type::INVALID)
  {
    // Advance the program counter.
    M->ProgramCounter += 2;

    // Execute the fetched instruction.
    ExecuteInstruction(M, Instruction);

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
  }
  else
  {
    Result.Continue = false;
  }

  return Result;
}

instruction
DecodeInstruction(instruction_decoder Decoder)
{
  using inst = instruction_type;
  using arg = argument_type;

  instruction Result{};

  switch(Decoder.Group)
  {
    case 0x0:
    {
      switch(Decoder.Data)
      {
        case 0x00E0: // 00E0 - CLS
          Result.Type = inst::CLS;
          break;
        case 0x00EE: // 00EE - RET
          Result.Type = inst::RET;
          break;
        default: // 0nnn - SYS addr
          Result.Type = inst::SYS;
          Result.Args[0].Type = arg::CONSTANT;
          Result.Args[0].Value = Decoder.Address;
          break;
      }
      break;
    }
    case 0x1: // 1nnn - JP addr
    {
      Result.Type = inst::JP;
      Result.Args[0].Type = arg::CONSTANT;
      Result.Args[0].Value = Decoder.Address;
      break;
    }
    case 0x2: // 2nnn - CALL addr
    {
      Result.Type = inst::CALL;
      Result.Args[0].Type = arg::CONSTANT;
      Result.Args[0].Value = Decoder.Address;
      break;
    }
    case 0x3: // 3xkk - SE Vx, byte
    {
      Result.Type = inst::SE;
      Result.Args[0].Type = arg::V;
      Result.Args[0].Value = Decoder.X;
      Result.Args[1].Type = arg::CONSTANT;
      Result.Args[1].Value = Decoder.LSB;
      break;
    }
    case 0x4: // 4xkk - SNE Vx, byte
    {
      Result.Type = inst::SNE;
      Result.Args[0].Type = arg::V;
      Result.Args[0].Value = Decoder.X;
      Result.Args[1].Type = arg::CONSTANT;
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
      Result.Args[1].Type = arg::CONSTANT;
      Result.Args[1].Value = Decoder.LSB;
      break;
    }
    case 0x7: // 7xkk - ADD Vx, byte
    {
      Result.Type = inst::ADD;
      Result.Args[0].Type = arg::V;
      Result.Args[0].Value = Decoder.X;
      Result.Args[1].Type = arg::CONSTANT;
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
      Result.Args[1].Type = arg::CONSTANT;
      Result.Args[1].Value = Decoder.Address;
      break;
    }
    case 0xB: // Bnnn - JP V0, addr
    {
      Result.Type = inst::LD;
      Result.Args[0].Type = arg::V;
      Result.Args[0].Value = 0;
      Result.Args[1].Type = arg::CONSTANT;
      Result.Args[1].Value = Decoder.Address;
      break;
    }
    case 0xC: // Cxkk - RND Vx, byte
    {
      Result.Type = inst::RND;
      Result.Args[0].Type = arg::V;
      Result.Args[0].Value = Decoder.X;
      Result.Args[1].Type = arg::CONSTANT;
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
      Result.Args[2].Type = arg::CONSTANT;
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

  // If the instruction is invalid, save the raw instructin value in Args[0].
  if(Result.Type == instruction_type::INVALID)
    Result.Args[0].Value = Decoder.Data;

  return Result;
}

u16
EncodeInstruction(instruction Instruction)
{
  instruction_decoder Decoder{};

  switch(Instruction.Type)
  {
    case instruction_type::CLS:
    {
      Decoder.Data = 0x00E0;
      break;
    }
    case instruction_type::RET:
    {
      Decoder.Data = 0x00EE;
      break;
    }
    case instruction_type::SYS:
    {
      switch(Instruction.Args[0].Type)
      {
        case argument_type::CONSTANT:
        {
          Decoder.Data = Instruction.Args[0].Value;
          break;
        }
      }
      break;
    }
    case instruction_type::JP:
    {
      // TODO
      break;
    }
    case instruction_type::CALL:
    {
      // TODO
      break;
    }
    case instruction_type::SE:
    {
      // TODO
      break;
    }
    case instruction_type::SNE:
    {
      // TODO
      break;
    }
    case instruction_type::LD:
    {
      switch(Instruction.Args[0].Type)
      {
        case argument_type::I:
        {
          switch(Instruction.Args[1].Type)
          {
            case argument_type::CONSTANT:
            {
              Decoder.Data = Instruction.Args[1].Value;
              Decoder.MSN = 0xA;
              break;
            }
          }
          break;
        }
      }
      // TODO
      break;
    }
    case instruction_type::ADD:
    {
      // TODO
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
      // TODO
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
  }

  return Decoder.Data;
}

void
ExecuteInstruction(machine* M, instruction Instruction)
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
      MTB_NOT_IMPLEMENTED;
      return;
    }
    case instruction_type::JP:
    {
      switch(Instruction.Args[0].Type)
      {
        case argument_type::V:
        {
          u8* Reg = M->V + Instruction.Args[0].Value;
          M->ProgramCounter = Instruction.Args[1].Value + *Reg;
          return;
        }
        case argument_type::CONSTANT:
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
        case argument_type::CONSTANT:
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
          u8 Lhs = M->V[Instruction.Args[0].Value];
          switch(Instruction.Args[1].Type)
          {
            case argument_type::V:
            {
              u8 Rhs = M->V[Instruction.Args[1].Value];
              if(Lhs == Rhs)
                M->ProgramCounter += 2;
              return;
            }

            case argument_type::CONSTANT:
            {
              u8 Rhs = (u8)Instruction.Args[1].Value;
              if(Lhs == Rhs)
                M->ProgramCounter += 2;
              return;
            }
          }
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
          u8 Lhs = M->V[Instruction.Args[0].Value];
          u8 Rhs = M->V[Instruction.Args[1].Value];
          if(Lhs != Rhs)
            M->ProgramCounter += 2;
          return;
        }
        case argument_type::CONSTANT:
        {
          u8 Lhs = M->V[Instruction.Args[0].Value];
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
              int Num = (int)Instruction.Args[1].Value;
              for (int Index = 0; Index < Num; ++Index, ++M->I)
              {
                u8* Reg = M->V + Index;
                *Reg = ReadByte(M->Memory + M->I);
              }

              while(Num > 0)
              {
                // u8* Reg = M->V + ReadByte(M, M->I);
                ++M->I;
                --Num;
              }
              u8* Dest = M->Memory + M->I;
              u8* Source = M->V;
              mtb_CopyBytes(Num, Dest, Source);
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
              u8* Reg = M->V + Instruction.Args[1].Value;
              u8* DDD = M->Memory + (M->I + 0);
              u8* DD  = M->Memory + (M->I + 1);
              u8* D   = M->Memory + (M->I + 2);

              *DDD = *Reg / 100; // % 10;
              *DD  = *Reg /  10 % 10;
              *D   = *Reg /   1 % 10;
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
              u8* Reg = M->V + Instruction.Args[1].Value;
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
              u8* Reg = M->V + Instruction.Args[1].Value;
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
            case argument_type::CONSTANT:
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
              u8* Reg = M->V + Instruction.Args[1].Value;
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
              u8* Dest = M->V;
              u8* Source = M->Memory + M->I;
              mtb_CopyBytes(Num, Dest, Source);
              return;
            }
            case argument_type::CONSTANT:
            {
              u8* Reg = M->V + Instruction.Args[0].Value;
              *Reg = (u8)Instruction.Args[1].Value;
              return;
            }
            case argument_type::DT:
            {
              u8* Reg = M->V + Instruction.Args[0].Value;
              *Reg = M->DT;
              return;
            }
            case argument_type::K:
            {
              // u8* Reg = M->V + Instruction.Args[0].Value;
              // TODO: All execution stops until a key is pressed, then the
              // value of that key is stored in Vx.
              return;
            }
            case argument_type::V:
            {
              u8* RegA = M->V + Instruction.Args[0].Value;
              u8* RegB = M->V + Instruction.Args[1].Value;
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
            case argument_type::V:
            {
              u8* Reg = M->V + Instruction.Args[1].Value;
              M->I += *Reg;
              return;
            }
          }
          break;
        }
        case argument_type::V:
        {
          u8* Reg = M->V + Instruction.Args[0].Value;
          switch(Instruction.Args[1].Type)
          {
            case argument_type::CONSTANT:
            {
              *Reg += (u8)Instruction.Args[1].Value;
              return;
            }
            case argument_type::V:
            {
              u8* OtherReg = M->V + Instruction.Args[1].Value;
              u16 Result = (u16)*Reg + (u16)*OtherReg;
              M->V[0xF] = (u8)(Result > 255);
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
      switch(Instruction.Args[0].Type)
      {
        case argument_type::V:
        {
          switch(Instruction.Args[1].Type)
          {
            case argument_type::V:
            {
              u8* RegA = M->V + Instruction.Args[0].Value;
              u8* RegB = M->V + Instruction.Args[1].Value;
              *RegA = *RegA | *RegB;
              return;
            }
          }
        break;
        }
      }
      break;
    }
    case instruction_type::AND:
    {
      switch(Instruction.Args[0].Type)
      {
        case argument_type::V:
        {
          switch(Instruction.Args[1].Type)
          {
            case argument_type::V:
            {
              u8* RegA = M->V + Instruction.Args[0].Value;
              u8* RegB = M->V + Instruction.Args[1].Value;
              *RegA = *RegA & *RegB;
              return;
            }
          }
        break;
        }
      }
      break;
    }
    case instruction_type::XOR:
    {
      switch(Instruction.Args[0].Type)
      {
        case argument_type::V:
        {
          switch(Instruction.Args[1].Type)
          {
            case argument_type::V:
            {
              u8* RegA = M->V + Instruction.Args[0].Value;
              u8* RegB = M->V + Instruction.Args[1].Value;
              *RegA = *RegA ^ *RegB;
              return;
            }
          }
        break;
        }
      }
      break;
    }
    case instruction_type::SUB:
    {
      switch(Instruction.Args[0].Type)
      {
        case argument_type::V:
        {
          switch(Instruction.Args[1].Type)
          {
            case argument_type::V:
            {
              u8* RegA = M->V + Instruction.Args[0].Value;
              u8* RegB = M->V + Instruction.Args[1].Value;
              M->V[0xF] = (*RegA > *RegB) ? 1 : 0;
              *RegA = *RegA - *RegB;
              return;
            }
          }
          break;
        }
      }
      break;
    }
    case instruction_type::SHR:
    {
      switch(Instruction.Args[0].Type)
      {
        case argument_type::V:
        {
          u8* RegA = M->V + Instruction.Args[0].Value;
          M->V[0xF] = *RegA & 0b0000'0001;
          *RegA /= 2;
          return;
        }
      }
      break;
    }
    case instruction_type::SUBN:
    {
      switch(Instruction.Args[0].Type)
      {
        case argument_type::V:
        {
          switch(Instruction.Args[1].Type)
          {
            case argument_type::V:
            {
              u8* RegA = M->V + Instruction.Args[0].Value;
              u8* RegB = M->V + Instruction.Args[1].Value;
              M->V[0xF] = (*RegB > *RegA) ? 1 : 0;
              *RegA = *RegB - *RegA;
              return;
            }
          }
          break;
        }
      }
      break;
    }
    case instruction_type::SHL:
    {
      switch(Instruction.Args[0].Type)
      {
        case argument_type::V:
        {
          u8* RegA = M->V + Instruction.Args[0].Value;
          M->V[0xF] = *RegA & 0b1000'0000;
          *RegA *= 2;
          return;
        }
      }
      break;
    }
    case instruction_type::RND:
    {
      MTB_NOT_IMPLEMENTED;
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
                case argument_type::CONSTANT:
                {
                  u8* RegA = M->V + Instruction.Args[0].Value;
                  u8* RegB = M->V + Instruction.Args[1].Value;
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
      MTB_NOT_IMPLEMENTED;
      break;
    }
    case instruction_type::SKNP:
    {
      MTB_NOT_IMPLEMENTED;
      break;
    }
    default: break;
  }

  MTB_INTERNAL_CODE( printf("Invalid instruction to execute.\n") );
}

#if COUSCOUS_ASSEMBLER
  static instruction_type
  ParseInstructionType(size_t CodeSize, char* Code)
  {
    instruction_type Result{};

    if(CodeSize >= 2)
    {
      switch(Code[0])
      {
        case 'A':
        {
          if(CodeSize == 3)
          {
            if(mtb_StringsAreEqual(CodeSize, "ADD", Code))
              Result = instruction_type::ADD;
            else if(mtb_StringsAreEqual(CodeSize, "AND", Code))
              Result = instruction_type::AND;
          }
          break;
        }

        case 'C':
        {
          if(CodeSize == 4 && mtb_StringsAreEqual(CodeSize, "CALL", Code))
            Result = instruction_type::CALL;
          else if(CodeSize == 3 && mtb_StringsAreEqual(CodeSize, "CLS", Code))
            Result = instruction_type::CLS;
          break;
        }

        case 'D':
        {
          if(CodeSize == 3 && mtb_StringsAreEqual(CodeSize, "DRW", Code))
            Result = instruction_type::DRW;
          break;
        }

        case 'J':
        {
          if(CodeSize == 2 && mtb_StringsAreEqual(CodeSize, "JP", Code))
            Result = instruction_type::JP;
          break;
        }

        case 'L':
        {
          if(CodeSize == 2 && mtb_StringsAreEqual(CodeSize, "LD", Code))
            Result = instruction_type::LD;
          break;
        }

        case 'O':
        {
          if(CodeSize == 2 && mtb_StringsAreEqual(CodeSize, "OR", Code))
            Result = instruction_type::OR;
          break;
        }

        case 'R':
        {
          if(CodeSize == 3)
          {
            if(mtb_StringsAreEqual(CodeSize, "RET", Code))
              Result = instruction_type::RET;
            else if(mtb_StringsAreEqual(CodeSize, "RND", Code))
              Result = instruction_type::RND;
          }
          break;
        }

        case 'S':
        {
          if(CodeSize == 2)
          {
            if(mtb_StringsAreEqual(3, "SE", Code))
              Result = instruction_type::SE;
          }
          else if(CodeSize == 3)
          {
            if(mtb_StringsAreEqual(CodeSize, "SHL", Code))
              Result = instruction_type::SHL;
            else if(mtb_StringsAreEqual(CodeSize, "SHR", Code))
              Result = instruction_type::SHR;
            else if(mtb_StringsAreEqual(CodeSize, "SKP", Code))
              Result = instruction_type::SKP;
            else if(mtb_StringsAreEqual(CodeSize, "SNE", Code))
              Result = instruction_type::SNE;
            else if(mtb_StringsAreEqual(CodeSize, "SUB", Code))
              Result = instruction_type::SUB;
            else if(mtb_StringsAreEqual(CodeSize, "SYS", Code))
              Result = instruction_type::SYS;
          }
          else if(CodeSize == 4)
          {
            if(mtb_StringsAreEqual(CodeSize, "SKNP", Code))
              Result = instruction_type::SKNP;
            else if(mtb_StringsAreEqual(CodeSize, "SUBN", Code))
              Result = instruction_type::SUBN;
          }
          break;
        }

        case 'X':
        {
          if(CodeSize == 3 && mtb_StringsAreEqual(CodeSize, "XOR", Code))
            Result = instruction_type::XOR;
          break;
        }

        default:
        {
          MTB_INVALID_CODE_PATH;
          break;
        }
      }
    }

    return Result;
  }

  static argument
  ParseArgument(size_t CodeSize, char* Code)
  {
    argument Result{};

    if(CodeSize > 0)
    {
      switch(Code[0])
      {
        case 'V':
        {
          if(CodeSize == 2)
          {
            switch(Code[1])
            {
              case '0': Result.Value = 0x0; break;
              case '1': Result.Value = 0x1; break;
              case '2': Result.Value = 0x2; break;
              case '3': Result.Value = 0x3; break;
              case '4': Result.Value = 0x4; break;
              case '5': Result.Value = 0x5; break;
              case '6': Result.Value = 0x6; break;
              case '7': Result.Value = 0x7; break;
              case '8': Result.Value = 0x8; break;
              case '9': Result.Value = 0x9; break;
              case 'A': Result.Value = 0xA; break;
              case 'B': Result.Value = 0xB; break;
              case 'C': Result.Value = 0xC; break;
              case 'D': Result.Value = 0xD; break;
              case 'E': Result.Value = 0xE; break;
              case 'F': Result.Value = 0xF; break;
              default: MTB_INVALID_CODE_PATH; break;
            }
          }
          break;
        }

        case 'I':
        {
          if(CodeSize == 1)
            Result.Type = argument_type::I;
          break;
        }

        case 'D':
        {
          if(CodeSize == 2 && mtb_StringsAreEqual(CodeSize, "DT", Code))
            Result.Type = argument_type::DT;
          break;
        }

        case 'S':
        {
          if(CodeSize == 2 && mtb_StringsAreEqual(CodeSize, "ST", Code))
            Result.Type = argument_type::ST;
          break;
        }

        case 'K':
        {
          if(CodeSize == 1)
            Result.Type = argument_type::K;
          break;
        }

        case 'F':
        {
          if(CodeSize == 1)
            Result.Type = argument_type::F;
          break;
        }

        case 'B':
        {
          if(CodeSize == 1)
            Result.Type = argument_type::B;
          break;
        }

        case '[':
        {
          if(CodeSize == 3)
          {
            if(mtb_StringsAreEqual(CodeSize, "[I]", Code))
              Result.Type = argument_type::ATI;
          }
          break;
        }

        default: // 0x123
        {
          Result.Type = argument_type::CONSTANT;

          unsigned int Value = 0;
          if(Code[0] == '0')
          {
            switch(Code[1])
            {
              case 'X': sscanf(Code + 2, "%3X", &Value); break;
              case 'B': MTB_NOT_IMPLEMENTED;
              case 'D': sscanf(Code + 2, "%3d", &Value);; break;
              default:  sscanf(Code + 1, "%3d", &Value);; break;
            }
          }
          else
          {
            sscanf(Code, "%3d", &Value);
          }

          Result.Value = (u16)Value;
        }
        break;
      }
    }

    return Result;
  }

  static instruction
  AssembleInstruction(assembler_code Code)
  {
    // Assume we are parsing "ABC D, E, F"

    instruction Result{};

    // Make it all uppercase.
    for(size_t CharIndex = 0; CharIndex < Code.Size; ++CharIndex)
    {
      char Char = Code.Data[CharIndex];
      if(Char >= 'a' && Char <= 'z')
        Char -= 0x20;
      Code.Data[CharIndex] = Char;
    }

    char* Current = Code.Data;
    char* OnePastLast = Current + Code.Size;

    // Skip whitespace
    while(Current < OnePastLast && mtb_IsWhitespace(*Current))
      ++Current;

    // Parse "ABC"
    char* NameStart = Current;
    while(Current < OnePastLast && !mtb_IsWhitespace(*Current))
      ++Current;
    char* NameOnePastLast = Current;

    Result.Type = ParseInstructionType(NameOnePastLast - NameStart, NameStart);

    int CurrentArgumentPos = 0;
    while(true)
    {
      // Skip whitespace
      while(Current < OnePastLast && mtb_IsWhitespace(*Current))
        ++Current;

      // Parse "D", then "E", then "F"
      char* ArgumentStart = Current;
      while(Current < OnePastLast && !mtb_IsWhitespace(*Current) && *Current != ',')
        ++Current;
      char* ArgumentOnePastLast = Current;
      Result.Args[CurrentArgumentPos] = ParseArgument(ArgumentOnePastLast - ArgumentStart, ArgumentStart);

      if(Current == OnePastLast)
        break;

      if(*Current == ',')
        ++Current;

      ++CurrentArgumentPos;
    }

    return Result;
  }

  static assembler_code
  DisassembleInstruction(instruction Instruction)
  {
    assembler_code Result{};
    if(Instruction.Type != instruction_type::INVALID)
    {
      char* BufferBegin = &Result.Data[0];
      char* BufferOnePastLast = BufferBegin + mtb_ArrayLengthOf(Result.Data);

      char const* InstructionToString[] =
      {
        "INVALID", // instruction_type::INVALID
        "CLS",     // instruction_type::CLS
        "RET",     // instruction_type::RET
        "SYS",     // instruction_type::SYS
        "JP",      // instruction_type::JP
        "CALL",    // instruction_type::CALL
        "SE",      // instruction_type::SE
        "SNE",     // instruction_type::SNE
        "LD",      // instruction_type::LD
        "ADD",     // instruction_type::ADD
        "OR",      // instruction_type::OR
        "AND",     // instruction_type::AND
        "XOR",     // instruction_type::XOR
        "SUB",     // instruction_type::SUB
        "SHR",     // instruction_type::SHR
        "SUBN",    // instruction_type::SUBN
        "SHL",     // instruction_type::SHL
        "RND",     // instruction_type::RND
        "DRW",     // instruction_type::DRW
        "SKP",     // instruction_type::SKP
        "SKNP",    // instruction_type::SKNP
      };

      MTB_AssertDebug(BufferBegin < BufferOnePastLast);
      BufferBegin += snprintf(BufferBegin, BufferOnePastLast - BufferBegin, "%s", InstructionToString[(int)Instruction.Type]);

      char const* Joiner = " ";
      for(argument& Arg : Instruction.Args)
      {
        if(Arg.Type == argument_type::NONE)
          break;

        MTB_AssertDebug(BufferBegin < BufferOnePastLast);
        BufferBegin += snprintf(BufferBegin, BufferOnePastLast - BufferBegin, "%s", Joiner);
        Joiner = ", ";

        MTB_AssertDebug(BufferBegin < BufferOnePastLast);
        switch(Arg.Type)
        {
          case argument_type::V: { BufferBegin += snprintf(BufferBegin, BufferOnePastLast - BufferBegin, "V%X", Arg.Value); break; }

          case argument_type::CONSTANT: { BufferBegin += snprintf(BufferBegin, BufferOnePastLast - BufferBegin, "0x%X", Arg.Value); break; }

          case argument_type::I: { BufferBegin += snprintf(BufferBegin, BufferOnePastLast - BufferBegin, "I");   break; }
          case argument_type::DT: { BufferBegin += snprintf(BufferBegin, BufferOnePastLast - BufferBegin, "DT");  break; }
          case argument_type::ST: { BufferBegin += snprintf(BufferBegin, BufferOnePastLast - BufferBegin, "ST");  break; }
          case argument_type::K: { BufferBegin += snprintf(BufferBegin, BufferOnePastLast - BufferBegin, "K");   break; }
          case argument_type::F: { BufferBegin += snprintf(BufferBegin, BufferOnePastLast - BufferBegin, "F");   break; }
          case argument_type::B: { BufferBegin += snprintf(BufferBegin, BufferOnePastLast - BufferBegin, "B");   break; }
          case argument_type::ATI: { BufferBegin += snprintf(BufferBegin, BufferOnePastLast - BufferBegin, "[I]"); break; }

          default:
          {
            MTB_INVALID_CODE_PATH;
            break;
          }
        }
      }

      MTB_AssertDebug(BufferBegin < BufferOnePastLast);

      Result.Size = BufferBegin - &Result.Data[0];
    }
    else
    {
      u16 RawInstruction = Instruction.Args[0].Value;
      int NumCharsWritten = snprintf(Result.Data, mtb_ArrayLengthOf(Result.Data), "<0x%04X>", RawInstruction);
      MTB_AssertDebug(NumCharsWritten == 3 + 4 + 1);
      Result.Size = (size_t)NumCharsWritten;
    }

    return Result;
  }
#endif // COUSCOUS_ASSEMBLER

#if defined(COUSCOUS_TESTS)
  #include "couscous_tests.cpp"
#endif
