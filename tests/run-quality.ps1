$ErrorActionPreference = 'Stop'

$raiz = Split-Path -Parent $PSScriptRoot
$compilador = 'C:\Program Files\Windhawk\Compiler\bin\clang++.exe'
$banderasWindhawk = 'C:\ProgramData\Windhawk\EditorWorkspace\compile_flags.txt'
$fuente = Join-Path $raiz 'src\main.wh.cpp'
$distribucion = Join-Path $raiz 'dist\useful-sideview.wh.cpp'
$dllTemporal = Join-Path $env:TEMP (
    'useful-sideview-' + [System.IO.Path]::GetRandomFileName() + '.dll')
$analisisTemporal = Join-Path $env:TEMP (
    'useful-sideview-' + [System.IO.Path]::GetRandomFileName() + '.plist')

if (-not (Test-Path -LiteralPath $compilador)) {
    throw 'No se encontró el compilador de Windhawk.'
}
if (-not (Test-Path -LiteralPath $banderasWindhawk)) {
    throw 'No se encontraron las banderas de compilación de Windhawk.'
}

$artefactosNoPermitidos = @(
    (Join-Path $raiz 'dist\bundled.cpp'),
    (Join-Path $raiz 'tools\bundle_clang.ps1'),
    (Join-Path $raiz 'src\host'),
    (Join-Path $raiz 'src\runtime'),
    (Join-Path $raiz 'src\interfaz\diseno_slider.hpp'),
    (Join-Path $raiz 'tests\slider_tiempo_test.cpp')
)
foreach ($artefacto in $artefactosNoPermitidos) {
    if (Test-Path -LiteralPath $artefacto) {
        throw "Artefacto residual detectado: $artefacto"
    }
}
$archivosDesktop = Get-ChildItem -LiteralPath $raiz -Filter desktop.ini `
    -File -Force -Recurse
if ($archivosDesktop) {
    throw 'Se detectaron archivos desktop.ini ajenos al código fuente.'
}

# Control estructural proporcional a ISO/IEC 5055: impide que regresen nombres
# opacos que obligan al lector a inferir el propósito de una variable.
$archivosCodigo = Get-ChildItem -Path (Join-Path $raiz 'src'), `
    (Join-Path $raiz 'tests') -Include *.cpp,*.hpp -File -Recurse
$patronNombreOpaco = '\b(?:const\s+)?(?:auto|bool|double|DWORD|float|int|long|' +
    'LONGLONG|short|size_t|std::wstring|wchar_t)\s+(?:\*+\s*)?' +
    '(?:i|j|k|x|y|data|temp|obj|item|valor|indice)\b'
$nombresOpacos = $archivosCodigo | Select-String -Pattern $patronNombreOpaco
if ($nombresOpacos) {
    $ubicaciones = $nombresOpacos |
        ForEach-Object { "$($_.Path):$($_.LineNumber)" }
    throw "Se detectaron nombres no semánticos: $($ubicaciones -join ', ')"
}
$terminosNoTraducidos = $archivosCodigo |
    Select-String -Pattern '\b(?:Slider|slider)\b'
if ($terminosNoTraducidos) {
    $ubicaciones = $terminosNoTraducidos |
        ForEach-Object { "$($_.Path):$($_.LineNumber)" }
    throw "Se detectaron términos propios sin traducir: " +
        ($ubicaciones -join ', ')
}

$metadatosMod = Get-Content -Raw -LiteralPath (
    Join-Path $raiz 'src\useful-sideview.cpp')
if ($metadatosMod -notmatch '(?m)^- autoplay: false\r?$' -or
    $metadatosMod -notmatch '(?m)^- sound: false\r?$') {
    throw 'Autoplay y audio deben permanecer desactivados por defecto.'
}

$documentoBugs = Get-Content -Raw -LiteralPath (
    Join-Path $raiz 'docs\bugs-trace.md')
if ($documentoBugs -notmatch '\| Hallazgos \|' -or
    $documentoBugs -notmatch '\| Causa raíz \|') {
    throw 'Cada bug debe separar hallazgos y causa raíz.'
}
$coincidenciasFolios = [regex]::Matches(
    $documentoBugs, '(?m)^\| BUG-(?<numero>\d{3}) \|')
if ($coincidenciasFolios.Count -eq 0) {
    throw 'El registro de bugs no contiene folios.'
}
$numerosFolios = $coincidenciasFolios | ForEach-Object {
    [int]$_.Groups['numero'].Value
}
$foliosDuplicados = $numerosFolios | Group-Object |
    Where-Object Count -gt 1
if ($foliosDuplicados) {
    throw 'El registro de bugs contiene folios duplicados.'
}
$mayorFolio = ($numerosFolios | Measure-Object -Maximum).Maximum
$foliosFaltantes = 1..$mayorFolio | Where-Object {
    $_ -notin $numerosFolios
}
if ($foliosFaltantes) {
    throw "Faltan folios consecutivos: $($foliosFaltantes -join ', ')."
}

Push-Location $raiz
try {
    & (Join-Path $PSScriptRoot 'run-tests.ps1')
    if ($LASTEXITCODE -ne 0) { throw 'Fallaron las pruebas unitarias.' }

    $avisosEstrictos = @(
        '-Wpedantic',
        '-Wconversion',
        '-Wsign-conversion',
        '-Wshadow',
        '-Wdouble-promotion',
        '-Wformat=2',
        '-Werror'
    )
    & $compilador "@$banderasWindhawk" '-Isrc' @avisosEstrictos `
        '-fsyntax-only' $fuente
    if ($LASTEXITCODE -ne 0) { throw 'Falló la compilación estricta.' }

    & $compilador "@$banderasWindhawk" '-Isrc' '--analyze' `
        '-o' $analisisTemporal $fuente
    if ($LASTEXITCODE -ne 0) { throw 'El análisis estático detectó defectos.' }

    & (Join-Path $raiz 'tools\build.ps1')
    if ($LASTEXITCODE -ne 0) { throw 'Falló la generación de dist.' }

    $bibliotecas = @(
        '-lole32', '-loleaut32', '-lmfplay', '-lmfplat', '-lmfuuid',
        '-lshlwapi', '-lgdi32', '-luuid'
    )
    & $compilador "@$banderasWindhawk" '-shared' $distribucion `
        '-o' $dllTemporal @bibliotecas
    if ($LASTEXITCODE -ne 0) { throw 'Falló el enlace completo del mod.' }
}
finally {
    Pop-Location
    if (Test-Path -LiteralPath $dllTemporal) {
        Remove-Item -LiteralPath $dllTemporal -Force
    }
    if (Test-Path -LiteralPath $analisisTemporal) {
        Remove-Item -LiteralPath $analisisTemporal -Force
    }
}

Write-Host 'Puerta de calidad completa: OK'
