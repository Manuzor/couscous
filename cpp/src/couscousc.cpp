#define _CRT_SECURE_NO_WARNINGS
#include <stddef.h>

#define MTB_IMPLEMENTATION
#include "couscous_mtb.h"

#include <stdio.h>

#define COUSCOUSC 1

using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;
using s8 = int8_t;
using s16 = int16_t;
using s32 = int32_t;
using s64 = int64_t;

using f32 = float;
using f64 = double;

using uint = unsigned int;
using bool32 = int;

#include "couscous.h"

#include "couscous.cpp"
#include "generated/all_generated.cpp"

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
                MTB_ASSERT(!"invalid code path");
            } break;

        }
    }

    fprintf(File, " (%s)", Signature->Hint);
}

static void
OnError(parser_context* BaseContext, parser_error_info* ErrorInfo)
{
    MTB_ASSERT(ErrorInfo->Type != ERR_NONE);
    MTB_ASSERT(ErrorInfo->Type != ERR_COUNT);

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
    fprintf(OutFile, "Usage: couscousc [-help] [-assemble|-disassemble] [-chd] <in_file> [<out_file>]\n");
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
    bool GenerateDebugInfos = false;

    int Result = -1;

    for (int ArgIndex = 1; ArgIndex < NumArgs; ++ArgIndex)
    {
        char const* Arg = Args[ArgIndex];
        size_t ArgSize = mtb::string::StringLengthZ(Arg);
        if (ArgSize > 0)
        {
            if (Arg[0] == '-' && ArgSize > 1)
            {
                char const* ArgContent = Arg + 1;
                while (*ArgContent == '-')
                    ++ArgContent;

                if (mtb::string::StringEquals(mtb::string::ConstZ(ArgContent), mtb::string::ConstZ("assemble")))
                {
                    Mode = commandline_mode::Assemble;
                }
                else if (mtb::string::StringEquals(mtb::string::ConstZ(ArgContent), mtb::string::ConstZ("disassemble")))
                {
                    Mode = commandline_mode::Disassemble;
                }
                else if(mtb::string::StringEquals(mtb::string::ConstZ(ArgContent), mtb::string::ConstZ("chd")))
                {
                    GenerateDebugInfos = true;
                }
                else if (mtb::string::StringEquals(mtb::string::ConstZ(ArgContent), mtb::string::ConstZ("help")))
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
                if (FileIndex < MTB_ARRAY_COUNT(Files))
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
        mtb::tSlice<char const> InputFileName = mtb::string::ConstZ(Files[0]);
        if (mtb::string::StringEndsWith(InputFileName, mtb::string::ConstZ(".ch8")))
        {
            Mode = commandline_mode::Disassemble;
        }
        else if (mtb::string::StringEndsWith(InputFileName, mtb::string::ConstZ(".couscous")))
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
    {
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
            if (mtb::string::StringEquals(mtb::string::ConstZ(Files[1]), mtb::string::ConstZ("-")))
            {
                OutFile = stdout;
                GenerateDebugInfos = false;
            }
            else
            {
                OutFile = fopen(Files[1], "wb");
            }

            if (Mode == commandline_mode::Assemble)
            {
                my_parser_context Context{};
                Context.BaseContext.ErrorHandler = OnError;
                Context.BaseContext.FileId = 1;
                Context.BaseContext.BaseMemoryOffset = 0x200u;
                Context.BaseContext.GatherDebugInfo = GenerateDebugInfos;
                Context.CurrentFileName = Str(Files[0]);
                Context.ErrorFile = stderr;
                assemble_code_result Assembled = AssembleCode((parser_context*)&Context, ContentsBegin, ContentsEnd);
                MTB_DEFER{ Deallocate(&Assembled); };

                Result = (int)Context.LastErrorType;

                // Write the result!
                fwrite(Assembled.ByteCode.Data(), Assembled.ByteCode.NumElements, 1, OutFile);

                if (GenerateDebugInfos && Context.LastErrorType == ERR_NONE)
                {
                    text1024 ChdPath = CreateText1024(Str(Files[1]));
                    ChangeFileNameExtension(&ChdPath, Str(".chd"));
                    FILE* ChdFile = fopen(ChdPath.Data, "wb");
                    if (ChdFile)
                    {
                        fprintf(ChdFile, "# BaseMemoryOffset;NumSourceFiles;NumTargetFiles;NumLabels;NumInfos\n");
                        fprintf(ChdFile, "0x%X;%d;%d;%d;%d\n",
                            Context.BaseContext.BaseMemoryOffset,
                            1,
                            1,
                            Assembled.Labels.NumElements,
                            Assembled.DebugInfos.NumElements
                        );

                        fprintf(ChdFile, "\n");
                        fprintf(ChdFile, "# SourceFiles (FileId;FilePath)\n");
                        fprintf(ChdFile, "1;%s\n", Files[1]);

                        fprintf(ChdFile, "\n");
                        fprintf(ChdFile, "# TargetFiles (FileId;FilePath)\n");
                        fprintf(ChdFile, "1;%s\n", ChdPath.Data);

                        fprintf(ChdFile, "\n");
                        fprintf(ChdFile, "# Labels (LabelName;MemoryOffset)\n");
                        for (int LabelIndex = 0;
                            LabelIndex < Assembled.Labels.NumElements;
                            ++LabelIndex)
                        {
                            label* Label = Assembled.Labels.Data() + LabelIndex;
                            strc LabelName = Str(Label->NameCursor);
                            fprintf(ChdFile, STR_FMT ";0x%04X\n", STR_FMTARG(LabelName), Label->MemoryOffset);
                        }

                        fprintf(ChdFile, "\n");
                        fprintf(ChdFile, "# Infos (FileId;Line;Column;MemoryOffset;GeneratedInstruction;SourceString)\n");
                        for (int InfoIndex = 0;
                            InfoIndex < Assembled.DebugInfos.NumElements;
                            ++InfoIndex)
                        {
                            debug_info* Info = Assembled.DebugInfos.Data() + InfoIndex;
                            fprintf(ChdFile, "%d;%d;%d;0x%04X;%04X;\"" STR_FMT "\"\n", Info->FileId, Info->Line, Info->Column, Info->MemoryOffset, Info->GeneratedInstruction, STR_FMTARG(Info->SourceLine));
                        }
                        fclose(ChdFile);
                    }
                    else
                    {
                        fprintf(stderr, "Unable to open file for writing: %s", (char const*)ChdPath.Data);
                    }
                }
            }
            else if (Mode == commandline_mode::Disassemble)
            {
                char* Current = ContentsBegin;
                while (Current < ContentsEnd)
                {
                    instruction_decoder Decoder{};
                    Decoder.Data = ReadWord(Current);
                    Current += sizeof(u16);

                    // Can only trigger if Current and ContentsEnd are not aligned to 2 bytes relative to each other!
                    MTB_ASSERT(Current <= ContentsEnd);

                    instruction Instruction = DecodeInstruction(Decoder);
                    text Code = DisassembleInstruction(Instruction);
                    fprintf(OutFile, STR_FMT "\n", STR_FMTARG(Code));
                }

                Result = 0;
            }
            else
            {
                MTB_ASSERT(!"invalid code path");
            }

            if (OutFile != stdout)
            {
                fclose(OutFile);
            }
        }

        if (ContentsBegin)
        {
            free(ContentsBegin);
        }
    }

    //
    //
    //

end:
    return Result;
}
