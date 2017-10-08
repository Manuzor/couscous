#define MTB_IMPLEMENTATION
#include "mtb.h"

#include <stdio.h>

#define COUSCOUS_ASSEMBLER 1
#include "couscous.h"
#include "_generated/arrays.h"

struct label
{
  text Text;
  int InstructionIndex;
};

struct patch
{
  token LabelName;
  int InstructionIndex;
  int ArgumentIndex;
};

#include "couscous.cpp"
#include "_generated/arrays.cpp"

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

  for(int ArgIndex = 1; ArgIndex < NumArgs; ++ArgIndex)
  {
    char const* Arg = Args[ArgIndex];
    size_t ArgSize = mtb_StringLengthOf(Arg);
    if(ArgSize > 0)
    {
      if(Arg[0] == '-' && ArgSize > 1)
      {
        char const* ArgContent = Arg + 1;
        while(*ArgContent == '-')
          ++ArgContent;

        if(mtb_StringsAreEqual(ArgContent, "assemble"))
        {
          Mode = commandline_mode::Assemble;
        }
        else if(mtb_StringsAreEqual(ArgContent, "disassemble"))
        {
          Mode = commandline_mode::Disassemble;
        }
        else if(mtb_StringsAreEqual(ArgContent, "help"))
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
        if(FileIndex < mtb_ArrayLengthOf(Files))
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

  if(Mode == commandline_mode::NONE)
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
  {
    FILE* InFile = fopen(Files[0], "rb");
    fseek(InFile, 0, SEEK_END);
    int InFileSize = ftell(InFile);
    fseek(InFile, 0, SEEK_SET);

    ContentsBegin = (char*)malloc(InFileSize);
    ContentsOnePastLast = ContentsBegin + InFileSize;
    fread(ContentsBegin, InFileSize, 1, InFile);

    fclose(InFile);
  }

  {
    FILE* OutFile = nullptr;
    if(mtb_StringsAreEqual(Files[1], "-"))
      OutFile = stdout;
    else
      OutFile = fopen(Files[1], "wb");

    char* Current = ContentsBegin;
    if(Mode == commandline_mode::Assemble)
    {
      instruction_array Instructions{};
      MTB_DEFER[&]{ Deallocate(&Instructions); };

      label_array Labels{};
      MTB_DEFER[&]{ Deallocate(&Labels); };

      patch_array Patches{};
      MTB_DEFER[&]{ Deallocate(&Patches); };

      while(true)
      {
        Current = SkipWhitespaceAndComments(Current, ContentsOnePastLast);
        if (Current >= ContentsOnePastLast)
          goto EndOfContentParsing;

        char* LineStart = Current;

        Current = ParseLine(Current, ContentsOnePastLast);

        text Text{};
        Text.Size = (int)(Current - LineStart);
        mtb_CopyBytes(Text.Size, Text.Data, LineStart);
        Text = Trim(Text);

        if (Text.Data[Text.Size - 1] == ':')
        {
          // We found a label!
          label Label{};
          // Copy without the trailing colon
          Label.Text.Size = Text.Size - 1;
          mtb_CopyBytes(Label.Text.Size, Label.Text.Data, Text.Data);
          Label.InstructionIndex = Instructions.NumElements;

          *Add(&Labels) = Label;
        }
        else if (Text.Size <= 32)
        {
          text Code{};
          Code.Size = Text.Size;
          mtb_CopyBytes(Code.Size, Code.Data, Text.Data);

          assembler_tokens Tokens = Tokenize(Code);
          instruction Instruction = AssembleInstruction(Tokens);

          patch Patch{};
          Patch.InstructionIndex = Instructions.NumElements;
          switch (Instruction.Type)
          {
            case instruction_type::JP:
            {
              if (Instruction.Args[0].Type == argument_type::NONE && Instruction.Args[0].Type == argument_type::NONE)
              {
                Patch.LabelName = Tokens.Tokens[1];
                Patch.ArgumentIndex = 0;
              }
              else if (Instruction.Args[1].Type == argument_type::NONE)
              {
                Patch.LabelName = Tokens.Tokens[2];
                Patch.ArgumentIndex = 1;
              }

              break;
            }

            case instruction_type::CALL:
            {
              if (Instruction.Args[0].Type == argument_type::NONE)
              {
                Patch.LabelName = Tokens.Tokens[1];
                Patch.ArgumentIndex = 0;
              }

              break;
            }
          }

          if (Patch.LabelName.Size)
            *Add(&Patches) = Patch;

          *Add(&Instructions) = Instruction;
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
            instruction* Instruction = At(&Instructions, Patch->InstructionIndex);
            argument* Argument = Instruction->Args + Patch->ArgumentIndex;

            u16 PC = 0x200 + (u16)Label->InstructionIndex * 2;
            Argument->Type = argument_type::CONSTANT;
            Argument->Value = PC;

            Found = true;
            break;
          }
        }

        if (!Found)
        {
          // TODO: Diagnostics?
        }
      }

      u16_array ByteCode{};
      MTB_DEFER[&]{ Deallocate(&ByteCode); };

      for (int InstructionIndex = 0;
        InstructionIndex < Instructions.NumElements;
        ++InstructionIndex)
      {
        instruction* Instruction = At(&Instructions, InstructionIndex);
        u16 EncodedInstruction = EncodeInstruction(*Instruction);
        WriteWord(Add(&ByteCode), EncodedInstruction);
      }

      fwrite(ByteCode.Data, ByteCode.NumElements * sizeof(*ByteCode.Data), 1, OutFile);
    }
    else
    {
      MTB_AssertDebug(Mode == commandline_mode::Disassemble);

      while(Current < ContentsOnePastLast)
      {
        instruction_decoder Decoder{};
        Decoder.Data = ReadWord(Current);
        Current += sizeof(u16);

        // Can only trigger if Current and ContentsOnePastLast are not aligned to 2 bytes relative to each other!
        MTB_AssertDebug(Current <= ContentsOnePastLast);

        instruction Instruction = DecodeInstruction(Decoder);
        text Code = DisassembleInstruction(Instruction);
        fprintf(OutFile, "%*s\n", (int)Code.Size, Code.Data);
      }
    }

    if(OutFile != stdout)
      fclose(OutFile);
  }

  if(ContentsBegin)
    free(ContentsBegin);

  //
  //
  //

  Result = 0;
  end:
  return Result;
}
