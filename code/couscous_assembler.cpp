#define MTB_IMPLEMENTATION
#include "mtb.h"

#include <stdio.h>

#define COUSCOUS_ASSEMBLER 1
#include "couscous.h"
#include "couscous.cpp"

struct raw_array
{
  int ElementSize;
  int NumElements;
  int Capacity;
  void* Data;
};

void
Deallocate(raw_array* Array)
{
  if (Array->Data)
  {
    free(Array->Data);
  }

  *Array = {};
}

void*
Add(raw_array* Array, int NumToAdd = 1)
{
  MTB_AssertDebug(NumToAdd > 0);
  int RequiredCapacity = Array->NumElements + NumToAdd;
  if (RequiredCapacity > Array->Capacity)
  {
    int NewCapacity = 32;

    if (Array->Capacity > 0)
      NewCapacity = Array->Capacity;

    while (NewCapacity < RequiredCapacity)
      NewCapacity *= 2;

    void* OldData = Array->Data;
    void* NewData = (instruction*)malloc(NewCapacity * Array->ElementSize);

    if (OldData)
    {
      mtb_CopyBytes(Array->NumElements * Array->ElementSize, NewData, OldData);
      free(OldData);
    }

    Array->Data = NewData;
    Array->Capacity = NewCapacity;
  }

  void* Result = ((u8*)Array->Data) + Array->NumElements * Array->ElementSize;
  Array->NumElements += NumToAdd;

  return Result;
}

template<typename T>
raw_array
ToRawArray(T* Array)
{
  raw_array Result{};
  Result.ElementSize = sizeof((*Array->Data));
  Result.NumElements = Array->NumElements;
  Result.Capacity = Array->Capacity;
  Result.Data = Array->Data;

  return Result;
}

template<typename T>
T
FromRawArray(raw_array* Array)
{
  T Result{};
  Result.NumElements = Array->NumElements;
  Result.Capacity = Array->Capacity;
  Result.Data = (decltype(Result.Data))Array->Data;

  return Result;
}

struct instruction_array
{
  int NumElements;
  int Capacity;
  instruction* Data;
};

void
Add(instruction_array* Array, instruction Instruction)
{
  raw_array Raw = ToRawArray(Array);
  instruction* Free = (instruction*)Add(&Raw);
  *Free = Instruction;
  *Array = FromRawArray<instruction_array>(&Raw);
}

instruction*
At(instruction_array* Array, int Index)
{
  MTB_AssertDebug(Index >= 0);
  MTB_AssertDebug(Index < Array->NumElements);
  return Array->Data + Index;
}

void
Deallocate(instruction_array* Array)
{
  raw_array Raw = ToRawArray(Array);
  Deallocate(&Raw);
  *Array = {};
}

struct text
{
  int Length;
  char Data[128];
};

text
Trim(text Text)
{
  int Front = 0;
  for (size_t Index = 0; Index < Text.Length; ++Index)
  {
    if (mtb_IsWhitespace(Text.Data[Index]))
    {
      ++Front;
    }
    else
    {
      break;
    }
  }

  int Back = 0;
  for (size_t IndexPlusOne = Text.Length; IndexPlusOne > 0; --IndexPlusOne)
  {
    size_t Index = IndexPlusOne - 1;
    if (mtb_IsWhitespace(Text.Data[Index]))
    {
      ++Back;
    }
    else
    {
      break;
    }
  }

  text Result{};
  if (Front > 0 || Back > 0)
  {
    int NewLength = Text.Length - Front - Back;
    if (Result.Length > 0)
    {
      Result.Length = NewLength;
      mtb_CopyBytes(NewLength, Result.Data, Text.Data + Front);
    }
  }
  else
  {
    Result = Text;
  }

  return Result;
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

      assembler_code Test{};
      Test.Size = 4;
      instruction Foo = AssembleInstruction(Test);

      while(true)
      {
        char* LineStart = Current;
        while (Current[0] != '\n' && Current[0] != '#')
        {
          ++Current;
            if (Current >= ContentsOnePastLast)
              goto EndOfContentParsing;
        }

        char* LineOnePastLast = Current;

        if (Current[0] == '\n')
        {
          ++Current;
        }
        else if (Current[0] == '#')
        {
          // Skip comments.
          while (Current[0] != '\n')
          {
            ++Current;
            if (Current >= ContentsOnePastLast)
              goto EndOfContentParsing;
          }
        }
        else
        {
          MTB_Fail("Unknown character.");
        }

        text Text{};
        Text.Length = (int)(LineOnePastLast - LineStart);
        mtb_CopyBytes(Text.Length, Text.Data, LineStart);
        Text = Trim(Text);

        if (Text.Data[Text.Length - 1] == ':')
        {
          // We found a label!
          // TODO: Continue here!
        }
        else if(Text.Length <= 32)
        {
          assembler_code Code{};
          Code.Size = Text.Length;
          mtb_CopyBytes(Code.Size, Code.Data, Text.Data);

          instruction Instruction = AssembleInstruction(Code);
          Add(&Instructions, Instruction);
        }
      }
      EndOfContentParsing:

      for (int InstructionIndex = 0;
        InstructionIndex < Instructions.NumElements;
        ++InstructionIndex)
      {
        instruction* Instruction = At(&Instructions, InstructionIndex);
        u16 EncodedInstruction = EncodeInstruction(*Instruction);
        MTB_DEBUG_CODE(int NumPrintedChars =) fprintf(OutFile, "%04X", EncodedInstruction);
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

        // Can only trigger if Current and ContentsOnePastLast are not aligned to 2 bytes relative to each other!
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
