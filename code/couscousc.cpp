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

#include "_generated/text.h"
#include "_generated/token.h"
#include "_generated/label_array.h"
#include "_generated/patch_array.h"
#include "_generated/u8_array.h"
#include "couscous.h"

#include "couscous.cpp"
#include "_generated/all_generated.cpp"

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

    int Result = 1;

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
    char* ContentsOnePastLast = nullptr;

    FILE* InFile = fopen(Files[0], "rb");
    fseek(InFile, 0, SEEK_END);
    int InFileSize = ftell(InFile);
    fseek(InFile, 0, SEEK_SET);

    ContentsBegin = (char*)malloc(InFileSize);
    ContentsOnePastLast = ContentsBegin + InFileSize;
    fread(ContentsBegin, InFileSize, 1, InFile);

    fclose(InFile);

    {
        FILE* OutFile = nullptr;
        if (mtb_StringsAreEqual(Files[1], "-"))
            OutFile = stdout;
        else
            OutFile = fopen(Files[1], "wb");

        char* Current = ContentsBegin;
        if (Mode == commandline_mode::Assemble)
        {
            u8_array ByteCode{};
            MTB_DEFER[&]{ Deallocate(&ByteCode); };

            AssembleCode(ContentsBegin, ContentsOnePastLast, &ByteCode);

            // Write the result!
            fwrite(ByteCode.Data, ByteCode.NumElements * sizeof(*ByteCode.Data), 1, OutFile);
        }
        else if (Mode == commandline_mode::Disassemble)
        {
            while (Current < ContentsOnePastLast)
            {
                instruction_decoder Decoder{};
                Decoder.Data = ReadWord(Current);
                Current += sizeof(u16);

                // Can only trigger if Current and ContentsOnePastLast are not aligned to 2 bytes relative to each other!
                MTB_AssertDebug(Current <= ContentsOnePastLast);

                instruction Instruction = DecodeInstruction(Decoder);
                text Code = DisassembleInstruction(Instruction);
                fprintf(OutFile, "%*s\n", Code.Size, Code.Data);
            }
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

    Result = 0;
end:
    return Result;
}
