<#
  TODO(manuzor): Docs.
  TODO(manuzor): Fixed $FBuildArgs! './build.ps1 -- foo bar baz' only catches 'foo'
#>
param(
  [string[]]$FBuildArgs,
  [switch]$Renew,
  [switch]$WhatIf
)

function Sanitize-PathName([string]$PathName)
{
  [System.IO.Path]::GetFullPath($PathName) -replace '[/\\]+$','' # Without trailing slashes
}

#
# Gather Platform info
#
$RepoRoot = Join-Path $PSCommandPath ".." -Resolve
$RepoName = Split-Path $RepoRoot -Leaf
$FBuildBFF = Join-Path $RepoRoot "fbuild.bff" -Resolve
[OperatingSystem]$OS = [Environment]::OSVersion

#
# Windows SDK
#
if($OS.Platform -eq [PlatformID]::Win32NT)
{
  <#
  Gets infos about all installed Windows SDKs sorted in descending order,
  i.e. latest version first.
  #>
  function Get-WindowsSDKs
  {
    $KeyCandidates =
    @(
      'HKLM:\SOFTWARE\Microsoft\Microsoft SDKs\Windows';
      'HKLM:\SOFTWARE\WOW6432Node\Microsoft\Microsoft SDKs\Windows';
    )

    $Results = foreach($Key in $KeyCandidates)
    {
      if(Test-Path $Key)
      {
        $SDKInfos = Get-ChildItem $Key | Get-ItemProperty
        foreach($SDKInfo in $SDKInfos)
        {
          $Version = New-Object Version
          if([Version]::TryParse($SDKInfo.ProductVersion, [ref]$Version))
          {
            [PSCustomObject]@{
              "Version" = $Version
              "RootDir" = Sanitize-PathName $SDKInfo.InstallationFolder
            }
          }
        }
      }
    }

    if($Results)
    {
      $Results | Sort-Object { $_.Version } -Descending
    }
  }

  $AllWindowsSDKs = Get-WindowsSDKs
  if(!($AllWindowsSDKs)) { Throw "Unable to find any Windows SDK" }

  $LatestWindowsSDK = $AllWindowsSDKs[0]
  if($LatestWindowsSDK.Version.Major -ne 10) { Throw "Need Windows 10 SDK. Highest version found: $LatestWindowsSDK" }
}

#
# Visual Studio
#
if($OS.Platform -eq [PlatformID]::Win32NT)
{
  function Get-VisualStudioInfos
  {
    $KeyCandidates =
    @(
      'HKLM:\SOFTWARE\Microsoft\VisualStudio';
      'HKLM:\SOFTWARE\WOW6432Node\Microsoft\VisualStudio';
    )

    $Results = foreach($Key in $KeyCandidates)
    {
      if(Test-Path $Key)
      {
        $Infos = Get-ChildItem $Key | Get-ItemProperty
        foreach($Info in $Infos)
        {
          $Version = New-Object Version
          if([Version]::TryParse($Info.PSChildName, [ref]$Version))
          {
            [PSCustomObject]@{
              "Version" = $Version
              "RootDir" = Sanitize-PathName $Info.ShellFolder
            }
          }
        }
      }
    }

    if($Results)
    {
      $Results | Sort-Object { $_.Version } -Descending
    }
  }

  $AllVisualStudioInfos = Get-VisualStudioInfos
  if(!($AllVisualStudioInfos)) { Throw "Visual Studio is not installed." }

  $LatestVisualStudio = $AllVisualStudioInfos[0]
  if($LatestVisualStudio.Version.Major -lt 10) { Throw "Need at least Visual Studio 2010. Latest version found: $LatestVisualStudio" }
}

#
# Ensure system.bff exists.
#
$WorkspaceDir = New-Item (Join-Path $RepoRoot "workspace") -ItemType Directory -Force
$SystemBFFPath = Join-Path $WorkspaceDir.FullName "system.bff"
if($Renew -or !(Test-Path $SystemBFFPath))
{
  $SystemBFFContent = @"
// Generated at $(Get-Date -Format 'yyyy-MM-ddTHH:mm:ss.fffffff')
#once

.RepoRoot = '$RepoRoot'

"@

  if($OS.Platform -eq [PlatformID]::Win32NT)
  {
    $SystemBFFContent += @"

// Windows specific
.WindowsSDKDir = '$($LatestWindowsSDK.RootDir)'
.WindowsSDKVersion = '$($LatestWindowsSDK.Version)'
.VSDir = '$($LatestVisualStudio.RootDir)'
.VSVersion = '$($LatestVisualStudio.Version)'
.VSVersionMajor = '$($LatestVisualStudio.Version.Major)'
"@
  }

  Set-Content -Path $SystemBFFPath -Value $SystemBFFContent -WhatIf:$WhatIf
}

#
# Invoke FASTBuild
#
$FBuildExe = if($OS.Platform -eq [PlatformID]::Win32NT)
{
  Join-Path $RepoRoot "tools/fastbuild/FBuild.exe"
}

$FBuildOptions = @(
  "-config", $FBuildBFF
)

if($WhatIf)
{
  Write-Host "What if: Invoking FASTBuild: $FBuildExe $FBuildOptions $FBuildArgs"
}
else
{
  # Move to an intermediate directory and execute from there.
  $FASTBuildWorkingDir = New-Item (Join-Path $WorkspaceDir "fastbuild") -ItemType Directory -Force
  Push-Location $FASTBuildWorkingDir
  & $FBuildExe -config (Join-Path $RepoRoot "fbuild.bff") $FBuildArgs
  Pop-Location
}
