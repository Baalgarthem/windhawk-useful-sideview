// ==WindhawkMod==
// @id              useful-sideview
// @name            Useful Side View
// @description     Adds a lightweight video player to the Windows 11 File Explorer preview pane
// @version         0.2.1
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
  $name: Reproducción automática
- sound: false
  $name: Audio habilitado por defecto
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
#include <new>
#include <string>

namespace {

// El orden documenta las dependencias y permite producir una sola unidad de
// traducción, requisito del formato distribuible de Windhawk.
//
// controles.cpp define buscarEnReproduccion y la caché de tiempo.
// ventanas.cpp define el delegado del hilo reproductor (HWND_MESSAGE).
// panel_previsualizacion.cpp crea las ventanas visibles.
// hilo_reproductor.cpp posee IMFPMediaPlayer y actualiza la caché.
#include "nucleo/constantes.hpp"
#include "nucleo/configuracion.hpp"
#include "nucleo/estado.hpp"
#include "nucleo/puntero_com.hpp"
#include "archivos/seleccion_explorador.cpp"
#include "interfaz/diseno_barra_progreso.hpp"
#include "reproductor/calculos_tiempo.hpp"
#include "reproductor/conversion_tiempo_media_foundation.hpp"
#include "reproductor/controles.cpp"
#include "reproductor/eventos_media_foundation.cpp"
#include "interfaz/renderizado.cpp"
#include "interfaz/ventanas.cpp"
#include "integracion/panel_previsualizacion.cpp"
#include "ejecucion/hilo_reproductor.cpp"

}  // namespace

#include "windhawk/ciclo_vida.cpp"
