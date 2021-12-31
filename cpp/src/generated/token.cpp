// Generated on 2021-12-31 12:16:59

#include "token.h"

static token
CreateToken(strc String)
{
    token Result{};
    Append(&Result, String);
    return Result;
}

static str
Str(token Text)
{
  str Result{ Text.Size, Text.Data };

  return Result;
}

void
EnsureZeroTerminated(token* Text)
{
  if(Text->Size < Text->FullCapacity)
    Text->Data[Text->Size] = 0;
}

void
Clear(token* Text)
{
  Text->Size = 0;
  EnsureZeroTerminated(Text);
}

token
Trim(token Text)
{
  str Trimmed = Trim(Str(Text));
  token Result = CreateToken(Trimmed);

  return Result;
}

static int
Append(token* Text, strc String)
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
Append(token* Text, char Char)
{
  int Result = Append(Text, str{ 1, &Char });

  return Result;
}

static int Compare(token* A, token* B)
{
  int Result = mtb_StringCompare(A->Size, A->Data, B->Size, B->Data);

  return Result;
}

static bool AreEqual(token* A, token* B)
{
  int ComparisonResult = Compare(A, B);

  return ComparisonResult == 0;
}

