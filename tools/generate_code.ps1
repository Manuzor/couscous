param(
  [string]$RepoRoot = "$PSScriptRoot/..",
  [string]$OutDir = "$RepoRoot/code/_generated",
  [string]$StaticFunctions = $true
  )

$OutDir = mkdir -Force $OutDir

$Arrays = @(
  @{ Name = "instruction_array"; Type = "instruction"; ForwardDeclareType = $true };
  @{ Name = "label_array"; Type = "label"; ForwardDeclareType = $true };
  @{ Name = "patch_array"; Type = "patch"; ForwardDeclareType = $true };
  @{ Name = "u16_array"; Type = "u16"; ForwardDeclareType = $false };
)

$ArraysUmbrellaHeaderIncludes = @()
$ArraysUmbrellaIncludes = @()

$FunctionQualifiers = "$(if($StaticFunctions) { "static " })"

foreach($Array in $Arrays)
{
  $HeaderContent = @"
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
  $ArraysUmbrellaHeaderIncludes += $HeaderFilePath
  Set-Content $HeaderFilePath $HeaderContent

  $Content = @"
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
  $ArraysUmbrellaIncludes += $FilePath
  Set-Content $FilePath $Content
}

$ArraysUmbrellaBaseFilePath = "$OutDir/arrays"

$ArraysUmbrellaHeaderContent = @"
// Generated on $(Get-Date -f "yyyy-MM-dd HH:mm:ss")
$($ArraysUmbrellaHeaderIncludes | % { "#include <$_>`n" })
"@
Set-Content "$ArraysUmbrellaBaseFilePath.h" $ArraysUmbrellaHeaderContent

$ArraysUmbrellaContent = @"
// Generated on $(Get-Date -f "yyyy-MM-dd HH:mm:ss")
$($ArraysUmbrellaIncludes | % { "#include <$_>`n" })
"@
Set-Content "$ArraysUmbrellaBaseFilePath.cpp" $ArraysUmbrellaContent
