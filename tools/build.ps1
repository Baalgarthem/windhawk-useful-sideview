$ErrorActionPreference = 'Stop'

$projectRoot = Split-Path -Parent $PSScriptRoot
$python = Get-Command python -ErrorAction Stop
$combiner = Join-Path $PSScriptRoot 'combiner\combine.py'
$entry = Join-Path $projectRoot 'src\main.wh.cpp'
$distDirectory = Join-Path $projectRoot 'dist'
$output = Join-Path $distDirectory 'useful-sideview.wh.cpp'
$temporaryOutput = Join-Path $distDirectory 'useful-sideview.wh.cpp.tmp'

if (-not (Test-Path -LiteralPath $combiner)) {
    throw 'Combiner is not installed. Expected tools\combiner\combine.py.'
}

New-Item -ItemType Directory -Force -Path $distDirectory | Out-Null
& $python.Source $combiner -r (Join-Path $projectRoot 'src') -o $temporaryOutput $entry
if ($LASTEXITCODE -ne 0) {
    throw "Source bundling failed with exit code $LASTEXITCODE."
}

Move-Item -LiteralPath $temporaryOutput -Destination $output -Force

Write-Host "Generated $output"
