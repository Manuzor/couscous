#define MTB_IMPLEMENTATION
#include "mtb.h"

#include <stdio.h>

#define COUSCOUSC 1

#include "mtb.h"
using u8 = mtb_u08;
using u16 = mtb_u16;
using u32 = mtb_u32;
using u64 = mtb_u64;
using s8 = mtb_s08;
using s16 = mtb_s16;
using s32 = mtb_s32;
using s64 = mtb_s64;

using f32 = mtb_f32;
using f64 = mtb_f64;

using uint = unsigned int;
using bool32 = int;

#include "couscous.h"

#include "couscous.cpp"
#include "_generated/all_generated.cpp"

struct my_parser_context
{
    parser_context BaseContext;
    FILE* ErrorFile;
    strc CurrentFileName;
    parser_error_type LastErrorType;
};

static void
PrintLocation(FILE* File, my_parser_context* Context, parser_cursor Cursor)
{
    int Line = Cursor.NumLineBreaks + 1;
    int Column = Cursor.LinePos + 1;
    fprintf(File, STR_FMT "(%d,%d)", STR_FMTARG(Context->CurrentFileName), Line, Column);
}

static void
PrintErrorType(FILE* File, parser_error_type ErrorType)
{
    fprintf(File, "error %d", (int)ErrorType);
}

static void
PrintSignature(FILE* File, instruction_signature* Signature)
{
    char const* TypeString = GetInstructionTypeAsString(Signature->Type);
    fprintf(File, "%s", TypeString);

    char const* RegisterSuffixes[]{ "x", "y" };
    char const** RegisterPlaceholder = RegisterSuffixes;

    char const* ConstantPlaceholders[]{ "<nnn>", "<nn>", "<n>" };

    char const* Sep = " ";
    for (int ParamIndex = 0;
        ParamIndex < Signature->NumParams;
        ++ParamIndex)
    {
        fprintf(File, Sep);
        Sep = ", ";

        argument_type ArgType = Signature->Params[ParamIndex];
        switch (ArgType)
        {
            case argument_type::NONE: fprintf(File, "NONE"); break;
            case argument_type::V: fprintf(File, "V%s", *RegisterPlaceholder++); break;
            case argument_type::I: fprintf(File, "I"); break;
            case argument_type::DT: fprintf(File, "DT"); break;
            case argument_type::ST: fprintf(File, "ST"); break;
            case argument_type::K: fprintf(File, "K"); break;
            case argument_type::F: fprintf(File, "F"); break;
            case argument_type::B: fprintf(File, "B"); break;
            case argument_type::ATI: fprintf(File, "[I]"); break;
            case argument_type::CONSTANT: fprintf(File, ConstantPlaceholders[ParamIndex]); break;

            default:
            {
                MTB_INVALID_CODE_PATH;
            } break;

        }
    }

    fprintf(File, " (%s)", Signature->Hint);
}

static void
OnError(parser_context* BaseContext, parser_error_info* ErrorInfo)
{
    MTB_AssertDebug(ErrorInfo->Type != ERR_NONE);
    MTB_AssertDebug(ErrorInfo->Type != ERR_COUNT);

    my_parser_context* Context = (my_parser_context*)BaseContext;
    Context->LastErrorType = ErrorInfo->Type;
    FILE* File = Context->ErrorFile;

    switch (ErrorInfo->Type)
    {
        case ERR_LabelNotFound:
        {
            parser_label_not_found* Info = (parser_label_not_found*)ErrorInfo;
            PrintLocation(File, Context, Info->LabelNameCursor);
            fprintf(File, ": ");
            PrintErrorType(File, ErrorInfo->Type);
            strc LabelName = Str(Info->LabelNameCursor);
            fprintf(File, ": Undefined label: " STR_FMT "\n", STR_FMTARG(LabelName));
        } break;

        case ERR_DuplicateLabel:
        {
            parser_duplicate_label* Info = (parser_duplicate_label*)ErrorInfo;
            PrintLocation(File, Context, Info->SecondaryCursor);
            fprintf(File, ": ");
            PrintErrorType(File, ErrorInfo->Type);
            fprintf(File, ": Duplicate label definition: " STR_FMT "\n", STR_FMTARG(Str(Info->SecondaryCursor)));
            fprintf(File, "    See original definition at: ");
            PrintLocation(File, Context, Info->MainCursor);
            fprintf(File, "\n");
        } break;

        case ERR_InvalidInstruction:
        {
            parser_invalid_instruction* Info = (parser_invalid_instruction*)ErrorInfo;
            PrintLocation(File, Context, Info->Cursor);
            fprintf(File, ": ");
            PrintErrorType(File, ErrorInfo->Type);
            fprintf(File, ": Invalid instruction: " STR_FMT "\n", STR_FMTARG(Str(Info->Cursor)));
            if (Info->BestMatchingSignature)
            {
                fprintf(File, "    Did you mean: ");
                PrintSignature(File, Info->BestMatchingSignature);
                fprintf(File, "\n");
            }
        } break;

        default:
        {
        } break;
    }
}

static void
PrintHelp(FILE* OutFile)
{
    fprintf(OutFile, "Usage: couscousc [-help] (-assemble|-disassemble) <in_file> [<out_file>]\n");
}

enum struct commandline_mode
{
    NONE,

    Assemble,
    Disassemble,
};

int main(int NumArgs, char const* Args[])
{
    char const* Files[2]{ nullptr, "-" };
    size_t FileIndex = 0;
    commandline_mode Mode{};

    int Result = -1;

    for (int ArgIndex = 1; ArgIndex < NumArgs; ++ArgIndex)
    {
        char const* Arg = Args[ArgIndex];
        size_t ArgSize = mtb_StringLengthOf(Arg);
        if (ArgSize > 0)
        {
            if (Arg[0] == '-' && ArgSize > 1)
            {
                char const* ArgContent = Arg + 1;
                while (*ArgContent == '-')
                    ++ArgContent;

                if (mtb_StringsAreEqual(ArgContent, "assemble"))
                {
                    Mode = commandline_mode::Assemble;
                }
                else if (mtb_StringsAreEqual(ArgContent, "disassemble"))
                {
                    Mode = commandline_mode::Disassemble;
                }
                else if (mtb_StringsAreEqual(ArgContent, "help"))
                {
                    PrintHelp(stderr);
                    goto end;
                }
                else
                {
                    fprintf(stderr, "Unknown option: %s\n", Arg);
                    PrintHelp(stderr);
                    goto end;
                }
            }
            else
            {
                if (FileIndex < mtb_ArrayLengthOf(Files))
                    Files[FileIndex++] = Arg;
            }
        }
        else
        {
            fprintf(stderr, "Need at least one argument.\n");
            PrintHelp(stderr);
            goto end;
        }
    }

    if (FileIndex < 1)
    {
        fprintf(stderr, "Missing input file path.\n");
        PrintHelp(stderr);
        goto end;
    }

    if (Mode == commandline_mode::NONE)
    {
        // Try to determine the mode from the file extension.
        char const* InputFileName = Files[0];
        size_t InputFileNameLength = mtb_StringLengthOf(InputFileName);
        if (InputFileNameLength > 3 && mtb_StringsAreEqual(InputFileName + (InputFileNameLength - 4), ".ch8"))
        {
            Mode = commandline_mode::Disassemble;
        }
        else if (InputFileNameLength > 9 && mtb_StringsAreEqual(InputFileName + (InputFileNameLength - 9), ".couscous"))
        {
            Mode = commandline_mode::Assemble;
        }
    }

    if (Mode == commandline_mode::NONE)
    {
        fprintf(stderr, "Missing -assemble or -disassemble.\n");
        PrintHelp(stderr);
        goto end;
    }

    //
    //
    //
    char* ContentsBegin = nullptr;
    char* ContentsEnd = nullptr;

    FILE* InFile = fopen(Files[0], "rb");
    fseek(InFile, 0, SEEK_END);
    int InFileSize = ftell(InFile);
    fseek(InFile, 0, SEEK_SET);

    ContentsBegin = (char*)malloc(InFileSize);
    ContentsEnd = ContentsBegin + InFileSize;
    fread(ContentsBegin, InFileSize, 1, InFile);

    fclose(InFile);

    {
        FILE* OutFile = nullptr;
        if (mtb_StringsAreEqual(Files[1], "-"))
            OutFile = stdout;
        else
            OutFile = fopen(Files[1], "wb");

        if (Mode == commandline_mode::Assemble)
        {
            u8_array ByteCode{};
            MTB_DEFER[&]{ Deallocate(&ByteCode); };

            my_parser_context Context{};
            Context.BaseContext.BaseMemoryOffset = 200u;
            Context.BaseContext.ErrorHandler = OnError;
            Context.CurrentFileName = Str(Files[0]);
            Context.ErrorFile = stderr;
            AssembleCode((parser_context*)&Context, ContentsBegin, ContentsEnd, &ByteCode);

            Result = (int)Context.LastErrorType;

            // Write the result!
            fwrite(ByteCode.Data(), ByteCode.NumElements, 1, OutFile);
        }
        else if (Mode == commandline_mode::Disassemble)
        {
            char* Current = ContentsBegin;
            while (Current < ContentsEnd)
            {
                instruction_decoder Decoder{};
                Decoder.Data = ReadWord(Current);
                Current += sizeof(u16);

                // Can only trigger if Current and ContentsOnePastLast are not aligned to 2 bytes relative to each other!
                MTB_AssertDebug(Current <= ContentsEnd);

                instruction Instruction = DecodeInstruction(Decoder);
                text Code = DisassembleInstruction(Instruction);
                fprintf(OutFile, STR_FMT "\n", STR_FMTARG(Code));
            }

            Result = 0;
        }
        else
        {
            MTB_INVALID_CODE_PATH;
        }

        if (OutFile != stdout)
            fclose(OutFile);
    }

    if (ContentsBegin)
        free(ContentsBegin);

    //
    //
    //

end:
    return Result;
}
