param(
  # [Parameter(Mandatory=$true)]
  [ValidateNotNullOrEmpty()]
  [string]$OutFilePath = (Join-Path (Resolve-Path "$PSScriptRoot/../code") "mtb.h")
)

$MtbHeaders = @(
  "mtb_memory.h";
  "mtb_rng.h";
)

# TODO(Manuzor): Find MTB in a more sophisticated way? Download it somewhere?!
$ScriptPath = Join-Path $PSScriptRoot "../../mtb/tools/generate_selfcontained.ps1"
if(Test-Path $ScriptPath)
{
  & $ScriptPath -MtbHeaders $MtbHeaders -OutFilePath $OutFilePath
}
