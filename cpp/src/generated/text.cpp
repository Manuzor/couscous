// Generated on 2022-08-21 23:27:38

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

static strc
StrConst(text Text)
{
    strc Result{ Text.Size, Text.Data };

    return Result;
}

void
EnsureZeroTerminated(text* Text)
{
    if (Text->Size < Text->FullCapacity)
    {
        Text->Data[Text->Size] = 0;
    }
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
    strc Trimmed = Trim(StrConst(Text));
    text Result = CreateText(Trimmed);

    return Result;
}

static int
Append(text* Text, strc String)
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
Append(text* Text, char Char)
{
    int Result = Append(Text, str{ 1, &Char });

    return Result;
}

static int Compare(text* A, text* B)
{
    int Result = mtb::SliceCompareBytes(mtb::PtrSlice(A->Data, A->Size), mtb::PtrSlice(B->Data, B->Size));

    return Result;
}

static bool AreEqual(text* A, text* B)
{
    int ComparisonResult = Compare(A, B);

    return ComparisonResult == 0;
}

