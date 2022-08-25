// Generated on 2022-08-21 23:27:38

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

static strc
StrConst(text1024 Text)
{
    strc Result{ Text.Size, Text.Data };

    return Result;
}

void
EnsureZeroTerminated(text1024* Text)
{
    if (Text->Size < Text->FullCapacity)
    {
        Text->Data[Text->Size] = 0;
    }
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
    strc Trimmed = Trim(StrConst(Text));
    text1024 Result = CreateText1024(Trimmed);

    return Result;
}

static int
Append(text1024* Text, strc String)
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
Append(text1024* Text, char Char)
{
    int Result = Append(Text, str{ 1, &Char });

    return Result;
}

static int Compare(text1024* A, text1024* B)
{
    int Result = mtb::SliceCompareBytes(mtb::PtrSlice(A->Data, A->Size), mtb::PtrSlice(B->Data, B->Size));

    return Result;
}

static bool AreEqual(text1024* A, text1024* B)
{
    int ComparisonResult = Compare(A, B);

    return ComparisonResult == 0;
}

