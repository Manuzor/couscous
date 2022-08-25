#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch"

//
// argument_type
//

char const*
GetArgumentTypeAsString(argument_type Value)
{
    char const* Result = nullptr;
    switch (Value)
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
            MTB_ASSERT(false);
            break;
    }

    return Result;
}


argument_type
MakeArgumentTypeFromString(size_t CodeLen, char const* Code)
{
#define MAKE_ARGUMENT_TYPE_FROM_STRING_CASE(EXPECTED) \
    if(mtb::string::StringEquals(CodeSlice, mtb::string::ConstZ(#EXPECTED))) \
    { \
      Result = argument_type::EXPECTED; \
      break; \
    }

    mtb::tSlice<char const> CodeSlice = mtb::PtrSlice(Code, CodeLen);
    argument_type Result = {};
    if (CodeLen > 0)
    {
        switch (Code[0])
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
        }
    }
    return Result;

#undef MAKE_ARGUMENT_TYPE_FROM_STRING_CASE
}


//
// argument
//
char
ToHexChar(u16 Value)
{
    MTB_ASSERT(Value <= 0xF);

    char Result;
    if (Value < 10)
        Result = '0' + (char)Value;
    else
        Result = 'A' + (char)(Value - 10);

    return Result;
}

size_t
GetArgumentAsString(argument Argument, size_t BufferSize, u8* Buffer)
{
    // Note(Manuzor): Use with care, this function is not tested...

    MTB_ASSERT(BufferSize >= 5);
    size_t Result = 0;

    if (BufferSize > 0) switch (Argument.Type)
    {
        case argument_type::V:
        {
            if (BufferSize > 1)
            {
                Buffer[0] = 'V';
                Buffer[1] = '0' + mtb::IntCast<u8>(Argument.Value);
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
            if (BufferSize > 1)
            {
                Buffer[0] = 'D';
                Buffer[1] = 'T';
                Result = 2;
            }
        } break;

        case argument_type::ST:
        {
            if (BufferSize > 1)
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
            if (BufferSize > 2)
            {
                Buffer[0] = '[';
                Buffer[1] = 'I';
                Buffer[2] = ']';
                Result = 3;
            }
        } break;

        case argument_type::CONSTANT:
        {
            if (Argument.Value <= 0xFFF)
            {
#define COUSCOUS_HEX_OUTPUT 1
#if COUSCOUS_HEX_OUTPUT
                Buffer[0] = '0';
                Buffer[1] = 'x';
                Buffer[2] = ToHexChar((Argument.Value >> 8) & 0xF);
                Buffer[3] = ToHexChar((Argument.Value >> 4) & 0xF);
                Buffer[4] = ToHexChar((Argument.Value >> 0) & 0xF);
                Result = 5;
#else
                Result = (size_t)snprintf((char*)Buffer, BufferSize, "%d", Argument.Value);
#endif
            }
            else
            {
                MTB_ASSERT(false);
            }
        } break;

        default:
            MTB_ASSERT(false);
            break;
    }

    return Result;
}

argument
MakeArgumentFromString(size_t CodeLen, char const* CodeInput)
{
    using mtb::PtrSlice;
    using mtb::string::ConstZ;

    argument Result{};

    if (CodeLen > 0)
    {
        if (CodeLen <= 3)
        {
            char Code[3]{};
            for (size_t CharIndex = 0; CharIndex < CodeLen; ++CharIndex)
                Code[CharIndex] = CodeInput[CharIndex];
            ToUpper(str{ (int)CodeLen, Code });

            switch (Code[0])
            {
                case 'V':
                {
                    if (CodeLen == 2)
                    {
                        Result.Type = argument_type::V;
                        switch (Code[1])
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
                            default: MTB_ASSERT(false); break;
                        }
                    }
                    break;
                }

                case 'I':
                {
                    if (CodeLen == 1)
                        Result.Type = argument_type::I;
                    break;
                }

                case 'D':
                {
                    if (CodeLen == 2 && mtb::string::StringEquals(PtrSlice(Code, CodeLen), ConstZ("DT")))
                        Result.Type = argument_type::DT;
                    break;
                }

                case 'S':
                {
                    if (CodeLen == 2 && mtb::string::StringEquals(PtrSlice(Code, CodeLen), ConstZ("ST")))
                        Result.Type = argument_type::ST;
                    break;
                }

                case 'K':
                {
                    if (CodeLen == 1)
                        Result.Type = argument_type::K;
                    break;
                }

                case 'F':
                {
                    if (CodeLen == 1)
                        Result.Type = argument_type::F;
                    break;
                }

                case 'B':
                {
                    if (CodeLen == 1)
                        Result.Type = argument_type::B;
                    break;
                }

                case '[':
                {
                    if (CodeLen == 3)
                    {
                        if (mtb::string::StringEquals(PtrSlice(Code, 3), ConstZ("[I]")))
                            Result.Type = argument_type::ATI;
                    }
                    break;
                }

                break;
            }
        }

        if (Result.Type == argument_type::NONE)
        {
            Result.Type = argument_type::CONSTANT;

            unsigned int Value = 0;
            if (CodeInput[0] == '0')
            {
                switch (CodeInput[1])
                {
                    case 'X': case 'x': sscanf(CodeInput + 2, "%X", &Value); break;
                    case 'B': case 'b': MTB_ASSERT(!"not implemented"); break;
                    case 'D': case 'd': sscanf(CodeInput + 2, "%d", &Value); break;
                    default: sscanf(CodeInput + 1, "%d", &Value); break;
                }
            }
            else
            {
                sscanf(CodeInput, "%d", &Value);
            }

            Result.Value = (u16)Value;
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
    switch (Value)
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
            MTB_ASSERT(false);
            break;
    }

    return Result;
}


// Note: The caller must guarantee that Code is uppercase.
instruction_type
MakeInstructionTypeFromString(size_t CodeLen, char const* CodeInput)
{
#define MAKE_INSTRUCTION_TYPE_FROM_STRING_CASE(EXPECTED) \
    if(mtb::string::StringEquals(CodeSlice, mtb::string::ConstZ(#EXPECTED))) \
    { \
      Result = instruction_type::EXPECTED; \
      break; \
    }

    mtb::tSlice<char const> CodeSlice = mtb::PtrSlice(CodeInput, CodeLen);
    instruction_type Result{};
    if (CodeLen > 0 && CodeLen <= 4)
    {
        char Code[4]{};
        for (size_t CharIndex = 0; CharIndex < CodeLen; ++CharIndex)
            Code[CharIndex] = CodeInput[CharIndex];
        ToUpper(str{ (int)CodeLen, Code });

        switch (Code[0])
        {
            case 'A': MAKE_INSTRUCTION_TYPE_FROM_STRING_CASE(ADD);
                MAKE_INSTRUCTION_TYPE_FROM_STRING_CASE(AND);
                break;

            case 'C': MAKE_INSTRUCTION_TYPE_FROM_STRING_CASE(CALL);
                MAKE_INSTRUCTION_TYPE_FROM_STRING_CASE(CLS);
                break;

            case 'D': MAKE_INSTRUCTION_TYPE_FROM_STRING_CASE(DRW);
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
                if (CodeLen > 1) switch (Code[1])
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
                }
            } break;

            case 'X': MAKE_INSTRUCTION_TYPE_FROM_STRING_CASE(XOR);
                break;
        }
    }

    return Result;

#undef MAKE_INSTRUCTION_TYPE_FROM_STRING_CASE
}

u16
GetDigitSpriteAddress(machine* M, u8 Digit)
{
    u16 Result = (u16)CHAR_MEMORY_OFFSET + (5 * (u16)Digit);
    return Result;
}

void
DrawSprite(machine* M, int StartX, int StartY, sprite Sprite)
{
    MTB_ASSERT(Sprite.Length <= 15); // As per 2.4 "Chip-8 sprites may be up to 15 bytes, [...]"

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

            bool32 SpriteColor = IsBitSet((u32)*SpritePixel, 7 - SpriteX);
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

bool
IsLittleEndian()
{
    uint16_t Test = 0x0001;
    return ((uint8_t const*)&Test)[0] == 0x01;
}

u16
ReadWord(void* Ptr)
{
    u16 Result = *(u16*)Ptr;
    if(IsLittleEndian()) {
        Result = (u16)((Result << 8) | (Result >> 8));
    }
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
    if(IsLittleEndian()) {
        Value = (u16)((Value << 8) | (Value >> 8));
    }
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
    text DisassembledInstruction = DisassembleInstruction(Instruction);
    printf("[%llu] " STR_FMT " (0x%04X)\n", M->CurrentCycle, STR_FMTARG(DisassembledInstruction), Decoder.Data);
#endif

    if (Instruction.Type != instruction_type::INVALID)
    {
        // Advance the program counter.
        M->ProgramCounter += 2;

        // Execute the fetched instruction.
        ExecuteInstruction(M, Instruction);
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

    switch (Decoder.Group)
    {
        case 0x0:
        {
            switch (Decoder.Data)
            {
                case 0x0000: // NULL instruction?
                    break;
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
            switch (Decoder.LSN)
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
            Result.Type = inst::JP;
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
            switch (Decoder.LSB)
            {
                case 0x9E: Result.Type = inst::SKP;  break; // Ex9E - SKP Vx
                case 0xA1: Result.Type = inst::SKNP; break; // ExA1 - SKNP Vx
            }
            break;
        }
        case 0xF:
        {
            switch (Decoder.LSB)
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
    if (Result.Type == instruction_type::INVALID)
        Result.Args[0].Value = Decoder.Data;

    return Result;
}

u16
EncodeInstruction(instruction Instruction)
{
    instruction_decoder Decoder{};

    switch (Instruction.Type)
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
            switch (Instruction.Args[0].Type)
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
            switch (Instruction.Args[0].Type)
            {
                case argument_type::V:
                {
                    Decoder.Group = 0xB;
                    Decoder.Address = Instruction.Args[0].Value;
                    break;
                }
                case argument_type::CONSTANT:
                {
                    Decoder.Group = 0x1;
                    Decoder.Address = Instruction.Args[0].Value;
                    break;
                }
            }
            break;
        }

        case instruction_type::CALL:
        {
            switch (Instruction.Args[0].Type)
            {
                case argument_type::CONSTANT:
                {
                    Decoder.Group = 0x2;
                    Decoder.Address = Instruction.Args[0].Value;
                    break;
                }
            }
            break;
        }

        case instruction_type::SE:
        {
            switch (Instruction.Args[0].Type)
            {
                case argument_type::V:
                {
                    Decoder.X = Instruction.Args[0].Value;
                    switch (Instruction.Args[1].Type)
                    {
                        case argument_type::V:
                        {
                            Decoder.Group = 0x5;
                            Decoder.Y = Instruction.Args[1].Value;
                            break;
                        }
                        case argument_type::CONSTANT:
                        {
                            Decoder.Group = 0x3;
                            Decoder.LSB = (u8)Instruction.Args[1].Value;
                            break;
                        }
                    }
                    break;
                }
            }
            break;
        }

        case instruction_type::SNE:
        {
            MTB_ASSERT(Instruction.Args[0].Type == argument_type::V);
            Decoder.X = Instruction.Args[0].Value;
            switch (Instruction.Args[1].Type)
            {
                case argument_type::V:
                {
                    Decoder.Group = 0x9;
                    Decoder.Y = Instruction.Args[1].Value;
                    break;
                }
                case argument_type::CONSTANT:
                {
                    Decoder.Group = 0x4;
                    Decoder.LSB = (u8)Instruction.Args[1].Value;
                    break;
                }
            }
            break;
        }

        case instruction_type::LD:
        {
            switch (Instruction.Args[0].Type)
            {
                case argument_type::ATI:
                {
                    switch (Instruction.Args[1].Type)
                    {
                        case argument_type::V:
                        {
                            Decoder.Group = 0xF;
                            Decoder.X = Instruction.Args[1].Value;
                            Decoder.LSB = 0x55;
                            break;
                        }
                    }
                    break;
                }
                case argument_type::B:
                {
                    switch (Instruction.Args[1].Type)
                    {
                        case argument_type::V:
                        {
                            Decoder.Group = 0xF;
                            Decoder.X = Instruction.Args[1].Value;
                            Decoder.LSB = 0x33;
                            break;
                        }
                    }
                    break;
                }
                case argument_type::DT:
                {
                    switch (Instruction.Args[1].Type)
                    {
                        case argument_type::V:
                        {
                            Decoder.Group = 0xF;
                            Decoder.X = Instruction.Args[1].Value;
                            Decoder.LSB = 0x15;
                            break;
                        }
                    }
                    break;
                }
                case argument_type::F:
                {
                    switch (Instruction.Args[1].Type)
                    {
                        case argument_type::V:
                        {
                            Decoder.Group = 0xF;
                            Decoder.X = Instruction.Args[1].Value;
                            Decoder.LSB = 0x29;
                            break;
                        }
                    }
                    break;
                }
                case argument_type::I:
                {
                    switch (Instruction.Args[1].Type)
                    {
                        case argument_type::CONSTANT:
                        {
                            Decoder.Group = 0xA;
                            Decoder.Address = Instruction.Args[1].Value;
                            break;
                        }
                    }
                    break;
                }
                case argument_type::ST:
                {
                    switch (Instruction.Args[1].Type)
                    {
                        case argument_type::V:
                        {
                            Decoder.Group = 0xF;
                            Decoder.X = Instruction.Args[1].Value;
                            Decoder.LSB = 0x18;
                            break;
                        }
                    }
                    break;
                }
                case argument_type::V:
                {
                    switch (Instruction.Args[1].Type)
                    {
                        case argument_type::ATI:
                        {
                            Decoder.Group = 0xF;
                            Decoder.X = Instruction.Args[0].Value;
                            Decoder.LSB = 0x65;
                            break;
                        }
                        case argument_type::CONSTANT:
                        {
                            Decoder.Group = 0x6;
                            Decoder.X = Instruction.Args[0].Value;
                            Decoder.LSB = Instruction.Args[1].Value;
                            break;
                        }
                        case argument_type::DT:
                        {
                            Decoder.Group = 0xF;
                            Decoder.X = Instruction.Args[0].Value;
                            Decoder.LSB = 0x07;
                            break;
                        }
                        case argument_type::K:
                        {
                            Decoder.Group = 0xF;
                            Decoder.X = Instruction.Args[0].Value;
                            Decoder.LSB = 0x0A;
                            break;
                        }
                        case argument_type::V:
                        {
                            Decoder.Group = 0x8;
                            Decoder.X = Instruction.Args[0].Value;
                            Decoder.Y = Instruction.Args[1].Value;
                            break;
                        }
                    }
                    break;
                }
            }
            break;
        }

        case instruction_type::ADD:
        {
            switch (Instruction.Args[0].Type)
            {
                case argument_type::I:
                {
                    switch (Instruction.Args[1].Type)
                    {
                        case argument_type::V:
                        {
                            Decoder.Group = 0xF;
                            Decoder.X = Instruction.Args[1].Value;
                            Decoder.LSB = 0x1E;
                            break;
                        }
                    }
                    break;
                }
                case argument_type::V:
                {
                    switch (Instruction.Args[1].Type)
                    {
                        case argument_type::CONSTANT:
                        {
                            Decoder.Group = 0x7;
                            Decoder.X = Instruction.Args[0].Value;
                            Decoder.LSB = Instruction.Args[1].Value;
                            break;
                        }
                        case argument_type::V:
                        {
                            Decoder.Group = 0x8;
                            Decoder.X = Instruction.Args[0].Value;
                            Decoder.Y = Instruction.Args[1].Value;
                            Decoder.LSN = 0x4;
                            break;
                        }
                    }
                    break;
                }
            }
            break;
        }

        case instruction_type::OR:
        {
            switch (Instruction.Args[0].Type)
            {
                case argument_type::V:
                {
                    switch (Instruction.Args[1].Type)
                    {
                        case argument_type::V:
                        {
                            Decoder.Group = 0x8;
                            Decoder.X = Instruction.Args[0].Value;
                            Decoder.Y = Instruction.Args[1].Value;
                            Decoder.LSN = 0x1;
                            break;
                        }
                    }
                    break;
                }
            }
            break;
        }

        case instruction_type::AND:
        {
            switch (Instruction.Args[0].Type)
            {
                case argument_type::V:
                {
                    switch (Instruction.Args[1].Type)
                    {
                        case argument_type::V:
                        {
                            Decoder.Group = 0x8;
                            Decoder.X = Instruction.Args[0].Value;
                            Decoder.Y = Instruction.Args[1].Value;
                            Decoder.LSN = 0x2;
                            break;
                        }
                    }
                    break;
                }
            }
            break;
        }

        case instruction_type::XOR:
        {
            switch (Instruction.Args[0].Type)
            {
                case argument_type::V:
                {
                    switch (Instruction.Args[1].Type)
                    {
                        case argument_type::V:
                        {
                            Decoder.Group = 0x8;
                            Decoder.X = Instruction.Args[0].Value;
                            Decoder.Y = Instruction.Args[1].Value;
                            Decoder.LSN = 0x3;
                            break;
                        }
                    }
                    break;
                }
            }
            break;
        }

        case instruction_type::SUB:
        {
            switch (Instruction.Args[0].Type)
            {
                case argument_type::V:
                {
                    switch (Instruction.Args[1].Type)
                    {
                        case argument_type::V:
                        {
                            Decoder.Group = 0x8;
                            Decoder.X = Instruction.Args[0].Value;
                            Decoder.Y = Instruction.Args[1].Value;
                            Decoder.LSN = 0x5;
                            break;
                        }
                    }
                    break;
                }
            }
            break;
        }

        case instruction_type::SHR:
        {
            switch (Instruction.Args[0].Type)
            {
                case argument_type::V:
                {
                    Decoder.Group = 0x8;
                    Decoder.X = Instruction.Args[0].Value;
                    if (Instruction.Args[1].Type == argument_type::V)
                        Decoder.Y = Instruction.Args[1].Value;
                    Decoder.LSN = 0x6;
                    break;
                }
            }
            break;
        }

        case instruction_type::SUBN:
        {
            switch (Instruction.Args[0].Type)
            {
                case argument_type::V:
                {
                    switch (Instruction.Args[1].Type)
                    {
                        case argument_type::V:
                        {
                            Decoder.Group = 0x8;
                            Decoder.X = Instruction.Args[0].Value;
                            Decoder.Y = Instruction.Args[1].Value;
                            Decoder.LSN = 0x7;
                            break;
                        }
                    }
                    break;
                }
            }
            break;
        }

        case instruction_type::SHL:
        {
            switch (Instruction.Args[0].Type)
            {
                case argument_type::V:
                {
                    Decoder.Group = 0x8;
                    Decoder.X = Instruction.Args[0].Value;
                    if (Instruction.Args[1].Type == argument_type::V)
                        Decoder.Y = Instruction.Args[1].Value;
                    Decoder.LSN = 0xE;
                    break;
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
                            Decoder.Group = 0xC;
                            Decoder.X = Instruction.Args[0].Value;
                            Decoder.LSB = Instruction.Args[1].Value;
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
            switch (Instruction.Args[0].Type)
            {
                case argument_type::V:
                {
                    switch (Instruction.Args[1].Type)
                    {
                        case argument_type::V:
                        {
                            switch (Instruction.Args[2].Type)
                            {
                                case argument_type::CONSTANT:
                                {
                                    Decoder.Group = 0xD;
                                    Decoder.X = Instruction.Args[0].Value;
                                    Decoder.Y = Instruction.Args[1].Value;
                                    Decoder.LSN = Instruction.Args[2].Value;
                                    break;
                                }
                            }
                            break;
                        }
                    }
                    break;
                }
            }
            break;
        }

        case instruction_type::SKP:
        {
            switch (Instruction.Args[0].Type)
            {
                case argument_type::V:
                {
                    Decoder.Group = 0xE;
                    Decoder.X = Instruction.Args[0].Value;
                    Decoder.LSB = 0x9E;
                    break;
                }
            }
            break;
        }

        case instruction_type::SKNP:
        {
            switch (Instruction.Args[0].Type)
            {
                case argument_type::V:
                {
                    Decoder.Group = 0xE;
                    Decoder.X = Instruction.Args[0].Value;
                    Decoder.LSB = 0xA1;
                    break;
                }
            }
            break;
        }
    }

    return Decoder.Data;
}

void
ExecuteInstruction(machine* M, instruction Instruction)
{
    switch (Instruction.Type)
    {
        case instruction_type::CLS:
        {
            mtb::SliceSetZero(mtb::ArraySlice(M->Screen));
        } return;

        case instruction_type::RET:
        {
            if (M->StackPointer > 0)
            {
                M->ProgramCounter = M->Stack[--M->StackPointer];
            }
        } return;

        case instruction_type::SYS:
        {
            MTB_ASSERT(!"not implemented");
        } return;

        case instruction_type::JP:
        {
            switch (Instruction.Args[0].Type)
            {
                case argument_type::V:
                {
                    u8* Reg = M->V + Instruction.Args[0].Value;
                    M->ProgramCounter = Instruction.Args[1].Value + *Reg;
                } return;

                case argument_type::CONSTANT:
                {
                    M->ProgramCounter = Instruction.Args[0].Value;
                } return;
            }
        } break;

        case instruction_type::CALL:
        {
            switch (Instruction.Args[0].Type)
            {
                case argument_type::CONSTANT:
                {
                    M->Stack[M->StackPointer++] = M->ProgramCounter;
                    M->ProgramCounter = Instruction.Args[0].Value;
                } return;
            }
        } break;

        case instruction_type::SE:
        {
            switch (Instruction.Args[0].Type)
            {
                case argument_type::V:
                {
                    u8 Lhs = M->V[Instruction.Args[0].Value];
                    switch (Instruction.Args[1].Type)
                    {
                        case argument_type::V:
                        {
                            u8 Rhs = M->V[Instruction.Args[1].Value];
                            if (Lhs == Rhs)
                                M->ProgramCounter += 2;
                        } return;

                        case argument_type::CONSTANT:
                        {
                            u8 Rhs = (u8)Instruction.Args[1].Value;
                            if (Lhs == Rhs)
                                M->ProgramCounter += 2;
                        } return;
                    }
                } break;
            }
        } break;

        case instruction_type::SNE:
        {
            switch (Instruction.Args[0].Type)
            {
                case argument_type::V:
                {
                    switch (Instruction.Args[1].Type)
                    {
                        case argument_type::V:
                        {
                            u8 Lhs = M->V[Instruction.Args[0].Value];
                            u8 Rhs = M->V[Instruction.Args[1].Value];
                            if (Lhs != Rhs)
                                M->ProgramCounter += 2;
                        } return;

                        case argument_type::CONSTANT:
                        {
                            u8 Lhs = M->V[Instruction.Args[0].Value];
                            u8 Rhs = (u8)Instruction.Args[1].Value;
                            if (Lhs != Rhs)
                                M->ProgramCounter += 2;
                        } return;
                    }
                } return;
            }
        } break;

        case instruction_type::LD:
        {
            switch (Instruction.Args[0].Type)
            {
                case argument_type::ATI:
                {
                    switch (Instruction.Args[1].Type)
                    {
                        case argument_type::V:
                        {
                            u16 Range = Instruction.Args[1].Value;
                            u8* Dest = M->Memory + M->I;
                            u8* Source = M->V + 0;
                            mtb::CopyBytes(Dest, Source, Range);
                        } return;
                    }
                } break;

                case argument_type::B:
                {
                    switch (Instruction.Args[1].Type)
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
                        } return;
                    }
                } break;

                case argument_type::DT:
                {
                    switch (Instruction.Args[1].Type)
                    {
                        case argument_type::V:
                        {
                            u8* Reg = M->V + Instruction.Args[1].Value;
                            M->DT = *Reg;
                        } return;
                    }
                } break;

                case argument_type::F:
                {
                    switch (Instruction.Args[1].Type)
                    {
                        case argument_type::V:
                        {
                            u8* Reg = M->V + Instruction.Args[1].Value;
                            M->I = GetDigitSpriteAddress(M, *Reg);
                        } return;
                    }
                } break;

                case argument_type::I:
                {
                    switch (Instruction.Args[1].Type)
                    {
                        case argument_type::CONSTANT:
                        {
                            M->I = Instruction.Args[1].Value;
                        } return;
                    }
                } break;

                case argument_type::ST:
                {
                    switch (Instruction.Args[1].Type)
                    {
                        case argument_type::V:
                        {
                            u8* Reg = M->V + Instruction.Args[1].Value;
                            M->ST = *Reg;
                        } return;
                    }
                } break;

                case argument_type::V:
                {
                    switch (Instruction.Args[1].Type)
                    {
                        case argument_type::ATI:
                        {
                            u8 Num = (u8)Instruction.Args[0].Value;
                            u8* Dest = M->V + 0;
                            u8* Source = M->Memory + M->I;
                            mtb::CopyBytes(Dest, Source, Num);
                        } return;

                        case argument_type::CONSTANT:
                        {
                            u8* Reg = M->V + Instruction.Args[0].Value;
                            *Reg = (u8)Instruction.Args[1].Value;
                        } return;

                        case argument_type::DT:
                        {
                            u8* Reg = M->V + Instruction.Args[0].Value;
                            *Reg = M->DT;
                        } return;

                        case argument_type::K:
                        {
                            // Note(Manuzor): All execution stops until a key is pressed, then the
                            // value of that key is stored in Reg.
                            M->RequiredInputRegisterIndexPlusOne = mtb::IntCast<u8>(Instruction.Args[0].Value + 1);
                        } return;

                        case argument_type::V:
                        {
                            u8* RegA = M->V + Instruction.Args[0].Value;
                            u8* RegB = M->V + Instruction.Args[1].Value;
                            *RegA = *RegB;
                        } return;
                    }
                } break;
            }
        } break;

        case instruction_type::ADD:
        {
            switch (Instruction.Args[0].Type)
            {
                case argument_type::I:
                {
                    switch (Instruction.Args[1].Type)
                    {
                        case argument_type::V:
                        {
                            u8* Reg = M->V + Instruction.Args[1].Value;
                            M->I += *Reg;
                        } return;
                    }
                } break;

                case argument_type::V:
                {
                    u8* Reg = M->V + Instruction.Args[0].Value;
                    switch (Instruction.Args[1].Type)
                    {
                        case argument_type::CONSTANT:
                        {
                            *Reg += (u8)Instruction.Args[1].Value;
                        } return;

                        case argument_type::V:
                        {
                            u8* OtherReg = M->V + Instruction.Args[1].Value;
                            u16 Result = (u16)*Reg + (u16)*OtherReg;
                            M->V[0xF] = (u8)(Result > 255);
                            *Reg = (u8)(Result & 0xFF);
                        } return;
                    }
                } break;
            }
        } break;

        case instruction_type::OR:
        {
            switch (Instruction.Args[0].Type)
            {
                case argument_type::V:
                {
                    switch (Instruction.Args[1].Type)
                    {
                        case argument_type::V:
                        {
                            u8* RegA = M->V + Instruction.Args[0].Value;
                            u8* RegB = M->V + Instruction.Args[1].Value;
                            *RegA = *RegA | *RegB;
                        } return;
                    }
                } break;
            }
        } break;

        case instruction_type::AND:
        {
            switch (Instruction.Args[0].Type)
            {
                case argument_type::V:
                {
                    switch (Instruction.Args[1].Type)
                    {
                        case argument_type::V:
                        {
                            u8* RegA = M->V + Instruction.Args[0].Value;
                            u8* RegB = M->V + Instruction.Args[1].Value;
                            *RegA = *RegA & *RegB;
                        } return;
                    }
                } break;
            }
        } break;

        case instruction_type::XOR:
        {
            switch (Instruction.Args[0].Type)
            {
                case argument_type::V:
                {
                    switch (Instruction.Args[1].Type)
                    {
                        case argument_type::V:
                        {
                            u8* RegA = M->V + Instruction.Args[0].Value;
                            u8* RegB = M->V + Instruction.Args[1].Value;
                            *RegA = *RegA ^ *RegB;
                        } return;
                    }
                } break;
            }
        } break;

        case instruction_type::SUB:
        {
            switch (Instruction.Args[0].Type)
            {
                case argument_type::V:
                {
                    switch (Instruction.Args[1].Type)
                    {
                        case argument_type::V:
                        {
                            u8* RegA = M->V + Instruction.Args[0].Value;
                            u8* RegB = M->V + Instruction.Args[1].Value;
                            M->V[0xF] = (*RegA > *RegB) ? 1 : 0;
                            *RegA = *RegA - *RegB;
                        } return;
                    }
                } break;
            }
        } break;

        case instruction_type::SHR:
        {
            switch (Instruction.Args[0].Type)
            {
                case argument_type::V:
                {
                    u8* RegA = M->V + Instruction.Args[0].Value;
                    u8* RegB = M->V + Instruction.Args[1].Value;
                    M->V[0xF] = *RegB & 0b0000'0001;
                    *RegB >>= 1;
                    *RegA = *RegB;
                } return;
            }
        } break;

        case instruction_type::SUBN:
        {
            switch (Instruction.Args[0].Type)
            {
                case argument_type::V:
                {
                    switch (Instruction.Args[1].Type)
                    {
                        case argument_type::V:
                        {
                            u8* RegA = M->V + Instruction.Args[0].Value;
                            u8* RegB = M->V + Instruction.Args[1].Value;
                            M->V[0xF] = (*RegB > *RegA) ? 1 : 0;
                            *RegA = *RegB - *RegA;
                        } return;
                    }
                } break;
            }
        } break;

        case instruction_type::SHL:
        {
            switch (Instruction.Args[0].Type)
            {
                case argument_type::V:
                {
                    u8* RegA = M->V + Instruction.Args[0].Value;
                    u8* RegB = M->V + Instruction.Args[1].Value;
                    M->V[0xF] = *RegB & 0b0000'0001;
                    *RegB <<= 1;
                    *RegA = *RegB;
                } return;
            }
        } break;

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
                            u8 Rand = (u8)M->RNG.RandomBetween_u32(0, 255);
                            *Reg = Byte & Rand;
                        } return;
                    }
                } break;
            }
        } break;

        case instruction_type::DRW:
        {
            switch (Instruction.Args[0].Type)
            {
                case argument_type::V:
                {
                    switch (Instruction.Args[1].Type)
                    {
                        case argument_type::V:
                        {
                            switch (Instruction.Args[2].Type)
                            {
                                case argument_type::CONSTANT:
                                {
                                    u8* RegA = M->V + Instruction.Args[0].Value;
                                    u8* RegB = M->V + Instruction.Args[1].Value;
                                    sprite Sprite;
                                    Sprite.Length = (int)Instruction.Args[2].Value;
                                    Sprite.Pixels = (u8*)(M->Memory + M->I);
                                    DrawSprite(M, *RegA, *RegB, Sprite);
                                } return;
                            }
                        } break;
                    }
                } break;
            }
        } break;

        case instruction_type::SKP:
        {
            switch (Instruction.Args[0].Type)
            {
                case argument_type::V:
                {
                    u8 KeyIndex = (u8)Instruction.Args[0].Value;
                    if (IsKeyDown(M->InputState, KeyIndex))
                    {
                        M->ProgramCounter += 2;
                    }
                } return;
            }
        } break;

        case instruction_type::SKNP:
        {
            switch (Instruction.Args[0].Type)
            {
                case argument_type::V:
                {
                    u8 KeyIndex = (u8)Instruction.Args[0].Value;
                    if (!IsKeyDown(M->InputState, KeyIndex))
                    {
                        M->ProgramCounter += 2;
                    }
                } return;
            }
        } break;
    }

    #if COUSCOUSC
    text Disassembly = DisassembleInstruction(Instruction);
    printf(STR_FMT ": Invalid instruction to execute.\n", STR_FMTARG(Disassembly));
    #endif
}

bool
IsKeyDown(u16 InputState, u16 KeyIndex)
{
    bool Result = IsBitSet(InputState, KeyIndex);

    return Result;
}

u16
SetKeyDown(u16 InputState, u16 KeyIndex, bool32 IsDown)
{
    u16 Result;
    if (IsDown) Result = SetBit(InputState, KeyIndex);
    else        Result = UnsetBit(InputState, KeyIndex);

    return Result;
}


int GetNumArguments(instruction Instruction)
{
    int Result = 0;
    for (int ArgIndex = 0;
        ArgIndex < MTB_ARRAY_COUNT(Instruction.Args);
        ++ArgIndex)
    {
        if (Instruction.Args[ArgIndex].Type == argument_type::NONE)
            break;

        ++Result;
    }

    return Result;
}

#if COUSCOUSC

#define I0(Hint, InstructionType)                               { Hint, instruction_type::InstructionType, 0 }
#define I1(Hint, InstructionType, ArgType0)                     { Hint, instruction_type::InstructionType, 1, { argument_type::ArgType0 } }
#define I2(Hint, InstructionType, ArgType0, ArgType1)           { Hint, instruction_type::InstructionType, 2, { argument_type::ArgType0, argument_type::ArgType1 } }
#define I3(Hint, InstructionType, ArgType0, ArgType1, ArgType2) { Hint, instruction_type::InstructionType, 3, { argument_type::ArgType0, argument_type::ArgType1, argument_type::ArgType2 } }

static instruction_signature InstructionSignatures[] =
{
    I2("Fx1E", ADD, I, V),           // ADD I, Vx          - Fx1E
    I2("7xkk", ADD, V, CONSTANT),    // ADD Vx, byte       - 7xkk
    I2("8xy4", ADD, V, V),           // ADD Vx, Vy         - 8xy4
    I2("8xy2", AND, V, V),           // AND Vx, Vy         - 8xy2
    I1("2nnn", CALL, CONSTANT),      // CALL addr          - 2nnn
    I0("00E0", CLS),                 // CLS                - 00E0
    I3("Dxyn", DRW, V, V, CONSTANT), // DRW Vx, Vy, nibble - Dxyn
    I1("1nnn", JP, CONSTANT),        // JP addr            - 1nnn
    I2("Bnnn", JP, V, CONSTANT),     // JP V0, addr        - Bnnn
    I2("Fx55", LD, ATI, V),          // LD [I], Vx         - Fx55
    I2("Fx33", LD, B, V),            // LD B, Vx           - Fx33
    I2("Fx15", LD, DT, V),           // LD DT, Vx          - Fx15
    I2("Fx29", LD, F, V),            // LD F, Vx           - Fx29
    I2("Annn", LD, I, CONSTANT),     // LD I, addr         - Annn
    I2("Fx18", LD, ST, V),           // LD ST, Vx          - Fx18
    I2("Fx65", LD, V, ATI),          // LD Vx, [I]         - Fx65
    I2("6xkk", LD, V, CONSTANT),     // LD Vx, byte        - 6xkk
    I2("Fx07", LD, V, DT),           // LD Vx, DT          - Fx07
    I2("Fx0A", LD, V, K),            // LD Vx, K           - Fx0A
    I2("8xy0", LD, V, V),            // LD Vx, Vy          - 8xy0
    I2("8xy1", OR, V, V),            // OR Vx, Vy          - 8xy1
    I0("00EE", RET),                 // RET                - 00EE
    I2("Cxkk", RND, V, CONSTANT),    // RND Vx, byte       - Cxkk
    I2("3xkk", SE, V, CONSTANT),     // SE Vx, byte        - 3xkk
    I2("5xy0", SE, V, V),            // SE Vx, Vy          - 5xy0
    I2("8xyE", SHL, V, V),           // SHL Vx {, Vy}      - 8xyE
    I2("8xy6", SHR, V, V),           // SHR Vx {, Vy}      - 8xy6
    I1("ExA1", SKNP, V),             // SKNP Vx            - ExA1
    I1("Ex9E", SKP, V),              // SKP Vx             - Ex9E
    I2("4xkk", SNE, V, CONSTANT),    // SNE Vx, byte       - 4xkk
    I2("9xy0", SNE, V, V),           // SNE Vx, Vy         - 9xy0
    I2("8xy5", SUB, V, V),           // SUB Vx, Vy         - 8xy5
    I2("8xy7", SUBN, V, V),          // SUBN Vx, Vy        - 8xy7
    I1("0nnn", SYS, CONSTANT),       // SYS addr           - 0nnn
    I2("8xy3", XOR, V, V),           // XOR Vx, Vy         - 8xy3
};

#undef I0
#undef I1
#undef I2
#undef I3

bool
IsCompatible(instruction Instruction, instruction_signature Signature)
{
    bool Result = false;

    if (Instruction.Type == Signature.Type && GetNumArguments(Instruction) == Signature.NumParams)
    {
        Result = true;

        for (int ParamIndex = 0;
            ParamIndex < Signature.NumParams;
            ++ParamIndex)
        {
            if (Instruction.Args[ParamIndex].Type != Signature.Params[ParamIndex])
            {
                Result = false;
                break;
            }
        }
    }

    return Result;
}

instruction_signature*
FindSignature(instruction instruction)
{
    instruction_signature* Result = nullptr;

    for (int SignatureIndex = 0;
        SignatureIndex < MTB_ARRAY_COUNT(InstructionSignatures);
        ++SignatureIndex)
    {
        instruction_signature* Signature = InstructionSignatures + SignatureIndex;
        if (IsCompatible(instruction, *Signature))
        {
            Result = Signature;
            break;
        }
    }

    return Result;
}

instruction_signature*
FindMostCompatibleSignature(instruction Instruction)
{
    struct signature_compatibility_score
    {
        int SignatureIndex;
        int Score;
    };
    signature_compatibility_score Scores[MTB_ARRAY_COUNT(InstructionSignatures)]{};

    int NumArgs = GetNumArguments(Instruction);
    if (NumArgs > 3)
        NumArgs = 3;

    for (int SignatureIndex = 0;
        SignatureIndex < MTB_ARRAY_COUNT(InstructionSignatures);
        ++SignatureIndex)
    {
        instruction_signature* Signature = InstructionSignatures + SignatureIndex;
        int BaseScore = 0;
        if (Signature->Type == Instruction.Type)
            BaseScore += 3;

        // BaseScore -= mtb_Abs(Signature->NumParams - NumArgs);

        int ParamScores[3]{};

        for (int ParamIndex = 0;
            ParamIndex < Signature->NumParams;
            ++ParamIndex)
        {
            for (int ArgIndex = 0;
                ArgIndex < MTB_ARRAY_COUNT(Instruction.Args) && Instruction.Args[ArgIndex].Type != argument_type::NONE;
                ++ArgIndex)
            {
                if (Signature->Params[ParamIndex] == Instruction.Args[ArgIndex].Type)
                {
                    if (ParamIndex == ArgIndex)
                        ParamScores[ParamIndex] = 2;
                    else
                        ParamScores[ParamIndex] = 1;
                }
            }
        }

        int ParamScoreTotal = 0;
        for (int ParamIndex = 0;
            ParamIndex < Signature->NumParams;
            ++ParamIndex)
        {
            ParamScoreTotal += ParamScores[ParamIndex];
        }

        signature_compatibility_score* Score = Scores + SignatureIndex;
        Score->SignatureIndex = SignatureIndex;
        Score->Score = BaseScore;
        if (Signature->NumParams > 0)
            Score->Score += ParamScoreTotal / Signature->NumParams;
    }

    qsort(Scores, MTB_ARRAY_COUNT(Scores), sizeof(signature_compatibility_score), [](void const* A_, void const* B_)
    {
        signature_compatibility_score const* A = (signature_compatibility_score const*)A_;
        signature_compatibility_score const* B = (signature_compatibility_score const*)B_;
        int Result = B->Score - A->Score;
        return Result;
    });

    instruction_signature* Result = nullptr;
    if (Scores[0].Score > 0)
    {
        Result = InstructionSignatures + Scores[0].SignatureIndex;
    }

#if 0
    for (int ScoreIndex = 0;
        ScoreIndex < MTB_ARRAY_COUNT(Scores) && Scores[ScoreIndex].Score > 0;
        ++ScoreIndex)
    {
        signature_compatibility_score* Score = Scores + ScoreIndex;
        instruction_signature* Signature = InstructionSignatures + Score->SignatureIndex;
        fprintf(stderr, "  Score %d: %s (%s)\n", Score->Score, GetInstructionTypeAsString(Signature->Type), Signature->Hint);
    }
#endif

    return Result;
}

strc Trim(strc Text)
{
    int Size = Text.Size;
    char const* Data = Text.Data;

    while (Size > 0)
    {
        if (!mtb::string::IsWhiteChar(Data[0]))
            break;

        ++Data;
        --Size;
    }

    while (Size > 0)
    {
        if (!mtb::string::IsWhiteChar(Data[Size - 1]))
            break;

        --Size;
    }

    return {Size, Data};
}

str Str(char* Stringz)
{

    str Result{ (int)mtb::IntCast<s32>(mtb::string::StringLengthZ(Stringz)), Stringz };

    return Result;
}

strc Str(char const* Stringz)
{
    strc Result{ (int)mtb::IntCast<s32>(mtb::string::StringLengthZ(Stringz)), Stringz };

    return Result;
}

str Str(char* Begin, char* End)
{
    str Result{ (int)(End - Begin), Begin };

    return Result;
}

strc StrConst(char* Stringz)
{
    return Str((char const*)Stringz);
}

str StrNoConst(char const* Stringz)
{
    return Str((char*)Stringz);
}

int Compare(strc A, strc B)
{
    int Result = mtb::SliceCompareBytes(mtb::PtrSlice(A.Data, A.Size), mtb::PtrSlice(B.Data, B.Size));

    return Result;
}

bool AreEqual(strc A, strc B)
{
    int ComparisonResult = Compare(A, B);

    return ComparisonResult == 0;
}

bool StartsWith(strc String, strc Start)
{
    bool Result = false;
    if (Start.Size <= String.Size)
    {
        int Size = String.Size < Start.Size ? String.Size : Start.Size;
        strc A{ Size, String.Data };
        strc B{ Size, Start.Data };
        Result = AreEqual(A, B);
    }

    return Result;
}

char
ToUpper(char Char)
{
    char Result = Char;

    if (Char >= 'a' && Char <= 'z')
    {
        char Offset = Char - 'a';
        Result = 'A' + Offset;
    }

    return Result;
}

void
ToUpper(str String)
{
    for (int Index = 0;
        Index < String.Size;
        ++Index)
    {
        String.Data[Index] = ToUpper(String.Data[Index]);
    }
}

static void
ChangeFileNameExtension(text1024* FileName, strc NewExtension)
{
    // Find the last dot.
    for (int SeekIndex = FileName->Size - 1;
        SeekIndex >= 0 && !IsDirectorySeparator(FileName->Data[SeekIndex]);
        --SeekIndex)
    {
        if (FileName->Data[SeekIndex] == '.')
        {
            // Truncate
            FileName->Size = SeekIndex;
            break;
        }
    }

    if (NewExtension.Size > 0)
    {
        if (NewExtension.Data[0] != '.')
            Append(FileName, '.');
        Append(FileName, NewExtension);
    }

    EnsureZeroTerminated(FileName);
}

cursor_array
Tokenize(parser_cursor Code, eat_flags TokenDelimiters, char const* AdditionalTokenDelimiters)
{
    cursor_array Tokens{};

    while (true)
    {
        Code = Eat(Code, TokenDelimiters, AdditionalTokenDelimiters);
        if (!IsValid(Code))
            break;

        parser_cursor Token = Code;
        Code = EatExcept(Code, TokenDelimiters, AdditionalTokenDelimiters);
        Token.End = Code.Begin;

        *Add(&Tokens) = Token;
    }

    return Tokens;
}

text
Detokenize(int NumTokens, str* Tokens)
{
    text Result{};

    strc TokenSeparator = Str(" ");
    strc Sep = Str("");

    for (int TokenIndex = 0;
        TokenIndex < NumTokens;
        ++TokenIndex)
    {
        Append(&Result, Sep);
        Sep = TokenSeparator;

        if (Result.Size >= Result.Capacity)
            break;

        str* Token = Tokens + TokenIndex;
        Append(&Result, *Token);

        if (Result.Size >= Result.Capacity)
            break;
    }

    return Result;
}

instruction
AssembleInstruction(parser_cursor Code)
{
    cursor_array Tokens = Tokenize(Code, eat_flags::Whitespace, ",");
    instruction Result = AssembleInstruction(Tokens.NumElements, Tokens.Data());
    Deallocate(&Tokens);

    return Result;
}

instruction
AssembleInstruction(int NumTokens, parser_cursor* Tokens)
{
    instruction Result{};

    if (NumTokens > 0)
    {
        str TypeToken = Str(Tokens[0]);
        Result.Type = MakeInstructionTypeFromString((size_t)TypeToken.Size, TypeToken.Data);

        for (int TokenIndex = 1;
            TokenIndex < NumTokens;
            ++TokenIndex)
        {
            str ArgToken = Str(Tokens[TokenIndex]);
            int ArgumentIndex = TokenIndex - 1;
            Result.Args[ArgumentIndex] = MakeArgumentFromString((size_t)ArgToken.Size, ArgToken.Data);
        }
    }

    return Result;
}

text
DisassembleInstruction(instruction Instruction)
{
    token_array Tokens = DisassembleInstructionTokens(Instruction);
    MTB_DEFER{ Deallocate(&Tokens); };

    str_array TokenStrings{};
    MTB_DEFER{ Deallocate(&TokenStrings); };

    for (int TokenIndex = 0;
        TokenIndex < Tokens.NumElements;
        ++TokenIndex)
    {
        *Add(&TokenStrings) = Str(Tokens.Data()[TokenIndex]);
    }

    text Result = Detokenize(Tokens.NumElements, TokenStrings.Data());

    return Result;
}

token_array
DisassembleInstructionTokens(instruction Instruction)
{
    token_array Result{};
    if (Instruction.Type != instruction_type::INVALID)
    {
        {
            strc TypeString = Str(GetInstructionTypeAsString(Instruction.Type));
            *Add(&Result) = CreateToken(TypeString);
        }

        for (int ArgIndex = 0;
            ArgIndex < MTB_ARRAY_COUNT(Instruction.Args) && Instruction.Args[ArgIndex].Type != argument_type::NONE;
            ++ArgIndex)
        {
            token* Token = Add(&Result);
            Token->Size = (int)GetArgumentAsString(Instruction.Args[ArgIndex], Token->Capacity, (u8*)Token->Data);
        }
    }
    else
    {
        u16 RawInstruction = Instruction.Args[0].Value;
        token* Token = Add(&Result);
        int NumCharsWritten = snprintf(Token->Data, Token->Capacity, "<0x%04X>", RawInstruction);
        MTB_ASSERT(NumCharsWritten == 3 + 4 + 1);
        Token->Size = NumCharsWritten;
    }

    return Result;
}

str
Str(parser_cursor Cursor)
{
    return Str(Cursor.Begin, Cursor.End);
}

bool
IsValid(parser_cursor Cursor)
{
    return Cursor.Begin < Cursor.End;
}

parser_cursor
Advance(parser_cursor Cursor, int NumToAdvance)
{
    MTB_ASSERT(NumToAdvance > 0);

    while (IsValid(Cursor) && NumToAdvance > 0)
    {
        ++Cursor.LinePos;
        if (Cursor.Begin[0] == '\n')
        {
            ++Cursor.NumLineBreaks;
            Cursor.LinePos = 0;
        }
        ++Cursor.Begin;
        --NumToAdvance;
    }

    return Cursor;
}

parser_cursor
Eat(parser_cursor Cursor, eat_flags Flags, char const* AdditionalCharsToEat)
{
    while (IsValid(Cursor))
    {
        parser_cursor Previous = Cursor;

        if ((Flags & eat_flags::Whitespace) == eat_flags::Whitespace)
        {
            while (IsValid(Cursor) && mtb::string::IsWhiteChar(Cursor.Begin[0]))
                Cursor = Advance(Cursor);
        }

        if ((Flags & eat_flags::Comments) == eat_flags::Comments)
        {
            Cursor = EatComments(Cursor);
        }

        if ((Flags & eat_flags::Strings) == eat_flags::Strings)
        {
            Cursor = EatBetween(Cursor, '"', '\\');
        }

        if (AdditionalCharsToEat)
        {
            char const* Seek = AdditionalCharsToEat;
            while (Seek[0])
            {
                while (IsValid(Cursor) && Cursor.Begin[0] == Seek[0])
                    Cursor = Advance(Cursor);

                ++Seek;
            }
        }

        if (Cursor.Begin == Previous.Begin)
        {
            // We didn't move, so we're done.
            break;
        }
    }

    return Cursor;
}

parser_cursor
EatExcept(parser_cursor Cursor, eat_flags Flags, char const* AdditionalCharsToStopAt /*= nullptr*/)
{
    bool StopAtWhitespace = (Flags & eat_flags::Whitespace) == eat_flags::Whitespace;
    bool StopAtComments = (Flags & eat_flags::Comments) == eat_flags::Comments;

    char LastChar = '\0';
    while (IsValid(Cursor))
    {
        char Char = Cursor.Begin[0];

        if (StopAtWhitespace && mtb::string::IsWhiteChar(Char))
        {
            return Cursor;
        }

        if (StopAtComments && Char == '#')
        {
            return Cursor;
        }

        if(AdditionalCharsToStopAt)
        {
            char const* Seek = AdditionalCharsToStopAt;
            while (Seek[0])
            {
                if (Seek[0] == Char)
                    return Cursor;
                ++Seek;
            }
        }

        {
            parser_cursor Previous = Cursor;
            Cursor = EatBetween(Cursor, '"', '\\');
            if (Cursor.Begin > Previous.Begin)
            {
                continue;
            }
        }

        Cursor = Advance(Cursor);

        LastChar = Char;
    }

    return Cursor;
}

parser_cursor EatComments(parser_cursor Cursor)
{
    if (IsValid(Cursor) && Cursor.Begin[0] == '#')
    {
        while (IsValid(Cursor) && Cursor.Begin[0] != '\n')
            Cursor = Advance(Cursor);
    }

    return Cursor;
}

parser_cursor EatBetween(parser_cursor Cursor, char Delimiter, char Escaper /*= '\\'*/)
{
    if (IsValid(Cursor) && Cursor.Begin[0] == Delimiter)
    {
        while (true)
        {
            Cursor = Advance(Cursor);
            if (!IsValid(Cursor))
                break;

            if (Cursor.Begin[0] == Delimiter)
            {
                Cursor = Advance(Cursor);
                break;
            }

            if (Cursor.Begin[0] == Escaper)
                Cursor = Advance(Cursor);
        }
    }

    return Cursor;
}

parser_cursor
ParseLine(parser_cursor Cursor)
{
    while (IsValid(Cursor))
    {
        if (Cursor.Begin[0] == '\n' || Cursor.Begin[0] == '#')
        {
            break;
        }
        else if (Cursor.Begin[0] == ':')
        {
            Cursor = Advance(Cursor);
            break;
        }

        Cursor = Advance(Cursor);
    }

    return Cursor;
}

assemble_code_result
AssembleCode(parser_context* Context, char* ContentsBegin, char* ContentsEnd)
{
    assemble_code_result Result{};
    u8_array* ByteCode = &Result.ByteCode;
    debug_info_array* DebugInfos = &Result.DebugInfos;
    label_array* Labels = &Result.Labels;

    parser_cursor Cursor{ ContentsBegin, ContentsEnd };
    u16 CurrentMemoryOffset = Context->BaseMemoryOffset;

    patch_array Patches{};
    MTB_DEFER{ Deallocate(&Patches); };

    while (true)
    {
        Cursor = Eat(Cursor, eat_flags::Whitespace | eat_flags::Comments);
        if (!IsValid(Cursor))
            goto EndOfContentParsing;

        parser_cursor LineCursor = Cursor;
        Cursor = ParseLine(Cursor);

        LineCursor.End = Cursor.Begin;

        strc Text = Trim(Str(LineCursor.Begin, Cursor.Begin));

        if (Text.Data[Text.Size - 1] == ':')
        {
            // We have a label!

            label Label{};
            Label.MemoryOffset = CurrentMemoryOffset;
            Label.NameCursor = LineCursor;
            // Ignore the trailing colon.
            --Label.NameCursor.End;

            // Copy without the trailing colon
            str LabelName = Str(Label.NameCursor);

            label* Existing = Find(Labels, [&](label* L) { return AreEqual(LabelName, Str(L->NameCursor)); });
            if (Existing)
            {
                ErrorDuplicateLabel(Context, Existing->NameCursor, Label.NameCursor);
            }
            else
            {
                *Add(Labels) = Label;
            }
        }
        else if (Text.Data[0] == '[' && Text.Data[Text.Size - 1] == ']' && Text.Size >= 3)
        {
            // We have a data section.

            // Skip the opening bracket.
            parser_cursor DataCursor = Advance(LineCursor);

            // Ignore the closing bracket.
            --DataCursor.End;

            MTB_ASSERT(DataCursor.End - DataCursor.Begin > 0);

            char DataType = DataCursor.Begin[0];
            DataCursor = Eat(DataCursor, eat_flags::Whitespace | eat_flags::Comments);

            switch (DataType)
            {
                case 'b':
                case 'B':
                {
                    while (IsValid(DataCursor))
                    {
                        u8 Byte = 0;
                        for (u8 BitIndex = 7; BitIndex >= 0; --BitIndex)
                        {
                            DataCursor = Eat(DataCursor, eat_flags::Whitespace);
                            if (!IsValid(DataCursor))
                                break;

                            if (DataCursor.Begin[0] == '1')
                                Byte |= (u8)(1u << BitIndex);

                            DataCursor = Advance(DataCursor);
                        }

                        *Add(ByteCode) = Byte;
                        // TODO: Debug info
                        ++CurrentMemoryOffset;
                    }
                } break;

                case 'x':
                case 'X':
                {
                    while (IsValid(DataCursor))
                    {
                        u8 Byte = 0;
                        for (u8 NibbleIndex = 1; NibbleIndex >= 0; --NibbleIndex)
                        {
                            DataCursor = Eat(DataCursor, eat_flags::Whitespace);
                            if (!IsValid(DataCursor))
                                break;

                            unsigned Shift = 4 * NibbleIndex;

                            char Char = DataCursor.Begin[0];
                            switch (Char)
                            {
                                case '1': Byte |= (u8)(1u << Shift); break;
                                case '2': Byte |= (u8)(2u << Shift); break;
                                case '3': Byte |= (u8)(3u << Shift); break;
                                case '4': Byte |= (u8)(4u << Shift); break;
                                case '5': Byte |= (u8)(5u << Shift); break;
                                case '6': Byte |= (u8)(6u << Shift); break;
                                case '7': Byte |= (u8)(7u << Shift); break;
                                case '8': Byte |= (u8)(8u << Shift); break;
                                case '9': Byte |= (u8)(9u << Shift); break;
                                case 'A':
                                case 'a': Byte |= (u8)(0xAu << Shift); break;
                                case 'B':
                                case 'b': Byte |= (u8)(0xBu << Shift); break;
                                case 'C':
                                case 'c': Byte |= (u8)(0xCu << Shift); break;
                                case 'D':
                                case 'd': Byte |= (u8)(0xDu << Shift); break;
                                case 'E':
                                case 'e': Byte |= (u8)(0xEu << Shift); break;
                                case 'F':
                                case 'f': Byte |= (u8)(0xFu << Shift); break;
                            }

                            DataCursor = Advance(DataCursor);
                        }

                        *Add(ByteCode) = Byte;
                        // TODO: Debug info
                        ++CurrentMemoryOffset;
                    }
                    MTB_ASSERT(!"not implemented");
                } break;

                default:
                {
                    MTB_ASSERT("Unsupported data type in data section. Must be either of [b] or [x] (case insensitive).");
                } break;
            }
        }
        else
        {
            cursor_array Tokens = Tokenize(LineCursor, eat_flags::Whitespace, ",");
            MTB_DEFER{ Deallocate(&Tokens); };

            instruction Instruction = AssembleInstruction(Tokens.NumElements, Tokens.Data());

            u16 EncodedInstruction = 0;
            instruction_signature* Signature = FindSignature(Instruction);
            if (Signature)
            {
                patch Patch{};
                Patch.InstructionMemoryOffset = CurrentMemoryOffset;
                switch (Instruction.Type)
                {
                    case instruction_type::JP:
                    {
                        if (Instruction.Args[0].Type == argument_type::CONSTANT && Instruction.Args[0].Value == 0)
                        {
                            // e.g. JP 0x234
                            Patch.LabelNameCursor = *At(&Tokens, 1);
                        }
                        else if (Instruction.Args[0].Type == argument_type::V && Instruction.Args[0].Value == 0 &&
                            Instruction.Args[1].Type == argument_type::CONSTANT && Instruction.Args[1].Value == 0)
                        {
                            // e.g. JP V0 0x234
                            Patch.LabelNameCursor = *At(&Tokens, 2);
                        }
                    } break;

                    case instruction_type::CALL:
                    {
                        if (Instruction.Args[0].Type == argument_type::CONSTANT && Instruction.Args[0].Value == 0)
                        {
                            // e.g. CALL 0x234
                            Patch.LabelNameCursor = *At(&Tokens, 1);
                        }
                    } break;

                    case instruction_type::LD:
                    {
                        if (Instruction.Args[0].Type == argument_type::I &&
                            Instruction.Args[1].Type == argument_type::CONSTANT && Instruction.Args[1].Value == 0)
                        {
                            // e.g. LD I 0x234
                            Patch.LabelNameCursor = *At(&Tokens, 2);
                        }
                    } break;
                }

                if (IsValid(Patch.LabelNameCursor))
                {
                    *Add(&Patches) = Patch;
                }

                EncodedInstruction = EncodeInstruction(Instruction);
            }
            else
            {
                instruction_signature* MostCompatibleSignature = FindMostCompatibleSignature(Instruction);
                ErrorInvalidInstruction(Context, LineCursor, MostCompatibleSignature);
            }

            u16* NewWord = (u16*)AddN(ByteCode, 2);
            WriteWord(NewWord, EncodedInstruction);

            if (Context->GatherDebugInfo)
            {
                //parser_cursor* InfoToken = At(&Tokens, 0);
                parser_cursor* InfoToken = &LineCursor;

                debug_info Info{};
                Info.FileId = Context->FileId;
                Info.Line = InfoToken->NumLineBreaks + 1;
                Info.Column = InfoToken->LinePos + 1;
                Info.MemoryOffset = CurrentMemoryOffset;

                // Note: Info.GeneratedInstruction is filled in later.
                Info.SourceLine = Str(*InfoToken);

                *Add(DebugInfos) = Info;
            }

            CurrentMemoryOffset += 2;
        }
    }
EndOfContentParsing:

    // Apply patches
    for (int PatchIndex = 0;
        PatchIndex < Patches.NumElements;
        ++PatchIndex)
    {
        patch* Patch = Patches.Data() + PatchIndex;

        bool Found = false;
        for (int LabelIndex = 0;
            LabelIndex < Labels->NumElements;
            ++LabelIndex)
        {
            label* Label = Labels->Data() + LabelIndex;
            str LabelName = Str(Label->NameCursor);
            str PatchLabelName = Str(Patch->LabelNameCursor);
            if (AreEqual(LabelName, PatchLabelName))
            {
                MTB_ASSERT(Patch->InstructionMemoryOffset >= Context->BaseMemoryOffset);
                u16 MemoryIndex = Patch->InstructionMemoryOffset - Context->BaseMemoryOffset;
                u16* InstructionLocation = (u16*)At(ByteCode, MemoryIndex);
                u16 EncodedInstruction = ReadWord(InstructionLocation);
                EncodedInstruction |= (Label->MemoryOffset & 0x0FFF);
                WriteWord(InstructionLocation, EncodedInstruction);

                Found = true;
                break;
            }
        }

        if (!Found)
        {
            ErrorLabelNotFound(Context, Patch->LabelNameCursor);
        }
    }

    if (Context->GatherDebugInfo)
    {
        for (int InfoIndex = 0;
            InfoIndex < Result.DebugInfos.NumElements;
            ++InfoIndex)
        {
            debug_info* Info = Result.DebugInfos.Data() + InfoIndex;
            int ByteCodeIndex = Info->MemoryOffset - Context->BaseMemoryOffset;
            Info->GeneratedInstruction = ReadWord(At(&Result.ByteCode, ByteCodeIndex));
        }
    }

    return Result;
}

#endif // COUSCOUSC

#pragma GCC diagnostic pop
