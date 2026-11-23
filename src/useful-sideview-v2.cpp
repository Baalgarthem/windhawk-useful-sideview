// ==WindhawkMod==
// @id              useful-sideview
// @name            Useful Side View
// @description     Transforma el explorador de Windows 11 con un panel lateral inyectado de alta fidelidad, fluido y cooperativo. Alterna con el atajo configurable (por defecto Ctrl + K).
// @version         4.1.10
// @author          Baalgarthem
// @include         explorer.exe
// @architecture    x86-64
// @compilerOptions -lole32 -loleaut32 -lshell32 -luser32 -lgdi32 -lcomctl32 -luuid -luxtheme -ldwmapi -lwindowscodecs -lmsimg32 -lshlwapi -ladvapi32 -lcomdlg32 -lbcrypt -lmfplat -lmfreadwrite -lmfuuid
// @license         MIT
// ==/WindhawkMod==

// ==WindhawkModSettings==
/*
- atajoTeclado: "Ctrl+K"
  $name: Atajo de teclado para alternar panel
  $description: >-
    Define la combinación de teclas para abrir o cerrar Useful Side View.
    Ejemplos: "Ctrl+K", "Alt+P", "Ctrl+Space", "F4".
- limiteCarpetasPesadas: 3
  $name: Cantidad de subcarpetas pesadas
  $description: >-
    Número de subcarpetas más pesadas a visualizar por defecto en la vista de carpetas (ej. 3, 5, 10, 20).
- mostrarNumerosLinea: true
  $name: Mostrar números de línea en texto y código
  $description: Activa o desactiva la numeración de líneas en las vistas de texto, Markdown y código.
- tamanoFuenteTexto: 10
  $name: Tamaño de letra para texto y Markdown
  $description: Tamaño compacto en píxeles para evitar desbordamientos. Se limita automáticamente entre 8 y 18.
- tamanoFuenteCodigo: 10
  $name: Tamaño de letra para código
  $description: Tamaño compacto en píxeles para archivos de programación. Se limita automáticamente entre 8 y 18.
- ajusteLineaTextoCodigo: true
  $name: Ajuste de línea (text wrapping)
  $description: Ajusta las líneas largas al ancho disponible en las vistas de texto, Markdown y código.
- reproductorVideoEnVista: false
  $name: Usar reproductor en la vista de videos
  $description: >-
    Sustituye la miniatura por el reproductor Media Foundation en la vista de un
    video. Desactivado por defecto para conservar la vista previa ligera.
- comparadorVideosExperimental: true
  $name: Comparador de videos (experimental)
  $description: >-
    Activa el comparador de videos y su reproductor Media Foundation. Sólo se abre
    al seleccionar exactamente dos videos del mismo tipo.
- comparadorVideosPantallaCompleta: true
  $name: Pantalla completa y PiP del comparador
  $description: >-
    Muestra los controles para alternar un video a pantalla completa y el otro
    en Picture in Picture dentro del comparador experimental.
- mostrarVersionMod: true
  $name: Mostrar versión del mod
  $description: >-
    Muestra la versión actual de Useful Side View en la vista general de
    carpetas cuando no hay una carpeta seleccionada.
- modoSemanticoDiffView: true
  $name: Modo semántico para diff view
  $description: >-
    Explica los cambios del diff con lenguaje didáctico para facilitar la lectura
    a personas que no programan.
- sincronizacionNotasWinKlok: false
  $name: Sincronización de notas con WinKlok (experimental)
  $description: >-
    Permite importar y exportar notas con WinKlok. Está desactivado por defecto;
    mientras permanezca desactivado, las notas de Useful Side View son atómicas.
- creadorDirectoriosIdioma: "es"
  $name: Idioma del creador de directorios
  $description: >-
    Idioma inicial para fechas semánticas y contadores escritos del Creador de
    directorios. Usa "es" para español o "en" para inglés.
- scriptBoxUbicacion: ""
  $name: Ubicación de Script Box
  $description: >-
    Carpeta raíz desde la que Script Box resolverá los scripts incluidos. Si se
    deja vacía, se usa la última ubicación elegida desde la vista general.
- scriptBoxScripts: ""
  $name: Scripts incluidos en Script Box
  $description: >-
    Lista blanca de nombres o rutas relativas, uno por línea. Sólo se muestran
    y ejecutan los archivos incluidos explícitamente; admite .ps1, .bat y .py
    (además de .cmd, .vbs y .js).
*/
// ==/WindhawkModSettings==

// ==WindhawkModReadme==
/*
# Useful Side View V2 (4.1.10)

Prototipo modular de nueva generación con arquitectura canónica de Windows 11:
- Inyección cooperativa con `CreateWindowExW` y subclaseo de `CabinetWClass` / `ShellTabWindowClass`.
- Renderizado de doble buffer GDI sin parpadeo (Anti-Flicker BitBlt).
- **Atajo interactivo configurable**: Por defecto **[ Ctrl + K ]**.
- **Pantalla Tutorial (Walkthrough)**: Se activa automáticamente al presionar el atajo sin archivos seleccionados.
- **Suite Funcional de Comprimidos & Cómics**: Portadas, árbol de archivos, navegación anterior/siguiente, "Abrir con" y selector de conversión.
- **Suite Funcional de Inspección de Carpetas**: Árbol jerárquico recursivo, cálculo de peso, Top 4 subcarpetas pesadas, enlaces simbólicos, copia de estructura y limpieza de carpetas vacías.
- **Vistas Especializadas Modulares**: Texto, Markdown, Imágenes, Video y Código con servicios inyectados.
- **Comparadores polimórficos**: Comparador general de nombres, peso y SHA-256; comparadores especializados de texto/documentos y video mediante servicios inyectables.
- **Comparadores de texto y documentos**: panel 50/50 con sliders individuales y sincronizados, resumen fijo de tres columnas y Diff View ampliado; el modo semántico explica los cambios en lenguaje claro.
- **Comparador de videos experimental**: Dos reproductores Media Foundation 50/50, sincronización confirmable con límite seguro para duraciones distintas, pantalla completa con PiP, swap de roles, resumen contraíble (inicia cerrado), botón para volver a lado a lado e información de archivos, activable desde Ajustes y únicamente para dos videos.
- **Persistencia nativa**: Historial de programas de "Abrir con" por categoría, migrado y validado con valores persistentes de Windhawk.
- **Liberación segura de comparadores**: al cerrar, contraer, cambiar de vista o descargar el mod se desmontan reproductores, callbacks, timers y capturas para no dejar archivos en uso.
- **Toolbox de notas**: La vista general permite crear, editar, copiar y eliminar notas persistentes. La sincronización con WinKlok es experimental y está desactivada por defecto.
- **Versión visible**: La vista general puede mostrar la versión actual desde el ajuste correspondiente.
- **Compatibilidad GPU**: El panel coopera con `gpu-accelerated-explorer-view` usando su overlay visual sin alterar su layout DirectComposition.
*/
// ==/WindhawkModReadme==

#include <windows.h>
#include <windowsx.h>
#include <shellapi.h>
#include <shlobj.h>
#include <shobjidl.h>
#include <shlguid.h>
#include <exdisp.h>
#include <commctrl.h>
#include <uxtheme.h>
#include <dwmapi.h>
#include <wincodec.h>
#include <shlwapi.h>
#include <commdlg.h>
#include <algorithm>
#include <atomic>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include <windhawk_utils.h>

#include "v2/ui/tema_visual.hpp"
#include "v2/ui/panel_interfaz.hpp"
#include "v2/ui/ventana_base.hpp"
#include "v2/nucleo/enrutador_vistas.hpp"
#include "v2/vistas/vista_walkthrough.hpp"
#include "v2/vistas/vista_comprimidos.hpp"
#include "v2/vistas/vista_carpetas.hpp"
#include "v2/vistas/vista_en_desarrollo.hpp"
#include "v2/nucleo/fabrica_vistas.hpp"
#include "v2/nucleo/shell_extractor.hpp"
#include "v2/nucleo/gestor_ventanas.hpp"
#include "v2/nucleo/hook_teclado.hpp"
#include "v2/nucleo/mod_core.hpp"

// ---------------------------------------------------------------------------
// Callbacks de Windhawk
// ---------------------------------------------------------------------------

BOOL Wh_ModInit() {
    Wh_Log(L"[UsefulSideView-v2] Inicializando mod canónico");
    if (!SideViewV2::InicializarModV2()) {
        Wh_Log(L"[UsefulSideView-v2] Error al inicializar mod");
        return FALSE;
    }
    return TRUE;
}

void Wh_ModAfterInit() {
    Wh_Log(L"[UsefulSideView-v2] Wh_ModAfterInit ejecutado");
    SideViewV2::DespuesDeIniciarModV2();
}

void Wh_ModBeforeUninit() {
    // Windhawk llama este punto antes de retirar los detours. Se cierra la
    // admisión de trabajo nuevo aquí para que ningún hook cree subclases o
    // publique mensajes mientras comienza el desmontaje.
    SideViewV2::PrepararDesinicializacionModV2();
}

void Wh_ModSettingsChanged() {
    Wh_Log(L"[UsefulSideView-v2] Configuración cambiada");
    SideViewV2::AlCambiarConfiguracionesV2();
}

void Wh_ModUninit() {
    Wh_Log(L"[UsefulSideView-v2] Descargando mod de forma segura");
    SideViewV2::DesinicializarModV2();
}
