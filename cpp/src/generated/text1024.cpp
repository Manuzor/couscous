// Generated on 2021-12-31 12:16:59

#include "text1024.h"

static text1024
CreateText1024(strc String)
{
    text1024 Result{};
    Append(&Result, String);
    return Result;
}

static str
Str(text1024 Text)
{
  str Result{ Text.Size, Text.Data };

  return Result;
}

void
EnsureZeroTerminated(text1024* Text)
{
  if(Text->Size < Text->FullCapacity)
    Text->Data[Text->Size] = 0;
}

void
Clear(text1024* Text)
{
  Text->Size = 0;
  EnsureZeroTerminated(Text);
}

text1024
Trim(text1024 Text)
{
  str Trimmed = Trim(Str(Text));
  text1024 Result = CreateText1024(Trimmed);

  return Result;
}

static int
Append(text1024* Text, strc String)
{
  int NewSize = Text->Size + String.Size;
  MTB_AssertDebug(NewSize < Text->Capacity, "Result would be too long to append");
  if (NewSize > Text->Capacity)
  {
    NewSize = Text->Capacity;
  }

  int NumCopies = NewSize - Text->Size;
  mtb_CopyBytes(NumCopies, Text->Data + Text->Size, String.Data);
  Text->Size = NewSize;

  return NumCopies;
}

static int
Append(text1024* Text, char Char)
{
  int Result = Append(Text, str{ 1, &Char });

  return Result;
}

static int Compare(text1024* A, text1024* B)
{
  int Result = mtb_StringCompare(A->Size, A->Data, B->Size, B->Data);

  return Result;
}

static bool AreEqual(text1024* A, text1024* B)
{
  int ComparisonResult = Compare(A, B);

  return ComparisonResult == 0;
}

