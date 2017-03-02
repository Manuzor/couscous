<#
#>
Param(
  [string]$DestBFF = (Join-Path $PSCommandPath "../../workspace/system.bff"),
  [switch]$Force = $false
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

if(!($Force) -and !($InstalledWindowsSDKs))
{
  Throw "Unable to locate ANY Windows SDK."
}

# Use the latest one.
$_Temp = New-Object System.Version
$InstalledWindowsSDKsSorted = $InstalledWindowsSDKs | ? { [Version]::TryParse($_.ProductVersion, [ref]$_Temp) } | Sort-Object { [Version]::Parse($_.ProductVersion) } -Descending
$WindowsSDK = $InstalledWindowsSDKsSorted[0]
$WindowsSDKVersion = $WindowsSDK.ProductVersion
$WindowsSDKDir = $WindowsSDK.InstallationFolder -replace '[/\\]+$','' # Without trailing slashes

# Require at least the Windows 10 SDK
if(!($Force) -and $WindowsSDKVersion -notmatch '^10\..*')
{
  Throw "Require a Windows 10 SDK, the highest we could find is: $($WindowsSDKVersion)"
}

if(!($Force) -and $WindowsSDKVersion -notmatch '^[0-9]{2}\.[0-9]\.[0-9]{1,5}(\.[0-9])?$')
{
  Throw "Unable to recognize Windows SDK version string: $($WindowsSDKVersion)"
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

if(!($Force) -and !($VisualStudioInfos))
{
  Throw "Unable to locate ANY Visual Studio installation."
}

# Use the latest one by sorting visual studio versions (treated as a `double` for comparison) in descending order.
$VisualStudioInfosSorted = $VisualStudioInfos | Sort-Object { $_.PSChildName -as [double] } -Descending
$VisualStudioInfo = $VisualStudioInfosSorted[0]
$VSVersion = $VisualStudioInfo.PSChildName
$VSDir = $VisualStudioInfo.ShellFolder -replace '[/\\]+$','' # Without trailing slashes

if(!($Force) -and (($VSVersion -as [double]) -lt 10.0))
{
  Throw "Need at least Visual Studio version 10.0 installed. The highest we could find is: $($VSVersion)"
}

if(!($Force) -and $VSVersion -notmatch '[0-9]{1,2}\.[0-9]')
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
if(!(Test-Path $BFFDir))
{
  mkdir $BFFDir *> $null
}

Set-Content -Path $DestBFF -Value $Content
