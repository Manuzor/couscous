#define MTB_IMPLEMENTATION
#include "mtb.h"

#include <stdio.h>

#define COUSCOUS_ASSEMBLER 1
#include "couscous.h"
#include "_generated/instruction_array.h"
#include "_generated/label_array.h"
#include "_generated/patch_array.h"
#include "_generated/u16_array.h"

#include "couscous.cpp"
#include "_generated/all_generated.cpp"

static char*
SkipWhitespaceAndComments(char* Begin, char* End)
{
    while (true)
    {
        while (Begin < End && mtb_IsWhitespace(Begin[0]))
            ++Begin;

        // Comments
        if (Begin < End && Begin[0] == '#')
        {
            while (Begin < End && Begin[0] != '\n')
                ++Begin;

            continue;
        }

        break;
    }

    return Begin;
}

static char*
ParseLine(char* Begin, char* End)
{
    while (Begin < End && Begin[0] != '\n' && Begin[0] != '#')
        ++Begin;

    return Begin;
}

static void
PrintHelp(FILE* OutFile)
{
    fprintf(OutFile, "Usage: couscous_assembler [-help] (-assemble|-disassemble) <in_file> [<out_file>]\n");
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
            u16 BaseMemoryOffset = 0x200u;
            u16 CurrentMemoryOffset = BaseMemoryOffset;

            label_array Labels{};
            MTB_DEFER[&]{ Deallocate(&Labels); };

            patch_array Patches{};
            MTB_DEFER[&]{ Deallocate(&Patches); };

            u16_array ByteCode{};
            MTB_DEFER[&]{ Deallocate(&ByteCode); };

            while (true)
            {
                Current = SkipWhitespaceAndComments(Current, ContentsOnePastLast);
                if (Current >= ContentsOnePastLast)
                    goto EndOfContentParsing;

                char* LineStart = Current;

                Current = ParseLine(Current, ContentsOnePastLast);

                text Text{};
                Append(&Text, (int)(Current - LineStart), LineStart);
                Text = Trim(Text);

                if (Text.Data[Text.Size - 1] == ':')
                {
                    // TODO: Continue here! Debug why it won't accept "LOOP:" as a valid label.

                    // We found a label!
                    label Label{};
                    // Copy without the trailing colon
                    Label.Text = CreateText(Text.Size - 1, Text.Data);
                    Label.MemoryOffset = CurrentMemoryOffset;

                    *Add(&Labels) = Label;
                }
                else if (Text.Data[0] == '[' && Text.Data[Text.Size - 1] == ']' && Text.Size >= 3)
                {
                    // We found a data section.

                    char* DataBegin = Text.Data;
                    char* DataEnd = DataBegin + Text.Size - 1;

                    char* DataCurrent = SkipWhitespaceAndComments(DataBegin, DataEnd);
                    if (DataCurrent >= DataEnd)
                        goto EndOfContentParsing;

                    char DataType = *DataCurrent;
                    DataCurrent++;
                    DataCurrent = SkipWhitespaceAndComments(DataBegin, DataEnd);

                    if (DataCurrent >= DataEnd)
                    {
                        MTB_AssertDebug(false, "Unexpected end of data section.");
                        goto EndOfContentParsing;
                    }

                    int DataSectionSize = (int)(DataEnd - DataCurrent);

                    // TODO: Complete the implementation below.
                    switch (DataType)
                    {
                        case 'b':
                        case 'B':
                        {
                            bool IsMultipleOf8 = (DataSectionSize & 0b0111) == 0;
                            if (!IsMultipleOf8)
                            {
                                MTB_AssertDebug(false, "Data section must represent full bytes!");
                                goto EndOfContentParsing;
                            }

                            CurrentMemoryOffset += DataSectionSize / 8;

                            u8 Byte = 0;
                            int NumShifts = 0;
                            while (DataCurrent < DataEnd)
                            {
                                if (DataCurrent[0] == '1')
                                    Byte |= 1;

                                Byte <<= 1;
                                ++NumShifts;
                                if (NumShifts >= 8)
                                {
                                    NumShifts = 0;
                                    ++CurrentMemoryOffset;
                                }
                            }
                        } break;

                        case 'x':
                        case 'X':
                        {
                            bool IsMultipleOf4 = (DataSectionSize & 0b0011) == 0;
                            if (!IsMultipleOf4)
                            {
                                MTB_AssertDebug(false, "Data section must represent full bytes!");
                                goto EndOfContentParsing;
                            }

                            CurrentMemoryOffset += DataSectionSize / 4;
                        } break;

                        default:
                            MTB_AssertDebug("Unsupported data type in data section. Must be either of [b] or [x].");
                            break;
                    }
                }
                else
                {
                    text Code = CreateText(Text.Size, Text.Data);

                    assembler_tokens Tokens = Tokenize(Code);
                    instruction Instruction = AssembleInstruction(Tokens);

                    patch Patch{};
                    Patch.InstructionMemoryOffset = CurrentMemoryOffset;
                    switch (Instruction.Type)
                    {
                        case instruction_type::JP:
                        {
                            if (Instruction.Args[0].Type == argument_type::NONE)
                            {
                                // e.g. JP 0x234
                                Patch.LabelName = Tokens.Tokens[1];
                            }
                            else if (Instruction.Args[0].Type == argument_type::V && Instruction.Args[0].Value == 0 &&
                                     Instruction.Args[1].Type == argument_type::NONE)
                            {
                                // e.g. JP V0 0x234
                                Patch.LabelName = Tokens.Tokens[2];
                            }
                        } break;

                        case instruction_type::CALL:
                        {
                            if (Instruction.Args[0].Type == argument_type::NONE)
                            {
                                // e.g. CALL 0x234
                                Patch.LabelName = Tokens.Tokens[1];
                            }
                        } break;

                        case instruction_type::LD:
                        {
                            if (Instruction.Args[0].Type == argument_type::I &&
                                Instruction.Args[1].Type == argument_type::NONE)
                            {
                                // e.g. LD I 0x234
                                Patch.LabelName = Tokens.Tokens[2];
                            }
                        } break;
                    }

                    if (Patch.LabelName.Size)
                        *Add(&Patches) = Patch;

                    u16 EncodedInstruction = EncodeInstruction(Instruction);
                    u16* NewWord = Add(&ByteCode);
                    WriteWord(NewWord, EncodedInstruction);

                    CurrentMemoryOffset += 2;
                }
            }
            EndOfContentParsing:

            // Apply patches
            for (int PatchIndex = 0;
                PatchIndex < Patches.NumElements;
                ++PatchIndex)
            {
                patch* Patch = Patches.Data + PatchIndex;

                bool Found = false;
                for (int LabelIndex = 0;
                    LabelIndex < Labels.NumElements;
                    ++LabelIndex)
                {
                    label* Label = Labels.Data + LabelIndex;
                    if (mtb_StringsAreEqual(Label->Text.Size, Label->Text.Data, Patch->LabelName.Size, Patch->LabelName.Data))
                    {
                        u16* InstructionLocation = At(&ByteCode, Patch->InstructionMemoryOffset);
                        u16 EncodedInstruction = ReadWord(InstructionLocation);
                        EncodedInstruction |= (Label->MemoryOffset & 0x0FFF);
                        WriteWord(InstructionLocation, EncodedInstruction);

                        Found = true;
                        break;
                    }
                }

                if (!Found)
                {
                    MTB_Fail("Unknown label");
                    // TODO: Diagnostics?
                }
            }

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
