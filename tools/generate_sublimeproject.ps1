<#
  .PARAMETER DestFile
    The destination sublime project file.
#>
Param(
  [string]$DestFilePath
)

$RepoRoot = Join-Path -Resolve $PSCommandPath "../.."
$RepoName = Split-Path $RepoRoot -Leaf

if(-not($DestFilePath))
{
  $DestFilePath = Join-Path $RepoRoot "workspace/$RepoName.sublime-project"
}


$Content = @{}
$Content["folders"] =
@(
  @{ "path" = "$RepoRoot"; }
)

$BuildSystemVariants =
@(
  @{
    "name" = "Build Only";
    "windows" =
    @{
      "cmd" = @( Join-Path $RepoRoot "code/build.bat" );
    };
  };
  @{
    "name" = "Build and Run";
    "windows" =
    @{
      "cmd" = @( Join-Path $RepoRoot "code/build.bat"; "run" );
    };
  };
);

$MTBDir = Join-Path $RepoRoot "../mtb"
if(Test-Path $MTBDir)
{
  $BuildSystemVariants +=
  @{
    "name" = "Update MTB";
    "windows" =
    @{
      "cmd" = @( "py"; "-3"; Join-Path -Resolve $MTBDir "tools/generate_self_contained.py"; "-o"; Join-Path $RepoRoot "code/mtb.hpp" );
    };
  };
}
else
{
  Write-Host "Warning: MTB dir not found."
}


$Content["build_systems"] =
@(
  @{
    "name" = "$RepoName";
    "file_regex" = "([A-z]:.*?)\\(([0-9]+)(?:,\\s*[0-9]+)?\\)";
    "variants" = $BuildSystemVariants
  };
)


ConvertTo-Json $Content -Depth 100 | Set-Content -Path $DestFilePath
