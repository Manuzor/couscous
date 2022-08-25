// Generated on 2022-08-21 23:27:38

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

static strc
StrConst(token Text)
{
    strc Result{ Text.Size, Text.Data };

    return Result;
}

void
EnsureZeroTerminated(token* Text)
{
    if (Text->Size < Text->FullCapacity)
    {
        Text->Data[Text->Size] = 0;
    }
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
    strc Trimmed = Trim(StrConst(Text));
    token Result = CreateToken(Trimmed);

    return Result;
}

static int
Append(token* Text, strc String)
{
    int NewSize = Text->Size + String.Size;
    MTB_ASSERT(NewSize < Text->Capacity);
    if (NewSize > Text->Capacity)
    {
        NewSize = Text->Capacity;
    }

    int NumCopies = NewSize - Text->Size;
    ::mtb::CopyBytes(Text->Data + Text->Size, String.Data, NumCopies);
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
    int Result = mtb::SliceCompareBytes(mtb::PtrSlice(A->Data, A->Size), mtb::PtrSlice(B->Data, B->Size));

    return Result;
}

static bool AreEqual(token* A, token* B)
{
    int ComparisonResult = Compare(A, B);

    return ComparisonResult == 0;
}

