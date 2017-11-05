param(
  [string]$RepoRoot = "$PSScriptRoot/..",
  [string]$OutDir = "$RepoRoot/code/_generated",
  [switch]$StaticFunctions = $true,
  [switch]$Clean
)

$ErrorActionPreference = "Stop"

$OutDir = mkdir -Force $OutDir
if($Clean)
{
  Write-Host "Cleaning directory: $OutDir"
  rm -Force -Recurse "$OutDir/*"
}

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
  @{ Name = "s32_array"; Type = "s32"; FixedCapacity = 32 };
  @{ Name = "str_array"; Type = "str"; FixedCapacity = 32 };
  @{ Name = "token_array"; Type = "token"; FixedCapacity = 8 };
  @{ Name = "cursor_array"; Type = "parser_cursor"; FixedCapacity = 8 };
  @{ Name = "debug_info_array"; Type = "debug_info"; FixedCapacity = 0 };
  @{ Name = "win32_window_event_array"; Type = "win32_window_event"; FixedCapacity = 32 };
)

foreach($Array in $Arrays)
{
  $HasFixed = $Array.FixedCapacity -gt 0;
  $MinCapacity = if($HasFixed) { $Array.FixedCapacity } else { 32 }
  $IncludeGuard = "GUARD_GENEREATED_$($Array.Name)"

  $HeaderContent = @"
// Generated on $DateStamp

#if !defined($IncludeGuard)
#define $IncludeGuard

struct $($Array.Name)
{
  int NumElements;
  int Capacity;
  $($Array.Type)* _Data;

$(if($HasFixed)
{@"
enum { FixedCapacity = $($Array.FixedCapacity) };
  $($Array.Type) _Fixed[FixedCapacity];

  $($Array.Type)* Data() { return _Data ? _Data : _Fixed; }
"@}
else
{@"
  $($Array.Type)* Data() { return _Data; }
"@})
};

${FunctionQualifiers}void
Reserve($($Array.Name)* Array, int RequiredCapacity);

${FunctionQualifiers}void
SetNumElements($($Array.Name)* Array, int NewNumElements);

${FunctionQualifiers}$($Array.Type)*
AddN($($Array.Name)* Array, int NumToAdd);

inline $($Array.Type)*
Add($($Array.Name)* Array) { return AddN(Array, 1); }

${FunctionQualifiers}void
Clear($($Array.Name)* Array);

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

#endif // !defined($IncludeGuard)
"@
  $HeaderFilePath = "$OutDir/$($Array.Name).h"
  $UmbrellaHeaderIncludes += $HeaderFilePath
  Set-Content $HeaderFilePath $HeaderContent

  $Content = @"
// Generated on $DateStamp

#if defined($IncludeGuard)

${FunctionQualifiers}void
Reserve($($Array.Name)* Array, int RequiredCapacity)
{
$(if($HasFixed)
{@"
  if (Array->_Data == nullptr || Array->Capacity == 0)
  {
    Array->Capacity = Array->FixedCapacity;
    Array->_Data = nullptr;
  }
"@}
)

  if (RequiredCapacity > Array->Capacity)
  {
    int NewCapacity = Array->Capacity > 0 ? Array->Capacity : $MinCapacity;

    while (NewCapacity < RequiredCapacity)
      NewCapacity *= 2;

    void* NewData = malloc(NewCapacity * sizeof($($Array.Type)));

    if (Array->_Data)
    {
      mtb_CopyBytes(Array->NumElements * sizeof($($Array.Type)), NewData, Array->_Data);
      free(Array->_Data);
    }
$(if($HasFixed)
{@"
    else
    {
      mtb_CopyBytes(Array->NumElements * sizeof($($Array.Type)), NewData, Array->_Fixed);
    }
"@}
)

    Array->_Data = ($($Array.Type)*)NewData;
    Array->Capacity = NewCapacity;
  }
}

${FunctionQualifiers}void
SetNumElements($($Array.Name)* Array, int NewNumElements)
{
    Reserve(Array, NewNumElements);
    Array->NumElements = NewNumElements;
}

${FunctionQualifiers}$($Array.Type)*
AddN($($Array.Name)* Array, int NumToAdd)
{
  Reserve(Array, Array->NumElements + NumToAdd);
  $($Array.Type)* Result = Array->Data() + Array->NumElements;
  Array->NumElements += NumToAdd;

  return Result;
}

${FunctionQualifiers}void
Clear($($Array.Name)* Array)
{
  Array->NumElements = 0;
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

#endif // defined($IncludeGuard)
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
  enum
  {
    FullCapacity = $($Text.FixedCapacity),
    Capacity = FullCapacity - 1, // We reserve one character for the null terminator.
  };

  int Size;
  char Data[FullCapacity];
};

// "Constructor"
${FunctionQualifiers}$($Text.Name) $($CtorName)(strc String);

// To str
${FunctionQualifiers}str Str($($Text.Name) Text);

// EnsureZeroTerminated
${FunctionQualifiers}void EnsureZeroTerminated($($Text.Name)* Text);

// Clear
${FunctionQualifiers}void Clear($($Text.Name)* Text);

// Trim
${FunctionQualifiers}$($Text.Name) Trim($($Text.Name) Text);

// Append
${FunctionQualifiers}int Append($($Text.Name)* Text, strc String);
${FunctionQualifiers}int Append($($Text.Name)* Text, char Char);

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
Str($($Text.Name) Text)
{
  str Result{ Text.Size, Text.Data };

  return Result;
}

void
EnsureZeroTerminated($($Text.Name)* Text)
{
  if(Text->Size < Text->FullCapacity)
    Text->Data[Text->Size] = 0;
}

void
Clear($($Text.Name)* Text)
{
  Text->Size = 0;
  EnsureZeroTerminated(Text);
}

$($Text.Name)
Trim($($Text.Name) Text)
{
  str Trimmed = Trim(Str(Text));
  $($Text.Name) Result = $($CtorName)(Trimmed);

  return Result;
}

${FunctionQualifiers}int
Append($($Text.Name)* Text, strc String)
{
  int NewSize = Text->Size + String.Size;
  MTB_AssertDebug(NewSize < Text->Capacity, `"Result would be too long to append`");
  if (NewSize > Text->Capacity)
  {
    NewSize = Text->Capacity;
  }

  int NumCopies = NewSize - Text->Size;
  mtb_CopyBytes(NumCopies, Text->Data + Text->Size, String.Data);
  Text->Size = NewSize;

  return NumCopies;
}

${FunctionQualifiers}int
Append($($Text.Name)* Text, char Char)
{
  int Result = Append(Text, str{ 1, &Char });

  return Result;
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

#
# Write the generated natvis file
#
$NatvisFilePath = "$OutDir/generated.natvis"
Set-Content $NatvisFilePath @"
<?xml version="1.0" encoding="utf-8"?>
<AutoVisualizer xmlns="http://schemas.microsoft.com/vstudio/debugger/natvis/2010">
$(foreach($Array in $Arrays)
{@"
    <Type Name="$($Array.Name)">
        <DisplayString>{{ NumElements={NumElements} }}</DisplayString>
        <Expand>
            $(if($Array.FixedCapacity -gt 0){ "<Item Name=`"IsFixed`">_Data ? false : true</Item>" })
            <Item Name="NumElements">NumElements</Item>
            <Item Name="Capacity">Capacity</Item>
            <ArrayItems>
                <Size>NumElements</Size>
                $(if($Array.FixedCapacity -gt 0)
                {
                  "<ValuePointer>_Data ? _Data : _Fixed</ValuePointer>"
                }
                else
                {
                  "<ValuePointer>_Data</ValuePointer>"
                })
            </ArrayItems>
        </Expand>
    </Type>

"@}
)
</AutoVisualizer>
"@
