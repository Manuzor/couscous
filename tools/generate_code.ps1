param(
  [string]$RepoRoot = "$PSScriptRoot/..",
  [string]$OutDir = "$RepoRoot/code/_generated",
  [switch]$StaticFunctions = $true,
  [string]$PragmaOnce
)

$ErrorActionPreference = "Stop"

$OutDir = mkdir -Force $OutDir

$UmbrellaHeaderIncludes = @()
$UmbrellaIncludes = @()

$FunctionQualifiers = "$(if($StaticFunctions) { "static " })"
$DateStamp = Get-Date -f "yyyy-MM-dd HH:mm:ss"

#
# Arrays
#
$Arrays = @(
  # @{ Name = "instruction_array"; Type = "instruction" };
  @{ Name = "label_array"; Type = "label"; FixedCapacity = 32 };
  @{ Name = "patch_array"; Type = "patch"; FixedCapacity = 32 };
  @{ Name = "u8_array"; Type = "u8"; FixedCapacity = 32 };
  @{ Name = "str_array"; Type = "str"; FixedCapacity = 32 };
  @{ Name = "token_array"; Type = "token"; FixedCapacity = 8 };
)

foreach($Array in $Arrays)
{
  $HeaderContent = @"
// Generated on $DateStamp
$(if($PragmaOnce) { "#pragma once" })

struct $($Array.Name)
{
  int NumElements;
  int Capacity;
  $($Array.Type)* _Data;

  enum { FixedCapacity = $($Array.FixedCapacity) };
  $($Array.Type) _Fixed[FixedCapacity];

  $($Array.Type)* Data() { return _Data ? _Data : _Fixed; }
};

${FunctionQualifiers}void
Reserve($($Array.Name)* Array, int RequiredCapacity);

${FunctionQualifiers}$($Array.Type)*
Add($($Array.Name)* Array, int NumToAdd = 1);

${FunctionQualifiers}$($Array.Type)*
At($($Array.Name)* Array, int Index);

${FunctionQualifiers}void
Deallocate($($Array.Name)* Array);

template<typename predicate>
${FunctionQualifiers}$($Array.Type)*
Find($($Array.Name)* Array, predicate Predicate)
{
    for (int Index = 0; Index < Array->NumElements; ++Index)
    {
        $($Array.Type)* Item = Array->Data() + Index;
        if (Predicate(Item))
        {
            return Item;
        }
    }

    return nullptr;
}

"@
  $HeaderFilePath = "$OutDir/$($Array.Name).h"
  $UmbrellaHeaderIncludes += $HeaderFilePath
  Set-Content $HeaderFilePath $HeaderContent

  $Content = @"
// Generated on $DateStamp

${FunctionQualifiers}void
Reserve($($Array.Name)* Array, int RequiredCapacity)
{
  if (Array->_Data == nullptr || Array->Capacity == 0)
  {
    Array->Capacity = Array->FixedCapacity;
    Array->_Data = nullptr;
  }

  if (RequiredCapacity > Array->Capacity)
  {
    int NewCapacity = 2 * $($Array.Name)::FixedCapacity;

    if (Array->Capacity > 0)
      NewCapacity = Array->Capacity;

    while (NewCapacity < RequiredCapacity)
      NewCapacity *= 2;

    void* OldData = Array->_Data;
    void* NewData = malloc(NewCapacity * sizeof($($Array.Type)));

    if (OldData)
    {
      mtb_CopyBytes(Array->NumElements * sizeof($($Array.Type)), NewData, OldData);
      free(OldData);
    }

    Array->_Data = ($($Array.Type)*)NewData;
    Array->Capacity = NewCapacity;
  }
}

${FunctionQualifiers}$($Array.Type)*
Add($($Array.Name)* Array, int NumToAdd)
{
  Reserve(Array, Array->NumElements + NumToAdd);
  $($Array.Type)* Result = Array->Data() + Array->NumElements;
  Array->NumElements += NumToAdd;

  return Result;
}

${FunctionQualifiers}$($Array.Type)*
At($($Array.Name)* Array, int Index)
{
  MTB_AssertDebug(Index >= 0);
  MTB_AssertDebug(Index < Array->NumElements);
  return Array->Data() + Index;
}

${FunctionQualifiers}void
Deallocate($($Array.Name)* Array)
{
  if (Array->_Data)
  {
    free(Array->_Data);
  }
  *Array = {};
}

"@
  $FilePath = "$OutDir/$($Array.Name).cpp"
  $UmbrellaIncludes += $FilePath
  Set-Content $FilePath $Content
}

#
# TextTypes
#
$TextTypes = @(
  @{ Name = "text"; FixedCapacity = "128" };
  @{ Name = "token"; FixedCapacity = "32" };
  @{ Name = "text1024"; FixedCapacity = "1024" };
)

foreach($Text in $TextTypes)
{
  $CtorName = "Create$([char]::ToUpper($Text.Name[0]) + $Text.Name.Substring(1))"
  $HeaderContent = @"
// Generated on $DateStamp
#pragma once

struct $($Text.Name)
{
  enum { Capacity = $($Text.FixedCapacity) };

  int Size;
  char Data[Capacity];
};

// "Constructor"
${FunctionQualifiers}$($Text.Name) $($CtorName)(strc String);

// To str
${FunctionQualifiers}str Str($($Text.Name)* Text);

// Trim
${FunctionQualifiers}$($Text.Name) Trim($($Text.Name) Text);

// Append
${FunctionQualifiers}int Append($($Text.Name)* Text, strc String);

// Comparison
${FunctionQualifiers}int Compare($($Text.Name)* A, $($Text.Name)* B);
${FunctionQualifiers}bool AreEqual($($Text.Name)* A, $($Text.Name)* B);

"@
  $HeaderFilePath = "$OutDir/$($Text.Name).h"
  $UmbrellaHeaderIncludes += $HeaderFilePath
  Set-Content $HeaderFilePath $HeaderContent

  $Content = @"
// Generated on $DateStamp

#include "$($Text.Name).h"

${FunctionQualifiers}$($Text.Name)
$($CtorName)(strc String)
{
    $($Text.Name) Result{};
    Append(&Result, String);
    return Result;
}

${FunctionQualifiers}str
Str($($Text.Name)* Text)
{
  str Result{ (size_t)Text->Size, Text->Data };

  return Result;
}

$($Text.Name)
Trim($($Text.Name) Text)
{
  str Trimmed = Trim(Str(&Text));
  $($Text.Name) Result = $($CtorName)(Trimmed);

  return Result;
}

${FunctionQualifiers}int
Append($($Text.Name)* Text, strc String)
{
  size_t NewSize = Text->Size + String.Size;
  MTB_AssertDebug(NewSize < Text->Capacity, `"Result would be too long to append`");
  if (NewSize > Text->Capacity)
  {
    NewSize = Text->Capacity;
  }

  int NumCopies = (int)NewSize - Text->Size;
  mtb_CopyBytes(NumCopies, Text->Data + Text->Size, String.Data);
  Text->Size = (int)NewSize;

  return NumCopies;
}

${FunctionQualifiers}int Compare($($Text.Name)* A, $($Text.Name)* B)
{
  int Result = mtb_StringCompare(A->Size, A->Data, B->Size, B->Data);

  return Result;
}

${FunctionQualifiers}bool AreEqual($($Text.Name)* A, $($Text.Name)* B)
{
  int ComparisonResult = Compare(A, B);

  return ComparisonResult == 0;
}

"@
  $FilePath = "$OutDir/$($Text.Name).cpp"
  $UmbrellaIncludes += $FilePath
  Set-Content $FilePath $Content
}

#
# Write the umbrella files
#
$UmbrellaBaseFilePath = "$OutDir/all_generated"

$UmbrellaHeaderContent = @"
// Generated on $DateStamp
$($UmbrellaHeaderIncludes | % { "#include <$_>`n" })
"@
Set-Content "$UmbrellaBaseFilePath.h" $UmbrellaHeaderContent

$UmbrellaContent = @"
// Generated on $DateStamp
$($UmbrellaIncludes | % { "#include <$_>`n" })
"@
Set-Content "$UmbrellaBaseFilePath.cpp" $UmbrellaContent
