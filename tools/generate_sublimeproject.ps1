<#
  .PARAMETER DestFilePath
    The destination sublime project file.

  .PARAMETER WhatIf
    Don't actually write to the destination file.

  .PARAMETER PassThru
    Return the content of the destination file object. By default, nothing is returned.
#>
Param(
  [string]$DestFilePath,
  [string[]]$FASTBuildTargets,

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
$Content.folders = @(
  @{
    "path" = "$RepoRoot";
  }
)

$MainBuildSystem =
@{
  "name" = "$RepoName";
  "file_regex" = "([A-z]:.*?)\\(([0-9]+)(?:,\\s*[0-9]+)?\\)";
};

$BuildScript = Join-Path $RepoRoot "build.ps1"

if(!$FASTBuildTargets)
{
  $Targets = (& $BuildScript -showtargets)
  foreach($Target in $Targets)
  {
    # FASTBuild emits targets indented, so only treat those lines as targets, that start with whitespace.
    if([char]::IsWhitespace($Target[0]))
    {
      $FASTBuildTargets += @($Target.Trim())
    }
  }
}

foreach($Action in @("Build", "Rebuild"))
{
  foreach($Target in $FASTBuildTargets)
  {
    $Cmd = @("powershell"; "-ExecutionPolicy"; "Bypass"; $BuildScript; $Target; "-ide";);
    if($Action -eq "Rebuild")
    {
      $Cmd += @("-clean")
    }

    if($Action -eq "Build" -and $Target -eq "all")
    {
      $MainBuildSystem.cmd = $Cmd
    }

    $BuildSystemVariants += @(
      @{
        "name" = "${Action}: $Target";
        "cmd" = $Cmd
      }
    )
  }
}

$MainBuildSystem.variants = $BuildSystemVariants
$Content.build_systems = @($MainBuildSystem)

$JsonContent = ConvertTo-Json $Content -Depth 100
$JsonContent | Set-Content $DestFile -WhatIf:$WhatIf
if($PassThru)
{
  $JsonContent
}
