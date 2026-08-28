# Useful Side View

Windhawk mod that turns the Windows 11 File Explorer preview pane into a
lightweight, adaptive video player.

## Features

- Play/pause by clicking the video or the playback button.
- Seekable timeline.
- Responsive layout that follows preview-pane resizing.
- Audio toggle; muted by default.
- Optional autoplay; disabled by default.
- Uses the native Windows Media Foundation pipeline and runs only in
  `prevhost.exe` to minimize memory and CPU overhead.

Install `useful-sideview.wh.cpp` as a local Windhawk mod and enable Explorer's
preview pane with `Alt+P`. Supported formats depend on the Media Foundation
codecs installed in Windows.
