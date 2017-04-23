<#
  TODO(Manuzor): Docs.
  TODO(Manuzor): Support providing preferences for the used Visual Studio and Windows SDK Versions.
#>
param(
  [switch]$RenewSystemBFF,
  [switch]$IgnoreFBuildInPath,
  [switch]$WhatIf
)

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
[OperatingSystem]$OS = [Environment]::OSVersion

#
# Find Windows SDK
#
if($OS.Platform -eq [PlatformID]::Win32NT)
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
if($OS.Platform -eq [PlatformID]::Win32NT)
{
  <#
  function Get-VisualStudioInfos
  {
    $KeyCandidates = @(
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
  #>

  if(!(Get-Command Get-VSSetupInstance))
  {
    # Infos about VSSetup can be found here:
    # https://blogs.msdn.microsoft.com/vcblog/2017/03/06/finding-the-visual-c-compiler-tools-in-visual-studio-2017/
    Install-Module VSSetup -Scope CurrentUser -Force -WhatIf:$WhatIf
  }
  if(!(Get-Command Get-VSSetupInstance))
  {
    Throw "Unable to install PS module 'VSSetup' which is required to find installed Visual Studio instances."
  }

  $VSInstance = Get-VSSetupInstance | Select-VSSetupInstance -Latest -Require Microsoft.VisualStudio.Component.VC.Tools.x86.x64
  if($VSInstance.InstallationVersion.Major -ge 15)
  {
    $MSVCToolsVersionFile = Join-Path -Resolve $VSInstance.InstallationPath "VC/Auxiliary/Build/Microsoft.VCToolsVersion.default.txt"
    $MSVCToolsVersion = New-Object System.Version (Get-Content $MSVCToolsVersionFile)
    $MSVCToolsDir = Join-Path -Resolve $VSInstance.InstallationPath "VC/Tools/MSVC/$MSVCToolsVersion"
  }
  else
  {
    # TODO
  }
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

  if($OS.Platform -eq [PlatformID]::Win32NT)
  {
    $SystemBFFContent += @"

// Windows specific
.WindowsSDKDir = '$($LatestWindowsSDK.RootDir)'
.WindowsSDKVersion = '$(Get-FullVersionString $LatestWindowsSDK.Version)'
.VSDir = '$($VSInstance.InstallationPath)'
.VSVersion = '$($VSInstance.InstallationVersion)'
.VSVersionMajor = '$($VSInstance.InstallationVersion.Major)'

"@

    if($VSInstance.InstallationVersion.Major -ge 15)
    {
      $HostArch = "x64"
      $SystemBFFContent += @"
.MSVCBin_x86 = '$MSVCToolsDir/bin/Host$($HostArch.ToUpper())/x86'
.MSVCBin_x64 = '$MSVCToolsDir/bin/Host$($HostArch.ToUpper())/x64'
"@
    }
    else
    {
    }
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

  $FBuildDownloadUrl = switch($OS.Platform)
  {
    ([PlatformID]::Win32NT) { "http://fastbuild.org/downloads/v0.93/FASTBuild-Windows-x64-v0.93.zip" }
    ([PlatformID]::Unix)    { "http://fastbuild.org/downloads/v0.93/FASTBuild-Linux-x64-v0.93.zip" }
    ([PlatformID]::MacOSX)  { "http://fastbuild.org/downloads/v0.93/FASTBuild-OSX-x64-v0.93.zip" }
  }
  $FBuildZipFile = Join-Path $FASTBuildWorkspaceDir "FASTBuild.zip"
  if(!$WhatIf)
  {
    Write-Host "Downloading: $FBuildDownloadUrl => $FBuildZipFile"
    Write-Progress -Id 1 "Fetching FBuild Client" "Downloading: $FBuildDownloadUrl"
    $DownloadTime = Measure-Command {
      (New-Object System.Net.WebClient).DownloadFile($FBuildDownloadUrl, $FBuildZipFile)
    }
    Write-Host ("Downloaded in {0:N2}s." -f $DownloadTime.TotalSeconds)

    if(Test-Path -PathType Leaf $FBuildZipFile)
    {
      Write-Host "Unzipping: $FBuildZipFile => $FASTBuildWorkspaceDir"
      Write-Progress -Id 1 "Fetching FBuild Client" "Unzipping: $FBuildZipFile => $FASTBuildWorkspaceDir"
      $UnzipTime = Measure-Command {
        Expand-Archive -Force $FBuildZipFile $FASTBuildWorkspaceDir
      }
      Write-Host ("Unzipped in {0:N2}s." -f $UnzipTime.TotalSeconds)

      Write-Progress -Id 1 "Fetching FBuild Client" "Veryfing"
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
  & $FBuildExe -config (Join-Path $RepoRoot "fbuild.bff") $Args
  Pop-Location
}
