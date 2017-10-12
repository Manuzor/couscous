param(
  [string]$RepoRoot = "$PSScriptRoot/..",
  [string]$OutDir = "$RepoRoot/code/_generated",
  [string]$StaticFunctions = $true
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
  # @{ Name = "instruction_array"; Type = "instruction"; ForwardDeclareType = $true };
  @{ Name = "label_array"; Type = "label"; ForwardDeclareType = $true };
  @{ Name = "patch_array"; Type = "patch"; ForwardDeclareType = $true };
  @{ Name = "u8_array"; Type = "u8"; ForwardDeclareType = $false };
)

foreach($Array in $Arrays)
{
  $HeaderContent = @"
// Generated on $DateStamp
#pragma once

$(if($Array.ForwardDeclareType) { "struct $($Array.Type);" })

struct $($Array.Name)
{
  int NumElements;
  int Capacity;
  $($Array.Type)* Data;
};

${FunctionQualifiers}void
Reserve($($Array.Name)* Array, int RequiredCapacity);

${FunctionQualifiers}$($Array.Type)*
Add($($Array.Name)* Array, int NumToAdd = 1);

${FunctionQualifiers}$($Array.Type)*
At($($Array.Name)* Array, int Index);

${FunctionQualifiers}void
Deallocate($($Array.Name)* Array);
"@
  $HeaderFilePath = "$OutDir/$($Array.Name).h"
  $UmbrellaHeaderIncludes += $HeaderFilePath
  Set-Content $HeaderFilePath $HeaderContent

  $Content = @"
// Generated on $DateStamp

${FunctionQualifiers}void
Reserve($($Array.Name)* Array, int RequiredCapacity)
{
  if (RequiredCapacity > Array->Capacity)
  {
    int NewCapacity = 32;

    if (Array->Capacity > 0)
      NewCapacity = Array->Capacity;

    while (NewCapacity < RequiredCapacity)
      NewCapacity *= 2;

    void* OldData = Array->Data;
    void* NewData = malloc(NewCapacity * sizeof($($Array.Type)));

    if (OldData)
    {
      mtb_CopyBytes(Array->NumElements * sizeof($($Array.Type)), NewData, OldData);
      free(OldData);
    }

    Array->Data = ($($Array.Type)*)NewData;
    Array->Capacity = NewCapacity;
  }
}

${FunctionQualifiers}$($Array.Type)*
Add($($Array.Name)* Array, int NumToAdd)
{
  Reserve(Array, Array->NumElements + NumToAdd);
  $($Array.Type)* Result = Array->Data + Array->NumElements;
  Array->NumElements += NumToAdd;

  return Result;
}

${FunctionQualifiers}$($Array.Type)*
At($($Array.Name)* Array, int Index)
{
  MTB_AssertDebug(Index >= 0);
  MTB_AssertDebug(Index < Array->NumElements);
  return Array->Data + Index;
}

${FunctionQualifiers}void
Deallocate($($Array.Name)* Array)
{
  if (Array->Data)
  {
    free(Array->Data);
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
  @{ Name = "token"; FixedCapacity = "128" };
)

foreach($Text in $TextTypes)
{
  $CtorName = "Create$([char]::ToUpper($Text.Name[0]) + $Text.Name.Substring(1))"
  $HeaderContent = @"
// Generated on $DateStamp
#pragma once

struct $($Text.Name)
{
  int Size;
  char Data[$($Text.FixedCapacity)];
};

${FunctionQualifiers}$($Text.Name)
$($CtorName)(char const* String);

${FunctionQualifiers}$($Text.Name)
$($CtorName)(size_t StringLength, char const* String);

${FunctionQualifiers}$($Text.Name)
Trim($($Text.Name) Text);

${FunctionQualifiers}int
Append($($Text.Name)* Text, char const* String);

${FunctionQualifiers}int
Append($($Text.Name)* Text, size_t StringLength, char const* String);
"@
  $HeaderFilePath = "$OutDir/$($Text.Name).h"
  $UmbrellaHeaderIncludes += $HeaderFilePath
  Set-Content $HeaderFilePath $HeaderContent

  $Content = @"
// Generated on $DateStamp

#include "$($Text.Name).h"

${FunctionQualifiers}$($Text.Name)
$($CtorName)(char const* String)
{
  size_t StringLength = mtb_StringLengthOf(String);
  return $($CtorName)(StringLength, String);
}

${FunctionQualifiers}$($Text.Name)
$($CtorName)(size_t StringLength, char const* String)
{
    $($Text.Name) Result{};
    Append(&Result, StringLength, String);
    return Result;
}

$($Text.Name)
Trim($($Text.Name) Text)
{
  int Front = 0;
  for (size_t Index = 0; Index < Text.Size; ++Index)
  {
    if (!mtb_IsWhitespace(Text.Data[Index]))
      break;

    ++Front;
  }

  int Back = 0;
  for (size_t IndexPlusOne = Text.Size; IndexPlusOne > 0; --IndexPlusOne)
  {
    size_t Index = IndexPlusOne - 1;
    if (!mtb_IsWhitespace(Text.Data[Index]))
      break;

    ++Back;
  }

  $($Text.Name) Result{};
  if (Front > 0 || Back > 0)
  {
    int NewLength = Text.Size - Front - Back;
    if (NewLength > 0)
    {
      Result.Size = NewLength;
      mtb_CopyBytes(Result.Size, Result.Data, Text.Data + Front);
    }
  }
  else
  {
    Result = Text;
  }

  return Result;
}

${FunctionQualifiers}int
Append($($Text.Name)* Text, char const* String)
{
  size_t StringLength = mtb_StringLengthOf(String);
  return Append(Text, StringLength, String);
}

${FunctionQualifiers}int
Append($($Text.Name)* Text, size_t StringLength, char const* String)
{
  int NewSize = Text->Size + (int)StringLength;
  MTB_AssertDebug(NewSize < $($Text.FixedCapacity), `"Result would be too long to append`");
  if (NewSize > $($Text.FixedCapacity))
  {
    NewSize = $($Text.FixedCapacity);
  }

  int NumCopies = NewSize - Text->Size;
  mtb_CopyBytes(NumCopies, Text->Data + Text->Size, String);
  Text->Size = NewSize;

  return NumCopies;
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
