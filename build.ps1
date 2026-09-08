# Script de construcción directa del bundle de distribución (1-click build)
$ErrorActionPreference = 'Stop'

$raizProyecto = $PSScriptRoot
$scriptBuild = Join-Path $raizProyecto 'tools\build.ps1'

if (-not (Test-Path -LiteralPath $scriptBuild)) {
    Write-Error "No se encontró el script de compilación en '$scriptBuild'."
    exit 1
}

try {
    Write-Host "Iniciando compilación del bundle..." -ForegroundColor Cyan
    & $scriptBuild
    $archivoSalida = Join-Path $raizProyecto 'dist\useful-sideview.wh.cpp'
    if (Test-Path -LiteralPath $archivoSalida) {
        $tamano = (Get-Item -LiteralPath $archivoSalida).Length
        Write-Host "`n[OK] Bundle generado con éxito: $archivoSalida ($tamano bytes)" -ForegroundColor Green
    } else {
        throw "El archivo dist/useful-sideview.wh.cpp no fue encontrado tras el build."
    }
} catch {
    Write-Error "Error durante la compilación del bundle: $_"
    exit 1
}
