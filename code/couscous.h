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

#define COUSCOUS_DISPOSE_LATER(Disposable) MTB_DEFER[&]{ Deallocate(&Disposable); }

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
#include "_generated/text1024.h"
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

static char
ToUpper(char Char);

static void
ToUpper(str String);

inline bool
IsDirectorySeparator(char Char) { return Char == '\\' || Char == '/'; }

static void
ChangeFileNameExtension(text1024* FileName, strc NewExtension);

struct parser_cursor
{
    char* Begin;
    char* End;
    int NumLineBreaks; // Relative to Begin
    int LinePos; // Relative to Begin
};
#include "_generated/cursor_array.h"

static str
Str(parser_cursor Cursor);

static bool
IsValid(parser_cursor Cursor);

static parser_cursor
Advance(parser_cursor Cursor, int NumToAdvance = 1);

enum struct eat_flags
{
    NONE,

    Whitespace      = 0b0001,
    Comments        = 0b0010,
    Strings         = 0b0100,

    ALL = 0b1111,
};
inline eat_flags operator|(eat_flags A, eat_flags B) { return (eat_flags)((u32)A | (u32)B); }
inline eat_flags operator&(eat_flags A, eat_flags B) { return (eat_flags)((u32)A & (u32)B); }
inline eat_flags operator~(eat_flags A) { return (eat_flags)(~(u32)A); }

static parser_cursor
Eat(parser_cursor Cursor, eat_flags Flags, char const* AdditionalCharsToEat = nullptr);

static parser_cursor
EatExcept(parser_cursor Cursor, eat_flags Flags, char const* AdditionalCharsToNotEat = nullptr);

static parser_cursor
EatComments(parser_cursor Cursor);

static parser_cursor
EatBetween(parser_cursor Cursor, char Delimiter, char Escaper = '\\');

static parser_cursor
ParseLine(parser_cursor Cursor);

struct label
{
    parser_cursor NameCursor;
    u16 MemoryOffset;
};
#include "_generated/label_array.h"

struct patch
{
    parser_cursor LabelNameCursor;
    u16 InstructionMemoryOffset;
};
#include "_generated/patch_array.h"

static cursor_array
Tokenize(parser_cursor Code, eat_flags TokenizerEatFlags, char* AdditionalTokenDelimiters = nullptr);

static text
Detokenize(int NumTokens, str* Tokens);

static instruction
AssembleInstruction(parser_cursor Code);

static instruction
AssembleInstruction(int NumTokens, parser_cursor* Tokens);

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
    int FileId;
    bool GatherDebugInfo;

    u16 BaseMemoryOffset;
};

struct debug_info
{
    int FileId;
    int Line; // 1-based
    int Column; // 1-based
    u16 MemoryOffset;

    u16 GeneratedInstruction;
    strc SourceLine;
};
#include "_generated/debug_info_array.h"

struct assemble_code_result
{
    u8_array ByteCode;
    debug_info_array DebugInfos;
    label_array Labels;
};
inline
void
Deallocate(assemble_code_result* Code)
{
    Deallocate(&Code->ByteCode);
    Deallocate(&Code->DebugInfos);
    Deallocate(&Code->Labels);
}

static assemble_code_result
AssembleCode(parser_context* Context, char* ContentsBegin, char* ContentsEnd);

struct parser_label_not_found
{
    parser_error_type ErrorType = ERR_LabelNotFound;
    parser_cursor LabelNameCursor;
};
inline void ErrorLabelNotFound(parser_context* Context, parser_cursor LabelNameCursor)
{
    if (Context->ErrorHandler)
    {
        parser_label_not_found Info{};
        Info.LabelNameCursor = LabelNameCursor;
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
