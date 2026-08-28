// ==WindhawkMod==
// @id              useful-sideview
// @name            Useful Side View
// @description     Adds a lightweight video player to the Windows 11 File Explorer preview pane
// @version         0.1.0
// @author          Reyr
// @include         explorer.exe
// @architecture    x86-64
// @compilerOptions -lole32 -loleaut32 -lmfplay -lmfplat -lmfuuid -lshlwapi -lgdi32 -luuid
// @license         MIT
// ==/WindhawkMod==

// ==WindhawkModReadme==
/*
# Useful Side View

Turns the Windows 11 File Explorer preview pane into a responsive video player.
Sound and autoplay are disabled by default. Supported formats depend on the
Media Foundation codecs installed in Windows.
*/
// ==/WindhawkModReadme==

// ==WindhawkModSettings==
/*
- autoplay: false
  $name: Autoplay videos
- sound: false
  $name: Enable sound
*/
// ==/WindhawkModSettings==

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>
#include <mfapi.h>
#include <mfplay.h>
#include <exdisp.h>
#include <shobjidl.h>
#include <shlguid.h>
#include <shlwapi.h>
#include <algorithm>
#include <atomic>
#include <mutex>
#include <string>

namespace {

// El orden documenta las dependencias y permite producir una sola unidad de
// traducción, requisito del formato distribuible de Windhawk.
#include "nucleo/constantes.hpp"
#include "nucleo/configuracion.hpp"
#include "nucleo/estado.hpp"
#include "archivos/seleccion_explorador.cpp"
#include "reproductor/eventos_media_foundation.cpp"
#include "reproductor/controles.cpp"
#include "interfaz/renderizado.cpp"
#include "interfaz/ventanas.cpp"
#include "host/panel_previsualizacion.cpp"
#include "runtime/hilo_reproductor.cpp"

}  // namespace

#include "windhawk/ciclo_vida.cpp"
