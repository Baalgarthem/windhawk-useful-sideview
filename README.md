# Useful Side View

Windhawk mod that turns the Windows 11 File Explorer preview pane into a
lightweight, adaptive video player.

## Features

- Play/pause by clicking the video or the playback button.
- Interactive timeline with click, drag, visible thumb and keyboard seeking.
- Centered Windows 11 Fluent controls for play, mute and loop.
- Responsive layout that follows preview-pane resizing.
- Audio toggle; muted by default.
- Optional autoplay; disabled by default.
- Uses the native Windows Media Foundation pipeline and integrates with
  `explorer.exe`, where the modern Windows 11 preview pane is hosted.

Install `dist/useful-sideview.wh.cpp` as a local Windhawk mod and enable Explorer's
preview pane with `Alt+P`. Supported formats depend on the Media Foundation
codecs installed in Windows.

Editable sources live in `src/`, organized by responsibility: `archivos`,
`ejecucion`, `integracion`, `interfaz`, `nucleo`, `reproductor`, and `windhawk`. Generate
the single-file Windhawk distribution with
`build.cmd` or `powershell -ExecutionPolicy Bypass -File build.ps1`.
