<#
  TODO(Manuzor): Docs.
  TODO(Manuzor): Support providing preferences for the used Visual Studio and Windows SDK Versions.
#>
param(
  [switch]$RenewSystemBFF,
  [switch]$IgnoreFBuildInPath,
  [switch]$WhatIf
)

$ErrorActionPreference = "Stop"

function Sanitize-PathName([string]$PathName)
{
  $PathName -replace '[/\\]+$','' # Without trailing slashes
}

function Get-FullVersionString([System.Version]$Version)
{
  if($Version.Revision -lt 0)
  {
    if($Version.Build -lt 0) { "${Version}.0.0" }
    else                     { "${Version}.0" }
  }
  else
  {
    "$Version"
  }
}

#
# Gather Platform info
#
$RepoRoot = Join-Path $PSCommandPath ".." -Resolve
$RepoName = Split-Path $RepoRoot -Leaf
$FBuildBFF = Join-Path $RepoRoot "fbuild.bff" -Resolve
$WorkspaceDir = New-Item (Join-Path $RepoRoot "_workspace") -ItemType Directory -Force
$BuildDir = New-Item (Join-Path $RepoRoot "_build") -ItemType Directory -Force

# For powershell versions < 6.0
if($PSVersionTable.PSVersion.Major -lt 6)
{
  # Emulate powershell v6.0 $Is* variables.
  try
  {
    switch([Environment]::OSVersion.Platform)
    {
      ([PlatformID]::Win32NT){ $IsWindows = $true }
      ([PlatformID]::Unix)   { $IsLinux = $true }
      ([PlatformID]::Unix)   { $IsOSX = $true }
    }
  }
  catch
  {
    Write-Error "Unable to determine platform."
  }
}

#
# Find Windows SDK
#
if($IsWindows)
{
  <#
  Gets infos about all installed Windows SDKs sorted in descending order,
  i.e. latest version first.
  #>
  function Get-WindowsSDKs
  {
    $KeyCandidates = @(
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
# Find Visual Studio
#
if($IsWindows)
{
  if(!(Get-Command Get-VSSetupInstance -ErrorAction Ignore))
  {
    Write-Host "Install powershell module 'VSSetup'..."
    # Infos about VSSetup can be found here:
    # https://blogs.msdn.microsoft.com/vcblog/2017/03/06/finding-the-visual-c-compiler-tools-in-visual-studio-2017/
    Find-Module VSSetup | Install-Module -Scope CurrentUser -Force -WhatIf:$WhatIf
  }

  $VSInstance = Get-VSSetupInstance | Select-VSSetupInstance -Latest -Require Microsoft.VisualStudio.Component.VC.Tools.x86.x64
  $MSVCToolsFile = Join-Path -Resolve $VSInstance.InstallationPath "VC/Auxiliary/Build/Microsoft.VCToolsVersion.default.txt"
  $MSVCVersion = New-Object System.Version (Get-Content $MSVCToolsFile).Trim()
}

#
# Find Powershell
#
$Powershell = [System.Diagnostics.Process]::GetCurrentProcess().MainModule.FileName

#
# Ensure system.bff exists.
#
$SystemBFFPath = Join-Path $WorkspaceDir.FullName "system.bff"
if($RenewSystemBFF -or !(Test-Path $SystemBFFPath))
{
  $SystemBFFContent = @"
// Generated at $(Get-Date -Format 'yyyy-MM-ddTHH:mm:ss.fffffff')
#once

.RepoRoot = '$RepoRoot'
.MyBuildRoot = '$BuildDir'
.MyWorkspaceRoot = '$WorkspaceDir'

.Powershell = '$Powershell'

"@

  if($IsWindows)
  {
    $HostArch = "x64".ToUpper()
    $SystemBFFContent += @"

// Windows specific
.WinSDKDir = '$($LatestWindowsSDK.RootDir)'
.WinSDKVersion = '$(Get-FullVersionString $LatestWindowsSDK.Version)'

.VSDir = '$($VSInstance.InstallationPath)'
.VSVersion = '$($VSInstance.InstallationVersion)'
.VSVersionMajor = '$($VSInstance.InstallationVersion.Major)'

.MSVCDir = '$(Join-Path -Resolve $VSInstance.InstallationPath "VC/Tools/MSVC")'
.MSVCVersion = '$MSVCVersion'
"@
  }
  else
  {
    $SystemBFFContent += @"

    // Non-Windows platforms
    // TODO(Manuzor)
"@
  }

  Set-Content -Path $SystemBFFPath -Value $SystemBFFContent -WhatIf:$WhatIf
}

#
# Find FASTBuild
#
$FASTBuildWorkspaceDir = New-Item (Join-Path $WorkspaceDir "fastbuild") -ItemType Directory -Force

function Find-FBuildExe(
  [string]$DirectoryToSearch,
  [string[]]$FBuildNames = @('FBuild', 'fbuild'),
  [switch]$SearchInPath
)
{
  $LocationsToTry = @(
    (Join-Path $FASTBuildWorkspaceDir 'fbuild');
    (Join-Path $FASTBuildWorkspaceDir 'FBuild');
  )

  if($SearchInPath)
  {
    $LocationsToTry += @(
      'fbuild'; # PATH (system).
      'FBuild'; # PATH (system).
    )
  }

  $Result = Get-Command $LocationsToTry -ErrorAction Ignore
  if($Result)
  {
    return $Result[0]
  }
}

# Try to find the FBuild executable in the FASTBuildWorkspaceDir or in the PATH (system).
$FBuildExe = Find-FBuildExe $FASTBuildWorkspaceDir -SearchInPath:(!$IgnoreFBuildInPath)

# If no FBuild executable was found, download one that is known to work.
if(!$FBuildExe)
{
  Write-Warning "No FBuild client was found on this machine."
  Write-Host "Was looking here:"
  Write-Host "  - '$FASTBuildWorkspaceDir'"
  if(!$IgnoreFBuildInPath) { Write-Host "  - System PATH" }

  $FBuildDownloadUrl = if($IsWindows)   { "http://fastbuild.org/downloads/v0.93/FASTBuild-Windows-x64-v0.93.zip" }
                       elseif($IsLinux) { "http://fastbuild.org/downloads/v0.93/FASTBuild-Linux-x64-v0.93.zip" }
                       elseif($IsOSX)   { "http://fastbuild.org/downloads/v0.93/FASTBuild-OSX-x64-v0.93.zip" }

  $FBuildZipFile = Join-Path $FASTBuildWorkspaceDir "FASTBuild.zip"
  if(!$WhatIf)
  {
    Write-Host "Downloading: $FBuildDownloadUrl => $FBuildZipFile"
    $DownloadTime = Measure-Command {
      Invoke-WebRequest $FBuildDownloadUrl -OutFile $FBuildZipFile
    }
    Write-Host ("Downloaded in {0:N2}s." -f $DownloadTime.TotalSeconds)

    if(Test-Path -PathType Leaf $FBuildZipFile)
    {
      Write-Host "Unzipping: $FBuildZipFile => $FASTBuildWorkspaceDir"
      $UnzipTime = Measure-Command {
        Expand-Archive -Force $FBuildZipFile $FASTBuildWorkspaceDir
      }
      Write-Host ("Unzipped in {0:N2}s." -f $UnzipTime.TotalSeconds)

      Write-Host "Verifying FBuild client"
      $FBuildExe = Find-FBuildExe $FASTBuildWorkspaceDir -SearchInPath:$false
      if(!$FBuildExe)
      {
        Throw "Unable to unzip the FASTBuild client. Please go here and unzip it manually: $FASTBuildWorkspaceDir"
      }
    }
    else
    {
      Throw "Unable to download a FASTBuild client. Please visit http://fastbuild.org and install it manually. Make sure it's in your PATH as well."
    }
  }
  else
  {
    Write-Host "What if: Downloading: $FBuildDownloadUrl => $FASTBuildWorkspaceDir"
  }
}

$FBuildOptions = @(
  "-config", $FBuildBFF;
)

if($WhatIf)
{
  Write-Host "What if: Invoking FASTBuild: $($FBuildExe.Source) $FBuildOptions $Args"
}
else
{
  # Move to an intermediate directory and execute from there.
  Push-Location $FASTBuildWorkspaceDir
  try
  {
    & $FBuildExe -config (Join-Path $RepoRoot "fbuild.bff") $Args
  }
  finally
  {
    Pop-Location
  }
}
