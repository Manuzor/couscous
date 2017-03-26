<#
  .PARAMETER DestFilePath
    The destination sublime project file.

  .PARAMETER WhatIf
    Don't actually write to the destination file.

  .PARAMETER PassThru
    Return the destination file object. By default, $null is returned.
#>
Param(
  [string]$DestFilePath,

  [switch]$PassThru,
  [switch]$WhatIf
)

$RepoRoot = Join-Path -Resolve $PSCommandPath "../.."
$RepoName = Split-Path $RepoRoot -Leaf
$WorkspaceDir = New-Item (Join-Path $RepoRoot "_workspace") -ItemType Directory -Force

if(!$DestFilePath)
{
  $DestFilePath = Join-Path $WorkspaceDir "$RepoName.sublime-project"
}
$DestFile = New-Item $DestFilePath -ItemType File -Force


$Content = @{}
$Content["folders"] = @(
  @{ "path" = "$RepoRoot"; }
)

$BuildSystemVariants = @(
  @{
    "name" = "Build Only";
    "windows" = @{
      "cmd" = @( Join-Path $RepoRoot "code/build.bat" );
    };
  };
  @{
    "name" = "Build and Run";
    "windows" = @{
      "cmd" = @( Join-Path $RepoRoot "code/build.bat"; "run" );
    };
  };
);

$MTBDir = Join-Path $RepoRoot "../mtb"
if(Test-Path $MTBDir)
{
  $BuildSystemVariants += @{
    "name" = "Update MTB";
    "windows" = @{
      "cmd" = @( "py"; "-3"; Join-Path -Resolve $MTBDir "tools/generate_self_contained.py"; "-o"; Join-Path $RepoRoot "code/mtb.hpp" );
    };
  };
}
else
{
  Write-Host "Warning: MTB dir not found."
}


$Content["build_systems"] = @(
  @{
    "name" = "$RepoName";
    "file_regex" = "([A-z]:.*?)\\(([0-9]+)(?:,\\s*[0-9]+)?\\)";
    "variants" = $BuildSystemVariants
  };
)

ConvertTo-Json $Content -Depth 100 | Set-Content $DestFile -WhatIf:$WhatIf
if($PassThru)
{
  $DestFile
}
