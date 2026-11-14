param()
$ErrorActionPreference = 'Stop'

Add-Type -AssemblyName System.Windows.Forms
Add-Type -AssemblyName System.Drawing
[System.Windows.Forms.Application]::EnableVisualStyles()

$root = $PSScriptRoot
$dist = Join-Path $root 'dist'
$output = Join-Path $dist 'useful-sideview.wh.cpp'
$target = 'C:\ProgramData\Windhawk\ModsSource\useful-sideview.wh.cpp'
$logFile = Join-Path $dist 'build-universal.log'
$script:logLines = [System.Collections.Generic.List[string]]::new()

function Log($text) {
    $line = "[$(Get-Date -Format HH:mm:ss)] $text"
    $script:logLines.Add($line)
    if ($script:logBox) { $script:logBox.AppendText($line + [Environment]::NewLine) }
    New-Item -ItemType Directory -Force -Path $dist | Out-Null
    Set-Content -LiteralPath $logFile -Value ($script:logLines -join [Environment]::NewLine) -Encoding utf8
}
function Require-Tool($name, $package) {
    $cmd = Get-Command $name -ErrorAction SilentlyContinue
    if ($cmd) { return $cmd.Source }
    $answer = [Windows.Forms.MessageBox]::Show("No se encontró '$name'. ¿Deseas instalarlo mediante winget?", 'Dependencia requerida', 'YesNo', 'Warning')
    if ($answer -ne 'Yes') { throw "Falta la dependencia '$name'." }
    if (-not (Get-Command winget -ErrorAction SilentlyContinue)) { throw 'winget no está disponible.' }
    Log "Instalando $name ($package)..."
    winget install --id $package --exact --silent --accept-source-agreements --accept-package-agreements
    if ($LASTEXITCODE -ne 0) { throw "La instalación de $name falló ($LASTEXITCODE)." }
    $cmd = Get-Command $name -ErrorAction SilentlyContinue
    if (-not $cmd) { throw "Reinicia el script para detectar '$name'." }
    return $cmd.Source
}
function Invoke-Step($title, [scriptblock]$action) {
    try { Log "INICIO: $title"; & $action; Log "OK: $title" }
    catch { Log "ERROR: $($_.Exception.Message)"; [Windows.Forms.MessageBox]::Show($_.Exception.ToString(), 'Error (registro conservado)', 'OK', 'Error') | Out-Null }
}
function Build-Bundle {
    $python = Require-Tool 'python' 'Python.Python.3.12'
    $temp = Join-Path $dist 'useful-sideview.wh.cpp.tmp'
    New-Item -ItemType Directory -Force -Path $dist | Out-Null
    # El bundle temporal y el final viven siempre en dist. Se purgan bundles
    # finales antiguos para que dist conserve un único archivo integrado.
    Get-ChildItem -LiteralPath $dist -Filter '*.wh.cpp' -File -ErrorAction SilentlyContinue |
        Where-Object { $_.FullName -ne $output } |
        ForEach-Object { Remove-Item -LiteralPath $_.FullName -Force }
    try {
        & $python -X utf8 (Join-Path $root 'tools\combiner\combine.py') -r (Join-Path $root 'src') -o $temp (Join-Path $root 'src\main.wh.cpp')
        if ($LASTEXITCODE -ne 0) { throw "El combinador terminó con código $LASTEXITCODE." }
        Move-Item -LiteralPath $temp -Destination $output -Force
    } finally {
        if (Test-Path -LiteralPath $temp) { Remove-Item -LiteralPath $temp -Force }
    }
    $finales = @(Get-ChildItem -LiteralPath $dist -Filter '*.wh.cpp' -File)
    if ($finales.Count -ne 1 -or $finales[0].FullName -ne $output) {
        throw 'La compilación no dejó exactamente un único bundle final en dist.'
    }
    Log "Bundle V2 generado: $output"
}
function Get-Version {
    $text = Get-Content -Raw -LiteralPath (Join-Path $root 'src\useful-sideview-v2.cpp')
    if ($text -match '(?m)^//\s*@version\s+([0-9]+\.[0-9]+\.[0-9]+)') { return $matches[1] }
    throw 'No se encontró @version en el entrypoint V2.'
}
function Set-Version($kind) {
    $old = Get-Version; $p = $old.Split('.') | ForEach-Object { [int]$_ }
    switch ($kind) { 'patch' { $p[2]++ } 'minor' { $p[1]++; $p[2]=0 } 'major' { $p[0]++; $p[1]=0; $p[2]=0 } }
    $new = "$($p[0]).$($p[1]).$($p[2])"
    $file = Join-Path $root 'src\useful-sideview-v2.cpp'; $text = Get-Content -Raw -LiteralPath $file
    $updated = [regex]::Replace($text, '(?m)^//\s*@version\s+[0-9]+\.[0-9]+\.[0-9]+', "// @version         $new", 1)
    [IO.File]::WriteAllText($file, $updated, [Text.UTF8Encoding]::new($false)); Log "Versión actualizada: $old -> $new"; Build-Bundle
}
function Bump-Version {
    $file = Join-Path $root 'src\useful-sideview-v2.cpp'; $old = Get-Content -Raw -LiteralPath $file
    $new = [regex]::Replace($old, '(@version\s+\d+\.\d+\.)(\d+)', { param($m) "$($m.Groups[1].Value)$([int]$m.Groups[2].Value + 1)" }, 1)
    if ($new -eq $old) { throw 'No se encontró una versión V2 válida.' }
    Set-Content -LiteralPath $file -Value $new -Encoding utf8NoBOM; Log 'Versión patch V2 incrementada.'
}
function Commit-Push {
    Build-Bundle
    $git = Require-Tool 'git' 'Git.Git'; & $git -C $root add src build-universal.ps1 dist/useful-sideview.wh.cpp
    & $git -C $root commit -m 'Actualiza estructura y build universal V2'
    if ($LASTEXITCODE -ne 0) { throw "git commit terminó con código $LASTEXITCODE." }
    $tag = "v$(Get-Version)"; & $git -C $root tag -a $tag -m "Release $tag" -f
    if ([Windows.Forms.MessageBox]::Show("Commit y tag $tag creados. ¿Deseas hacer push y push de tags?", 'Confirmar push', 'YesNo', 'Question') -eq 'Yes') { & $git -C $root push; & $git -C $root push --tags; if ($LASTEXITCODE -ne 0) { throw "git push terminó con código $LASTEXITCODE." } }
}
function Inject-Bundle {
    if (-not (Test-Path -LiteralPath $output)) { throw 'Primero genera el bundle.' }
    New-Item -ItemType Directory -Force -Path (Split-Path $target) | Out-Null
    try { Copy-Item -LiteralPath $output -Destination $target -Force -ErrorAction Stop }
    catch {
        Log 'Se requiere elevación para ModsSource; solicitando administrador...'
        $src = $output.Replace("'", "''"); $dst = $target.Replace("'", "''")
        $p = Start-Process powershell.exe -Verb RunAs -Wait -PassThru -ArgumentList "-NoProfile -ExecutionPolicy Bypass -Command Copy-Item -LiteralPath '$src' -Destination '$dst' -Force"
        if ($p.ExitCode -ne 0) { throw "La inyección elevada falló ($($p.ExitCode))." }
    }
    Log "Inyectado exclusivamente desde dist en $target"
}

$form = New-Object Windows.Forms.Form; $form.Text = 'Useful Side View — Build Universal V2'; $form.Size = New-Object Drawing.Size(900,650); $form.StartPosition = 'CenterScreen'; $form.BackColor = [Drawing.Color]::FromArgb(24,27,34); $form.ForeColor = [Drawing.Color]::White
$title = New-Object Windows.Forms.Label; $title.Text = '╔══ USEFUL SIDE VIEW · BUILD UNIVERSAL V2 ══╗'; $title.Font = New-Object Drawing.Font('Consolas',16,[Drawing.FontStyle]::Bold); $title.AutoSize = $true; $title.Location = New-Object Drawing.Point(24,18); $form.Controls.Add($title)
$hint = New-Object Windows.Forms.Label; $hint.Text = 'Mouse o flechas + Enter. Los errores permanecen visibles y se guardan en dist\build-universal.log.'; $hint.AutoSize = $true; $hint.Location = New-Object Drawing.Point(27,55); $form.Controls.Add($hint)
$buttons = @()
function Add-ActionButton($text, $y, $action) { $b = New-Object Windows.Forms.Button; $b.Text=$text; $b.Tag=$action; $b.Size=New-Object Drawing.Size(270,42); $b.Location=New-Object Drawing.Point(24,$y); $b.FlatStyle='Flat'; $b.BackColor=[Drawing.Color]::FromArgb(38,116,190); $b.ForeColor=[Drawing.Color]::White; $b.Add_Click({ Invoke-Step $this.Text $this.Tag }); $form.Controls.Add($b); $script:buttons += $b }
Add-ActionButton '⚙  Compilar bundle V2' 140 { Build-Bundle }
Add-ActionButton '📦  Compilar + inyectar' 190 { Build-Bundle; Inject-Bundle }
Add-ActionButton '✓  Commit y push guiado' 240 { Commit-Push }
Add-ActionButton '🚀  Inyectar bundle existente' 290 { Inject-Bundle }
$versionMenu = New-Object Windows.Forms.ComboBox; $versionMenu.Items.AddRange(@('patch','minor','major')); $versionMenu.SelectedIndex=0; $versionMenu.DropDownStyle='DropDownList'; $versionMenu.DrawMode='OwnerDrawFixed'; $versionMenu.ItemHeight=38; $versionMenu.Size=New-Object Drawing.Size(130,42); $versionMenu.Location=New-Object Drawing.Point(24,350)
$versionMenu.Add_DrawItem({ param($sender,$event) $seleccionado = (($event.State -band [Windows.Forms.DrawItemState]::Selected) -ne 0); $colorFondo = if ($seleccionado) { [Drawing.Color]::FromArgb(38,116,190) } else { [Drawing.Color]::FromArgb(40,44,52) }; $fondo = New-Object Drawing.SolidBrush($colorFondo); $event.Graphics.FillRectangle($fondo, $event.Bounds); $fondo.Dispose(); if ($event.Index -ge 0) { [Windows.Forms.TextRenderer]::DrawText($event.Graphics, $sender.Items[$event.Index].ToString(), $sender.Font, $event.Bounds, [Drawing.Color]::White, [Windows.Forms.TextFormatFlags]::HorizontalCenter -bor [Windows.Forms.TextFormatFlags]::VerticalCenter -bor [Windows.Forms.TextFormatFlags]::NoPrefix) } if ($seleccionado) { $event.DrawFocusRectangle() } }); $form.Controls.Add($versionMenu)
$versionButton = New-Object Windows.Forms.Button; $versionButton.Text='Subir versión + compilar'; $versionButton.Size=New-Object Drawing.Size(130,42); $versionButton.Location=New-Object Drawing.Point(164,350); $versionButton.FlatStyle='Flat'; $versionButton.BackColor=[Drawing.Color]::FromArgb(38,116,190); $versionButton.ForeColor=[Drawing.Color]::White; $versionButton.Add_Click({ Invoke-Step 'Versionar y compilar' { Set-Version $versionMenu.SelectedItem } }); $form.Controls.Add($versionButton); $script:buttons += $versionButton
$close = New-Object Windows.Forms.Button; $close.Text='Cerrar'; $close.Size=New-Object Drawing.Size(270,42); $close.Location=New-Object Drawing.Point(24,410); $close.Add_Click({$form.Close()}); $form.Controls.Add($close); $script:buttons += $close
$script:logBox = New-Object Windows.Forms.TextBox; $script:logBox.Multiline=$true; $script:logBox.ReadOnly=$true; $script:logBox.ScrollBars='Both'; $script:logBox.WordWrap=$false; $script:logBox.Font=New-Object Drawing.Font('Consolas',10); $script:logBox.BackColor=[Drawing.Color]::FromArgb(12,14,18); $script:logBox.ForeColor=[Drawing.Color]::LightGreen; $script:logBox.Location=New-Object Drawing.Point(590,90); $script:logBox.Size=New-Object Drawing.Size(275,470); $form.Controls.Add($script:logBox)
$form.KeyPreview=$true; $form.Add_KeyDown({if($_.KeyCode -eq 'Escape'){$form.Close()}}); $form.Add_Shown({$buttons[0].Focus(); Log 'Menú listo. No se modificará legacy desde este flujo.'}); [void]$form.ShowDialog()
