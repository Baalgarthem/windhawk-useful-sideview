$ErrorActionPreference = 'Stop'

$raizProyecto = Split-Path -Parent $PSScriptRoot
$interpretePython = Get-Command python -ErrorAction Stop
$combinadorFuentes = Join-Path $PSScriptRoot 'combiner\combine.py'
$fuenteEntrada = Join-Path $raizProyecto 'src\main.wh.cpp'
$directorioDistribucion = Join-Path $raizProyecto 'dist'
$fuenteDistribuible = Join-Path $directorioDistribucion 'useful-sideview.wh.cpp'
$fuenteTemporal = Join-Path $directorioDistribucion 'useful-sideview.wh.cpp.tmp'

if (-not (Test-Path -LiteralPath $combinadorFuentes)) {
    throw 'No se encontró Combiner en tools\combiner\combine.py.'
}

New-Item -ItemType Directory -Force -Path $directorioDistribucion | Out-Null
& $interpretePython.Source $combinadorFuentes `
    -r (Join-Path $raizProyecto 'src') `
    -o $fuenteTemporal `
    $fuenteEntrada
if ($LASTEXITCODE -ne 0) {
    throw "La generación del bundle falló con código $LASTEXITCODE."
}

Move-Item -LiteralPath $fuenteTemporal -Destination $fuenteDistribuible -Force

Write-Host "Generado: $fuenteDistribuible"
