<#
#>
Param(
  [string]$DestBFF = Join-Path $PSCommandPath "../../workspace/system.bff"
)

$RepoRoot = Join-Path -Resolve $PSCommandPath "../.."
$RepoName = Split-Path $RepoRoot -Leaf

#
# Windows SDK info
#
$WindowsSDK_KeyCandidates =
@(
  'HKLM:\SOFTWARE\Microsoft\Microsoft SDKs\Windows';
  'HKLM:\SOFTWARE\WOW6432Node\Microsoft\Microsoft SDKs\Windows';
)

foreach($Key in $WindowsSDK_KeyCandidates)
{
  if(Test-Path $Key)
  {
    $InstalledWindowsSDKs += @(ls $Key | Get-ItemProperty)
  }
}

if(-not($InstalledWindowsSDKs))
{
  Throw "Unable to locate ANY Windows SDK."
}

# Use the latest one.
$WindowsSDK = ($InstalledWindowsSDKs | sort -Property ProductVersion)[0]
$WindowsSDKVersion = $WindowsSDK.ProductVersion
$WindowsSDKDir = $WindowsSDK.InstallationFolder

# Require at least the Windows 10 SDK
if($WindowsSDKVersion -notmatch '[0-9]{1,2}\.[0-9]\.[0-9]{1,5}\.[0-9]')
{
  Throw "Weird format for Windows SDK version: $WindowsSDKVersion"
}

if($WindowsSDKVersion -notmatch '10\..*')
{
  Throw "Require a Windows 10 SDK, got: $WindowsSDKVersion"
}


#
# Visual Studio info
#
$KeyCandidates =
@(
  'HKLM:\SOFTWARE\Microsoft\VisualStudio';
  'HKLM:\SOFTWARE\WOW6432Node\Microsoft\VisualStudio';
)

foreach($Key in $KeyCandidates)
{
  if(Test-Path $Key)
  {
    $VisualStudioInfos += @(ls $Key | Get-ItemProperty)
  }
}

if(-not($VisualStudioInfos))
{
  Throw "Unable to locate ANY Visual Studio installation."
}

# Use the latest one.
$VisualStudioInfo = ($VisualStudioInfos | sort -Property PSChildName)[0]
$VSVersion = $VisualStudioInfo.PSChildName
$VSDir = $VisualStudioInfo.ShellFolder

if($VSVersion -notmatch '[0-9]{1,2}\.[0-9]')
{
  Throw "Weird format for Visual Studio version: $VSVersion"
}

$VSVersionMajor = $VSVersion -replace "\.[0-9]$"

#
# Generate the bff file
#
$Content = @"
// Generated at $(Get-Date -Format 'yyyy-MM-ddTHH:mm:ss.fffffff')
#once

.RepoRoot = '$RepoRoot'
.WindowsSDKDir = '$WindowsSDKDir'
.WindowsSDKVersion = '$WindowsSDKVersion'
.VSDir = '$VSDir'
.VSVersion = '$VSVersion'
.VSVersionMajor = '$VSVersionMajor'
"@

# Create the directory, if it does not exist.
$BFFDir = Split-Path -Parent $DestBFF
if(-not(Test-Path $BFFDir))
{
  mkdir $BFFDir
}

Set-Content -Path $DestBFF -Value $Content
