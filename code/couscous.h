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

static int
GetNumArguments(instruction Instruction);

struct instruction_signature
{
    char const* Hint;
    instruction_type Type;
    int NumParams;
    argument_type Params[3];
};

static bool
IsCompatible(instruction Instruction, instruction_signature Signature);

static instruction_signature*
FindSignature(instruction instruction);

#include "_generated/u8_array.h"
#include "_generated/int_array.h"

#define STR_FMT "%*.*s"
#define STR_FMTARG(Str) (int)(Str).Size, (int)(Str).Size, (char const*)(Str).Data

struct strc
{
    int Size;
    char const* Data;
};

struct str
{
    int Size;
    char* Data;

    // Implicit conversion to const string.
    inline operator strc() const { return { Size, Data }; }
};
#include "_generated/str_array.h"

#include "_generated/text.h"
#include "_generated/token.h"
#include "_generated/token_array.h"

static str
Trim(str Text);

static str
Str(char* Stringz);

static strc
StrConst(char* Stringz);

static str
Str(char* Begin, char* End);

static strc
Str(char const* Stringz);

static str
StrNoConst(char const* Stringz);

static int
Compare(strc A, strc B);

static bool
AreEqual(strc A, strc B);

static bool
StartsWith(strc String, strc Start);

struct parser_cursor
{
    char* Begin;
    char* End;
    int NumLineBreaks;
    int LinePos;
};

static str
Str(parser_cursor Cursor);

static bool
IsValid(parser_cursor Cursor);

static parser_cursor
Advance(parser_cursor Cursor, int NumToAdvance = 1);

static parser_cursor
SkipWhitespace(parser_cursor Cursor);

static parser_cursor
SkipWhitespaceAndComments(parser_cursor Cursor);

static parser_cursor
ParseLine(parser_cursor Cursor);


struct label
{
    parser_cursor Cursor;
    u16 MemoryOffset;
};
#include "_generated/label_array.h"

struct patch
{
    parser_cursor Cursor;
    str LabelName;
    u16 InstructionMemoryOffset;
};
#include "_generated/patch_array.h"

static str_array
Tokenize(str Code);

static text
Detokenize(int NumTokens, str* Tokens);

static instruction
AssembleInstruction(str Code);

static instruction
AssembleInstruction(int NumTokens, str* Tokens);

static token_array
DisassembleInstructionTokens(instruction Instruction);

static text
DisassembleInstruction(instruction Instruction);

enum parser_error_type
{
    ERR_NONE,

    ERR_LabelNotFound,
    ERR_DuplicateLabel,
    ERR_InvalidInstruction,

    ERR_COUNT,
};

struct parser_error_info
{
    parser_error_type Type;
};

using parser_error_handler = void(*)(struct parser_context* Context, parser_error_info* ErrorInfo);

struct parser_context
{
    parser_error_handler ErrorHandler;
    u16 BaseMemoryOffset;
};

static void
AssembleCode(parser_context* Context, char* ContentsBegin, char* ContentsEnd, u8_array* ByteCode);

struct parser_label_not_found
{
    parser_error_type ErrorType = ERR_LabelNotFound;
    parser_cursor PatchCursor;
};
inline void ErrorLabelNotFound(parser_context* Context, parser_cursor PatchCursor)
{
    if (Context->ErrorHandler)
    {
        parser_label_not_found Info{};
        Info.PatchCursor = PatchCursor;
        Context->ErrorHandler(Context, (parser_error_info*)&Info);
    }
}

struct parser_duplicate_label
{
    parser_error_type ErrorType = ERR_DuplicateLabel;
    parser_cursor MainCursor;
    parser_cursor SecondaryCursor;
};
inline void ErrorDuplicateLabel(parser_context* Context, parser_cursor MainLabelCursor, parser_cursor SecondaryLabelCursor)
{
    if (Context->ErrorHandler)
    {
        parser_duplicate_label Info{};
        Info.MainCursor = MainLabelCursor;
        Info.SecondaryCursor = SecondaryLabelCursor;
        Context->ErrorHandler(Context, (parser_error_info*)&Info);
    }
}

struct parser_invalid_instruction
{
    parser_error_type ErrorType = ERR_InvalidInstruction;
    parser_cursor Cursor;
    instruction_signature* BestMatchingSignature;
};
inline void ErrorInvalidInstruction(parser_context* Context, parser_cursor Cursor, instruction_signature* BestMatchingSignature)
{
    if (Context->ErrorHandler)
    {
        parser_invalid_instruction Info{};
        Info.Cursor = Cursor;
        Info.BestMatchingSignature = BestMatchingSignature;
        Context->ErrorHandler(Context, (parser_error_info*)&Info);
    }
}

#endif // COUSCOUSC
