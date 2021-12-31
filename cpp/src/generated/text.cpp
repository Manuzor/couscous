// Generated on 2021-12-31 12:16:59

#include "text.h"

static text
CreateText(strc String)
{
    text Result{};
    Append(&Result, String);
    return Result;
}

static str
Str(text Text)
{
  str Result{ Text.Size, Text.Data };

  return Result;
}

void
EnsureZeroTerminated(text* Text)
{
  if(Text->Size < Text->FullCapacity)
    Text->Data[Text->Size] = 0;
}

void
Clear(text* Text)
{
  Text->Size = 0;
  EnsureZeroTerminated(Text);
}

text
Trim(text Text)
{
  str Trimmed = Trim(Str(Text));
  text Result = CreateText(Trimmed);

  return Result;
}

static int
Append(text* Text, strc String)
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
Append(text* Text, char Char)
{
  int Result = Append(Text, str{ 1, &Char });

  return Result;
}

static int Compare(text* A, text* B)
{
  int Result = mtb_StringCompare(A->Size, A->Data, B->Size, B->Data);

  return Result;
}

static bool AreEqual(text* A, text* B)
{
  int ComparisonResult = Compare(A, B);

  return ComparisonResult == 0;
}

