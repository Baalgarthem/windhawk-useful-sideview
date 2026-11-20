param()
$ErrorActionPreference = 'Stop'

Add-Type -AssemblyName System.Windows.Forms
Add-Type -AssemblyName System.Drawing
[System.Windows.Forms.Application]::EnableVisualStyles()

$root = $PSScriptRoot
$dist = Join-Path $root 'dist'
$output = Join-Path $dist 'useful-sideview.wh.cpp'
$target = 'C:\ProgramData\Windhawk\ModsSource\useful-sideview.wh.cpp'
$logFile = Join-Path $dist 'compilador.log'
$script:logLines = [System.Collections.Generic.List[string]]::new()
$script:mutex = New-Object System.Threading.Mutex($false, 'Local\UsefulSideView.Compilador.V2')

try {
    $mutexAdquirido = $script:mutex.WaitOne(0)
} catch [System.Threading.AbandonedMutexException] {
    $mutexAdquirido = $true
}
if (-not $mutexAdquirido) {
    [Windows.Forms.MessageBox]::Show('Ya hay una instancia del compilador ejecutándose.', 'Compilador V2', 'OK', 'Information') | Out-Null
    $script:mutex.Dispose()
    exit 1
}

function Log($text) {
    $line = "[$(Get-Date -Format HH:mm:ss)] $text"
    $script:logLines.Add($line)
    if ($script:logBox) { $script:logBox.AppendText($line + [Environment]::NewLine) }
    New-Item -ItemType Directory -Force -Path $dist | Out-Null
    Set-Content -LiteralPath $logFile -Value ($script:logLines -join [Environment]::NewLine) -Encoding utf8
}
function Resolve-ToolPath($command) {
    if ($command.Path) { return $command.Path }
    if ($command.Source) { return $command.Source }
    if ($command.Definition) { return $command.Definition }
    return $null
}
function Require-Tool($name, $package) {
    $cmd = Get-Command $name -ErrorAction SilentlyContinue
    if ($cmd) {
        $path = Resolve-ToolPath $cmd
        if ($path) { return $path }
    }
    $answer = [Windows.Forms.MessageBox]::Show("No se encontró '$name'. ¿Deseas instalarlo mediante winget?", 'Dependencia requerida', 'YesNo', 'Warning')
    if ($answer -ne 'Yes') { throw "Falta la dependencia '$name'." }
    if (-not (Get-Command winget -ErrorAction SilentlyContinue)) { throw 'winget no está disponible.' }
    Log "Instalando $name ($package)..."
    $salidaInstalacion = & winget install --id $package --exact --silent --accept-source-agreements --accept-package-agreements 2>&1
    $codigoInstalacion = $LASTEXITCODE
    $salidaInstalacion | ForEach-Object { Log $_.ToString() }
    if ($codigoInstalacion -ne 0) { throw "La instalación de $name falló ($codigoInstalacion)." }
    $cmd = Get-Command $name -ErrorAction SilentlyContinue
    $path = if ($cmd) { Resolve-ToolPath $cmd } else { $null }
    if (-not $path) { throw "Reinicia el script para detectar '$name'." }
    return $path
}
function Require-File($path, $description) {
    if (-not (Test-Path -LiteralPath $path -PathType Leaf)) {
        throw "No se encontró ${description}: $path"
    }
}
function Assert-Bundle($path) {
    Require-File $path 'el bundle generado'
    $info = Get-Item -LiteralPath $path -ErrorAction Stop
    if ($info.Length -le 0) { throw "El bundle está vacío: $path" }
    $cabecera = ((Get-Content -LiteralPath $path -TotalCount 40 -ErrorAction Stop) -join [Environment]::NewLine)
    if ($cabecera -notmatch '==WindhawkMod==') {
        throw "El archivo no contiene metadatos válidos de Windhawk: $path"
    }
    return $info
}
function Assert-VersionSynchronized($bundlePath, $expectedVersion) {
    $bundle = Get-Content -Raw -LiteralPath $bundlePath -ErrorAction Stop
    $matchMetadata = [regex]::Match($bundle, '(?m)^//\s*@version\s+([0-9]+\.[0-9]+\.[0-9]+)')
    $matchConstante = [regex]::Match($bundle, 'VERSION_MOD_V2\[\]\s*=\s*L"([0-9]+\.[0-9]+\.[0-9]+)"')
    if (-not $matchMetadata.Success -or -not $matchConstante.Success) {
        throw "El bundle no contiene metadato y constante de versión verificables: $bundlePath"
    }
    if ($matchMetadata.Groups[1].Value -ne $expectedVersion -or
        $matchConstante.Groups[1].Value -ne $expectedVersion) {
        throw "Versión desincronizada en el bundle: esperado $expectedVersion, metadato $($matchMetadata.Groups[1].Value), constante $($matchConstante.Groups[1].Value)."
    }
}
function Invoke-Step($title, [scriptblock]$action) {
    try { Log "INICIO: $title"; & $action; Log "OK: $title" }
    catch {
        $detalle = $_.Exception.ToString()
        Log "ERROR: $detalle"
        [Windows.Forms.MessageBox]::Show($detalle, 'Error (registro conservado)', 'OK', 'Error') | Out-Null
    }
}
function Build-Bundle {
    $version = Get-Version
    $python = Require-Tool 'python' 'Python.Python.3.12'
    $combiner = Join-Path $root 'tools\combiner\combine.py'
    $entrypoint = Join-Path $root 'src\main.wh.cpp'
    Require-File $combiner 'el combinador'
    Require-File $entrypoint 'el entrypoint V2'
    $temp = Join-Path $dist 'useful-sideview.wh.cpp.tmp'
    New-Item -ItemType Directory -Force -Path $dist | Out-Null
    try {
        $argumentos = @('-X', 'utf8', $combiner, '-r', (Join-Path $root 'src'), '-o', $temp, $entrypoint)
        $salida = & $python @argumentos 2>&1
        $codigo = $LASTEXITCODE
        $salida | ForEach-Object { Log $_.ToString() }
        if ($codigo -ne 0) { throw "El combinador terminó con código $codigo." }
        Assert-Bundle $temp | Out-Null
        Assert-VersionSynchronized $temp $version
        Move-Item -LiteralPath $temp -Destination $output -Force
    } finally {
        if (Test-Path -LiteralPath $temp) { Remove-Item -LiteralPath $temp -Force }
    }
    # Solo después de validar y sustituir correctamente el bundle se eliminan
    # artefactos finales antiguos. El bundle válido anterior se conserva si el
    # combinador o la validación fallan.
    Get-ChildItem -LiteralPath $dist -Filter '*.wh.cpp' -File -ErrorAction SilentlyContinue |
        Where-Object { $_.FullName -ne $output } |
        ForEach-Object { Remove-Item -LiteralPath $_.FullName -Force }
    $finales = @(Get-ChildItem -LiteralPath $dist -Filter '*.wh.cpp' -File)
    if ($finales.Count -ne 1 -or $finales[0].FullName -ne $output) {
        throw 'La compilación no dejó exactamente un único bundle final en dist.'
    }
    Assert-Bundle $output | Out-Null
    Assert-VersionSynchronized $output $version
    Log "Bundle V2 generado y sincronizado en versión ${version}: $output"
}
function Get-Version {
    $entryText = Get-Content -Raw -LiteralPath (Join-Path $root 'src\useful-sideview-v2.cpp')
    $versionText = Get-Content -Raw -LiteralPath (Join-Path $root 'src\v2\nucleo\servicio_version_mod.hpp')
    $metadato = [regex]::Match($entryText, '(?m)^//\s*@version\s+([0-9]+\.[0-9]+\.[0-9]+)')
    $constante = [regex]::Match($versionText, 'VERSION_MOD_V2\[\]\s*=\s*L"([0-9]+\.[0-9]+\.[0-9]+)"')
    if (-not $metadato.Success -or -not $constante.Success) {
        throw 'No se encontraron el metadato @version y la constante VERSION_MOD_V2 en el entrypoint V2.'
    }
    if ($metadato.Groups[1].Value -ne $constante.Groups[1].Value) {
        throw "La versión del metadato ($($metadato.Groups[1].Value)) no coincide con VERSION_MOD_V2 ($($constante.Groups[1].Value))."
    }
    return $metadato.Groups[1].Value
}
function Get-DefaultCommitMessage($kind, $version) {
    switch ($kind) {
        'patch' { return "Useful Side View v$version — actualización patch" }
        'minor' { return "Useful Side View v$version — actualización minor" }
        'major' { return "Useful Side View v$version — actualización major" }
        default { return "Useful Side View v$version — actualización de versión" }
    }
}
function Set-Version($kind, $customMessage) {
    if ($kind -notin @('patch', 'minor', 'major')) { throw "Tipo de versión no válido: $kind" }
    $old = Get-Version; $p = $old.Split('.') | ForEach-Object { [int]$_ }
    switch ($kind) { 'patch' { $p[2]++ } 'minor' { $p[1]++; $p[2]=0 } 'major' { $p[0]++; $p[1]=0; $p[2]=0 } }
    $new = "$($p[0]).$($p[1]).$($p[2])"
    $file = Join-Path $root 'src\useful-sideview-v2.cpp'
    $text = Get-Content -Raw -LiteralPath $file
    $updated = [regex]::Replace($text, '(?m)^//\s*@version\s+[0-9]+\.[0-9]+\.[0-9]+', "// @version         $new", 1)
    [IO.File]::WriteAllText($file, $updated, [Text.UTF8Encoding]::new($false))
    $versionFile = Join-Path $root 'src\v2\nucleo\servicio_version_mod.hpp'
    $versionText = Get-Content -Raw -LiteralPath $versionFile
    $versionUpdated = [regex]::Replace($versionText, '(VERSION_MOD_V2\[\]\s*=\s*L")([0-9]+\.[0-9]+\.[0-9]+)(")', "`${1}$new`${3}", 1)
    [IO.File]::WriteAllText($versionFile, $versionUpdated, [Text.UTF8Encoding]::new($false))
    Log "Versión actualizada: $old -> $new"; Build-Bundle
    $script:pendingCommitMessage = if ([string]::IsNullOrWhiteSpace($customMessage)) {
        Get-DefaultCommitMessage $kind $new
    } else { $customMessage.Trim() }
    Log "Mensaje preparado: $script:pendingCommitMessage"
}
function Commit-Push($customMessage) {
    Build-Bundle
    $git = Require-Tool 'git' 'Git.Git'

    & $git -C $root add -A -- src docs compilador.ps1 dist/useful-sideview.wh.cpp tools README.md
    if ($LASTEXITCODE -ne 0) { throw "git add terminó con código $LASTEXITCODE." }

    & $git -C $root diff --cached --quiet
    $hayCambios = $LASTEXITCODE -ne 0
    if ($hayCambios) {
        $mensajeCommit = if (-not [string]::IsNullOrWhiteSpace($customMessage)) {
            $customMessage.Trim()
        } elseif (-not [string]::IsNullOrWhiteSpace($script:pendingCommitMessage)) {
            $script:pendingCommitMessage
        } else {
            Get-DefaultCommitMessage 'release' (Get-Version)
        }
        $salidaCommit = & $git -C $root commit -m $mensajeCommit 2>&1
        $salidaCommit | ForEach-Object { Log $_.ToString() }
        if ($LASTEXITCODE -ne 0) { throw "git commit terminó con código $LASTEXITCODE." }
    } else {
        Log 'No hay cambios preparados para commit; se conserva el estado actual.'
    }

    $rama = ([string](& $git -C $root branch --show-current 2>$null)).Trim()
    $remoto = ([string](& $git -C $root remote 2>$null | Select-Object -First 1)).Trim()
    if ([string]::IsNullOrWhiteSpace($rama) -or [string]::IsNullOrWhiteSpace($remoto)) {
        throw 'No se pudo determinar la rama actual o el remoto principal.'
    }

    $tag = "v$(Get-Version)"
    $head = ([string](& $git -C $root rev-parse HEAD)).Trim()
    $tagLocal = ([string](& $git -C $root rev-parse --verify "refs/tags/$tag" 2>$null)).Trim()
    if ($tagLocal) {
        $commitDelTag = ([string](& $git -C $root rev-list -n 1 $tag)).Trim()
        if ($commitDelTag -ne $head) { throw "El tag local $tag ya existe y apunta a otro commit; no se sobrescribirá." }
        Log "El tag local $tag ya apunta al commit actual."
    } else {
        & $git -C $root tag -a $tag -m "Release $tag"
        if ($LASTEXITCODE -ne 0) { throw "No fue posible crear el tag $tag ($LASTEXITCODE)." }
        Log "Tag local creado: $tag"
    }

    if ([Windows.Forms.MessageBox]::Show("Commit y tag local preparados. ¿Deseas hacer push de la rama y del tag $tag?", 'Confirmar push', 'YesNo', 'Question') -ne 'Yes') { return }

    $salidaPush = & $git -C $root push $remoto $rama 2>&1
    $salidaPush | ForEach-Object { Log $_.ToString() }
    if ($LASTEXITCODE -ne 0) { throw "git push de la rama terminó con código $LASTEXITCODE." }

    & $git -C $root ls-remote --exit-code --tags $remoto "refs/tags/$tag" *> $null
    if ($LASTEXITCODE -eq 0) {
        Log "El tag remoto $tag ya existe; se omite su sobrescritura."
        return
    }

    $salidaTag = & $git -C $root push $remoto "refs/tags/$tag" 2>&1
    $salidaTag | ForEach-Object { Log $_.ToString() }
    if ($LASTEXITCODE -ne 0) { throw "git push del tag terminó con código $LASTEXITCODE." }
}
function Inject-Bundle {
    $origen = Assert-Bundle $output
    $directorioDestino = Split-Path $target
    try {
        New-Item -ItemType Directory -Force -Path $directorioDestino -ErrorAction Stop | Out-Null
        Copy-Item -LiteralPath $output -Destination $target -Force -ErrorAction Stop
    }
    catch {
        Log 'Se requiere elevación para ModsSource; solicitando administrador...'
        $src = $output.Replace("'", "''"); $dst = $target.Replace("'", "''"); $dir = $directorioDestino.Replace("'", "''")
        $comando = "New-Item -ItemType Directory -Force -LiteralPath '$dir' | Out-Null; Copy-Item -LiteralPath '$src' -Destination '$dst' -Force"
        $p = Start-Process powershell.exe -Verb RunAs -Wait -PassThru -ArgumentList @('-NoProfile', '-ExecutionPolicy', 'Bypass', '-Command', $comando)
        if (-not $p -or $p.ExitCode -ne 0) { throw "La inyección elevada falló ($($p.ExitCode))." }
    }
    $inyectado = Assert-Bundle $target
    if ($inyectado.Length -ne $origen.Length) { throw 'La copia inyectada no coincide con el bundle generado.' }
    Log "Inyectado exclusivamente desde dist en $target"
}

try {
$form = New-Object Windows.Forms.Form; $form.Text = 'Useful Side View — Compilador V2'; $form.Size = New-Object Drawing.Size(900,650); $form.StartPosition = 'CenterScreen'; $form.BackColor = [Drawing.Color]::FromArgb(24,27,34); $form.ForeColor = [Drawing.Color]::White
$title = New-Object Windows.Forms.Label; $title.Text = '╔══ USEFUL SIDE VIEW · COMPILADOR V2 ══╗'; $title.Font = New-Object Drawing.Font('Consolas',16,[Drawing.FontStyle]::Bold); $title.AutoSize = $true; $title.Location = New-Object Drawing.Point(24,18); $form.Controls.Add($title)
$hint = New-Object Windows.Forms.Label; $hint.Text = 'Mouse o flechas + Enter. Los errores permanecen visibles y se guardan en dist\compilador.log.'; $hint.AutoSize = $true; $hint.Location = New-Object Drawing.Point(27,55); $form.Controls.Add($hint)
$buttons = @()
function Add-ActionButton($text, $y, $action) { $b = New-Object Windows.Forms.Button; $b.Text=$text; $b.Tag=$action; $b.Size=New-Object Drawing.Size(270,42); $b.Location=New-Object Drawing.Point(24,$y); $b.FlatStyle='Flat'; $b.BackColor=[Drawing.Color]::FromArgb(38,116,190); $b.ForeColor=[Drawing.Color]::White; $b.Add_Click({ Invoke-Step $this.Text $this.Tag }); $form.Controls.Add($b); $script:buttons += $b }
Add-ActionButton '⚙  Compilar bundle V2' 140 { Build-Bundle }
Add-ActionButton '📦  Compilar + inyectar' 190 { Build-Bundle; Inject-Bundle }
Add-ActionButton '✓  Commit y push guiado' 240 { Commit-Push $messageBox.Text }
Add-ActionButton '🚀  Inyectar bundle existente' 290 { Inject-Bundle }
$versionMenu = New-Object Windows.Forms.ComboBox; $versionMenu.Items.AddRange(@('patch','minor','major')); $versionMenu.SelectedIndex=0; $versionMenu.DropDownStyle='DropDownList'; $versionMenu.DrawMode='OwnerDrawFixed'; $versionMenu.ItemHeight=38; $versionMenu.Size=New-Object Drawing.Size(130,42); $versionMenu.Location=New-Object Drawing.Point(24,350)
$versionMenu.Add_DrawItem({ param($sender,$event) $seleccionado = (($event.State -band [Windows.Forms.DrawItemState]::Selected) -ne 0); $colorFondo = if ($seleccionado) { [Drawing.Color]::FromArgb(38,116,190) } else { [Drawing.Color]::FromArgb(40,44,52) }; $fondo = New-Object Drawing.SolidBrush($colorFondo); $event.Graphics.FillRectangle($fondo, $event.Bounds); $fondo.Dispose(); if ($event.Index -ge 0) { [Windows.Forms.TextRenderer]::DrawText($event.Graphics, $sender.Items[$event.Index].ToString(), $sender.Font, $event.Bounds, [Drawing.Color]::White, [Windows.Forms.TextFormatFlags]::HorizontalCenter -bor [Windows.Forms.TextFormatFlags]::VerticalCenter -bor [Windows.Forms.TextFormatFlags]::NoPrefix) } if ($seleccionado) { $event.DrawFocusRectangle() } }); $form.Controls.Add($versionMenu)
$versionButton = New-Object Windows.Forms.Button; $versionButton.Text='Subir versión + compilar'; $versionButton.Size=New-Object Drawing.Size(130,42); $versionButton.Location=New-Object Drawing.Point(164,350); $versionButton.FlatStyle='Flat'; $versionButton.BackColor=[Drawing.Color]::FromArgb(38,116,190); $versionButton.ForeColor=[Drawing.Color]::White; $versionButton.Add_Click({ Invoke-Step 'Versionar y compilar' { Set-Version $versionMenu.SelectedItem $messageBox.Text } }); $form.Controls.Add($versionButton); $script:buttons += $versionButton
$messageLabel = New-Object Windows.Forms.Label; $messageLabel.Text='Mensaje de commit (opcional; vacío = genérico)'; $messageLabel.AutoSize=$true; $messageLabel.Location=New-Object Drawing.Point(24,405); $form.Controls.Add($messageLabel)
$messageBox = New-Object Windows.Forms.TextBox; $messageBox.Size=New-Object Drawing.Size(400,30); $messageBox.Location=New-Object Drawing.Point(24,428); $messageBox.BackColor=[Drawing.Color]::FromArgb(12,14,18); $messageBox.ForeColor=[Drawing.Color]::White; $messageBox.BorderStyle='FixedSingle'; $form.Controls.Add($messageBox)
$close = New-Object Windows.Forms.Button; $close.Text='Cerrar'; $close.Size=New-Object Drawing.Size(270,42); $close.Location=New-Object Drawing.Point(24,475); $close.Add_Click({$form.Close()}); $form.Controls.Add($close); $script:buttons += $close
$script:logBox = New-Object Windows.Forms.TextBox; $script:logBox.Multiline=$true; $script:logBox.ReadOnly=$true; $script:logBox.ScrollBars='Both'; $script:logBox.WordWrap=$false; $script:logBox.Font=New-Object Drawing.Font('Consolas',10); $script:logBox.BackColor=[Drawing.Color]::FromArgb(12,14,18); $script:logBox.ForeColor=[Drawing.Color]::LightGreen; $script:logBox.Location=New-Object Drawing.Point(590,90); $script:logBox.Size=New-Object Drawing.Size(275,470); $form.Controls.Add($script:logBox)
$form.KeyPreview=$true; $form.Add_KeyDown({if($_.KeyCode -eq 'Escape'){$form.Close()}}); $form.Add_Shown({$buttons[0].Focus(); Log 'Menú listo. No se modificará legacy desde este flujo.'}); [void]$form.ShowDialog()
} finally {
    if ($script:mutex) {
        try { $script:mutex.ReleaseMutex() } catch {}
        $script:mutex.Dispose()
    }
}
