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
    u64 CurrentCycle;
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
        u16 Group : 4;  // x000
    };

    struct
    {
        u16 LSN : 4; // 000x (Least significant nibble)
        u16 Y : 4; // 00x0
        u16 X : 4; // 0x00
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

static instruction
DecodeInstruction(instruction_decoder Decoder);

static u16
EncodeInstruction(instruction Instruction);

static void
ExecuteInstruction(machine* M, instruction Instruction);

static bool
IsKeyDown(u16 InputState, u16 KeyIndex);

static u16
SetKeyDown(u16 InputState, u16 KeyIndex, bool32 IsDown);


#if COUSCOUSC

struct label
{
    text Text;
    u16 MemoryOffset;
};

struct patch
{
    token LabelName;
    u16 InstructionMemoryOffset;
};

struct assembler_tokens
{
    int NumTokens;
    token Tokens[4];
};

static assembler_tokens
Tokenize(text Code);

static text
Detokenize(assembler_tokens Tokens);

static instruction
AssembleInstruction(text Code);

static instruction
AssembleInstruction(assembler_tokens Tokens);

static assembler_tokens
DisassembleInstructionTokens(instruction Instruction);

static text
DisassembleInstruction(instruction Instruction);

static void
AssembleCode(char* ContentsBegin, char* ContentsEnd, u8_array* ByteCode, u16 BaseMemoryOffset = 0x200u);

#endif // COUSCOUSC
