<#
.SYNOPSIS
    Gestor interactivo y por comandos de despliegue, build, versionado y autor para Useful Side View.

.DESCRIPTION
    Presenta un menú interactivo en consola o ejecuta directamente la acción indicada
    en el parámetro -Action (build, quality, inject, tests, patch, minor, major, custom, author).

    Cumple con los requerimientos de Agents (main).md:
    1. Ejecutar el build rápidamente.
    2. Subir de versión fácilmente (ejecutando build obligatorio antes de cualquier acción de Git).
    3. Cambiar el autor y reflejarlo en los artefactos generados.
#>

param(
    [Parameter(Position = 0, ValueFromPipeline = $true)]
    [string]$Action = ''
)

$ErrorActionPreference = 'Continue'
$projectRoot   = Split-Path -Parent $PSScriptRoot
$buildScript   = Join-Path $PSScriptRoot 'build.ps1'
$qualityScript = Join-Path $projectRoot 'tests\run-quality.ps1'
$testScript    = Join-Path $projectRoot 'tests\run-tests.ps1'
$modFile       = Join-Path $projectRoot 'src\useful-sideview.cpp'
$distFile      = Join-Path $projectRoot 'dist\useful-sideview.wh.cpp'

# Ubicación oficial de mods en desarrollo en Windhawk
$modsSourceDir = Join-Path $env:ProgramData 'Windhawk\ModsSource'
$targetFile    = Join-Path $modsSourceDir 'local@useful-sideview.wh.cpp'

function Get-CurrentVersion {
    if (Test-Path -LiteralPath $modFile) {
        $content = Get-Content -LiteralPath $modFile -Raw
        if ($content -match '(?m)^\/\/\s*@version\s+([0-9\.]+)') {
            return $matches[1]
        }
    }
    return "0.3.0"
}

function Get-CurrentAuthor {
    if (Test-Path -LiteralPath $modFile) {
        $content = Get-Content -LiteralPath $modFile -Raw
        if ($content -match '(?m)^\/\/\s*@author\s+(.+)\r?$') {
            return $matches[1].Trim()
        }
    }
    return "Reyr"
}

function Set-Author([string]$newAuthor = '') {
    if ([string]::IsNullOrWhiteSpace($newAuthor)) {
        $actual = Get-CurrentAuthor
        Write-Host "`nAutor actual: '$actual'" -ForegroundColor Cyan
        $newAuthor = Read-Host "Ingresa el nuevo nombre de autor"
    }
    if ([string]::IsNullOrWhiteSpace($newAuthor)) {
        Write-Host "    [AVISO] No se realizaron cambios en el autor." -ForegroundColor Yellow
        return
    }

    $current = Get-CurrentAuthor
    Write-Host "`n[AUTOR] Actualizando autor: '$current' -> '$newAuthor'" -ForegroundColor Cyan
    if (Test-Path -LiteralPath $modFile) {
        $m = [System.IO.File]::ReadAllText($modFile, [System.Text.Encoding]::UTF8)
        $m = $m -replace '(?m)^\/\/\s*@author\s+.+$', "// @author          $newAuthor"
        $utf8SinBom = New-Object System.Text.UTF8Encoding($false)
        [System.IO.File]::WriteAllText($modFile, $m, $utf8SinBom)
        Write-Host "    Autor actualizado en src\useful-sideview.cpp." -ForegroundColor Green
        Write-Host "    Ejecutando build para reflejar el cambio en dist\useful-sideview.wh.cpp..." -ForegroundColor DarkGray
        Invoke-BuildMod
    }
}

function Invoke-BuildMod {
    Write-Host "`n>>> [BUILD] Ejecutando generación rápida del bundle..." -ForegroundColor Yellow
    if (Test-Path -LiteralPath $buildScript) {
        & powershell -ExecutionPolicy Bypass -File $buildScript
        if ($LASTEXITCODE -eq 0) {
            Write-Host "    OK  Bundle generado exitosamente en dist\useful-sideview.wh.cpp" -ForegroundColor Green
            return $true
        } else {
            Write-Host "    ERROR  Falló la generación del bundle (código: $LASTEXITCODE)." -ForegroundColor Red
            return $false
        }
    } else {
        Write-Host "    ERROR  No se encontró el script $buildScript." -ForegroundColor Red
        return $false
    }
}

function Invoke-QualityGate {
    Write-Host "`n>>> [QUALITY] Ejecutando puerta completa de calidad (tests + clang + analyze + linker)..." -ForegroundColor Yellow
    if (Test-Path -LiteralPath $qualityScript) {
        & powershell -ExecutionPolicy Bypass -File $qualityScript
        if ($LASTEXITCODE -eq 0) {
            Write-Host "    OK  Puerta de calidad completada exitosamente." -ForegroundColor Green
            return $true
        } else {
            Write-Host "    ERROR  La puerta de calidad falló." -ForegroundColor Red
            return $false
        }
    } else {
        Write-Host "    ERROR  No se encontró el script $qualityScript." -ForegroundColor Red
        return $false
    }
}

function Invoke-UnitTests {
    Write-Host "`n>>> [TESTS] Ejecutando pruebas unitarias de tiempo y geometría..." -ForegroundColor Cyan
    if (Test-Path -LiteralPath $testScript) {
        & powershell -ExecutionPolicy Bypass -File $testScript
    } else {
        Write-Host "    ERROR  No se encontró el script $testScript." -ForegroundColor Red
    }
}

function Invoke-InjectMod {
    Write-Host "`n>>> [INJECT] Inyectando mod en Windhawk ModsSource..." -ForegroundColor Yellow
    Write-Host "    Origen : $distFile (Exclusivamente desde carpeta dist)" -ForegroundColor DarkGray
    Write-Host "    Destino: $targetFile" -ForegroundColor DarkGray

    if (-not (Test-Path -LiteralPath $distFile)) {
        Write-Host "    dist\useful-sideview.wh.cpp no existe. Ejecutando build obligatorio primero..." -ForegroundColor DarkGray
        $buildOk = Invoke-BuildMod
        if (-not $buildOk) { return }
    }

    try {
        if (-not (Test-Path -LiteralPath $modsSourceDir)) {
            New-Item -ItemType Directory -Path $modsSourceDir -Force | Out-Null
        }
        Copy-Item -LiteralPath $distFile -Destination $targetFile -Force -ErrorAction Stop
        Write-Host "    OK  Mod inyectado exitosamente desde dist\useful-sideview.wh.cpp en:" -ForegroundColor Green
        Write-Host "        $targetFile" -ForegroundColor Cyan
        Write-Host "    -> Recarga el mod en Windhawk para activar los cambios." -ForegroundColor Gray
    } catch {
        Write-Host "    Aviso: Se requiere elevación para escribir en ModsSource ($($_))." -ForegroundColor Yellow
        Write-Host "    Solicitando elevación de Administrador..." -ForegroundColor DarkGray
        try {
            $cmd = "Copy-Item -LiteralPath '$distFile' -Destination '$targetFile' -Force"
            $proc = Start-Process powershell -Verb RunAs -ArgumentList "-NoProfile -ExecutionPolicy Bypass -Command `"$cmd`"" -PassThru -Wait
            if ($proc.ExitCode -eq 0) {
                Write-Host "    OK  Mod inyectado exitosamente desde dist\useful-sideview.wh.cpp con privilegios elevados en:" -ForegroundColor Green
                Write-Host "        $targetFile" -ForegroundColor Cyan
                Write-Host "    -> Recarga el mod en Windhawk para activar los cambios." -ForegroundColor Gray
            } else {
                Write-Host "    No se pudo inyectar el mod (código de salida: $($proc.ExitCode))." -ForegroundColor Red
            }
        } catch {
            Write-Host "    Error solicitando elevación: $_" -ForegroundColor Red
        }
    }
}

function Invoke-GitActions([string]$version) {
    Write-Host "`n>>> [GIT] Gestión de control de versiones y tags Git..." -ForegroundColor Magenta
    $gitAvailable = Get-Command git -ErrorAction SilentlyContinue
    if (-not $gitAvailable) {
        Write-Host "    Git no está disponible en este entorno." -ForegroundColor DarkGray
        return
    }

    $gitStatus = & git status --porcelain
    if (-not [string]::IsNullOrWhiteSpace($gitStatus)) {
        Write-Host "    Cambios detectados tras actualizar la versión y generar el build." -ForegroundColor Cyan
        $confirmCommit = $null
        try {
            $confirmCommit = Read-Host "    ¿Deseas ejecutar 'git add .' y 'git commit' ahora? (S/N)"
        } catch {}

        if ($confirmCommit -match '^(s|si|y|yes)$') {
            & git add .
            $defaultMsg = "v${version}: actualizacion de version y bundle generado"
            $customMsg = $null
            try {
                $customMsg = Read-Host "    Mensaje de commit (Enter para '$defaultMsg')"
            } catch {}
            $finalMsg = if ([string]::IsNullOrWhiteSpace($customMsg)) { $defaultMsg } else { $customMsg }
            & git commit -m "$finalMsg"
            if ($LASTEXITCODE -eq 0) {
                Write-Host "    OK  Commit realizado exitosamente." -ForegroundColor Green
            }
        }
    } else {
        Write-Host "    El repositorio está limpio. No hay cambios pendientes por commitear." -ForegroundColor DarkGray
    }

    # Creación automática del git tag correspondiente a la versión modificada
    $tagName = "v${version}"
    Write-Host "`n    [TAG] Creando git tag '$tagName'..." -ForegroundColor Cyan
    $existingTag = & git tag -l $tagName
    if (-not [string]::IsNullOrWhiteSpace($existingTag)) {
        Write-Host "    Aviso: El tag '$tagName' ya existe localmente. Actualizando anotación..." -ForegroundColor Yellow
        & git tag -a $tagName -m "Release $tagName" -f
        if ($LASTEXITCODE -eq 0) {
            Write-Host "    OK  Tag '$tagName' actualizado exitosamente." -ForegroundColor Green
        }
    } else {
        & git tag -a $tagName -m "Release $tagName"
        if ($LASTEXITCODE -eq 0) {
            Write-Host "    OK  Tag '$tagName' creado exitosamente." -ForegroundColor Green
        } else {
            Write-Host "    Aviso: No se pudo crear el tag '$tagName' (código: $LASTEXITCODE)." -ForegroundColor Yellow
        }
    }

    $confirmPush = $null
    try {
        $confirmPush = Read-Host "`n    ¿Deseas ejecutar 'git push' y 'git push --tags' ahora? (S/N)"
    } catch {}

    if ($confirmPush -match '^(s|si|y|yes)$') {
        Write-Host "    Ejecutando git push..." -ForegroundColor DarkGray
        & git push
        Write-Host "    Ejecutando git push --tags..." -ForegroundColor DarkGray
        & git push --tags
        if ($LASTEXITCODE -eq 0) {
            Write-Host "    OK  Cambios y tags enviados al repositorio remoto." -ForegroundColor Green
        } else {
            Write-Host "    Aviso: git push finalizó con código $LASTEXITCODE." -ForegroundColor Yellow
        }
    }
}

function Step-Version([string]$type, [string]$customVer = '') {
    $current = Get-CurrentVersion
    $newVer = ""

    if ($type.ToLower() -eq 'custom') {
        if ([string]::IsNullOrWhiteSpace($customVer)) {
            $customVer = Read-Host "`nIngresa la nueva versión (ej. 0.3.5)"
        }
        if ($customVer -notmatch '^[0-9]+\.[0-9]+\.[0-9]+') {
            Write-Host "    Formato de versión inválido. Debe ser X.Y.Z" -ForegroundColor Red
            return
        }
        $newVer = $customVer.Trim()
    } else {
        $parts = $current.Split('.')
        $major = [int]$parts[0]
        $minor = [int]$parts[1]
        $patch = if ($parts.Length -gt 2) { [int]$parts[2] } else { 0 }

        switch ($type.ToLower()) {
            'major' { $major++; $minor = 0; $patch = 0 }
            'minor' { $minor++; $patch = 0 }
            'patch' { $patch++ }
        }
        $newVer = "$major.$minor.$patch"
    }

    Write-Host "`n[VERSION] Incrementando versión: $current -> $newVer ($type)" -ForegroundColor Cyan

    # 1. Actualizar src/useful-sideview.cpp en UTF-8 sin BOM
    if (Test-Path -LiteralPath $modFile) {
        $m = [System.IO.File]::ReadAllText($modFile, [System.Text.Encoding]::UTF8)
        $m = $m -replace '(?m)^\/\/\s*@version\s+[0-9\.]+', "// @version         $newVer"
        $utf8SinBom = New-Object System.Text.UTF8Encoding($false)
        [System.IO.File]::WriteAllText($modFile, $m, $utf8SinBom)
    }

    Write-Host "    Versión actualizada a v$newVer en src\useful-sideview.cpp." -ForegroundColor Green
    Write-Host "    Ejecutando build para empaquetar los cambios más recientes..." -ForegroundColor DarkGray

    # 2. Ejecutar build obligatorio
    $buildOk = Invoke-BuildMod
    if ($buildOk) {
        Write-Host "    Build completado exitosamente con la nueva versión v$newVer." -ForegroundColor Green

        # 3. Acciones de Git y creación de Tag posteriores a la subida de versión y build
        Invoke-GitActions $newVer
    } else {
        Write-Host "    ERROR: El build falló tras actualizar la versión. No se realizarán acciones de Git." -ForegroundColor Red
    }
}

function Invoke-GitSyncGeneral {
    Write-Host "`n>>> [GIT SYNC] Ejecutando sincronización general de Git (add, commit y push)..." -ForegroundColor Magenta
    $gitAvailable = Get-Command git -ErrorAction SilentlyContinue
    if (-not $gitAvailable) {
        Write-Host "    ERROR: Git no está disponible en este entorno." -ForegroundColor Red
        return $false
    }

    $gitStatus = & git status --porcelain
    if ([string]::IsNullOrWhiteSpace($gitStatus)) {
        Write-Host "    El repositorio está limpio. No hay cambios pendientes por commitear." -ForegroundColor DarkGray
        Write-Host "    Verificando y enviando commits locales pendientes con 'git push'..." -ForegroundColor Cyan
        & git push
        if ($LASTEXITCODE -eq 0) {
            Write-Host "    OK  Repositorio sincronizado con el remoto." -ForegroundColor Green
            return $true
        } else {
            Write-Host "    Aviso: git push finalizó con código $LASTEXITCODE." -ForegroundColor Yellow
            return $false
        }
    }

    Write-Host "    Cambios pendientes detectados. Agregando archivos con 'git add .'..." -ForegroundColor Cyan
    & git add .

    $mensajeCommit = "mejoras en código de software"
    Write-Host "    Creando commit: '$mensajeCommit'..." -ForegroundColor Cyan
    & git commit -m "$mensajeCommit"
    if ($LASTEXITCODE -ne 0) {
        Write-Host "    ERROR: Falló 'git commit' (código: $LASTEXITCODE)." -ForegroundColor Red
        return $false
    }
    Write-Host "    OK  Commit realizado exitosamente." -ForegroundColor Green

    Write-Host "    Enviando cambios al repositorio remoto con 'git push'..." -ForegroundColor Cyan
    & git push
    if ($LASTEXITCODE -eq 0) {
        Write-Host "    OK  Sincronización Git completada exitosamente." -ForegroundColor Green
        return $true
    } else {
        Write-Host "    Aviso: git push finalizó con código $LASTEXITCODE." -ForegroundColor Yellow
        return $false
    }
}

function Show-Header {
    Clear-Host
    $ver = Get-CurrentVersion
    $aut = Get-CurrentAuthor
    Write-Host "==========================================================" -ForegroundColor Cyan
    Write-Host "       USEFUL SIDE VIEW - DEPLOYMENT MANAGER              " -ForegroundColor White
    Write-Host "==========================================================" -ForegroundColor Cyan
    Write-Host "  Versión actual del mod : v$ver" -ForegroundColor Yellow
    Write-Host "  Autor actual           : $aut" -ForegroundColor Yellow
    Write-Host "  Artefacto distribuible : dist\useful-sideview.wh.cpp" -ForegroundColor DarkGray
    Write-Host "  Destino de inyección   : $targetFile" -ForegroundColor DarkGray
    Write-Host "==========================================================" -ForegroundColor Cyan
    Write-Host ""
}

function Show-Menu {
    Show-Header
    Write-Host "  --- Construcción y Calidad ---" -ForegroundColor DarkCyan
    Write-Host "  [1] Build Rápido          (Generar dist\useful-sideview.wh.cpp)" -ForegroundColor Green
    Write-Host "  [2] Puerta de Calidad     (Tests + Clang estricto + Analyze + Linker)" -ForegroundColor White
    Write-Host "  [3] Inyectar en Windhawk  (Copiar a ModsSource\local@useful-sideview.wh.cpp)" -ForegroundColor White
    Write-Host "  [4] Ejecutar Tests        (Pruebas unitarias de tiempo y geometría)" -ForegroundColor Cyan
    Write-Host ""
    Write-Host "  --- Gestión de Versionado y Git ---" -ForegroundColor DarkCyan
    Write-Host "  [5] Subir versión PATCH   (+0.0.1 -> Build -> Git Tag)" -ForegroundColor Gray
    Write-Host "  [6] Subir versión MINOR   (+0.1.0 -> Build -> Git Tag)" -ForegroundColor Gray
    Write-Host "  [7] Subir versión MAJOR   (+1.0.0 -> Build -> Git Tag)" -ForegroundColor Gray
    Write-Host "  [8] Versión personalizada (Ingresar versión -> Build -> Git Tag)" -ForegroundColor Gray
    Write-Host "  [9] Sincronizar Git       (git add . -> commit 'mejoras en código de software' -> push)" -ForegroundColor Magenta
    Write-Host "  [10] Cambiar Autor        (Actualizar autor y re-empaquetar)" -ForegroundColor DarkGray
    Write-Host ""
    Write-Host "  [0] Salir (o presiona 'Q')" -ForegroundColor DarkYellow
    Write-Host "==========================================================" -ForegroundColor Cyan
}

function Pause-Menu {
    Write-Host "`nPresiona cualquier tecla para continuar..." -ForegroundColor DarkGray
    try {
        [void][System.Console]::ReadKey($true)
    } catch {}
}

function Execute-Action([string]$act) {
    switch -Regex ($act.Trim()) {
        '^(1|build|b)$' {
            return (Invoke-BuildMod)
        }
        '^(2|quality|qg)$' {
            return (Invoke-QualityGate)
        }
        '^(3|inject|i)$' {
            Invoke-InjectMod
            return $true
        }
        '^(4|tests?|t)$' {
            Invoke-UnitTests
            return ($LASTEXITCODE -eq 0)
        }
        '^(5|patch|p)$' {
            Step-Version 'patch'
            return $true
        }
        '^(6|minor|m)$' {
            Step-Version 'minor'
            return $true
        }
        '^(7|major)$' {
            Step-Version 'major'
            return $true
        }
        '^(8|custom|v)$' {
            Step-Version 'custom'
            return $true
        }
        '^(9|sync|git|s)$' {
            return (Invoke-GitSyncGeneral)
        }
        '^(10|author|a)$' {
            Set-Author
            return $true
        }
        '^(0|q|exit)$' {
            Write-Host "`nSaliendo del Deployment Manager. ¡Hasta pronto!`n" -ForegroundColor Cyan
            return $false
        }
        default {
            Write-Host "`nOpción no válida: '$act'" -ForegroundColor Red
            return $false
        }
    }
}

# Si se pasó una acción directa por parámetro o pipeline
if ($Action -ne '') {
    $resultadoAccion = Execute-Action $Action
    if ($resultadoAccion -eq $true) {
        exit 0
    } else {
        exit 1
    }
}

# Modo interactivo en bucle
do {
    Show-Menu
    $inputChoice = Read-Host "`nSelecciona una opción [0-9, o Q]"
    if ($null -eq $inputChoice -or $inputChoice -eq '') {
        $inputChoice = '0'
    }
    $keepGoing = Execute-Action $inputChoice
    if ($keepGoing -and $inputChoice -notin @('0', 'q', 'Q', 'exit')) {
        Pause-Menu
    }
} while ($keepGoing -and $inputChoice -notin @('0', 'q', 'Q', 'exit'))
