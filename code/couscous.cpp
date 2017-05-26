#include "charmap.cpp"

//
// argument_type
//

char const*
GetArgumentTypeAsString(argument_type Value)
{
  char const* Result = nullptr;
  switch(Value)
  {
    case argument_type::NONE:     Result = "NONE"; break;
    case argument_type::V:        Result = "V"; break;
    case argument_type::I:        Result = "I"; break;
    case argument_type::DT:       Result = "DT"; break;
    case argument_type::ST:       Result = "ST"; break;
    case argument_type::K:        Result = "K"; break;
    case argument_type::F:        Result = "F"; break;
    case argument_type::B:        Result = "B"; break;
    case argument_type::ATI:      Result = "ATI"; break;
    case argument_type::CONSTANT: Result = "CONSTANT"; break;
    default:
      MTB_INVALID_CODE_PATH;
      break;
  }

  return Result;
}


argument_type
MakeArgumentTypeFromString(size_t CodeLen, char const* Code)
{
  #define MAKE_ARGUMENT_TYPE_FROM_STRING_CASE(EXPECTED) \
    if(mtb_StringsAreEqual(sizeof(#EXPECTED) - 1, Code, #EXPECTED)) \
    { \
      Result = argument_type::EXPECTED; \
      break; \
    }

  argument_type Result = {};
  if(CodeLen > 0)
  {
    switch(Code[0])
    {
      case 'N': MAKE_ARGUMENT_TYPE_FROM_STRING_CASE(NONE);     break;
      case 'V': MAKE_ARGUMENT_TYPE_FROM_STRING_CASE(V);        break;
      case 'I': MAKE_ARGUMENT_TYPE_FROM_STRING_CASE(I);        break;
      case 'D': MAKE_ARGUMENT_TYPE_FROM_STRING_CASE(DT);       break;
      case 'S': MAKE_ARGUMENT_TYPE_FROM_STRING_CASE(ST);       break;
      case 'K': MAKE_ARGUMENT_TYPE_FROM_STRING_CASE(K);        break;
      case 'F': MAKE_ARGUMENT_TYPE_FROM_STRING_CASE(F);        break;
      case 'B': MAKE_ARGUMENT_TYPE_FROM_STRING_CASE(B);        break;
      case 'A': MAKE_ARGUMENT_TYPE_FROM_STRING_CASE(ATI);      break;
      case 'C': MAKE_ARGUMENT_TYPE_FROM_STRING_CASE(CONSTANT); break;
      default: break;
    }
  }
  return Result;

  #undef MAKE_ARGUMENT_TYPE_FROM_STRING_CASE
}


//
// argument
//
size_t
GetArgumentAsString(argument Argument, size_t BufferSize, u8* Buffer)
{
  // Note(Manuzor): Use with care, this function is not tested...

  size_t Result = 0;

  if(BufferSize > 0) switch(Argument.Type)
  {
    case argument_type::V:
    {
      if(BufferSize > 1)
      {
        Buffer[0] = 'V';
        Buffer[1] = '0' + mtb_SafeConvert_u08(Argument.Value);
        Result = 2;
      }
    } break;

    case argument_type::I:
    {
      Buffer[0] = 'I';
      Result = 1;
    } break;

    case argument_type::DT:
    {
      if(BufferSize > 1)
      {
        Buffer[0] = 'D';
        Buffer[1] = 'T';
        Result = 2;
      }
    } break;

    case argument_type::ST:
    {
      if(BufferSize > 1)
      {
        Buffer[0] = 'S';
        Buffer[1] = 'T';
        Result = 2;
      }
    } break;

    case argument_type::K:
    {
      Buffer[0] = 'K';
      Result = 1;
    } break;

    case argument_type::F:
    {
      Buffer[0] = 'F';
      Result = 1;
    } break;

    case argument_type::B:
    {
      Buffer[0] = 'B';
      Result = 1;
    } break;

    case argument_type::ATI:
    {
      if(BufferSize > 2)
      {
        Buffer[0] = '[';
        Buffer[1] = 'I';
        Buffer[2] = ']';
        Result = 3;
      }
    } break;

    case argument_type::CONSTANT:
    {
      if(Argument.Value < 10)
      {
        Buffer[0] = '0' + (char)Argument.Value;
        Result = 1;
      }
      else if(Argument.Value < 100)
      {
        Buffer[0] = '0' + (char)(Argument.Value % 10);
        Buffer[1] = '0' + (char)(Argument.Value / 10);
        Result = 2;
      }
      else
      {
        MTB_AssertDebug(Argument.Value < 1000);
        Buffer[0] = '0' + (char)(Argument.Value % 10);
        Buffer[1] = '0' + (char)((Argument.Value / 10) % 10);
        Buffer[2] = '0' + (char)((Argument.Value / 10) / 10);
        Result = 3;
      }
    } break;

    default:
      MTB_INVALID_CODE_PATH;
      break;
  }

  return Result;
}

argument
MakeArgumentFromString(size_t CodeLen, char const* Code)
{
  argument Result{};

  if(CodeLen > 0)
  {
    switch(Code[0])
    {
      case 'V':
      {
        if(CodeLen == 2)
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
        if(CodeLen == 1)
          Result.Type = argument_type::I;
        break;
      }

      case 'D':
      {
        if(CodeLen == 2 && mtb_StringsAreEqual(CodeLen, "DT", Code))
          Result.Type = argument_type::DT;
        break;
      }

      case 'S':
      {
        if(CodeLen == 2 && mtb_StringsAreEqual(CodeLen, "ST", Code))
          Result.Type = argument_type::ST;
        break;
      }

      case 'K':
      {
        if(CodeLen == 1)
          Result.Type = argument_type::K;
        break;
      }

      case 'F':
      {
        if(CodeLen == 1)
          Result.Type = argument_type::F;
        break;
      }

      case 'B':
      {
        if(CodeLen == 1)
          Result.Type = argument_type::B;
        break;
      }

      case '[':
      {
        if(CodeLen == 3)
        {
          if(mtb_StringsAreEqual(CodeLen, "[I]", Code))
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


//
// instruction_type
//

char const*
GetInstructionTypeAsString(instruction_type Value)
{
  char const* Result = nullptr;
  switch(Value)
  {
    case instruction_type::INVALID: Result = "INVALID"; break;
    case instruction_type::CLS:     Result = "CLS"; break;
    case instruction_type::RET:     Result = "RET"; break;
    case instruction_type::SYS:     Result = "SYS"; break;
    case instruction_type::JP:      Result = "JP"; break;
    case instruction_type::CALL:    Result = "CALL"; break;
    case instruction_type::SE:      Result = "SE"; break;
    case instruction_type::SNE:     Result = "SNE"; break;
    case instruction_type::LD:      Result = "LD"; break;
    case instruction_type::ADD:     Result = "ADD"; break;
    case instruction_type::OR:      Result = "OR"; break;
    case instruction_type::AND:     Result = "AND"; break;
    case instruction_type::XOR:     Result = "XOR"; break;
    case instruction_type::SUB:     Result = "SUB"; break;
    case instruction_type::SHR:     Result = "SHR"; break;
    case instruction_type::SUBN:    Result = "SUBN"; break;
    case instruction_type::SHL:     Result = "SHL"; break;
    case instruction_type::RND:     Result = "RND"; break;
    case instruction_type::DRW:     Result = "DRW"; break;
    case instruction_type::SKP:     Result = "SKP"; break;
    case instruction_type::SKNP:    Result = "SKNP"; break;
    default:
      MTB_INVALID_CODE_PATH;
      break;
  }

  return Result;
}


instruction_type
MakeInstructionTypeFromString(size_t CodeLen, char const* Code)
{
  #define MAKE_INSTRUCTION_TYPE_FROM_STRING_CASE(EXPECTED) \
    if(mtb_StringsAreEqual(sizeof(#EXPECTED) - 1, Code, #EXPECTED)) \
    { \
      Result = instruction_type::EXPECTED; \
      break; \
    }

  instruction_type Result{};
  if(CodeLen > 0) switch(Code[0])
  {
    case 'A': MAKE_INSTRUCTION_TYPE_FROM_STRING_CASE(ADD);
              MAKE_INSTRUCTION_TYPE_FROM_STRING_CASE(AND);
              break;

    case 'C': MAKE_INSTRUCTION_TYPE_FROM_STRING_CASE(CALL);
              MAKE_INSTRUCTION_TYPE_FROM_STRING_CASE(CLS);
              break;

    case 'D': MAKE_INSTRUCTION_TYPE_FROM_STRING_CASE(DRW);
              break;

    case 'I': MAKE_INSTRUCTION_TYPE_FROM_STRING_CASE(INVALID);
              break;

    case 'J': MAKE_INSTRUCTION_TYPE_FROM_STRING_CASE(JP);
              break;

    case 'L': MAKE_INSTRUCTION_TYPE_FROM_STRING_CASE(LD);
              break;

    case 'O': MAKE_INSTRUCTION_TYPE_FROM_STRING_CASE(OR);
              break;

    case 'R': MAKE_INSTRUCTION_TYPE_FROM_STRING_CASE(RET);
              MAKE_INSTRUCTION_TYPE_FROM_STRING_CASE(RND);
              break;

    case 'S':
    {
      if(CodeLen > 1) switch(Code[1])
      {
        case 'E': MAKE_INSTRUCTION_TYPE_FROM_STRING_CASE(SE);
                  break;

        case 'H': MAKE_INSTRUCTION_TYPE_FROM_STRING_CASE(SHL);
                  MAKE_INSTRUCTION_TYPE_FROM_STRING_CASE(SHR);
                  break;

        case 'K': MAKE_INSTRUCTION_TYPE_FROM_STRING_CASE(SKNP);
                  MAKE_INSTRUCTION_TYPE_FROM_STRING_CASE(SKP);
                  break;

        case 'N': MAKE_INSTRUCTION_TYPE_FROM_STRING_CASE(SNE);
                  break;

        case 'U': MAKE_INSTRUCTION_TYPE_FROM_STRING_CASE(SUB);
                  MAKE_INSTRUCTION_TYPE_FROM_STRING_CASE(SUBN);
                  break;

        case 'Y': MAKE_INSTRUCTION_TYPE_FROM_STRING_CASE(SYS);
                  break;

        default: break;
      }
    } break;

    case 'X': MAKE_INSTRUCTION_TYPE_FROM_STRING_CASE(XOR);
              break;

    default: break;
  }

  return Result;

  #undef MAKE_INSTRUCTION_TYPE_FROM_STRING_CASE
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
      mtb_SetBytes(mtb_ArrayByteSizeOf(M->Screen), M->Screen, 0);
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
              u16 Range = Instruction.Args[1].Value;
              u8* Dest = M->Memory + M->I;
              u8* Source = M->V + 0;
              mtb_CopyBytes(Range, Dest, Source);
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
              u8* HundredsDigit = M->Memory + (M->I + 0);
              u8* TensDigit = M->Memory + (M->I + 1);
              u8* SingleDigit = M->Memory + (M->I + 2);

              *HundredsDigit = (*Reg / 100);
              *TensDigit = (*Reg / 10) % 10;
              *SingleDigit = *Reg % 10;
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
              u8* Dest = M->V + 0;
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
              // Note(Manuzor): All execution stops until a key is pressed, then the
              // value of that key is stored in Reg.
              M->RequiredInputRegisterIndexPlusOne = mtb_SafeConvert_u08(Instruction.Args[0].Value + 1);
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
      switch (Instruction.Args[0].Type)
      {
        case argument_type::V:
        {
          switch (Instruction.Args[1].Type)
          {
            case argument_type::CONSTANT:
            {
              u8* Reg = M->V + Instruction.Args[0].Value;
              u8 Byte = (u8)Instruction.Args[1].Value;
              u8 Rand = (u8)mtb_RandomBetween_u32(&M->RNG, 0, 255);
              *Reg = Byte & Rand;
              break;
            }
          }
          break;
        }
      }
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
      switch(Instruction.Args[0].Type)
      {
        case argument_type::V:
        {
          u8 KeyIndex = (u8)Instruction.Args[0].Value;
          if(IsKeyDown(M->InputState, KeyIndex))
          {
            M->ProgramCounter += 2;
          }
        } break;
      }
      break;
    }
    case instruction_type::SKNP:
    {
      switch(Instruction.Args[0].Type)
      {
        case argument_type::V:
        {
          u8 KeyIndex = (u8)Instruction.Args[0].Value;
          if(!IsKeyDown(M->InputState, KeyIndex))
          {
            M->ProgramCounter += 2;
          }
        } break;
      }
      break;
    }
    default: break;
  }

  MTB_INTERNAL_CODE( printf("Invalid instruction to execute.\n") );
}

u16
MapCharToKeyIndex(char Key)
{
  u16 Result = (u16)-1;

  switch(Key)
  {
    case '0': Result =  0; break;
    case '1': Result =  1; break;
    case '2': Result =  2; break;
    case '3': Result =  3; break;
    case '4': Result =  4; break;
    case '5': Result =  5; break;
    case '6': Result =  6; break;
    case '7': Result =  7; break;
    case '8': Result =  8; break;
    case '9': Result =  9; break;
    case 'A': Result = 10; break;
    case 'B': Result = 11; break;
    case 'C': Result = 12; break;
    case 'D': Result = 13; break;
    case 'E': Result = 14; break;
    case 'F': Result = 15; break;
    default: break;
  }

  return Result;
}

char
MapKeyIndexToChar(u16 KeyIndex)
{
  char Result = 0;
  if(KeyIndex < 16)
  {
    char const* Map = "0123456789ABCDEF";
    Result = Map[KeyIndex];
  }
  return Result;
}

bool
IsKeyDown(u16 InputState, u16 KeyIndex)
{
  bool Result = mtb_IsBitSet(InputState, KeyIndex);

  return Result;
}

u16
SetKeyDown(u16 InputState, u16 KeyIndex, bool32 IsDown)
{
  u16 Result;
  // TODO: Check this code.
  if(IsDown) Result = mtb_SetBit(InputState, KeyIndex);
  else       Result = mtb_UnsetBit(InputState, KeyIndex);

  return Result;
}

#if COUSCOUS_ASSEMBLER
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

    Result.Type = MakeInstructionTypeFromString(NameOnePastLast - NameStart, NameStart);

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
      Result.Args[CurrentArgumentPos] = MakeArgumentFromString(ArgumentOnePastLast - ArgumentStart, ArgumentStart);

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

      MTB_AssertDebug(BufferBegin < BufferOnePastLast);
      BufferBegin += snprintf(BufferBegin, BufferOnePastLast - BufferBegin, "%s", GetInstructionTypeAsString(Instruction.Type));

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
