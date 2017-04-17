#define MTB_IMPLEMENTATION
#include "mtb.h"

#include <stdio.h>

#define COUSCOUS_ASSEMBLER 1
#include "couscous.hpp"
#include "couscous.cpp"


static void
PrintHelp(FILE* OutFile)
{
  fprintf(OutFile, "Usage: couscous_assembler [-help] [-assemble|-disassemble] <in_file> <out_file>\n");
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

  if(Mode == commandline_mode::NONE)
  {
    fprintf(stderr, "Missing -assemble or -disassemble.\n");
    PrintHelp(stderr);
    goto end;
  }

  if(FileIndex < 1)
  {
    fprintf(stderr, "Missing input file path.\n");
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
      while(Current < ContentsOnePastLast)
      {
        char* LineStart = Current;
        while(Current < ContentsOnePastLast && Current[0] != '\n' && Current[0] != ';')
          ++Current;
        char* LineOnePastLast = Current;

        if(Current < ContentsOnePastLast && Current[0] == '\n' || Current[0] == ';')
          ++Current;

        assembler_code Code{};
        Code.Size = LineOnePastLast - LineStart;
        mtb_CopyBytes(Code.Size, Code.Data, LineStart);
        instruction Instruction = AssembleInstruction(Code);
        u16 EncodedInstruction = EncodeInstruction(Instruction);
        int NumPrintedChars = fprintf(OutFile, "%04X", EncodedInstruction);
        MTB_AssertDebug(NumPrintedChars == 4);
      }
    }
    else
    {
      MTB_AssertDebug(Mode == commandline_mode::Disassemble);

      while(Current < ContentsOnePastLast)
      {
        instruction_decoder Decoder{};
        Decoder.Data = ReadWord(Current);
        Current += sizeof(u16);
        MTB_AssertDebug(Current <= ContentsOnePastLast);

        instruction Instruction = DecodeInstruction(Decoder);
        assembler_code Code = DisassembleInstruction(Instruction);
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
