$ErrorActionPreference = 'Stop'

$raiz = Split-Path -Parent $PSScriptRoot
$compilador = 'C:\Program Files\Windhawk\Compiler\bin\clang++.exe'
$fuente = Join-Path $PSScriptRoot 'barra_progreso_tiempo_test.cpp'
$ejecutable = Join-Path $env:TEMP 'useful-sideview-barra-progreso-test.exe'

& $compilador -std=c++23 -target x86_64-w64-mingw32 `
    -DUNICODE -D_UNICODE -I (Join-Path $raiz 'src') `
    $fuente -o $ejecutable -luser32
if ($LASTEXITCODE -ne 0) {
    throw 'No fue posible compilar la prueba de la barra de progreso.'
}

& $ejecutable
if ($LASTEXITCODE -ne 0) { throw 'La prueba de la barra de progreso falló.' }

function Obtener-SegmentoFuncion {
    param(
        [string]$CodigoFuente,
        [string]$FirmaInicial,
        [string]$FirmaSiguiente
    )

    $inicioFuncion = $CodigoFuente.IndexOf($FirmaInicial)
    $inicioFuncionSiguiente = $CodigoFuente.IndexOf(
        $FirmaSiguiente, $inicioFuncion + $FirmaInicial.Length)
    if ($inicioFuncion -lt 0 -or $inicioFuncionSiguiente -lt 0) {
        throw "No fue posible localizar la función $FirmaInicial."
    }
    return $CodigoFuente.Substring(
        $inicioFuncion, $inicioFuncionSiguiente - $inicioFuncion)
}

$codigoControles = Get-Content -Raw -LiteralPath (
    Join-Path $raiz 'src\reproductor\controles.cpp')
$segmentoInicioArrastre = Obtener-SegmentoFuncion $codigoControles `
    'void iniciarArrastreBarraProgreso' `
    'void actualizarArrastreBarraProgreso'
$segmentoActualizacionArrastre = Obtener-SegmentoFuncion $codigoControles `
    'void actualizarArrastreBarraProgreso' `
    'void finalizarArrastreBarraProgreso'
$segmentoFinalizacionArrastre = Obtener-SegmentoFuncion $codigoControles `
    'void finalizarArrastreBarraProgreso' `
    'void desplazarBarraProgresoConTeclado'

if ($segmentoInicioArrastre -match 'buscarEnReproduccion\s*\(' -or
    $segmentoActualizacionArrastre -match 'buscarEnReproduccion\s*\(') {
    throw 'El arrastre no debe ejecutar búsquedas antes de soltar el mouse.'
}
$busquedasAlFinalizar = [regex]::Matches(
    $segmentoFinalizacionArrastre, 'buscarEnReproduccion\s*\(').Count
if ($busquedasAlFinalizar -ne 1) {
    throw 'La finalización debe emitir exactamente una búsqueda.'
}

$codigoEventos = Get-Content -Raw -LiteralPath (
    Join-Path $raiz 'src\reproductor\eventos_media_foundation.cpp')
if ($codigoEventos -notmatch 'GetCharacteristics' -or
    $codigoEventos -notmatch 'MFP_MEDIAITEM_CAN_SEEK') {
    throw 'Debe validarse la capacidad de posicionamiento del medio.'
}

Remove-Item -LiteralPath $ejecutable -Force
Write-Host 'Pruebas de tiempo, arrastre y posicionamiento: OK'
