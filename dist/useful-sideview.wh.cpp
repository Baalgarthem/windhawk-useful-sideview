/**** start inlining useful-sideview-v2.cpp ****/
// ==WindhawkMod==
// @id              useful-sideview
// @name            Useful Side View
// @description     Transforma el explorador de Windows 11 con un panel lateral inyectado de alta fidelidad, fluido y cooperativo. Alterna con el atajo configurable (por defecto Ctrl + K).
// @version         4.1.4
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
*/
// ==/WindhawkModSettings==

// ==WindhawkModReadme==
/*
# Useful Side View V2 (4.1.4)

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
- **Comparador de videos experimental**: Dos reproductores Media Foundation 50/50, sincronización confirmable con límite seguro para duraciones distintas, pantalla completa con PiP, swap de roles e información de archivos, activable desde Ajustes y únicamente para dos videos.
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

/**** start inlining v2/ui/tema_visual.hpp ****/

#include <windows.h>
#include <string>

namespace SideViewV2 {

struct TemaVisual {
    bool modoOscuro{true};
    COLORREF fondo{RGB(32, 32, 32)};
    COLORREF fondoCabecera{RGB(40, 40, 40)};
    COLORREF fondoTarjeta{RGB(45, 45, 45)};
    COLORREF borde{RGB(60, 60, 60)};
    COLORREF textoPrincipal{RGB(240, 240, 240)};
    COLORREF textoSecundario{RGB(160, 160, 160)};
    COLORREF acento{RGB(0, 120, 215)};
    COLORREF botonFondo{RGB(50, 50, 50)};
};

inline TemaVisual ObtenerTemaActual() {
    TemaVisual tema;
    // Detección de tema oscuro de Windows
    HKEY claveConfig = nullptr;
    if (RegOpenKeyExW(HKEY_CURRENT_USER,
                      L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
                      0, KEY_READ, &claveConfig) == ERROR_SUCCESS) {
        DWORD valorApps = 1;
        DWORD tamano = sizeof(valorApps);
        if (RegQueryValueExW(claveConfig, L"AppsUseLightTheme", nullptr, nullptr,
                             reinterpret_cast<LPBYTE>(&valorApps), &tamano) == ERROR_SUCCESS) {
            tema.modoOscuro = (valorApps == 0);
        }
        RegCloseKey(claveConfig);
    }

    if (!tema.modoOscuro) {
        tema.fondo = RGB(248, 249, 250);
        tema.fondoCabecera = RGB(240, 242, 245);
        tema.fondoTarjeta = RGB(255, 255, 255);
        tema.borde = RGB(225, 228, 232);
        tema.textoPrincipal = RGB(33, 37, 41);
        tema.textoSecundario = RGB(108, 117, 125);
        tema.botonFondo = RGB(235, 238, 242);
    }

    return tema;
}

} // namespace SideViewV2
/**** ended inlining v2/ui/tema_visual.hpp ****/
/**** start inlining v2/ui/panel_interfaz.hpp ****/

#include <windows.h>
#include <string>
#include <vector>
/**** skipping file: v2/ui/tema_visual.hpp ****/

namespace SideViewV2 {

inline constexpr UINT WM_SIDEVIEW_CLOSE_COMPARADOR_ARCHIVOS = WM_APP + 0x4A3;
inline constexpr UINT WM_SIDEVIEW_VIDEO_EVENT_V2 = WM_APP + 0x4A4;
inline constexpr UINT WM_SIDEVIEW_COMPARADOR_TICK = WM_APP + 0x4A5;
inline constexpr UINT WM_SIDEVIEW_COMPARADOR_PIP_CLICK = WM_APP + 0x4A6;
inline constexpr UINT WM_SIDEVIEW_REQUEST_COMPARADOR_ARCHIVOS = WM_APP + 0x4A7;

class IPanelContenido {
public:
    virtual ~IPanelContenido() = default;

    // Renderizado cooperativo en doble buffer
    virtual void Renderizar(HDC hdc, const RECT& rect, const TemaVisual& tema) = 0;

    // Notificación de cambio de selección en Explorer
    virtual void AlCambiarSeleccion(const std::vector<std::wstring>& archivos, const std::wstring& carpeta) = 0;
    // Montaje opcional de superficies/recursos inyectados después de crear el panel.
    virtual void AlActivar(HWND) {}
    virtual void ActualizarConfiguracion() {}

    // Manejo de eventos de ratón/teclado en el panel (retornan true si consumieron el evento)
    virtual bool ProcesarMensajeRaton(UINT msg, WPARAM wParam, LPARAM lParam, HWND hwndPanel) { return false; }
    virtual bool ProcesarMensajeTeclado(UINT msg, WPARAM wParam, LPARAM lParam, HWND hwndPanel) { return false; }
    virtual bool ProcesarMensajeVentana(UINT, WPARAM, LPARAM, HWND) { return false; }

    // Liberación segura de recursos pesados
    virtual void Liberar() {}
};

} // namespace SideViewV2
/**** ended inlining v2/ui/panel_interfaz.hpp ****/
/**** start inlining v2/ui/ventana_base.hpp ****/

#include <windows.h>
#include <commctrl.h>
#include <dwmapi.h>
#include <algorithm>
#include <atomic>
#include <memory>
#include <string>
#include <vector>
/**** skipping file: v2/ui/tema_visual.hpp ****/
/**** skipping file: v2/ui/panel_interfaz.hpp ****/
/**** start inlining v2/nucleo/servicio_compatibilidad_gpu.hpp ****/

#include <windows.h>

namespace SideViewV2 {

// Contrato de interoperabilidad observado en
// refs/gpu-accelerated-explorer-view.wh.cpp. No se enlaza contra ese mod ni
// se invoca su código: sólo se consume su propiedad HWND documentada en el
// propio bundle para localizar la superficie visual que reemplaza a
// SHELLDLL_DefView.
struct SuperficieExploradorV2 {
    HWND vistaShell{nullptr};
    HWND superficieVisual{nullptr};
    HWND contenedor{nullptr};
    RECT limites{};
    bool gpuAcelerada{false};

    bool EsValida() const noexcept {
        return vistaShell && superficieVisual && contenedor &&
               limites.right > limites.left && limites.bottom > limites.top;
    }
};

class ServicioCompatibilidadGpuV2 final {
public:
    // El mod GPU coloca este valor en la vista Shell y lo apunta a su overlay.
    // El nombre se mantiene literal para permitir cooperación entre mods sin
    // una dependencia binaria o un include cruzado.
    static constexpr wchar_t PROPIEDAD_OVERLAY_GPU[] =
        L"WindhawkExplorerGpuOverlayOwnsShellView";

    SuperficieExploradorV2 Resolver(HWND frame, HWND tab, HWND vistaShell) const noexcept {
        SuperficieExploradorV2 resultado{};
        if (!IsWindow(frame) || !IsWindow(vistaShell)) return resultado;

        resultado.vistaShell = vistaShell;
        const HWND overlay = reinterpret_cast<HWND>(
            GetPropW(vistaShell, PROPIEDAD_OVERLAY_GPU));
        if (EsOverlayGpuActivo(frame, overlay)) {
            RECT limitesOverlay{};
            if (CapturarLimites(overlay, frame, limitesOverlay)) {
                resultado.superficieVisual = overlay;
                resultado.contenedor = frame;
                resultado.limites = limitesOverlay;
                resultado.gpuAcelerada = true;
                return resultado;
            }
        }

        HWND contenedor = GetParent(vistaShell);
        if (!contenedor) contenedor = tab ? tab : frame;
        if (!IsWindow(contenedor)) return {};

        resultado.contenedor = contenedor;
        resultado.superficieVisual = vistaShell;
        if (!CapturarLimites(vistaShell, contenedor, resultado.limites)) return {};
        return resultado;
    }

    bool ActualizarLimites(SuperficieExploradorV2& superficie) const noexcept {
        if (!superficie.EsValida() || !IsWindow(superficie.superficieVisual) ||
            !IsWindow(superficie.contenedor)) return false;
        RECT limites{};
        if (!CapturarLimites(superficie.superficieVisual, superficie.contenedor, limites))
            return false;
        superficie.limites = limites;
        return true;
    }

private:
    static bool CapturarLimites(HWND ventana, HWND contenedor, RECT& destino) noexcept {
        if (!IsWindow(ventana) || !IsWindow(contenedor) ||
            !GetWindowRect(ventana, &destino)) return false;
        MapWindowPoints(HWND_DESKTOP, contenedor,
                        reinterpret_cast<POINT*>(&destino), 2);
        return destino.right > destino.left && destino.bottom > destino.top;
    }

    static bool EsOverlayGpuActivo(HWND frame, HWND overlay) noexcept {
        return IsWindow(overlay) && IsWindowVisible(overlay) &&
               GetParent(overlay) == frame &&
               GetAncestor(overlay, GA_ROOT) == frame;
    }
};

} // namespace SideViewV2
/**** ended inlining v2/nucleo/servicio_compatibilidad_gpu.hpp ****/

namespace SideViewV2 {


constexpr wchar_t CLASE_PANEL_V2[] = L"UsefulSideView_Panel_V2";

inline int EscalarDpi(int valor, UINT dpi) {
    return MulDiv(valor, static_cast<int>(dpi), 96);
}

inline UINT ObtenerDpiVentana(HWND hwnd) {
    using GetDpiForWindow_t = UINT(WINAPI*)(HWND);
    static auto pfnGetDpi = reinterpret_cast<GetDpiForWindow_t>(
        GetProcAddress(GetModuleHandleW(L"user32.dll"), "GetDpiForWindow"));
    if (pfnGetDpi && hwnd) {
        UINT dpi = pfnGetDpi(hwnd);
        if (dpi > 0) return dpi;
    }
    return 96;
}

// Estructura de estado de panel por ventana/pestaña
struct EstadoPanelV2 {
    HWND top{nullptr};           // CabinetWClass
    HWND tab{nullptr};           // ShellTabWindowClass o fallback
    HWND view{nullptr};          // Superficie visual nativa o overlay GPU
    HWND parent{nullptr};        // Contenedor de la superficie visual
    HWND panel{nullptr};         // Ventana del panel inyectado
    HWND vistaShell{nullptr};    // SHELLDLL_DefView usado por COM/selección
    SuperficieExploradorV2 superficie{};

    RECT viewOriginal{};        // Coordenadas completas de la vista antes de reflow
    RECT parentCliente{};       // Tamaño cliente del contenedor
    bool abierto{false};
    bool comparadorArchivosActivo{false};
    std::atomic_bool destruyendo{false};
    int anchoDIP{380};          // Ancho por defecto en DIP

    // Doble buffer para renderizado sin parpadeo (anti-flicker)
    HDC dcMemoria{nullptr};
    HBITMAP bmpMemoria{nullptr};
    HBITMAP bmpAnterior{nullptr};
    int anchoBuffer{0};
    int altoBuffer{0};

    // Tema visual activo
    TemaVisual tema{};

    // Proveedor de contenido actual (inyectado)
    std::shared_ptr<IPanelContenido> vistaContenido;

    std::vector<std::wstring> archivosSeleccionados;
    std::wstring carpetaContenedora;
};

// Liberación del doble buffer
inline void LiberarBufferMemoria(EstadoPanelV2* estado) {
    if (!estado) return;
    if (estado->dcMemoria) {
        if (estado->bmpAnterior) {
            SelectObject(estado->dcMemoria, estado->bmpAnterior);
            estado->bmpAnterior = nullptr;
        }
        if (estado->bmpMemoria) {
            DeleteObject(estado->bmpMemoria);
            estado->bmpMemoria = nullptr;
        }
        DeleteDC(estado->dcMemoria);
        estado->dcMemoria = nullptr;
    }
    estado->anchoBuffer = 0;
    estado->altoBuffer = 0;
}

// Asegurar buffer de memoria adecuado
inline bool AsegurarBufferMemoria(EstadoPanelV2* estado, HDC hdcReferencia, int ancho, int alto) {
    if (!estado || ancho <= 0 || alto <= 0) return false;
    if (estado->dcMemoria && estado->bmpMemoria &&
        estado->anchoBuffer == ancho && estado->altoBuffer == alto) {
        return true;
    }

    LiberarBufferMemoria(estado);

    estado->dcMemoria = CreateCompatibleDC(hdcReferencia);
    if (!estado->dcMemoria) return false;

    estado->bmpMemoria = CreateCompatibleBitmap(hdcReferencia, ancho, alto);
    if (!estado->bmpMemoria) {
        DeleteDC(estado->dcMemoria);
        estado->dcMemoria = nullptr;
        return false;
    }

    HGDIOBJ anterior = SelectObject(estado->dcMemoria, estado->bmpMemoria);
    if (!anterior || anterior == HGDI_ERROR) {
        DeleteObject(estado->bmpMemoria);
        estado->bmpMemoria = nullptr;
        DeleteDC(estado->dcMemoria);
        estado->dcMemoria = nullptr;
        return false;
    }
    estado->bmpAnterior = static_cast<HBITMAP>(anterior);
    estado->anchoBuffer = ancho;
    estado->altoBuffer = alto;
    return true;
}

// Prototipo de la función de layout
void DisponerPanel(EstadoPanelV2* estado);

} // namespace SideViewV2
/**** ended inlining v2/ui/ventana_base.hpp ****/
/**** start inlining v2/nucleo/enrutador_vistas.hpp ****/

#include <shlwapi.h>
#include <string>
#include <vector>
#include <algorithm>
#include <cwctype>

namespace SideViewV2 {

enum class TipoVistaContenido {
    Tutorial,      // Walkthrough Screen (cuando no hay selección)
    Comprimidos,   // ZIP, RAR, 7Z, TAR y otros comprimidos comunes.
    Comics,        // CBZ, CBR, CBX y formatos equivalentes.
    Texto,         // Vista de TXT, Markdown y formatos de texto
    Imagen,        // Vista de PNG, JPG, WebP, GIF, etc.
    Video,         // Vista de MP4, MKV, AVI, etc.
    Documentos,    // Vista de PDF, DOCX, XLSX, etc. (en desarrollo)
    Codigo,        // Vista de C++, Python, JS, etc.
    Carpetas,      // Vista de inspección de árbol de carpetas
    Desconocido    // Archivo genérico / visor pendiente (en desarrollo)
};

namespace {

inline void NormalizarExtension(const wchar_t* origen, wchar_t* destino, size_t capacidad) {
    if (!origen || !destino || capacidad == 0) return;
    size_t i = 0;
    while (origen[i] != L'\0' && i + 1 < capacidad) {
        destino[i] = towlower(origen[i]);
        ++i;
    }
    destino[i] = L'\0';
}

inline bool CoincideExtension(const wchar_t* extensionMin, const wchar_t* const* lista, size_t cantidad) {
    if (!extensionMin || *extensionMin == L'\0' || cantidad == 0) return false;
    return std::binary_search(lista, lista + cantidad, extensionMin,
                              [](const wchar_t* a, const wchar_t* b) {
                                  return wcscmp(a, b) < 0;
                              });
}

} // namespace

inline bool EsExtensionComprimido(const wchar_t* extension) {
    if (!extension || *extension == L'\0') return false;
    wchar_t ext[32]{};
    NormalizarExtension(extension, ext, ARRAYSIZE(ext));
    static const wchar_t* const EXT_COMPRIMIDOS[] = {
        L".7z", L".bz2", L".bzip2", L".gz", L".gzip",
        L".iso", L".rar", L".tar", L".tbz2", L".tgz", L".txz", L".xz",
        L".z", L".zip", L".zipx"
    };
    return CoincideExtension(ext, EXT_COMPRIMIDOS, ARRAYSIZE(EXT_COMPRIMIDOS));
}

inline bool EsExtensionComic(const wchar_t* extension) {
    if (!extension || *extension == L'\0') return false;
    wchar_t ext[32]{};
    NormalizarExtension(extension, ext, ARRAYSIZE(ext));
    static const wchar_t* const EXT_COMICS[] = {
        L".cb7", L".cba", L".cbr", L".cbt", L".cbx", L".cbz"
    };
    return CoincideExtension(ext, EXT_COMICS, ARRAYSIZE(EXT_COMICS));
}

inline bool EsExtensionImagen(const wchar_t* extension) {
    if (!extension || *extension == L'\0') return false;
    wchar_t ext[32]{};
    NormalizarExtension(extension, ext, ARRAYSIZE(ext));
    static const wchar_t* const EXT_IMAGENES[] = {
        L".avif", L".bmp", L".dib", L".gif", L".heic", L".heif", L".ico",
        L".jfif", L".jpe", L".jpeg", L".jpg", L".png", L".tif", L".tiff", L".webp"
    };
    return CoincideExtension(ext, EXT_IMAGENES, ARRAYSIZE(EXT_IMAGENES));
}

inline bool EsExtensionVideo(const wchar_t* extension) {
    if (!extension || *extension == L'\0') return false;
    wchar_t ext[32]{};
    NormalizarExtension(extension, ext, ARRAYSIZE(ext));
    static const wchar_t* const EXT_VIDEO[] = {
        L".3g2", L".3gp", L".avi", L".flv", L".m2ts", L".m4v", L".mkv",
        L".mov", L".mp4", L".mpeg", L".mpg", L".mts", L".ts", L".webm", L".wmv"
    };
    return CoincideExtension(ext, EXT_VIDEO, ARRAYSIZE(EXT_VIDEO));
}

inline bool EsExtensionTexto(const wchar_t* extension) {
    if (!extension || *extension == L'\0') return false;
    wchar_t ext[32]{};
    NormalizarExtension(extension, ext, ARRAYSIZE(ext));
    static const wchar_t* const EXT_TEXTO[] = {
        L".csv", L".ini", L".log", L".markdown", L".md", L".nfo", L".rst", L".tsv", L".txt"
    };
    return CoincideExtension(ext, EXT_TEXTO, ARRAYSIZE(EXT_TEXTO));
}

inline bool EsExtensionCodigo(const wchar_t* extension) {
    if (!extension || *extension == L'\0') return false;
    wchar_t ext[32]{};
    NormalizarExtension(extension, ext, ARRAYSIZE(ext));
    static const wchar_t* const EXT_CODIGO[] = {
        L".c", L".cc", L".cpp", L".cs", L".css", L".cxx", L".go", L".h",
        L".hh", L".hpp", L".htm", L".html", L".hxx", L".java", L".js",
        L".json", L".m", L".php", L".ps1", L".py", L".rs", L".sh", L".sql",
        L".swift", L".ts", L".xml", L".yaml", L".yml"
    };
    return CoincideExtension(ext, EXT_CODIGO, ARRAYSIZE(EXT_CODIGO));
}

inline bool EsExtensionDocumento(const wchar_t* extension) {
    if (!extension || *extension == L'\0') return false;
    wchar_t ext[32]{};
    NormalizarExtension(extension, ext, ARRAYSIZE(ext));
    static const wchar_t* const EXT_DOCS[] = {
        L".doc", L".docx", L".epub", L".mobi", L".odp", L".ods", L".odt",
        L".pdf", L".pps", L".ppsx", L".ppt", L".pptx", L".rtf", L".xls", L".xlsx"
    };
    return CoincideExtension(ext, EXT_DOCS, ARRAYSIZE(EXT_DOCS));
}

inline bool EsExtensionDocumentoAdmitidoParaComparacion(const wchar_t* extension) {
    if (!extension || *extension == L'\0') return false;
    wchar_t ext[32]{};
    NormalizarExtension(extension, ext, ARRAYSIZE(ext));
    static const wchar_t* const EXT_COMPARABLES[] = {
        L".doc", L".docx", L".pdf", L".pps", L".ppsx", L".ppt", L".pptx"
    };
    return CoincideExtension(ext, EXT_COMPARABLES, ARRAYSIZE(EXT_COMPARABLES));
}

inline int FamiliaDocumentoComparacionV2(const wchar_t* extension) {
    if (!extension || *extension == L'\0') return 0;
    wchar_t ext[32]{};
    NormalizarExtension(extension, ext, ARRAYSIZE(ext));
    if (wcscmp(ext, L".pdf") == 0) return 1;
    if (wcscmp(ext, L".doc") == 0 || wcscmp(ext, L".docx") == 0) return 2;
    if (wcscmp(ext, L".pps") == 0 || wcscmp(ext, L".ppsx") == 0 ||
        wcscmp(ext, L".ppt") == 0 || wcscmp(ext, L".pptx") == 0) return 3;
    return 0;
}

inline bool EsRutaCarpeta(const wchar_t* ruta) {
    if (!ruta || *ruta == L'\0') return false;
    DWORD attr = GetFileAttributesW(ruta);
    return (attr != INVALID_FILE_ATTRIBUTES && (attr & FILE_ATTRIBUTE_DIRECTORY) != 0);
}

inline TipoVistaContenido ClasificarRuta(const wchar_t* ruta) {
    if (!ruta || *ruta == L'\0') return TipoVistaContenido::Tutorial;
    if (EsRutaCarpeta(ruta)) return TipoVistaContenido::Carpetas;

    const wchar_t* ext = PathFindExtensionW(ruta);
    if (EsExtensionComic(ext)) return TipoVistaContenido::Comics;
    if (EsExtensionComprimido(ext)) return TipoVistaContenido::Comprimidos;
    if (EsExtensionImagen(ext)) return TipoVistaContenido::Imagen;
    if (EsExtensionVideo(ext)) return TipoVistaContenido::Video;
    if (EsExtensionTexto(ext)) return TipoVistaContenido::Texto;
    if (EsExtensionCodigo(ext)) return TipoVistaContenido::Codigo;
    if (EsExtensionDocumento(ext)) return TipoVistaContenido::Documentos;

    return TipoVistaContenido::Desconocido;
}

inline TipoVistaContenido ClasificarSeleccion(const std::vector<std::wstring>& seleccion,
                                              const std::wstring& carpeta = L"") {
    if (seleccion.empty()) {
        // Sin archivos seleccionados seguimos dentro de una carpeta: la vista
        // general de carpetas es la misma vista inyectada que la de una carpeta
        // seleccionada, no el walkthrough.
        return carpeta.empty() ? TipoVistaContenido::Tutorial : TipoVistaContenido::Carpetas;
    }
    if (seleccion.size() == 1) {
        return ClasificarRuta(seleccion.front().c_str());
    }

    size_t cComprimidos = 0;
    size_t cComics = 0;
    size_t cImagenes = 0;
    size_t cVideos = 0;
    size_t cTexto = 0;
    size_t cCodigo = 0;
    size_t cDocs = 0;
    size_t cCarpetas = 0;
    for (const auto& r : seleccion) {
        TipoVistaContenido t = ClasificarRuta(r.c_str());
        switch (t) {
            case TipoVistaContenido::Comprimidos: cComprimidos++; break;
            case TipoVistaContenido::Comics: cComics++; break;
            case TipoVistaContenido::Imagen: cImagenes++; break;
            case TipoVistaContenido::Video: cVideos++; break;
            case TipoVistaContenido::Texto: cTexto++; break;
            case TipoVistaContenido::Codigo: cCodigo++; break;
            case TipoVistaContenido::Documentos: cDocs++; break;
            case TipoVistaContenido::Carpetas: cCarpetas++; break;
            default: break;
        }
    }

    size_t total = seleccion.size();
    if (cComprimidos == total) return TipoVistaContenido::Comprimidos;
    if (cComics == total) return TipoVistaContenido::Comics;
    if (cImagenes == total) return TipoVistaContenido::Imagen;
    if (cVideos == total) return TipoVistaContenido::Video;
    if (cTexto == total) return TipoVistaContenido::Texto;
    if (cCarpetas == total) return TipoVistaContenido::Carpetas;
    if (cDocs == total) return TipoVistaContenido::Documentos;
    if (cCodigo == total) return TipoVistaContenido::Codigo;

    // Preferencias prioritarias en selecciones mixtas
    if (cComprimidos > 0) return TipoVistaContenido::Comprimidos;
    if (cComics > 0) return TipoVistaContenido::Comics;
    if (cImagenes > 0) return TipoVistaContenido::Imagen;
    if (cVideos > 0) return TipoVistaContenido::Video;
    if (cTexto > 0) return TipoVistaContenido::Texto;
    if (cDocs > 0) return TipoVistaContenido::Documentos;
    if (cCodigo > 0) return TipoVistaContenido::Codigo;
    if (cCarpetas > 0) return TipoVistaContenido::Carpetas;

    return TipoVistaContenido::Desconocido;
}

inline bool PuedeCompararSeleccionV2(
    const std::vector<std::wstring>& seleccion,
    TipoVistaContenido* tipoResultado = nullptr) {
    if (seleccion.size() != 2) return false;

    const TipoVistaContenido primero = ClasificarRuta(seleccion[0].c_str());
    const TipoVistaContenido segundo = ClasificarRuta(seleccion[1].c_str());
    if (tipoResultado) *tipoResultado = primero;
    if (primero != segundo ||
        primero == TipoVistaContenido::Tutorial ||
        primero == TipoVistaContenido::Carpetas ||
        primero == TipoVistaContenido::Desconocido) {
        return false;
    }

    if (primero == TipoVistaContenido::Documentos) {
        const wchar_t* extIzquierda = PathFindExtensionW(seleccion[0].c_str());
        const wchar_t* extDerecha = PathFindExtensionW(seleccion[1].c_str());
        return EsExtensionDocumentoAdmitidoParaComparacion(extIzquierda) &&
               EsExtensionDocumentoAdmitidoParaComparacion(extDerecha) &&
               FamiliaDocumentoComparacionV2(extIzquierda) ==
                   FamiliaDocumentoComparacionV2(extDerecha);
    }
    return true;
}

} // namespace SideViewV2
/**** ended inlining v2/nucleo/enrutador_vistas.hpp ****/
/**** start inlining v2/vistas/vista_walkthrough.hpp ****/

/**** skipping file: v2/ui/panel_interfaz.hpp ****/
/**** skipping file: v2/ui/ventana_base.hpp ****/
/**** start inlining v2/nucleo/servicio_enlaces.hpp ****/
#include <windows.h>
#include <shellapi.h>
#include <string>
namespace SideViewV2 {
class ServicioEnlacesV2 {
public:
    bool Abrir(const std::wstring& destino) const {
        if (destino.empty()) return false;
        return reinterpret_cast<INT_PTR>(ShellExecuteW(nullptr, L"open", destino.c_str(), nullptr, nullptr, SW_SHOWNORMAL)) > 32;
    }
};
}
/**** ended inlining v2/nucleo/servicio_enlaces.hpp ****/
#include <memory>

namespace SideViewV2 {

class VistaWalkthrough : public IPanelContenido {
public:
    explicit VistaWalkthrough(std::shared_ptr<ServicioEnlacesV2> enlaces) : m_enlaces(std::move(enlaces)) {}
    void AlCambiarSeleccion(const std::vector<std::wstring>& archivos, const std::wstring& carpeta) override {
        m_carpeta = carpeta;
    }

    bool ProcesarMensajeRaton(UINT msg, WPARAM wParam, LPARAM lParam, HWND hwndPanel) override {
        if (msg == WM_LBUTTONDOWN) {
            POINT pt{GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
            if (PtInRect(&m_rectLinkGithub, pt)) {
                m_enlaces->Abrir(m_urlGithub);
                return true;
            }
        }
        return false;
    }

    void Renderizar(HDC hdc, const RECT& rect, const TemaVisual& tema) override {
        int ancho = rect.right - rect.left;
        int alto = rect.bottom - rect.top;
        if (ancho <= 0 || alto <= 0) return;

        // 1. Fondo principal
        HBRUSH brochaFondo = CreateSolidBrush(tema.fondo);
        FillRect(hdc, &rect, brochaFondo);
        DeleteObject(brochaFondo);

        // 2. Cabecera Fluent
        int altoCabecera = 54;
        RECT rectCabecera{rect.left, rect.top, rect.right, rect.top + altoCabecera};
        HBRUSH brochaCabecera = CreateSolidBrush(tema.fondoCabecera);
        FillRect(hdc, &rectCabecera, brochaCabecera);
        DeleteObject(brochaCabecera);

        // Línea divisoria suave bajo la cabecera
        HPEN plumaBorde = CreatePen(PS_SOLID, 1, tema.borde);
        HPEN plumaVieja = static_cast<HPEN>(SelectObject(hdc, plumaBorde));
        MoveToEx(hdc, rect.left, rect.top + altoCabecera, nullptr);
        LineTo(hdc, rect.right, rect.top + altoCabecera);

        // Borde izquierdo vertical separador entre vista de Explorer y el panel
        MoveToEx(hdc, rect.left, rect.top, nullptr);
        LineTo(hdc, rect.left, rect.bottom);
        SelectObject(hdc, plumaVieja);
        DeleteObject(plumaBorde);

        SetBkMode(hdc, TRANSPARENT);

        HFONT fuenteTitulo = CreateFontW(-16, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE,
                                         DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                         CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI Variable Display");
        HFONT fuenteSubtitulo = CreateFontW(-12, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                                            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                            CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI Variable Text");
        HFONT fuenteBold = CreateFontW(-13, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                                       DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                       CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI Variable Text");

        HFONT fuenteVieja = static_cast<HFONT>(SelectObject(hdc, fuenteTitulo));

        // Título Cabecera
        SetTextColor(hdc, tema.textoPrincipal);
        RECT rectTextoTitulo{rect.left + 16, rect.top + 8, rect.right - 16, rect.top + 28};
        DrawTextW(hdc, L"Useful Side View • Guía Rápida", -1, &rectTextoTitulo,
                  DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS | DT_NOPREFIX);

        SelectObject(hdc, fuenteSubtitulo);
        SetTextColor(hdc, tema.textoSecundario);
        RECT rectTextoSub{rect.left + 16, rect.top + 28, rect.right - 16, rect.top + 46};
        DrawTextW(hdc, L"Pantalla Tutorial (Walkthrough) • Sin selección activa", -1, &rectTextoSub,
                  DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS | DT_NOPREFIX);

        int margen = 16;
        int yCursor = rect.top + altoCabecera + 14;

        // Tarjeta 1: Cómo se usa el mod
        RECT rcTarjeta1{rect.left + margen, yCursor, rect.right - margen, yCursor + 115};
        DibujarTarjeta(hdc, rcTarjeta1, tema);

        SelectObject(hdc, fuenteBold);
        SetTextColor(hdc, tema.acento);
        RECT rcT1Tit{rcTarjeta1.left + 12, rcTarjeta1.top + 8, rcTarjeta1.right - 12, rcTarjeta1.top + 26};
        DrawTextW(hdc, L"🚀 Cómo utilizar Useful Side View", -1, &rcT1Tit, DT_SINGLELINE | DT_NOPREFIX);

        SelectObject(hdc, fuenteSubtitulo);
        SetTextColor(hdc, tema.textoPrincipal);
        std::wstring descUso =
            L"• Selecciona cualquier archivo o carpeta en Explorer para ver su previsualización automática.\n"
            L"• Presiona el comando de activación para alternar el panel lateral.\n"
            L"• Si no hay archivos seleccionados, verás siempre esta pantalla de tutorial.";
        RECT rcT1Desc{rcTarjeta1.left + 12, rcTarjeta1.top + 30, rcTarjeta1.right - 12, rcTarjeta1.bottom - 8};
        DrawTextW(hdc, descUso.c_str(), -1, &rcT1Desc, DT_WORDBREAK | DT_NOPREFIX);

        yCursor = rcTarjeta1.bottom + 12;

        // Tarjeta 2: Vistas Disponibles
        RECT rcTarjeta2{rect.left + margen, yCursor, rect.right - margen, yCursor + 155};
        DibujarTarjeta(hdc, rcTarjeta2, tema);

        SelectObject(hdc, fuenteBold);
        SetTextColor(hdc, tema.acento);
        RECT rcT2Tit{rcTarjeta2.left + 12, rcTarjeta2.top + 8, rcTarjeta2.right - 12, rcTarjeta2.top + 26};
        DrawTextW(hdc, L"📂 Centro de Vistas Especializadas", -1, &rcT2Tit, DT_SINGLELINE | DT_NOPREFIX);

        SelectObject(hdc, fuenteSubtitulo);
        SetTextColor(hdc, tema.textoPrincipal);
        std::wstring descVistas =
            L"📦 Comprimidos & Cómics: Visor de portadas, navegación secuencial, descompresión y convertidor.\n"
            L"📄 Texto & Markdown: Lector GitHub con tablas, checklist y barras (En desarrollo).\n"
            L"🖼️ Imágenes & GIFs: Reproducción fluida en bucle infinito y miniaturas (En desarrollo).\n"
            L"🎬 Video Suite: Reproductor de video interactivo por Media Foundation (En desarrollo).\n"
            L"📑 Documentos & Código: Visor multipropósito de documentos (En desarrollo).";
        RECT rcT2Desc{rcTarjeta2.left + 12, rcTarjeta2.top + 30, rcTarjeta2.right - 12, rcTarjeta2.bottom - 8};
        DrawTextW(hdc, descVistas.c_str(), -1, &rcT2Desc, DT_WORDBREAK | DT_NOPREFIX);

        yCursor = rcTarjeta2.bottom + 12;

        // Tarjeta 3: Comandos y Ubicación
        RECT rcTarjeta3{rect.left + margen, yCursor, rect.right - margen, yCursor + 95};
        DibujarTarjeta(hdc, rcTarjeta3, tema);

        SelectObject(hdc, fuenteBold);
        SetTextColor(hdc, tema.acento);
        RECT rcT3Tit{rcTarjeta3.left + 12, rcTarjeta3.top + 8, rcTarjeta3.right - 12, rcTarjeta3.top + 26};
        DrawTextW(hdc, L"⌨️ Comandos y Ubicación", -1, &rcT3Tit, DT_SINGLELINE | DT_NOPREFIX);

        SelectObject(hdc, fuenteSubtitulo);
        SetTextColor(hdc, tema.textoPrincipal);
        std::wstring descComandos =
            L"• Comando de Alternancia: [ Ctrl + K ] (Configurable en Windhawk)\n"
            L"• Ubicación del Mod: Inyectado cooperativamente en explorer.exe (Sin DLLs invasivas)\n"
            L"• Carpeta activa: " + (m_carpeta.empty() ? L"(Raíz o Equipo)" : m_carpeta);
        RECT rcT3Desc{rcTarjeta3.left + 12, rcTarjeta3.top + 30, rcTarjeta3.right - 12, rcTarjeta3.bottom - 8};
        DrawTextW(hdc, descComandos.c_str(), -1, &rcT3Desc, DT_WORDBREAK | DT_NOPREFIX);

        yCursor = rcTarjeta3.bottom + 12;

        // Tarjeta 4: Enlace GitHub
        RECT rcTarjeta4{rect.left + margen, yCursor, rect.right - margen, yCursor + 50};
        DibujarTarjeta(hdc, rcTarjeta4, tema);

        SelectObject(hdc, fuenteBold);
        SetTextColor(hdc, tema.textoPrincipal);
        RECT rcT4Texto{rcTarjeta4.left + 12, rcTarjeta4.top + 14, rcTarjeta4.left + 140, rcTarjeta4.top + 34};
        DrawTextW(hdc, L"Repositorio oficial:", -1, &rcT4Texto, DT_SINGLELINE | DT_VCENTER | DT_NOPREFIX);

        // Enlace clicable en azul acento
        m_rectLinkGithub = {rcT4Texto.right + 4, rcTarjeta4.top + 14, rcTarjeta4.right - 12, rcTarjeta4.top + 34};
        SetTextColor(hdc, tema.acento);
        DrawTextW(hdc, m_urlGithub.c_str(), -1, &m_rectLinkGithub, DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS | DT_NOPREFIX);

        // Limpieza de fuentes
        SelectObject(hdc, fuenteVieja);
        DeleteObject(fuenteTitulo);
        DeleteObject(fuenteSubtitulo);
        DeleteObject(fuenteBold);
    }

private:
    void DibujarTarjeta(HDC hdc, const RECT& rc, const TemaVisual& tema) {
        HBRUSH br = CreateSolidBrush(tema.fondoTarjeta);
        FillRect(hdc, &rc, br);
        DeleteObject(br);

        HPEN pen = CreatePen(PS_SOLID, 1, tema.borde);
        HPEN oldPen = static_cast<HPEN>(SelectObject(hdc, pen));
        HBRUSH nullBr = static_cast<HBRUSH>(GetStockObject(NULL_BRUSH));
        HBRUSH oldBr = static_cast<HBRUSH>(SelectObject(hdc, nullBr));
        RoundRect(hdc, rc.left, rc.top, rc.right, rc.bottom, 8, 8);
        SelectObject(hdc, oldBr);
        SelectObject(hdc, oldPen);
        DeleteObject(pen);
    }

    std::wstring m_carpeta;
    std::wstring m_urlGithub{L"https://github.com/ramonrey/windhawk-useful-sideview"};
    std::shared_ptr<ServicioEnlacesV2> m_enlaces;
    RECT m_rectLinkGithub{};
};

} // namespace SideViewV2
/**** ended inlining v2/vistas/vista_walkthrough.hpp ****/
/**** start inlining v2/vistas/vista_comprimidos.hpp ****/

/**** skipping file: v2/ui/panel_interfaz.hpp ****/
/**** skipping file: v2/ui/ventana_base.hpp ****/
/**** skipping file: v2/nucleo/enrutador_vistas.hpp ****/
/**** start inlining v2/nucleo/servicios_vistas.hpp ****/
/**** start inlining v2/nucleo/servicio_navegacion.hpp ****/
#include <cstddef>
namespace SideViewV2 {
class ServicioNavegacionV2 {
public:
    size_t Anterior(size_t actual, size_t total) const {
        if (total == 0) return 0;
        if (actual >= total) return total - 1;
        return actual == 0 ? total - 1 : actual - 1;
    }

    size_t Siguiente(size_t actual, size_t total) const {
        if (total == 0) return 0;
        if (actual >= total - 1) return 0;
        return actual + 1;
    }
};
}
/**** ended inlining v2/nucleo/servicio_navegacion.hpp ****/
/**** start inlining v2/nucleo/servicio_abrir_con.hpp ****/

#include <windows.h>
#include <commdlg.h>
#include <shellapi.h>
#include <algorithm>
#include <cwchar>
#include <limits>
#include <memory>
#include <mutex>
#include <string>
#include <utility>
#include <vector>
/**** start inlining v2/nucleo/servicio_persistencia_windhawk.hpp ****/

#include <windhawk_api.h>
#include <algorithm>
#include <string>
#include <utility>
#include <vector>

namespace SideViewV2 {

// Adaptador único para los valores persistentes propios del mod. Los ajustes
// visibles siguen usando Wh_*Setting; este servicio sólo encapsula el estado
// operativo que debe sobrevivir a reinicios de Explorer y del equipo.
class ServicioPersistenciaWindhawkV2 final {
public:
    explicit ServicioPersistenciaWindhawkV2(
        std::wstring prefijo = L"useful-sideview.v3")
        : m_prefijo(std::move(prefijo)) {}

    int LeerEntero(const std::wstring& clave, int predeterminado) const {
        return Wh_GetIntValue(clave.c_str(), predeterminado);
    }

    bool GuardarEntero(const std::wstring& clave, int valor) const {
        return Wh_SetIntValue(clave.c_str(), valor) != FALSE;
    }

    std::wstring LeerCadena(const std::wstring& clave) const {
        constexpr std::size_t capacidadInicial = 256;
        constexpr std::size_t capacidadMaxima = 65536;
        std::size_t capacidad = capacidadInicial;
        while (capacidad <= capacidadMaxima) {
            std::vector<wchar_t> buffer(capacidad, L'\0');
            const std::size_t copiados = Wh_GetStringValue(
                clave.c_str(), buffer.data(), buffer.size());
            if (copiados > 0 && copiados < buffer.size()) {
                return std::wstring(buffer.data(), copiados);
            }
            // La API devuelve cero tanto para ausencia como para un buffer
            // insuficiente. Reintentar con crecimiento acotado cubre rutas
            // largas sin reservar memoria sin límite.
            capacidad *= 2;
        }
        return {};
    }

    bool GuardarCadena(const std::wstring& clave,
                       const std::wstring& valor) const {
        return Wh_SetStringValue(clave.c_str(), valor.c_str()) != FALSE;
    }

    bool Eliminar(const std::wstring& clave) const {
        return Wh_DeleteValue(clave.c_str()) != FALSE;
    }

    std::wstring CrearClave(const std::wstring& dominio,
                            const wchar_t* sufijo) const {
        return m_prefijo + L"." + dominio + L"." + (sufijo ? sufijo : L"");
    }

    static std::wstring CrearClaveLegacy(const std::wstring& dominio,
                                         const wchar_t* sufijo) {
        return L"abrirCon.v2." + dominio + L"." +
               (sufijo ? sufijo : L"");
    }

private:
    std::wstring m_prefijo;
};

} // namespace SideViewV2
/**** ended inlining v2/nucleo/servicio_persistencia_windhawk.hpp ****/

namespace SideViewV2 {

class ServicioAbrirConV2 {
public:
    ServicioAbrirConV2(
        std::shared_ptr<ServicioPersistenciaWindhawkV2> persistencia,
        std::wstring clavePersistencia,
        std::vector<std::wstring> inicial = {})
        : m_persistencia(std::move(persistencia)),
          m_clavePersistencia(std::move(clavePersistencia)),
          m_programas(std::move(inicial)) {
        CargarPersistencia();
    }

    std::vector<std::wstring> Programas() const {
        std::lock_guard<std::mutex> lock(m_bloqueo);
        return m_programas;
    }

    std::wstring ProgramaActual() const {
        std::lock_guard<std::mutex> lock(m_bloqueo);
        return m_ultimo.empty() && m_programas.size() == 1
            ? m_programas.front() : m_ultimo;
    }

    void Seleccionar(const std::wstring& ruta) {
        if (ruta.empty()) return;
        std::lock_guard<std::mutex> lock(m_bloqueo);
        const auto encontrado = std::find_if(
            m_programas.begin(), m_programas.end(),
            [&ruta](const std::wstring& programa) {
                return EsMismoPrograma(programa, ruta);
            });
        if (encontrado == m_programas.end()) m_programas.push_back(ruta);
        if (!m_ultimo.empty() && EsMismoPrograma(m_ultimo, ruta)) return;
        m_ultimo = ruta;
        GuardarPersistenciaBloqueada();
    }

    bool AbrirDialogo(HWND owner) {
        wchar_t ruta[MAX_PATH]{};
        OPENFILENAMEW dialogo{};
        dialogo.lStructSize = sizeof(dialogo);
        dialogo.hwndOwner = owner;
        dialogo.lpstrFilter = L"Programas ejecutables (*.exe)\0*.exe\0"
                              L"Todos los archivos (*.*)\0*.*\0";
        dialogo.lpstrFile = ruta;
        dialogo.nMaxFile = ARRAYSIZE(ruta);
        dialogo.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
        if (!GetOpenFileNameW(&dialogo)) return false;
        Seleccionar(ruta);
        return true;
    }

    bool Ejecutar(const std::vector<std::wstring>& archivos) {
        const std::wstring programa = ProgramaActual();
        if (programa.empty()) return false;

        // Registra el uso también en las vistas que no tienen un selector
        // visible propio, antes de lanzar el programa elegido.
        Seleccionar(programa);

        bool algunoAbierto = false;
        for (const auto& archivo : archivos) {
            HINSTANCE resultado = ShellExecuteW(
                nullptr, L"open", programa.c_str(),
                (L"\"" + archivo + L"\"").c_str(), nullptr, SW_SHOWNORMAL);
            if (reinterpret_cast<INT_PTR>(resultado) > 32) algunoAbierto = true;
        }
        return algunoAbierto;
    }

private:
    static constexpr int kVersionPersistencia = 1;
    static constexpr int kCantidadMaximaSegura = 100000;

    static bool EsMismoPrograma(const std::wstring& izquierdo,
                                const std::wstring& derecho) {
        return _wcsicmp(izquierdo.c_str(), derecho.c_str()) == 0;
    }

    std::wstring ClaveNueva(const wchar_t* sufijo) const {
        return m_persistencia
            ? m_persistencia->CrearClave(L"abrirCon." + m_clavePersistencia, sufijo)
            : std::wstring{};
    }

    static std::wstring ClaveLegacy(const std::wstring& dominio,
                                    const wchar_t* sufijo) {
        return ServicioPersistenciaWindhawkV2::CrearClaveLegacy(
            dominio, sufijo);
    }

    void AgregarSiNoExiste(const std::wstring& programa) {
        if (programa.empty()) return;
        const auto encontrado = std::find_if(
            m_programas.begin(), m_programas.end(),
            [&programa](const std::wstring& existente) {
                return EsMismoPrograma(existente, programa);
            });
        if (encontrado == m_programas.end()) m_programas.push_back(programa);
    }

    bool CargarFormatoNuevo() {
        if (!m_persistencia) return false;
        if (m_persistencia->LeerEntero(ClaveNueva(L"schema"), 0) <
            kVersionPersistencia) return false;

        const int cantidad = m_persistencia->LeerEntero(
            ClaveNueva(L"count"), -1);
        if (cantidad < 0 || cantidad > kCantidadMaximaSegura) return false;

        for (int i = 0; i < cantidad; ++i) {
            const std::wstring clave = ClaveNueva(
                (L"program." + std::to_wstring(i)).c_str());
            AgregarSiNoExiste(m_persistencia->LeerCadena(clave));
        }
        m_ultimo = m_persistencia->LeerCadena(ClaveNueva(L"last"));
        AgregarSiNoExiste(m_ultimo);
        return true;
    }

    bool CargarFormatoLegacy() {
        if (!m_persistencia || m_clavePersistencia.empty()) return false;
        const std::wstring textoCantidad = m_persistencia->LeerCadena(
            ClaveLegacy(m_clavePersistencia, L"count"));
        if (textoCantidad.empty()) return false;

        wchar_t* fin = nullptr;
        const unsigned long long cantidadRaw = wcstoull(
            textoCantidad.c_str(), &fin, 10);
        if (fin == textoCantidad.c_str() || *fin != L'\0' ||
            cantidadRaw > static_cast<unsigned long long>(kCantidadMaximaSegura) ||
            cantidadRaw > std::numeric_limits<size_t>::max()) {
            return false;
        }

        const size_t cantidad = static_cast<size_t>(cantidadRaw);
        for (size_t i = 0; i < cantidad; ++i) {
            const std::wstring clave = ClaveLegacy(
                m_clavePersistencia,
                (L"program." + std::to_wstring(i)).c_str());
            AgregarSiNoExiste(m_persistencia->LeerCadena(clave));
        }
        m_ultimo = m_persistencia->LeerCadena(
            ClaveLegacy(m_clavePersistencia, L"last"));
        AgregarSiNoExiste(m_ultimo);
        return true;
    }

    void CargarPersistencia() {
        if (m_clavePersistencia.empty() || !m_persistencia) return;
        if (CargarFormatoNuevo()) return;

        // Migración idempotente: conserva las claves históricas y publica el
        // formato nuevo únicamente después de cargar un estado válido.
        if (CargarFormatoLegacy()) GuardarPersistenciaBloqueada();
    }

    void GuardarPersistenciaBloqueada() {
        if (!m_persistencia || m_clavePersistencia.empty()) return;

        bool correcto = true;
        for (size_t i = 0; i < m_programas.size(); ++i) {
            const std::wstring clave = ClaveNueva(
                (L"program." + std::to_wstring(i)).c_str());
            correcto = m_persistencia->GuardarCadena(clave, m_programas[i]) &&
                       correcto;
        }
        correcto = m_persistencia->GuardarCadena(
            ClaveNueva(L"last"), m_ultimo) && correcto;
        correcto = m_persistencia->GuardarEntero(
            ClaveNueva(L"schema"), kVersionPersistencia) && correcto;

        // El contador es el marcador de commit lógico de la instantánea.
        correcto = m_persistencia->GuardarEntero(
            ClaveNueva(L"count"), static_cast<int>(m_programas.size())) &&
                   correcto;
        if (!correcto) {
            Wh_Log(L"[UsefulSideView] No fue posible guardar Abrir con (%s)",
                   m_clavePersistencia.c_str());
        }
    }

    std::shared_ptr<ServicioPersistenciaWindhawkV2> m_persistencia;
    std::wstring m_clavePersistencia;
    std::vector<std::wstring> m_programas;
    std::wstring m_ultimo;
    mutable std::mutex m_bloqueo;
};

} // namespace SideViewV2
/**** ended inlining v2/nucleo/servicio_abrir_con.hpp ****/
/**** start inlining v2/nucleo/servicio_extension.hpp ****/
#include <windows.h>
#include <shlwapi.h>
#include <algorithm>
#include <string>
#include <vector>
namespace SideViewV2 {
class ServicioExtensionV2 {
public:
    size_t Cambiar(const std::vector<std::wstring>& archivos, const std::wstring& extension) const {
        std::wstring ext = extension;
        if (ext.empty()) return 0;
        if (ext.front() != L'.') ext.insert(ext.begin(), L'.');
        if (ext.size() > 32 || ext.find_first_of(L"\\/:*?\"<>|\r\n") != std::wstring::npos) return 0;

        size_t total = 0;
        for (const auto& origen : archivos) {
            DWORD atributos = GetFileAttributesW(origen.c_str());
            if (atributos == INVALID_FILE_ATTRIBUTES ||
                (atributos & (FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_REPARSE_POINT))) {
                continue;
            }

            // PathRenameExtensionW escribe sobre el buffer recibido. Reservar
            // espacio adicional evita desbordamientos si la nueva extensión
            // es más larga que la original.
            std::vector<wchar_t> buffer(origen.size() + ext.size() + 2, L'\0');
            std::copy(origen.begin(), origen.end(), buffer.begin());
            if (!PathRenameExtensionW(buffer.data(), ext.c_str())) continue;
            std::wstring destino(buffer.data());
            if (_wcsicmp(origen.c_str(), destino.c_str()) == 0 ||
                GetFileAttributesW(destino.c_str()) != INVALID_FILE_ATTRIBUTES) {
                continue;
            }
            if (MoveFileExW(origen.c_str(), destino.c_str(), MOVEFILE_COPY_ALLOWED)) ++total;
        }
        return total;
    }
};
}
/**** ended inlining v2/nucleo/servicio_extension.hpp ****/
/**** start inlining v2/nucleo/servicios_carpetas.hpp ****/

#include <windows.h>
#include <atomic>
#include <string>
#include <vector>
#include <thread>
#include <exception>
#include <mutex>
#include <cwchar>

namespace SideViewV2 {

struct ResultadoFusionCarpetasV2 {
    size_t elementosTransferidos{0};
    size_t errores{0};
};

class ServicioFusionCarpetasV2 {
public:
    static constexpr UINT WM_FUSION_COMPLETADA = WM_APP + 0x4A1;

    ~ServicioFusionCarpetasV2() { FinalizarTrabajo(); }

    bool EjecutarEnSegundoPlano(const std::vector<std::wstring>& origenes,
                                const std::wstring& destino,
                                HWND hwndNotificacion) {
        std::lock_guard<std::mutex> lock(m_bloqueo);
        if (m_hilo.joinable()) {
            if (!m_trabajoTerminado.load(std::memory_order_acquire)) return false;
            m_hilo.join();
        }
        m_cancelar.store(false, std::memory_order_release);
        m_trabajoTerminado.store(false, std::memory_order_release);
        try {
            m_hilo = std::thread([this, origenes, destino, hwndNotificacion]() {
                ResultadoFusionCarpetasV2 resultado{};
                try {
                    resultado = Ejecutar(origenes, destino);
                } catch (...) {
                    // Ninguna excepción del trabajo de archivos debe escapar del hilo del mod.
                    resultado.errores = 1;
                }
                {
                    std::lock_guard<std::mutex> resultadoLock(m_bloqueo);
                    m_ultimoResultado = resultado;
                }
                m_trabajoTerminado.store(true, std::memory_order_release);
                if (IsWindow(hwndNotificacion)) PostMessageW(hwndNotificacion, WM_FUSION_COMPLETADA, 0, 0);
            });
            return true;
        } catch (const std::exception&) {
            // La operación queda sin iniciar si el sistema no puede crear el hilo.
            return false;
        }
    }

    ResultadoFusionCarpetasV2 UltimoResultado() const {
        std::lock_guard<std::mutex> lock(m_bloqueo);
        return m_ultimoResultado;
    }

    void FinalizarTrabajo() {
        m_cancelar.store(true, std::memory_order_release);
        std::thread trabajo;
        {
            std::lock_guard<std::mutex> lock(m_bloqueo);
            if (!m_hilo.joinable()) return;
            trabajo = std::move(m_hilo);
        }
        if (trabajo.joinable()) trabajo.join();
    }

    ResultadoFusionCarpetasV2 Ejecutar(const std::vector<std::wstring>& origenes,
                                       const std::wstring& destino) {
        ResultadoFusionCarpetasV2 resultado{};
        if (origenes.empty() || destino.empty()) { resultado.errores = 1; return resultado; }
        const std::wstring destinoCompleto = RutaCompleta(destino);
        std::vector<std::wstring> origenesCompletos;
        origenesCompletos.reserve(origenes.size());
        for (const auto& origen : origenes) origenesCompletos.push_back(RutaCompleta(origen));
        for (size_t i = 0; i < origenes.size(); ++i) {
            if (m_cancelar.load(std::memory_order_acquire)) {
                resultado.errores = origenes.size();
                return resultado;
            }
            const std::wstring& origenCompleto = origenesCompletos[i];
            if (EsMismaRutaOContenida(origenCompleto, destinoCompleto)) {
                resultado.errores = origenes.size();
                return resultado;
            }
            for (size_t j = i + 1; j < origenes.size(); ++j) {
                const std::wstring& otroOrigen = origenesCompletos[j];
                if (EsMismaRutaOContenida(origenCompleto, otroOrigen) ||
                    EsMismaRutaOContenida(otroOrigen, origenCompleto)) {
                    resultado.errores = origenes.size();
                    return resultado;
                }
            }
        }
        DWORD atributos = GetFileAttributesW(destino.c_str());
        if (atributos != INVALID_FILE_ATTRIBUTES && (!(atributos & FILE_ATTRIBUTE_DIRECTORY) || (atributos & FILE_ATTRIBUTE_REPARSE_POINT))) {
            resultado.errores = origenes.size();
            return resultado;
        }
        if (atributos == INVALID_FILE_ATTRIBUTES && !CreateDirectoryW(destino.c_str(), nullptr)) {
            resultado.errores = origenes.size();
            return resultado;
        }
        for (const auto& origen : origenes) {
            if (m_cancelar.load(std::memory_order_acquire)) {
                ++resultado.errores;
                break;
            }
            DWORD origenAtributos = GetFileAttributesW(origen.c_str());
            if (origenAtributos == INVALID_FILE_ATTRIBUTES || !(origenAtributos & FILE_ATTRIBUTE_DIRECTORY) || (origenAtributos & FILE_ATTRIBUTE_REPARSE_POINT)) {
                ++resultado.errores;
                continue;
            }
            size_t erroresAntes = resultado.errores;
            Transferir(origen, destino, resultado, 0);
            if (resultado.errores == erroresAntes && !RemoveDirectoryW(origen.c_str())) ++resultado.errores;
        }
        return resultado;
    }

private:
    static std::wstring RutaCompleta(const std::wstring& ruta) {
        DWORD capacidad = MAX_PATH;
        while (capacidad <= 32768) {
            std::vector<wchar_t> buffer(capacidad, L'\0');
            DWORD longitud = GetFullPathNameW(ruta.c_str(), capacidad, buffer.data(), nullptr);
            if (longitud == 0) return ruta;
            if (longitud < capacidad) {
                std::wstring completa(buffer.data(), longitud);
                while (completa.size() > 3 &&
                       (completa.back() == L'\\' || completa.back() == L'/')) {
                    completa.pop_back();
                }
                return completa;
            }
            capacidad *= 2;
        }
        return ruta;
    }

    static bool EsMismaRutaOContenida(const std::wstring& padre,
                                      const std::wstring& posibleDescendiente) {
        if (_wcsicmp(padre.c_str(), posibleDescendiente.c_str()) == 0) return true;
        if (posibleDescendiente.size() <= padre.size() ||
            _wcsnicmp(padre.c_str(), posibleDescendiente.c_str(), padre.size()) != 0) {
            return false;
        }
        wchar_t separador = posibleDescendiente[padre.size()];
        return separador == L'\\' || separador == L'/';
    }

    void Transferir(const std::wstring& origen, const std::wstring& destinoActual,
                    ResultadoFusionCarpetasV2& resultado, int profundidad) const {
        if (profundidad >= 64) { ++resultado.errores; return; }
        WIN32_FIND_DATAW datos{};
        HANDLE busqueda = FindFirstFileW((origen + L"\\*").c_str(), &datos);
        if (busqueda == INVALID_HANDLE_VALUE) { ++resultado.errores; return; }
        bool enumeracionCompleta = true;
        do {
            if (m_cancelar.load(std::memory_order_acquire)) {
                ++resultado.errores;
                enumeracionCompleta = false;
                break;
            }
            if (!wcscmp(datos.cFileName, L".") || !wcscmp(datos.cFileName, L"..")) continue;
            std::wstring origenItem = origen + L"\\" + datos.cFileName;
            std::wstring destinoItem = destinoActual + L"\\" + datos.cFileName;
            if (datos.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                if (datos.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) continue;
                DWORD destinoAtributos = GetFileAttributesW(destinoItem.c_str());
                if (destinoAtributos != INVALID_FILE_ATTRIBUTES && !(destinoAtributos & FILE_ATTRIBUTE_DIRECTORY)) { ++resultado.errores; continue; }
                if (destinoAtributos == INVALID_FILE_ATTRIBUTES && !CreateDirectoryW(destinoItem.c_str(), nullptr)) { ++resultado.errores; continue; }
                size_t erroresAntes = resultado.errores;
                Transferir(origenItem, destinoItem, resultado, profundidad + 1);
                if (resultado.errores == erroresAntes && RemoveDirectoryW(origenItem.c_str())) {
                    ++resultado.elementosTransferidos;
                }
            } else if (MoveFileExW(origenItem.c_str(), destinoItem.c_str(), MOVEFILE_COPY_ALLOWED)) {
                ++resultado.elementosTransferidos;
            } else {
                ++resultado.errores;
            }
        } while (FindNextFileW(busqueda, &datos));
        if (enumeracionCompleta && GetLastError() != ERROR_NO_MORE_FILES) ++resultado.errores;
        FindClose(busqueda);
    }
    std::thread m_hilo;
    std::atomic_bool m_cancelar{false};
    std::atomic_bool m_trabajoTerminado{false};
    ResultadoFusionCarpetasV2 m_ultimoResultado{};
    mutable std::mutex m_bloqueo;
};
}
/**** ended inlining v2/nucleo/servicios_carpetas.hpp ****/
/**** start inlining v2/nucleo/servicio_texto.hpp ****/
#include <windows.h>
#include <commdlg.h>
#include <string>
#include <vector>
#include <algorithm>
#include <cstring>
#include <atomic>

namespace SideViewV2 {
inline std::atomic_bool g_mostrarNumerosLineaV2{true};

struct DocumentoTextoV2 { std::wstring ruta; std::vector<std::wstring> lineas; std::vector<size_t> indice; };

class ServicioTextoV2 {
public:
    DocumentoTextoV2 Cargar(const std::wstring& ruta) const {
        DocumentoTextoV2 documento; documento.ruta = ruta;
        HANDLE archivo = CreateFileW(ruta.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, nullptr);
        if (archivo == INVALID_HANDLE_VALUE) return documento;
        LARGE_INTEGER tam{}; if (!GetFileSizeEx(archivo, &tam) || tam.QuadPart < 0 || tam.QuadPart > 16 * 1024 * 1024) { CloseHandle(archivo); return documento; }
        std::string bytes(static_cast<size_t>(tam.QuadPart), '\0'); DWORD leidos = 0; size_t totalLeido = 0;
        while (totalLeido < bytes.size()) { DWORD bloque = 0; if (!ReadFile(archivo, bytes.data() + totalLeido, static_cast<DWORD>(bytes.size() - totalLeido), &bloque, nullptr) || bloque == 0) break; totalLeido += bloque; }
        leidos = static_cast<DWORD>(totalLeido);
        CloseHandle(archivo); bytes.resize(leidos);
        int caracteres = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, bytes.data(), static_cast<int>(bytes.size()), nullptr, 0);
        UINT pagina = CP_UTF8;
        if (caracteres <= 0) { caracteres = MultiByteToWideChar(CP_ACP, 0, bytes.data(), static_cast<int>(bytes.size()), nullptr, 0); pagina = CP_ACP; }
        std::wstring texto(static_cast<size_t>(std::max(0, caracteres)), L'\0');
        if (caracteres > 0) MultiByteToWideChar(pagina, 0, bytes.data(), static_cast<int>(bytes.size()), texto.data(), caracteres);
        if (!texto.empty() && texto.front() == L'\ufeff') texto.erase(texto.begin());
        size_t inicio = 0;
        while (inicio <= texto.size()) { size_t fin = texto.find_first_of(L"\r\n", inicio); documento.lineas.push_back(texto.substr(inicio, fin == std::wstring::npos ? fin : fin - inicio)); if (fin == std::wstring::npos) break; inicio = fin + 1; if (texto[fin] == L'\r' && inicio < texto.size() && texto[inicio] == L'\n') ++inicio; }
        for (size_t i=0;i<documento.lineas.size();++i) if (documento.lineas[i].find_first_not_of(L" \t") != std::wstring::npos && (documento.lineas[i][0] == L'#' || documento.lineas[i].find(L"TODO") != std::wstring::npos)) documento.indice.push_back(i);
        return documento;
    }
    bool Copiar(const std::wstring& texto) const { if(!OpenClipboard(nullptr)) return false; if(!EmptyClipboard()){CloseClipboard();return false;} HGLOBAL h=GlobalAlloc(GMEM_MOVEABLE,(texto.size()+1)*sizeof(wchar_t)); if(!h){CloseClipboard();return false;} void* p=GlobalLock(h); if(!p){GlobalFree(h);CloseClipboard();return false;} memcpy(p,texto.c_str(),(texto.size()+1)*sizeof(wchar_t)); GlobalUnlock(h); if(!SetClipboardData(CF_UNICODETEXT,h)){GlobalFree(h);CloseClipboard();return false;} CloseClipboard(); return true; }
    bool Renombrar(HWND owner, const std::wstring& actual) const {
        if (actual.empty()) return false;
        size_t separador = actual.find_last_of(L"\\/");
        std::wstring carpeta = separador == std::wstring::npos ? L"" : actual.substr(0, separador);
        const wchar_t* nombreActual = separador == std::wstring::npos ? actual.c_str() : actual.c_str() + separador + 1;

        std::vector<wchar_t> destino(32768, L'\0');
        wcsncpy_s(destino.data(), destino.size(), nombreActual, _TRUNCATE);
        OPENFILENAMEW dialogo{};
        dialogo.lStructSize = sizeof(dialogo);
        dialogo.hwndOwner = owner;
        dialogo.lpstrFile = destino.data();
        dialogo.nMaxFile = static_cast<DWORD>(destino.size());
        dialogo.lpstrTitle = L"Renombrar archivo";
        dialogo.lpstrFilter = L"Todos los archivos (*.*)\0*.*\0";
        dialogo.Flags = OFN_PATHMUSTEXIST | OFN_NOREADONLYRETURN | OFN_NODEREFERENCELINKS;
        if (!GetSaveFileNameW(&dialogo)) return false;

        std::wstring nuevoNombre = destino.data();
        size_t ultimoSeparador = nuevoNombre.find_last_of(L"\\/");
        if (ultimoSeparador != std::wstring::npos) nuevoNombre = nuevoNombre.substr(ultimoSeparador + 1);
        if (nuevoNombre.empty() || nuevoNombre == L"." || nuevoNombre == L".." ||
            nuevoNombre.find_first_of(L"\\/:*?\"<>|\r\n") != std::wstring::npos) {
            return false;
        }

        std::wstring nueva = carpeta.empty() ? nuevoNombre : carpeta + L"\\" + nuevoNombre;
        if (_wcsicmp(actual.c_str(), nueva.c_str()) == 0 ||
            GetFileAttributesW(nueva.c_str()) != INVALID_FILE_ATTRIBUTES) return false;
        return MoveFileW(actual.c_str(), nueva.c_str()) != FALSE;
    }
};
}
/**** ended inlining v2/nucleo/servicio_texto.hpp ****/
/**** start inlining v2/nucleo/servicio_previsualizacion_media.hpp ****/

#include <windows.h>
#include <wincodec.h>
#include <shobjidl.h>
#include <string>
/**** start inlining v2/nucleo/servicio_com.hpp ****/

#include <windows.h>

namespace SideViewV2 {

class AmbitoComV2 final {
public:
    AmbitoComV2() : m_resultado(CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED)) {}

    ~AmbitoComV2() {
        if (m_resultado == S_OK) CoUninitialize();
    }

    bool Disponible() const {
        return SUCCEEDED(m_resultado) || m_resultado == RPC_E_CHANGED_MODE;
    }

private:
    HRESULT m_resultado{E_FAIL};
};

} // namespace SideViewV2
/**** ended inlining v2/nucleo/servicio_com.hpp ****/

namespace SideViewV2 {
class ServicioPrevisualizacionMediaV2 {
public:
    HBITMAP CargarImagen(const std::wstring& ruta) const {
        AmbitoComV2 com;
        if (!com.Disponible()) return nullptr;
        IWICImagingFactory* fabrica = nullptr; IWICBitmapDecoder* decoder = nullptr;
        IWICBitmapFrameDecode* frame = nullptr; IWICFormatConverter* converter = nullptr;
        HBITMAP resultado = nullptr;
        if (FAILED(CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&fabrica))) ||
            FAILED(fabrica->CreateDecoderFromFilename(ruta.c_str(), nullptr, GENERIC_READ, WICDecodeMetadataCacheOnLoad, &decoder)) ||
            FAILED(decoder->GetFrame(0, &frame)) || FAILED(fabrica->CreateFormatConverter(&converter)) ||
            FAILED(converter->Initialize(frame, GUID_WICPixelFormat24bppBGR, WICBitmapDitherTypeNone, nullptr, 0.0, WICBitmapPaletteTypeCustom))) {
            if (converter) converter->Release(); if (frame) frame->Release(); if (decoder) decoder->Release(); if (fabrica) fabrica->Release(); return nullptr;
        }
        UINT ancho=0, alto=0;
        if (FAILED(converter->GetSize(&ancho,&alto)) || ancho == 0 || alto == 0 || ancho > 8192 || alto > 8192 || static_cast<ULONGLONG>(ancho) * alto > 16ULL * 1024 * 1024) { converter->Release(); frame->Release(); decoder->Release(); fabrica->Release(); return nullptr; }
        BITMAPINFO info{}; info.bmiHeader.biSize=sizeof(BITMAPINFOHEADER); info.bmiHeader.biWidth=static_cast<LONG>(ancho); info.bmiHeader.biHeight=-static_cast<LONG>(alto); info.bmiHeader.biPlanes=1; info.bmiHeader.biBitCount=24; info.bmiHeader.biCompression=BI_RGB;
        void* bits=nullptr; HDC dc=GetDC(nullptr);
        if (!dc) { converter->Release(); frame->Release(); decoder->Release(); fabrica->Release(); return nullptr; }
        resultado=CreateDIBSection(dc,&info,DIB_RGB_COLORS,&bits,nullptr,0); ReleaseDC(nullptr,dc);
        UINT stride=((ancho*3+3)/4)*4; if(resultado && bits && FAILED(converter->CopyPixels(nullptr,stride,stride*alto,static_cast<BYTE*>(bits)))) { DeleteObject(resultado); resultado=nullptr; }
        converter->Release(); frame->Release(); decoder->Release(); fabrica->Release(); return resultado;
    }

    HBITMAP CargarMiniatura(const std::wstring& ruta, SIZE size = SIZE{900, 600}) const {
        AmbitoComV2 com;
        if (!com.Disponible()) return nullptr;
        IShellItem* item=nullptr; IShellItemImageFactory* factory=nullptr; HBITMAP bitmap=nullptr;
        if (SUCCEEDED(SHCreateItemFromParsingName(ruta.c_str(), nullptr, IID_PPV_ARGS(&item))) &&
            item && SUCCEEDED(item->QueryInterface(IID_PPV_ARGS(&factory))) && factory) {
            if (FAILED(factory->GetImage(size, SIIGBF_RESIZETOFIT | SIIGBF_THUMBNAILONLY, &bitmap)) && bitmap) {
                DeleteObject(bitmap);
                bitmap = nullptr;
            }
        }
        if(factory) factory->Release(); if(item) item->Release(); return bitmap;
    }
};
}
/**** ended inlining v2/nucleo/servicio_previsualizacion_media.hpp ****/
/**** start inlining v2/nucleo/servicio_comprimidos.hpp ****/

#include <windows.h>
#include <shlwapi.h>
#include <cstdint>
#include <string>
#include <vector>

namespace SideViewV2 {
#pragma pack(push, 1)
struct CabeceraZipLocalV2 {
    uint32_t firma; uint16_t versionMinima; uint16_t banderaGeneral; uint16_t metodoCompresion;
    uint16_t horaModificacion; uint16_t fechaModificacion; uint32_t sumaCrc32;
    uint32_t tamanoComprimido; uint32_t tamanoDescomprimido; uint16_t longitudNombreArchivo;
    uint16_t longitudCampoExtra;
};
#pragma pack(pop)

struct EntradaComprimidoV2 {
    std::wstring nombre;
    bool esDirectorio{false};
    uintmax_t tamanoBytes{0};
};

class ServicioContenidoComprimidoV2 {
public:
    std::vector<EntradaComprimidoV2> Cargar(const std::wstring& ruta) const {
        std::vector<EntradaComprimidoV2> entradas;
        const wchar_t* ext = PathFindExtensionW(ruta.c_str());
        if (_wcsicmp(ext, L".zip") != 0 && _wcsicmp(ext, L".jar") != 0 && _wcsicmp(ext, L".apk") != 0) return entradas;
        HANDLE archivo = CreateFileW(ruta.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (archivo == INVALID_HANDLE_VALUE) return entradas;
        CabeceraZipLocalV2 cabecera{};
        while (entradas.size() < 2000 && Leer(archivo,&cabecera,sizeof(cabecera))) {
            if (cabecera.firma != 0x04034b50 || cabecera.longitudNombreArchivo == 0 || cabecera.longitudNombreArchivo >= 4096) break;
            std::vector<char> nombre(cabecera.longitudNombreArchivo + 1, 0);
            if (!Leer(archivo,nombre.data(),cabecera.longitudNombreArchivo)) break;
            if (cabecera.longitudCampoExtra > 0 && !Saltar(archivo,cabecera.longitudCampoExtra)) break;
            for (char& caracter : nombre) if (caracter == '/') caracter = '\\';
            int longitud = MultiByteToWideChar(CP_UTF8, 0, nombre.data(), -1, nullptr, 0);
            if (longitud <= 0) longitud = MultiByteToWideChar(CP_ACP, 0, nombre.data(), -1, nullptr, 0);
            if (longitud <= 0) break;
            std::vector<wchar_t> nombreW(static_cast<size_t>(longitud));
            UINT pagina = MultiByteToWideChar(CP_UTF8, 0, nombre.data(), -1, nombreW.data(), longitud) > 0 ? CP_UTF8 : CP_ACP;
            if (pagina == CP_ACP) MultiByteToWideChar(CP_ACP, 0, nombre.data(), -1, nombreW.data(), longitud);
            std::wstring relativa=nombreW.data(); bool directorio=!relativa.empty() && relativa.back()==L'\\';
            const wchar_t* solo=PathFindFileNameW(relativa.c_str());
            entradas.push_back({(solo&&*solo)?solo:relativa,directorio,cabecera.tamanoDescomprimido});
            if (cabecera.banderaGeneral & 0x0008) break; // descriptor posterior: no es seguro saltar sin leerlo
            if (!Saltar(archivo,cabecera.tamanoComprimido)) break;
        }
        CloseHandle(archivo); return entradas;
    }
private:
    static bool Leer(HANDLE archivo, void* destino, DWORD bytes) {
        BYTE* salida=static_cast<BYTE*>(destino); DWORD total=0;
        while (total < bytes) { DWORD leidos=0; if (!ReadFile(archivo,salida+total,bytes-total,&leidos,nullptr) || leidos==0) return false; total+=leidos; }
        return true;
    }

    static bool Saltar(HANDLE archivo, DWORD bytes) {
        LARGE_INTEGER desplazamiento{}; desplazamiento.QuadPart=bytes;
        return SetFilePointerEx(archivo, desplazamiento, nullptr, FILE_CURRENT) != FALSE;
    }
};
}
/**** ended inlining v2/nucleo/servicio_comprimidos.hpp ****/
/**** skipping file: v2/nucleo/servicio_enlaces.hpp ****/
/**** start inlining v2/nucleo/servicio_inspeccion_carpetas.hpp ****/

#include <windows.h>
#include <algorithm>
#include <cstdint>
#include <limits>
#include <string>
#include <utility>
#include <vector>

namespace SideViewV2 {

struct ItemArbolCarpetaV2 {
    std::wstring nombre;
    std::wstring rutaCompleta;
    bool esDirectorio{false};
    uintmax_t tamanoBytes{0};
    int nivelJerarquia{0};
};

struct CarpetaPesadaV2 {
    std::wstring nombre;
    uintmax_t tamanoBytes{0};
    double porcentaje{0.0};
};

struct ResultadoInspeccionCarpetaV2 {
    std::vector<ItemArbolCarpetaV2> elementos;
    std::vector<CarpetaPesadaV2> pesadas;
    std::vector<std::wstring> vacias;
    uintmax_t tamanoTotalBytes{0};
    size_t totalSubcarpetas{0};
    size_t totalArchivos{0};
};

class ServicioInspeccionCarpetasV2 {
public:
    ResultadoInspeccionCarpetaV2 Ejecutar(const std::wstring& ruta, size_t limitePesadas) const {
        ResultadoInspeccionCarpetaV2 resultado;
        if (ruta.empty()) return resultado;
        Inspeccionar(ruta, 0, 4, resultado);
        CalcularPesadas(ruta, limitePesadas, resultado);
        return resultado;
    }

private:
    static constexpr size_t MAX_ELEMENTOS_ARBOL = 500;
    static constexpr size_t MAX_CARPETAS_VACIAS = 5000;
    static constexpr size_t MAX_ELEMENTOS_TAMANO = 100000;
    static constexpr int MAX_PROFUNDIDAD_TAMANO = 64;

    static void Inspeccionar(const std::wstring& ruta, int nivel, int maxNivel,
                             ResultadoInspeccionCarpetaV2& resultado) {
        if (nivel >= maxNivel || resultado.elementos.size() >= MAX_ELEMENTOS_ARBOL) return;
        WIN32_FIND_DATAW datos{};
        HANDLE busqueda = FindFirstFileW((ruta + L"\\*").c_str(), &datos);
        if (busqueda == INVALID_HANDLE_VALUE) return;
        std::vector<ItemArbolCarpetaV2> subcarpetas;
        std::vector<ItemArbolCarpetaV2> archivos;
        size_t elementosEnCarpeta = 0;
        do {
            if (!wcscmp(datos.cFileName, L".") || !wcscmp(datos.cFileName, L"..")) continue;
            if (datos.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) continue;
            ++elementosEnCarpeta;
            ItemArbolCarpetaV2 item;
            item.nombre = datos.cFileName;
            item.rutaCompleta = ruta + L"\\" + datos.cFileName;
            item.esDirectorio = (datos.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
            item.nivelJerarquia = nivel;
            if (item.esDirectorio) {
                ++resultado.totalSubcarpetas;
                ConservarPrimeros(subcarpetas, std::move(item));
            } else {
                ++resultado.totalArchivos;
                item.tamanoBytes = TamanoDeDatos(datos);
                ConservarPrimeros(archivos, std::move(item));
            }
        } while (FindNextFileW(busqueda, &datos));
        FindClose(busqueda);
        if (elementosEnCarpeta == 0 && nivel > 0 &&
            resultado.vacias.size() < MAX_CARPETAS_VACIAS) {
            resultado.vacias.push_back(ruta);
        }
        Ordenar(subcarpetas); Ordenar(archivos);
        for (const auto& carpeta : subcarpetas) {
            if (resultado.elementos.size() >= MAX_ELEMENTOS_ARBOL) break;
            resultado.elementos.push_back(carpeta);
            if (nivel + 1 < maxNivel) Inspeccionar(carpeta.rutaCompleta, nivel + 1, maxNivel, resultado);
        }
        for (const auto& archivo : archivos) {
            if (resultado.elementos.size() >= MAX_ELEMENTOS_ARBOL) break;
            resultado.elementos.push_back(archivo);
        }
    }

    static void Ordenar(std::vector<ItemArbolCarpetaV2>& elementos) {
        std::sort(elementos.begin(), elementos.end(), [](const auto& a, const auto& b) {
            return StrCmpLogicalW(a.nombre.c_str(), b.nombre.c_str()) < 0;
        });
    }

    static uintmax_t TamanoDeDatos(const WIN32_FIND_DATAW& datos) {
        return (static_cast<uintmax_t>(datos.nFileSizeHigh) << 32) | datos.nFileSizeLow;
    }

    static uintmax_t SumarSaturado(uintmax_t izquierdo, uintmax_t derecho) {
        const uintmax_t maximo = std::numeric_limits<uintmax_t>::max();
        return derecho > maximo - izquierdo ? maximo : izquierdo + derecho;
    }

    static void ConservarPrimeros(std::vector<ItemArbolCarpetaV2>& elementos,
                                  ItemArbolCarpetaV2 elemento) {
        if (elementos.size() < MAX_ELEMENTOS_ARBOL) {
            elementos.push_back(std::move(elemento));
            return;
        }

        auto ultimo = std::max_element(elementos.begin(), elementos.end(),
            [](const auto& izquierdo, const auto& derecho) {
                return StrCmpLogicalW(izquierdo.nombre.c_str(), derecho.nombre.c_str()) < 0;
            });
        if (StrCmpLogicalW(elemento.nombre.c_str(), ultimo->nombre.c_str()) < 0) {
            *ultimo = std::move(elemento);
        }
    }

    static uintmax_t CalcularTamano(const std::wstring& ruta, int profundidad, size_t& visitados) {
        if (profundidad >= MAX_PROFUNDIDAD_TAMANO || visitados >= MAX_ELEMENTOS_TAMANO) return 0;
        WIN32_FIND_DATAW datos{};
        HANDLE busqueda = FindFirstFileW((ruta + L"\\*").c_str(), &datos);
        if (busqueda == INVALID_HANDLE_VALUE) return 0;
        uintmax_t total = 0;
        do {
            if (!wcscmp(datos.cFileName, L".") || !wcscmp(datos.cFileName, L"..")) continue;
            if (++visitados >= MAX_ELEMENTOS_TAMANO) break;
            if (datos.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) continue;
            std::wstring elemento = ruta + L"\\" + datos.cFileName;
            if (datos.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                total += CalcularTamano(elemento, profundidad + 1, visitados);
            } else {
                total = SumarSaturado(total, TamanoDeDatos(datos));
            }
        } while (FindNextFileW(busqueda, &datos));
        FindClose(busqueda);
        return total;
    }

    static void CalcularPesadas(const std::wstring& ruta, size_t limite,
                                ResultadoInspeccionCarpetaV2& resultado) {
        WIN32_FIND_DATAW datos{};
        HANDLE busqueda = FindFirstFileW((ruta + L"\\*").c_str(), &datos);
        if (busqueda == INVALID_HANDLE_VALUE) return;
        uintmax_t totalCompleto = 0;
        do {
            if (!wcscmp(datos.cFileName, L".") || !wcscmp(datos.cFileName, L"..")) continue;
            if (datos.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) continue;
            if (!(datos.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                totalCompleto = SumarSaturado(totalCompleto, TamanoDeDatos(datos));
                continue;
            }
            size_t visitados = 0;
            uintmax_t tamano = CalcularTamano(ruta + L"\\" + datos.cFileName, 0, visitados);
            totalCompleto = SumarSaturado(totalCompleto, tamano);
            if (tamano > 0 && limite > 0) {
                resultado.pesadas.push_back({datos.cFileName, tamano, 0.0});
            }
        } while (FindNextFileW(busqueda, &datos));
        FindClose(busqueda);
        resultado.tamanoTotalBytes = totalCompleto;
        std::sort(resultado.pesadas.begin(), resultado.pesadas.end(), [](const auto& a, const auto& b) { return a.tamanoBytes > b.tamanoBytes; });
        if (resultado.pesadas.size() > limite) resultado.pesadas.resize(limite);
        for (auto& carpeta : resultado.pesadas) {
            carpeta.porcentaje = totalCompleto > 0
                ? (static_cast<double>(carpeta.tamanoBytes) / static_cast<double>(totalCompleto)) * 100.0
                : 0.0;
        }
    }
};
}
/**** ended inlining v2/nucleo/servicio_inspeccion_carpetas.hpp ****/
/**** start inlining v2/nucleo/servicio_comparador_videos.hpp ****/

/**** start inlining v2/nucleo/servicio_comparador_archivos.hpp ****/

#include <windows.h>
#include <bcrypt.h>
#include <shlwapi.h>
#include <algorithm>
#include <cstdint>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

namespace SideViewV2 {

struct DatosArchivoComparacionV2 {
    std::wstring ruta;
    std::wstring nombre;
    std::wstring extension;
    std::wstring fps{L"Pendiente"};
    std::uintmax_t bytes{0};
    std::uintmax_t lineas{0};
    std::uintmax_t paginas{0};
    std::wstring sha256{L"No disponible"};
};

struct EstadoComparadorArchivosV2 {
    DatosArchivoComparacionV2 izquierda;
    DatosArchivoComparacionV2 derecha;
    std::wstring mensaje{L"Comparación general de archivos."};
};

class ServicioComparadorArchivosV2 {
public:
    virtual ~ServicioComparadorArchivosV2() = default;

    virtual EstadoComparadorArchivosV2 CrearComparacion(
        const std::vector<std::wstring>& seleccion) const {
        EstadoComparadorArchivosV2 estado;
        if (seleccion.size() != 2) {
            estado.mensaje = L"Se requieren exactamente dos archivos del mismo tipo.";
            return estado;
        }
        estado.izquierda = LeerDatos(seleccion[0]);
        estado.derecha = LeerDatos(seleccion[1]);
        estado.mensaje = L"Comparación general lista.";
        return estado;
    }

    std::wstring FormatearTamano(std::uintmax_t bytes) const {
        static constexpr const wchar_t* unidades[] = {L"B", L"KB", L"MB", L"GB", L"TB"};
        double valor = static_cast<double>(bytes);
        std::size_t unidad = 0;
        constexpr std::size_t cantidad = sizeof(unidades) / sizeof(unidades[0]);
        while (valor >= 1024.0 && unidad + 1 < cantidad) {
            valor /= 1024.0;
            ++unidad;
        }
        std::wostringstream salida;
        salida << std::fixed << std::setprecision(
            unidad == 0 ? 0 : valor < 10.0 ? 2 : 1)
               << valor << L" " << unidades[unidad];
        return salida.str();
    }

    std::wstring FormatearDiferencia(std::uintmax_t izquierda,
                                     std::uintmax_t derecha) const {
        const bool negativa = derecha < izquierda;
        const std::uintmax_t magnitud = negativa ? izquierda - derecha : derecha - izquierda;
        return std::wstring(negativa ? L"-" : L"+") + FormatearTamano(magnitud);
    }

protected:
    static DatosArchivoComparacionV2 LeerDatos(const std::wstring& ruta) {
        DatosArchivoComparacionV2 datos;
        datos.ruta = ruta;
        const wchar_t* nombre = PathFindFileNameW(ruta.c_str());
        datos.nombre = (nombre && *nombre) ? nombre : L"Archivo";
        const wchar_t* extension = PathFindExtensionW(ruta.c_str());
        datos.extension = extension ? extension : L"";

        WIN32_FILE_ATTRIBUTE_DATA atributos{};
        if (GetFileAttributesExW(ruta.c_str(), GetFileExInfoStandard, &atributos)) {
            ULARGE_INTEGER tamano{};
            tamano.HighPart = atributos.nFileSizeHigh;
            tamano.LowPart = atributos.nFileSizeLow;
            datos.bytes = static_cast<std::uintmax_t>(tamano.QuadPart);
        }
        datos.sha256 = CalcularSha256(ruta);
        return datos;
    }

private:
    static std::wstring CalcularSha256(const std::wstring& ruta) {
        HANDLE archivo = CreateFileW(ruta.c_str(), GENERIC_READ,
                                     FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                                     nullptr, OPEN_EXISTING,
                                     FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, nullptr);
        if (archivo == INVALID_HANDLE_VALUE) return L"No disponible";

        BCRYPT_ALG_HANDLE algoritmo = nullptr;
        BCRYPT_HASH_HANDLE hash = nullptr;
        PUCHAR objeto = nullptr;
        std::wstring resultado = L"No disponible";
        do {
            if (BCryptOpenAlgorithmProvider(&algoritmo, BCRYPT_SHA256_ALGORITHM,
                                            nullptr, 0) != 0) break;
            DWORD tamanoObjeto = 0;
            DWORD bytesResultado = 0;
            if (BCryptGetProperty(algoritmo, BCRYPT_OBJECT_LENGTH,
                                  reinterpret_cast<PUCHAR>(&tamanoObjeto),
                                  sizeof(tamanoObjeto), &bytesResultado, 0) != 0 ||
                tamanoObjeto == 0) break;
            objeto = static_cast<PUCHAR>(HeapAlloc(GetProcessHeap(), 0, tamanoObjeto));
            if (!objeto || BCryptCreateHash(algoritmo, &hash, objeto, tamanoObjeto,
                                            nullptr, 0, 0) != 0) break;

            std::vector<BYTE> buffer(1024 * 1024);
            bool lecturaValida = true;
            for (;;) {
                DWORD leidos = 0;
                if (!ReadFile(archivo, buffer.data(), static_cast<DWORD>(buffer.size()),
                              &leidos, nullptr)) {
                    lecturaValida = false;
                    break;
                }
                if (leidos == 0) break;
                if (BCryptHashData(hash, buffer.data(), leidos, 0) != 0) {
                    lecturaValida = false;
                    break;
                }
            }
            if (!lecturaValida) break;

            std::vector<BYTE> digest(32);
            if (BCryptFinishHash(hash, digest.data(), static_cast<ULONG>(digest.size()), 0) != 0) break;
            std::wostringstream hex;
            hex << std::hex << std::setfill(L'0');
            for (BYTE byte : digest) hex << std::setw(2) << static_cast<unsigned>(byte);
            resultado = hex.str();
        } while (false);

        if (hash) BCryptDestroyHash(hash);
        if (objeto) HeapFree(GetProcessHeap(), 0, objeto);
        if (algoritmo) BCryptCloseAlgorithmProvider(algoritmo, 0);
        CloseHandle(archivo);
        return resultado;
    }
};

} // namespace SideViewV2
/**** ended inlining v2/nucleo/servicio_comparador_archivos.hpp ****/
/**** start inlining v2/nucleo/servicio_reproductor_video.hpp ****/

/**** skipping file: v2/ui/panel_interfaz.hpp ****/
#include <windows.h>
#include <windowsx.h>
#include <mfapi.h>
#include <mfmediaengine.h>
#include <shlwapi.h>
#include <algorithm>
#include <atomic>
#include <cmath>
#include <cwchar>
#include <memory>
#include <mutex>
#include <new>
#include <oleauto.h>
#include <string>

namespace SideViewV2 {

class IReproductorVideoV2 {
public:
    virtual ~IReproductorVideoV2() = default;
    virtual bool Montar(HWND panel, int ranura, const std::wstring& ruta) = 0;
    virtual void ActualizarRect(const RECT& rect, bool esPip) = 0;
    virtual bool Reproducir() = 0;
    virtual void Pausar() = 0;
    virtual void Buscar(double segundos) = 0;
    virtual bool EstaListo() const = 0;
    virtual double Posicion() const = 0;
    virtual double Duracion() const = 0;
    virtual void Liberar() = 0;
};

class MediaEngineNotifyV2 final : public IMFMediaEngineNotify {
public:
    MediaEngineNotifyV2(HWND destino, DWORD generacion)
        : m_destino(destino), m_generacion(generacion) {}

    void Desconectar() { m_destino.store(nullptr, std::memory_order_release); }

    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, void** salida) override {
        if (!salida) return E_POINTER;
        *salida = nullptr;
        if (IsEqualIID(iid, IID_IUnknown) ||
            IsEqualIID(iid, IID_IMFMediaEngineNotify)) {
            *salida = static_cast<IMFMediaEngineNotify*>(this);
            AddRef();
            return S_OK;
        }
        return E_NOINTERFACE;
    }

    ULONG STDMETHODCALLTYPE AddRef() override {
        return static_cast<ULONG>(InterlockedIncrement(&m_referencias));
    }

    ULONG STDMETHODCALLTYPE Release() override {
        ULONG restantes = static_cast<ULONG>(InterlockedDecrement(&m_referencias));
        if (restantes == 0) delete this;
        return restantes;
    }

    HRESULT STDMETHODCALLTYPE EventNotify(DWORD evento, DWORD_PTR param1,
                                           DWORD param2) override {
        if (evento == MF_MEDIA_ENGINE_EVENT_NOTIFYSTABLESTATE) {
            if (param1) SetEvent(reinterpret_cast<HANDLE>(param1));
            return S_OK;
        }
        HWND destino = m_destino.load(std::memory_order_acquire);
        if (destino) {
            // El callback de Media Foundation puede entregar eventos después
            // de que el archivo anterior se haya desmontado. Empaquetar el
            // código y HRESULT, y transportar la generación, evita que un
            // evento tardío cambie el estado del reproductor nuevo.
            const ULONGLONG paquete =
                (static_cast<ULONGLONG>(param2) << 32) | evento;
            PostMessageW(destino, WM_SIDEVIEW_VIDEO_EVENT_V2,
                         static_cast<WPARAM>(m_generacion),
                         static_cast<LPARAM>(paquete));
        }
        return S_OK;
    }

private:
    ~MediaEngineNotifyV2() = default;
    LONG m_referencias{1};
    std::atomic<HWND> m_destino{nullptr};
    DWORD m_generacion{0};
};

class ReproductorMediaFoundationV2 final : public IReproductorVideoV2 {
public:
    ~ReproductorMediaFoundationV2() override { Liberar(); }

    bool Montar(HWND panel, int ranura, const std::wstring& ruta) override {
        Liberar();
        ++m_generacion;
        m_panel = panel;
        m_ranura = ranura;
        m_mensaje.clear();

        RegistrarClase();
        m_ventana = CreateWindowExW(
            0, ClaseVideo(), L"", WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
            0, 0, 1, 1, panel, nullptr, GetModuleHandleW(nullptr), this);
        if (!m_ventana) {
            m_mensaje = L"No se pudo crear el espacio de reproducción.";
            return false;
        }

        if (ruta.empty()) {
            m_mensaje = L"Selecciona dos videos para iniciar la comparación.";
            Liberar();
            return false;
        }

        DWORD atributos = GetFileAttributesW(ruta.c_str());
        if (atributos == INVALID_FILE_ATTRIBUTES ||
            (atributos & FILE_ATTRIBUTE_DIRECTORY) != 0) {
            m_mensaje = L"No se pudo abrir el archivo de video.";
            Liberar();
            return false;
        }

        HRESULT hr = MFStartup(MF_VERSION);
        if (FAILED(hr)) {
            m_mensaje = L"Media Foundation no está disponible en este sistema.";
            Liberar();
            return false;
        }
        m_mfIniciado = true;

        MediaEngineNotifyV2* callback =
            new (std::nothrow) MediaEngineNotifyV2(m_ventana, m_generacion);
        if (!callback) {
            m_mensaje = L"No hubo memoria suficiente para iniciar el reproductor.";
            Liberar();
            return false;
        }
        m_callback = callback;

        IMFAttributes* atributosMedia = nullptr;
        IMFMediaEngineClassFactory* fabrica = nullptr;
        hr = MFCreateAttributes(&atributosMedia, 3);
        if (SUCCEEDED(hr)) hr = atributosMedia->SetUnknown(MF_MEDIA_ENGINE_CALLBACK, callback);
        if (SUCCEEDED(hr)) hr = atributosMedia->SetUINT64(
            MF_MEDIA_ENGINE_PLAYBACK_HWND, reinterpret_cast<UINT64>(m_ventana));
        if (SUCCEEDED(hr)) hr = CoCreateInstance(
            CLSID_MFMediaEngineClassFactory, nullptr, CLSCTX_INPROC_SERVER,
            IID_IMFMediaEngineClassFactory, reinterpret_cast<void**>(&fabrica));
        if (SUCCEEDED(hr)) hr = fabrica->CreateInstance(
            0, atributosMedia, &m_engine);
        if (SUCCEEDED(hr)) hr = m_engine->QueryInterface(
            IID_IMFMediaEngineEx, reinterpret_cast<void**>(&m_engineEx));
        if (SUCCEEDED(hr)) hr = m_engine->SetAutoPlay(FALSE);
        if (SUCCEEDED(hr)) hr = m_engine->SetPreload(MF_MEDIA_ENGINE_PRELOAD_AUTOMATIC);

        if (SUCCEEDED(hr)) {
            wchar_t url[2048]{};
            DWORD longitud = ARRAYSIZE(url);
            hr = UrlCreateFromPathW(ruta.c_str(), url, &longitud, 0);
            if (hr == E_POINTER && longitud > ARRAYSIZE(url)) {
                std::wstring urlAmplia(longitud, L'\0');
                hr = UrlCreateFromPathW(ruta.c_str(), urlAmplia.data(), &longitud, 0);
                if (SUCCEEDED(hr)) {
                    BSTR origen = SysAllocString(urlAmplia.c_str());
                    hr = origen ? m_engine->SetSource(origen) : E_OUTOFMEMORY;
                    SysFreeString(origen);
                }
            } else if (SUCCEEDED(hr)) {
                BSTR origen = SysAllocString(url);
                hr = origen ? m_engine->SetSource(origen) : E_OUTOFMEMORY;
                SysFreeString(origen);
            }
        }

        if (atributosMedia) atributosMedia->Release();
        if (fabrica) fabrica->Release();
        if (FAILED(hr)) {
            m_mensaje = L"No se pudo reproducir este formato de video.";
            Liberar();
            return false;
        }

        m_mensaje = L"Cargando video...";
        return true;
    }

    void ActualizarRect(const RECT& rect, bool esPip) override {
        m_esPip = esPip;
        if (!IsWindow(m_ventana)) return;

        RECT render = rect;
        render.left += 1;
        render.right = std::max(render.left + 1, render.right - 1);
        render.top += 30; // Reserva la franja superior para título y controles.
        render.bottom = std::max(render.top + 1, render.bottom - 1);
        if (!m_rectValido || !EqualRect(&m_rectActual, &render) ||
            !IsWindowVisible(m_ventana)) {
            SetWindowPos(m_ventana, HWND_TOP, render.left, render.top,
                         render.right - render.left, render.bottom - render.top,
                         SWP_NOACTIVATE | SWP_SHOWWINDOW);
            m_rectActual = render;
            m_rectValido = true;
        }
        if (m_engineEx) ActualizarFrame();
    }

    bool Reproducir() override {
        m_reproduccionSolicitada = true;
        if (!m_engine || !m_listo) return false;
        HRESULT hr = m_engine->Play();
        if (SUCCEEDED(hr)) {
            m_reproduciendo = true;
            SetTimer(m_ventana, kTimerReproductor, 33, nullptr);
        }
        return SUCCEEDED(hr);
    }

    void Pausar() override {
        m_reproduccionSolicitada = false;
        m_reproduciendo = false;
        if (IsWindow(m_ventana)) KillTimer(m_ventana, kTimerReproductor);
        if (m_engine) m_engine->Pause();
    }

    void Buscar(double segundos) override {
        m_posicionPendiente = std::max(0.0, segundos);
        if (m_engine && m_listo) {
            m_engine->SetCurrentTime(m_posicionPendiente);
        }
    }

    bool EstaListo() const override { return m_listo; }
    double Posicion() const override {
        if (m_engine && m_listo) {
            double actual = m_engine->GetCurrentTime();
            if (std::isfinite(actual) && actual >= 0.0) return actual;
        }
        return m_posicionPendiente;
    }

    double Duracion() const override {
        if (m_engine && m_listo) {
            double duracion = m_engine->GetDuration();
            if (std::isfinite(duracion) && duracion > 0.0) return duracion;
        }
        return 0.0;
    }

    void Liberar() override {
        LiberarBackend();
        if (IsWindow(m_ventana)) DestroyWindow(m_ventana);
        m_ventana = nullptr;
        m_panel = nullptr;
        m_listo = false;
        m_reproduciendo = false;
        m_reproduccionSolicitada = false;
        m_posicionPendiente = 0.0;
        m_rectActual = {};
        m_rectValido = false;
    }

private:
    static constexpr UINT_PTR kTimerReproductor = 17;

    static const wchar_t* ClaseVideo() {
        return L"UsefulSideView_MediaFoundationVideo_V2";
    }

    static void RegistrarClase() {
        static std::once_flag unaVez;
        std::call_once(unaVez, [] {
            WNDCLASSW clase{};
            clase.lpfnWndProc = ProcedimientoVideo;
            clase.hInstance = GetModuleHandleW(nullptr);
            clase.hCursor = LoadCursorW(nullptr, IDC_ARROW);
            clase.hbrBackground = static_cast<HBRUSH>(GetStockObject(BLACK_BRUSH));
            clase.lpszClassName = ClaseVideo();
            RegisterClassW(&clase);
        });
    }

    static LRESULT CALLBACK ProcedimientoVideo(HWND hwnd, UINT mensaje,
                                               WPARAM wParam, LPARAM lParam) {
        auto* reproductor = reinterpret_cast<ReproductorMediaFoundationV2*>(
            GetWindowLongPtrW(hwnd, GWLP_USERDATA));
        if (mensaje == WM_NCCREATE) {
            auto* crear = reinterpret_cast<CREATESTRUCTW*>(lParam);
            reproductor = static_cast<ReproductorMediaFoundationV2*>(crear->lpCreateParams);
            SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(reproductor));
        }
        if (!reproductor) return DefWindowProcW(hwnd, mensaje, wParam, lParam);
        if (mensaje == WM_NCDESTROY) {
            SetWindowLongPtrW(hwnd, GWLP_USERDATA, 0);
            return DefWindowProcW(hwnd, mensaje, wParam, lParam);
        }
        switch (mensaje) {
            case WM_ERASEBKGND:
                return 1;
            case WM_SHOWWINDOW:
                if (!wParam) reproductor->Pausar();
                return 0;
            case WM_DESTROY:
                KillTimer(hwnd, kTimerReproductor);
                return 0;
            case WM_SIZE:
                reproductor->ActualizarFrame();
                return 0;
            case WM_TIMER:
                if (wParam == kTimerReproductor && reproductor->m_reproduciendo &&
                    reproductor->m_panel) {
                    PostMessageW(reproductor->m_panel, WM_SIDEVIEW_COMPARADOR_TICK,
                                 static_cast<WPARAM>(reproductor->m_ranura), 0);
                    return 0;
                }
                break;
            case WM_LBUTTONDOWN:
                if (reproductor->m_esPip && reproductor->m_panel) {
                    PostMessageW(reproductor->m_panel, WM_SIDEVIEW_COMPARADOR_PIP_CLICK,
                                 static_cast<WPARAM>(reproductor->m_ranura), 0);
                    return 0;
                }
                break;
            case WM_PAINT: {
                PAINTSTRUCT ps{};
                HDC hdc = BeginPaint(hwnd, &ps);
                if (reproductor->m_listo && reproductor->m_engineEx) {
                    // Media Foundation pinta el frame en la superficie
                    // asociada al HWND. No rellenar de negro antes de cada
                    // frame evita el destello que aparecía entre eventos.
                    EndPaint(hwnd, &ps);
                    reproductor->ActualizarFrame();
                    return 0;
                } else {
                    RECT rect{};
                    GetClientRect(hwnd, &rect);
                    FillRect(hdc, &rect, static_cast<HBRUSH>(GetStockObject(BLACK_BRUSH)));
                    SetBkMode(hdc, TRANSPARENT);
                    SetTextColor(hdc, RGB(220, 220, 220));
                    DrawTextW(hdc, reproductor->m_mensaje.c_str(), -1, &rect,
                              DT_CENTER | DT_VCENTER | DT_SINGLELINE |
                              DT_END_ELLIPSIS | DT_NOPREFIX);
                }
                EndPaint(hwnd, &ps);
                return 0;
            }
            case WM_SIDEVIEW_VIDEO_EVENT_V2:
                if (static_cast<DWORD>(wParam) != reproductor->m_generacion) {
                    return 0;
                }
                {
                    const ULONGLONG paquete = static_cast<ULONGLONG>(lParam);
                    const DWORD evento = static_cast<DWORD>(paquete & 0xffffffffULL);
                    const HRESULT error = static_cast<HRESULT>(
                        static_cast<DWORD>(paquete >> 32));
                    reproductor->ProcesarEvento(evento, error);
                }
                return 0;
        }
        return DefWindowProcW(hwnd, mensaje, wParam, lParam);
    }

    void ProcesarEvento(DWORD evento, HRESULT error) {
        if (!m_engine) return;
        switch (evento) {
            case MF_MEDIA_ENGINE_EVENT_ERROR:
                m_listo = false;
                m_reproduciendo = false;
                m_reproduccionSolicitada = false;
                if (IsWindow(m_ventana)) KillTimer(m_ventana, kTimerReproductor);
                m_mensaje = L"No se pudo reproducir este archivo: formato no admitido o archivo corrupto.";
                (void)error;
                break;
            case MF_MEDIA_ENGINE_EVENT_LOADEDMETADATA:
            case MF_MEDIA_ENGINE_EVENT_DURATIONCHANGE:
                if (m_posicionPendiente > 0.0) m_engine->SetCurrentTime(m_posicionPendiente);
                break;
            case MF_MEDIA_ENGINE_EVENT_CANPLAY:
            case MF_MEDIA_ENGINE_EVENT_CANPLAYTHROUGH:
                m_listo = true;
                m_mensaje.clear();
                if (m_posicionPendiente > 0.0) m_engine->SetCurrentTime(m_posicionPendiente);
                if (m_reproduccionSolicitada) Reproducir();
                break;
            case MF_MEDIA_ENGINE_EVENT_PLAY:
            case MF_MEDIA_ENGINE_EVENT_PLAYING:
                m_listo = true;
                m_reproduciendo = true;
                SetTimer(m_ventana, kTimerReproductor, 33, nullptr);
                break;
            case MF_MEDIA_ENGINE_EVENT_PAUSE:
                m_reproduciendo = false;
                if (IsWindow(m_ventana)) KillTimer(m_ventana, kTimerReproductor);
                break;
            case MF_MEDIA_ENGINE_EVENT_ENDED:
                Pausar();
                Buscar(0.0);
                break;
            default:
                break;
        }
        InvalidateRect(m_ventana, nullptr, FALSE);
        if (m_panel) PostMessageW(m_panel, WM_SIDEVIEW_COMPARADOR_TICK,
                                  static_cast<WPARAM>(m_ranura), 0);
    }

    void ActualizarFrame() {
        if (!m_engineEx || !IsWindow(m_ventana)) return;
        RECT destino{};
        GetClientRect(m_ventana, &destino);
        MFARGB borde{};
        borde.rgbAlpha = 255;
        m_engineEx->UpdateVideoStream(nullptr, &destino, &borde);
    }

    template <typename T>
    static void LiberarCom(T*& objeto) {
        if (objeto) {
            objeto->Release();
            objeto = nullptr;
        }
    }

    void LiberarMediaFoundation() {
        if (m_mfIniciado) {
            MFShutdown();
            m_mfIniciado = false;
        }
    }

    void LiberarBackend() {
        Pausar();
        if (m_callback) m_callback->Desconectar();
        if (m_engine) m_engine->Shutdown();
        LiberarCom(m_engineEx);
        LiberarCom(m_engine);
        if (m_callback) {
            m_callback->Release();
            m_callback = nullptr;
        }
        LiberarMediaFoundation();
        m_listo = false;
        m_reproduciendo = false;
        m_reproduccionSolicitada = false;
    }

    HWND m_panel{nullptr};
    HWND m_ventana{nullptr};
    int m_ranura{0};
    bool m_esPip{false};
    bool m_mfIniciado{false};
    bool m_listo{false};
    bool m_reproduciendo{false};
    bool m_reproduccionSolicitada{false};
    double m_posicionPendiente{0.0};
    DWORD m_generacion{0};
    RECT m_rectActual{};
    bool m_rectValido{false};
    std::wstring m_mensaje;
    IMFMediaEngine* m_engine{nullptr};
    IMFMediaEngineEx* m_engineEx{nullptr};
    MediaEngineNotifyV2* m_callback{nullptr};
};

class ServicioReproductorVideoV2 {
public:
    std::unique_ptr<IReproductorVideoV2> Crear() const {
        // Punto de sustitución futuro: otro backend puede implementar la misma
        // interfaz sin cambiar la vista ni el comparador.
        return std::make_unique<ReproductorMediaFoundationV2>();
    }
};

} // namespace SideViewV2
/**** ended inlining v2/nucleo/servicio_reproductor_video.hpp ****/
#include <propkey.h>
#include <shobjidl.h>
#include <algorithm>
#include <cmath>
#include <cwchar>
#include <memory>
#include <string>
#include <vector>

namespace SideViewV2 {

using DatosVideoComparacionV2 = DatosArchivoComparacionV2;

struct EstadoComparadorVideosV2 : public EstadoComparadorArchivosV2 {
    double posicionSegundos{0.0};
    double duracionSegundos{600.0};
    double duracionIzquierda{0.0};
    double duracionDerecha{0.0};
    bool reproduciendo{false};
    bool sincronizacionConfirmada{false};
    bool duracionesSimetricas{false};
    bool modoPantallaCompleta{false};
    int indicePantallaCompleta{0};
};

// Comparador especializado que hereda las reglas comunes de peso, nombre y
// SHA-256, y agrega el reloj sincronizado y los dos reproductores inyectados.
class ServicioComparadorVideosV2 : public ServicioComparadorArchivosV2 {
public:
    explicit ServicioComparadorVideosV2(
        std::shared_ptr<ServicioReproductorVideoV2> reproductor =
            std::make_shared<ServicioReproductorVideoV2>())
        : m_reproductor(std::move(reproductor)) {}

    EstadoComparadorVideosV2 CrearFlujo(
        const std::vector<std::wstring>& seleccion = {}) const {
        EstadoComparadorVideosV2 estado;
        EstadoComparadorArchivosV2 comun = CrearComparacion(seleccion);
        estado.izquierda = EnriquecerVideo(comun.izquierda);
        estado.derecha = EnriquecerVideo(comun.derecha);
        estado.mensaje = seleccion.size() == 2
            ? L"Confirma la sincronización para validar la duración de ambos videos."
            : L"Selecciona dos videos para iniciar la comparación.";
        return estado;
    }

    std::unique_ptr<IReproductorVideoV2> CrearReproductor() const {
        return m_reproductor ? m_reproductor->Crear() : nullptr;
    }

    void AlternarPantallaCompleta(EstadoComparadorVideosV2& estado, int indice) const {
        if (indice < 0 || indice > 1) return;
        estado.modoPantallaCompleta = true;
        estado.indicePantallaCompleta = indice;
    }

    void IntercambiarPantallaCompleta(EstadoComparadorVideosV2& estado) const {
        if (estado.modoPantallaCompleta)
            estado.indicePantallaCompleta = 1 - estado.indicePantallaCompleta;
    }

    void AlternarEstadoReproduccion(EstadoComparadorVideosV2& estado,
                                    IReproductorVideoV2* izquierda,
                                    IReproductorVideoV2* derecha) const {
        const bool reproducir = !estado.reproduciendo;
        if (reproducir) {
            if (izquierda) izquierda->Reproducir();
            if (derecha) derecha->Reproducir();
        } else {
            if (izquierda) izquierda->Pausar();
            if (derecha) derecha->Pausar();
        }
        estado.reproduciendo = reproducir;
    }

    void BuscarAmbos(EstadoComparadorVideosV2& estado, double segundos,
                     IReproductorVideoV2* izquierda,
                     IReproductorVideoV2* derecha) const {
        estado.posicionSegundos = std::clamp(segundos, 0.0, estado.duracionSegundos);
        if (izquierda) izquierda->Buscar(estado.posicionSegundos);
        if (derecha) derecha->Buscar(estado.posicionSegundos);
    }

    void ActualizarDuraciones(EstadoComparadorVideosV2& estado,
                              double izquierda, double derecha) const {
        estado.duracionIzquierda = std::max(0.0, izquierda);
        estado.duracionDerecha = std::max(0.0, derecha);
        if (estado.sincronizacionConfirmada) {
            estado.duracionSegundos = LimiteSincronizacion(estado);
            estado.posicionSegundos = std::clamp(
                estado.posicionSegundos, 0.0, estado.duracionSegundos);
            return;
        }
        if (estado.duracionIzquierda > 0.0 && estado.duracionDerecha > 0.0) {
            estado.duracionSegundos = std::max(estado.duracionIzquierda, estado.duracionDerecha);
        } else if (estado.duracionIzquierda > 0.0) {
            estado.duracionSegundos = estado.duracionIzquierda;
        } else if (estado.duracionDerecha > 0.0) {
            estado.duracionSegundos = estado.duracionDerecha;
        }
    }

    void ConfirmarSincronizacion(EstadoComparadorVideosV2& estado) const {
        if (estado.duracionIzquierda <= 0.0 || estado.duracionDerecha <= 0.0) {
            estado.mensaje = L"Espera a que ambos videos carguen para confirmar la sincronización.";
            return;
        }
        estado.sincronizacionConfirmada = true;
        estado.duracionesSimetricas = std::abs(
            estado.duracionIzquierda - estado.duracionDerecha) < 0.05;
        estado.duracionSegundos = LimiteSincronizacion(estado);
        estado.posicionSegundos = std::clamp(
            estado.posicionSegundos, 0.0, estado.duracionSegundos);
        estado.mensaje = estado.duracionesSimetricas
            ? L"Sincronización confirmada: ambos videos tienen la misma duración."
            : L"Sincronización confirmada con límite: el video más corto marca el final.";
    }

    std::wstring FormatearTiempo(double segundos) const {
        const int total = std::max(0, static_cast<int>(std::lround(segundos)));
        wchar_t buffer[32]{};
        swprintf_s(buffer, L"%02d:%02d", (total % 3600) / 60, total % 60);
        return buffer;
    }

private:
    static double LimiteSincronizacion(const EstadoComparadorVideosV2& estado) {
        if (estado.duracionIzquierda > 0.0 && estado.duracionDerecha > 0.0) {
            return std::min(estado.duracionIzquierda, estado.duracionDerecha);
        }
        return estado.duracionIzquierda > 0.0 ? estado.duracionIzquierda :
               estado.duracionDerecha > 0.0 ? estado.duracionDerecha : 0.0;
    }

    static DatosVideoComparacionV2 EnriquecerVideo(DatosVideoComparacionV2 datos) {
        if (datos.ruta.empty()) return datos;
        IShellItem2* elemento = nullptr;
        if (SUCCEEDED(SHCreateItemFromParsingName(
                datos.ruta.c_str(), nullptr, IID_IShellItem2,
                reinterpret_cast<void**>(&elemento))) && elemento) {
            ULONG fpsMil = 0;
            if (SUCCEEDED(elemento->GetUInt32(PKEY_Video_FrameRate, &fpsMil)) && fpsMil) {
                wchar_t fps[32]{};
                swprintf_s(fps, L"%.2f fps", static_cast<double>(fpsMil) / 1000.0);
                datos.fps = fps;
            }
            elemento->Release();
        }
        return datos;
    }

    std::shared_ptr<ServicioReproductorVideoV2> m_reproductor;
};

} // namespace SideViewV2
/**** ended inlining v2/nucleo/servicio_comparador_videos.hpp ****/
/**** start inlining v2/nucleo/servicio_diff.hpp ****/

/**** skipping file: v2/nucleo/servicio_comparador_archivos.hpp ****/
/**** skipping file: v2/nucleo/servicio_texto.hpp ****/
#include <propkey.h>
#include <shobjidl.h>
#include <algorithm>
#include <memory>
#include <string>
#include <vector>

namespace SideViewV2 {

enum class TipoLineaDiffV2 { Igual, Agregada, Eliminada, Modificada };

struct LineaDiffV2 {
    TipoLineaDiffV2 tipo{TipoLineaDiffV2::Igual};
    std::size_t lineaIzquierda{0};
    std::size_t lineaDerecha{0};
    std::wstring izquierda;
    std::wstring derecha;
};

struct EstadoDiffV2 {
    std::vector<LineaDiffV2> lineas;
    std::wstring mensaje;
};

// Contrato inyectable para cualquier diff. El algoritmo de alineación común
// vive aquí para que texto, documentos y futuras variantes compartan reglas.
class ServicioDiffViewV2 : public ServicioComparadorArchivosV2 {
public:
    ~ServicioDiffViewV2() override = default;
    virtual EstadoDiffV2 CrearDiff(const std::vector<std::wstring>& seleccion) const = 0;

protected:
    static EstadoDiffV2 CompararLineasGenerico(
        const std::vector<std::wstring>& izquierda,
        const std::vector<std::wstring>& derecha) {
        EstadoDiffV2 resultado;
        std::size_t i = 0;
        std::size_t j = 0;
        constexpr std::size_t ventana = 24;
        while (i < izquierda.size() || j < derecha.size()) {
            if (i < izquierda.size() && j < derecha.size() && izquierda[i] == derecha[j]) {
                resultado.lineas.push_back({TipoLineaDiffV2::Igual, i + 1, j + 1,
                                            izquierda[i], derecha[j]});
                ++i;
                ++j;
                continue;
            }

            std::size_t siguienteDerecha = derecha.size();
            for (std::size_t k = j + 1; k < std::min(derecha.size(), j + ventana); ++k) {
                if (i < izquierda.size() && izquierda[i] == derecha[k]) {
                    siguienteDerecha = k;
                    break;
                }
            }
            std::size_t siguienteIzquierda = izquierda.size();
            for (std::size_t k = i + 1; k < std::min(izquierda.size(), i + ventana); ++k) {
                if (j < derecha.size() && izquierda[k] == derecha[j]) {
                    siguienteIzquierda = k;
                    break;
                }
            }

            if (siguienteDerecha != derecha.size() &&
                (siguienteIzquierda == izquierda.size() ||
                 siguienteDerecha - j <= siguienteIzquierda - i)) {
                resultado.lineas.push_back({TipoLineaDiffV2::Agregada, 0, j + 1,
                                            L"", derecha[j]});
                ++j;
            } else if (siguienteIzquierda != izquierda.size()) {
                resultado.lineas.push_back({TipoLineaDiffV2::Eliminada, i + 1, 0,
                                            izquierda[i], L""});
                ++i;
            } else {
                resultado.lineas.push_back({TipoLineaDiffV2::Modificada,
                                            i < izquierda.size() ? i + 1 : 0,
                                            j < derecha.size() ? j + 1 : 0,
                                            i < izquierda.size() ? izquierda[i] : L"",
                                            j < derecha.size() ? derecha[j] : L""});
                if (i < izquierda.size()) ++i;
                if (j < derecha.size()) ++j;
            }
        }
        return resultado;
    }

    static std::vector<std::wstring> DatosComoLineas(
        const DatosArchivoComparacionV2& datos) {
        std::vector<std::wstring> lineas;
        lineas.push_back(L"Nombre: " + datos.nombre);
        lineas.push_back(L"Formato: " + datos.extension);
        lineas.push_back(L"SHA-256: " + datos.sha256);
        lineas.push_back(L"Peso en bytes: " + std::to_wstring(datos.bytes));
        lineas.push_back(datos.paginas ? L"Páginas: " + std::to_wstring(datos.paginas)
                                       : L"Páginas: No disponible");
        lineas.push_back(L"Contenido: el adaptador documental compara metadatos hasta habilitar extracción segura.");
        return lineas;
    }
};

class ServicioComparadorTextoV2 final : public ServicioDiffViewV2 {
public:
    explicit ServicioComparadorTextoV2(std::shared_ptr<ServicioTextoV2> texto)
        : m_texto(std::move(texto)) {}

    EstadoComparadorArchivosV2 CrearComparacion(
        const std::vector<std::wstring>& seleccion) const override {
        EstadoComparadorArchivosV2 estado =
            ServicioComparadorArchivosV2::CrearComparacion(seleccion);
        if (seleccion.size() != 2 || !m_texto) return estado;
        const DocumentoTextoV2 izquierda = m_texto->Cargar(seleccion[0]);
        const DocumentoTextoV2 derecha = m_texto->Cargar(seleccion[1]);
        estado.izquierda.lineas = izquierda.lineas.size();
        estado.derecha.lineas = derecha.lineas.size();
        estado.mensaje = L"Comparación de texto lista.";
        return estado;
    }

    EstadoDiffV2 CrearDiff(const std::vector<std::wstring>& seleccion) const override {
        EstadoDiffV2 resultado;
        if (seleccion.size() != 2 || !m_texto) {
            resultado.mensaje = L"Selecciona exactamente dos archivos de texto.";
            return resultado;
        }
        const DocumentoTextoV2 izquierda = m_texto->Cargar(seleccion[0]);
        const DocumentoTextoV2 derecha = m_texto->Cargar(seleccion[1]);
        resultado = CompararLineasGenerico(izquierda.lineas, derecha.lineas);
        resultado.mensaje = L"Diff de texto listo.";
        return resultado;
    }

private:
    std::shared_ptr<ServicioTextoV2> m_texto;
};

class ServicioComparadorDocumentosV2 final : public ServicioDiffViewV2 {
public:
    ServicioComparadorDocumentosV2() = default;

    EstadoComparadorArchivosV2 CrearComparacion(
        const std::vector<std::wstring>& seleccion) const override {
        EstadoComparadorArchivosV2 estado =
            ServicioComparadorArchivosV2::CrearComparacion(seleccion);
        if (seleccion.size() != 2) return estado;
        estado.izquierda.paginas = LeerPaginas(estado.izquierda.ruta);
        estado.derecha.paginas = LeerPaginas(estado.derecha.ruta);
        estado.mensaje = L"Comparación documental lista para Diff View.";
        return estado;
    }

    EstadoDiffV2 CrearDiff(const std::vector<std::wstring>& seleccion) const override {
        EstadoDiffV2 resultado;
        if (seleccion.size() != 2) {
            resultado.mensaje = L"Selecciona exactamente dos documentos del mismo formato.";
            return resultado;
        }
        const EstadoComparadorArchivosV2 estado = CrearComparacion(seleccion);
        resultado = CompararLineasGenerico(
            DatosComoLineas(estado.izquierda), DatosComoLineas(estado.derecha));
        resultado.mensaje = L"Diff documental preparado con un resumen semántico.";
        return resultado;
    }

private:
    static std::uintmax_t LeerPaginas(const std::wstring& ruta) {
        IShellItem2* elemento = nullptr;
        if (FAILED(SHCreateItemFromParsingName(
                ruta.c_str(), nullptr, IID_IShellItem2,
                reinterpret_cast<void**>(&elemento))) || !elemento) {
            return 0;
        }
        ULONG paginas = 0;
        const HRESULT resultado = elemento->GetUInt32(PKEY_Document_PageCount, &paginas);
        elemento->Release();
        return SUCCEEDED(resultado) ? static_cast<std::uintmax_t>(paginas) : 0;
    }
};

} // namespace SideViewV2
/**** ended inlining v2/nucleo/servicio_diff.hpp ****/
/**** skipping file: v2/nucleo/servicio_reproductor_video.hpp ****/
/**** skipping file: v2/nucleo/servicio_persistencia_windhawk.hpp ****/
/**** start inlining v2/nucleo/servicio_version_mod.hpp ****/

#include <string>

namespace SideViewV2 {

// Fuente de verdad legible por la UI. El compilador comprueba que coincida con
// el campo @version antes de crear el bundle distribuible.
inline constexpr wchar_t VERSION_MOD_V2[] = L"4.1.4";

class ServicioVersionModV2 final {
public:
    std::wstring TextoVersion() const {
        return L"Useful Side View v" + std::wstring(VERSION_MOD_V2);
    }
};

} // namespace SideViewV2
/**** ended inlining v2/nucleo/servicio_version_mod.hpp ****/
/**** skipping file: v2/nucleo/servicio_compatibilidad_gpu.hpp ****/
/**** start inlining v2/nucleo/servicio_notas.hpp ****/

#include <windows.h>
/**** skipping file: v2/nucleo/servicio_persistencia_windhawk.hpp ****/

#include <algorithm>
#include <cstddef>
#include <cwchar>
#include <cstring>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <utility>
#include <vector>

namespace SideViewV2 {

enum class FuenteNotaV2 {
    Nativa,
    WinKlok,
};

struct NotaV2 {
    int id{0};
    std::wstring titulo;
    std::wstring cuerpo;
    long long modificado{0};
    FuenteNotaV2 fuente{FuenteNotaV2::Nativa};
    int idWinKlok{0};
};

struct ResultadoSincronizacionNotasV2 {
    bool disponible{false};
    bool realizada{false};
    int importadas{0};
    int exportadas{0};
    std::wstring mensaje;
};

struct NotaWinKlokV2 {
    int id{0};
    std::wstring titulo;
    std::wstring cuerpo;
    long long modificado{0};
};

// Adaptador de compatibilidad estrictamente encapsulado. No usa el almacén de
// Useful Side View para fingir que es el de otro mod: resuelve únicamente el
// almacén LocalStorage de WinKlok a través del engine.ini de Windhawk.
class AlmacenNotasWinKlokV2 final {
public:
    bool Leer(std::vector<NotaWinKlokV2>& notas, int& siguienteId, bool crear = false) const {
        Backend backend{};
        if (!Resolver(backend, crear)) return false;

        std::wstring ids;
        if (!LeerCadena(backend, L"noteIds", ids)) ids.clear();
        notas.clear();
        int maxId = 0;
        for (const std::wstring& token : Separar(ids, L',')) {
            const int id = AEntero(token, 0);
            if (id <= 0 || std::any_of(notas.begin(), notas.end(), [id](const auto& n) { return n.id == id; })) {
                continue;
            }

            std::wstring valor;
            if (!LeerGrande(backend, L"note" + std::to_wstring(id), valor)) continue;
            NotaWinKlokV2 nota{};
            if (!DeserializarNota(valor, nota)) continue;
            nota.id = id;
            notas.push_back(std::move(nota));
            maxId = std::max(maxId, id);
        }

        int guardado = 1;
        LeerEntero(backend, L"noteNext", guardado);
        siguienteId = std::max(maxId + 1, std::max(1, guardado));
        std::stable_sort(notas.begin(), notas.end(), [](const auto& a, const auto& b) {
            return a.modificado > b.modificado;
        });
        return true;
    }

    bool Guardar(const std::vector<NotaWinKlokV2>& notas, int siguienteId,
                 const std::vector<int>& eliminar) const {
        Backend backend{};
        if (!Resolver(backend, true)) return false;

        std::vector<int> ids;
        ids.reserve(notas.size());
        for (const auto& nota : notas) {
            if (nota.id <= 0 || std::find(eliminar.begin(), eliminar.end(), nota.id) != eliminar.end()) continue;
            if (std::find(ids.begin(), ids.end(), nota.id) == ids.end()) ids.push_back(nota.id);
            if (!GuardarGrande(backend, L"note" + std::to_wstring(nota.id), SerializarNota(nota))) return false;
        }

        for (int id : eliminar) {
            EliminarGrande(backend, L"note" + std::to_wstring(id));
        }

        std::wstring indice;
        for (int id : ids) {
            if (!indice.empty()) indice += L',';
            indice += std::to_wstring(id);
        }
        if (!GuardarCadena(backend, L"noteIds", indice) || !GuardarEntero(backend, L"noteNext", std::max(1, siguienteId))) {
            return false;
        }
        VaciarCacheIni(backend);
        return true;
    }

    static constexpr std::size_t TamanoFragmento() { return kTamanoFragmento; }
    static constexpr int MaximoFragmentos() { return kMaximoFragmentos; }

private:
    enum class TipoBackend {
        Ninguno,
        Registro,
        Ini,
    };

    struct Backend {
        TipoBackend tipo{TipoBackend::Ninguno};
        HKEY raiz{nullptr};
        std::wstring subclave;
        std::wstring archivoIni;
    };

    static constexpr std::size_t kTamanoFragmento = 30000;
    static constexpr int kMaximoFragmentos = 512;

    static std::wstring Recortar(const std::wstring& valor) {
        const std::size_t primero = valor.find_first_not_of(L" \t\r\n");
        if (primero == std::wstring::npos) return {};
        const std::size_t ultimo = valor.find_last_not_of(L" \t\r\n");
        return valor.substr(primero, ultimo - primero + 1);
    }

    static std::wstring ExpandirVariables(const std::wstring& valor) {
        if (valor.empty()) return {};
        DWORD longitud = ExpandEnvironmentStringsW(valor.c_str(), nullptr, 0);
        if (longitud == 0) return {};
        std::vector<wchar_t> buffer(longitud, L'\0');
        if (!ExpandEnvironmentStringsW(valor.c_str(), buffer.data(), longitud)) return {};
        return std::wstring(buffer.data());
    }

    static std::wstring RutaEngineIni() {
        HMODULE modulo = GetModuleHandleW(L"windhawk.dll");
        if (!modulo) return {};
        std::vector<wchar_t> buffer(32768, L'\0');
        DWORD longitud = GetModuleFileNameW(modulo, buffer.data(), static_cast<DWORD>(buffer.size()));
        if (longitud == 0 || longitud >= buffer.size()) return {};
        std::wstring ruta(buffer.data(), longitud);
        const std::size_t ultima = ruta.find_last_of(L"\\/");
        if (ultima == std::wstring::npos) return {};
        ruta.resize(ultima);
        const std::size_t penultima = ruta.find_last_of(L"\\/");
        if (penultima == std::wstring::npos) return {};
        ruta.resize(penultima + 1);
        return ruta + L"engine.ini";
    }

    static bool ClaveExiste(HKEY raiz, const std::wstring& subclave) {
        HKEY clave = nullptr;
        const LSTATUS estado = RegOpenKeyExW(raiz, subclave.c_str(), 0, KEY_READ | KEY_WOW64_64KEY, &clave);
        if (clave) RegCloseKey(clave);
        return estado == ERROR_SUCCESS;
    }

    static HKEY ResolverRaiz(const std::wstring& nombre) {
        if (_wcsicmp(nombre.c_str(), L"HKCU") == 0 || _wcsicmp(nombre.c_str(), L"HKEY_CURRENT_USER") == 0) {
            return HKEY_CURRENT_USER;
        }
        if (_wcsicmp(nombre.c_str(), L"HKU") == 0 || _wcsicmp(nombre.c_str(), L"HKEY_USERS") == 0) {
            return HKEY_USERS;
        }
        if (_wcsicmp(nombre.c_str(), L"HKLM") == 0 || _wcsicmp(nombre.c_str(), L"HKEY_LOCAL_MACHINE") == 0) {
            return HKEY_LOCAL_MACHINE;
        }
        return nullptr;
    }

    static bool Resolver(Backend& backend, bool crear) {
        const std::wstring engineIni = RutaEngineIni();
        if (engineIni.empty()) return false;

        wchar_t portable[16]{};
        GetPrivateProfileStringW(L"Storage", L"Portable", L"0", portable, ARRAYSIZE(portable), engineIni.c_str());
        wchar_t appDataBuffer[32768]{};
        GetPrivateProfileStringW(L"Storage", L"AppDataPath", L"", appDataBuffer, ARRAYSIZE(appDataBuffer), engineIni.c_str());
        const std::wstring appData = ExpandirVariables(Recortar(appDataBuffer));
        if (appData.empty()) return false;

        const bool esPortable = _wtoi(portable) != 0;
        if (esPortable) {
            backend.tipo = TipoBackend::Ini;
            backend.archivoIni = appData + L"\\ModsWritable\\winklok.ini";
            const std::wstring mods = appData + L"\\Mods\\winklok.ini";
            const std::wstring writable = appData + L"\\ModsWritable\\winklok.ini";
            const bool instalado = GetFileAttributesW(mods.c_str()) != INVALID_FILE_ATTRIBUTES ||
                                   GetFileAttributesW(writable.c_str()) != INVALID_FILE_ATTRIBUTES;
            if (!instalado &&
                GetFileAttributesW(backend.archivoIni.c_str()) == INVALID_FILE_ATTRIBUTES) {
                return false;
            }
            if (crear) {
                WritePrivateProfileStringW(L"LocalStorage", L"__useful_sideview_probe", L"1", backend.archivoIni.c_str());
                WritePrivateProfileStringW(L"LocalStorage", L"__useful_sideview_probe", nullptr, backend.archivoIni.c_str());
            }
            return true;
        }

        wchar_t registryBuffer[32768]{};
        GetPrivateProfileStringW(L"Storage", L"RegistryKey", L"", registryBuffer, ARRAYSIZE(registryBuffer), engineIni.c_str());
        const std::wstring registryKey = Recortar(registryBuffer);
        const std::size_t separador = registryKey.find(L'\\');
        if (separador == std::wstring::npos) return false;
        const HKEY raiz = ResolverRaiz(registryKey.substr(0, separador));
        if (!raiz) return false;
        const std::wstring base = registryKey.substr(separador + 1);
        const std::wstring instalado = base + L"\\Mods\\winklok";
        const std::wstring localStorage = base + L"\\ModsWritable\\winklok\\LocalStorage";
        if (!ClaveExiste(raiz, localStorage) && !ClaveExiste(raiz, instalado)) return false;
        if (crear) {
            HKEY clave = nullptr;
            const LSTATUS estado = RegCreateKeyExW(raiz, localStorage.c_str(), 0, nullptr, 0,
                                                   KEY_READ | KEY_SET_VALUE | KEY_WOW64_64KEY,
                                                   nullptr, &clave, nullptr);
            if (estado != ERROR_SUCCESS) return false;
            RegCloseKey(clave);
        }
        backend.tipo = TipoBackend::Registro;
        backend.raiz = raiz;
        backend.subclave = localStorage;
        return true;
    }

    static bool LeerCadena(const Backend& backend, const std::wstring& clave, std::wstring& salida) {
        salida.clear();
        if (backend.tipo == TipoBackend::Ini) {
            std::vector<wchar_t> buffer(kTamanoFragmento + 2, L'\0');
            const DWORD longitud = GetPrivateProfileStringW(L"LocalStorage", clave.c_str(), L"", buffer.data(),
                                                             static_cast<DWORD>(buffer.size()), backend.archivoIni.c_str());
            if (longitud == 0 || longitud >= buffer.size() - 1) return false;
            salida.assign(buffer.data(), longitud);
            return true;
        }

        HKEY registro = nullptr;
        if (RegOpenKeyExW(backend.raiz, backend.subclave.c_str(), 0, KEY_QUERY_VALUE | KEY_WOW64_64KEY, &registro) != ERROR_SUCCESS) {
            return false;
        }
        DWORD tipo = 0;
        DWORD bytes = 0;
        LSTATUS estado = RegQueryValueExW(registro, clave.c_str(), nullptr, &tipo, nullptr, &bytes);
        if (estado != ERROR_SUCCESS || (tipo != REG_SZ && tipo != REG_EXPAND_SZ) || bytes < sizeof(wchar_t)) {
            RegCloseKey(registro);
            return false;
        }
        std::vector<wchar_t> buffer(bytes / sizeof(wchar_t) + 1, L'\0');
        estado = RegQueryValueExW(registro, clave.c_str(), nullptr, &tipo,
                                  reinterpret_cast<LPBYTE>(buffer.data()), &bytes);
        RegCloseKey(registro);
        if (estado != ERROR_SUCCESS) return false;
        salida.assign(buffer.data());
        return true;
    }

    static bool LeerEntero(const Backend& backend, const std::wstring& clave, int& salida) {
        std::wstring texto;
        if (LeerCadena(backend, clave, texto)) {
            salida = AEntero(texto, salida);
            return true;
        }
        if (backend.tipo != TipoBackend::Registro) return false;
        HKEY registro = nullptr;
        if (RegOpenKeyExW(backend.raiz, backend.subclave.c_str(), 0, KEY_QUERY_VALUE | KEY_WOW64_64KEY, &registro) != ERROR_SUCCESS) return false;
        DWORD tipo = 0;
        DWORD valor = 0;
        DWORD bytes = sizeof(valor);
        const LSTATUS estado = RegQueryValueExW(registro, clave.c_str(), nullptr, &tipo,
                                                 reinterpret_cast<LPBYTE>(&valor), &bytes);
        RegCloseKey(registro);
        if (estado != ERROR_SUCCESS || tipo != REG_DWORD || bytes != sizeof(valor)) return false;
        salida = static_cast<int>(valor);
        return true;
    }

    static bool GuardarCadena(const Backend& backend, const std::wstring& clave, const std::wstring& valor) {
        if (backend.tipo == TipoBackend::Ini) {
            return WritePrivateProfileStringW(L"LocalStorage", clave.c_str(), valor.c_str(), backend.archivoIni.c_str()) != FALSE;
        }
        HKEY registro = nullptr;
        if (RegOpenKeyExW(backend.raiz, backend.subclave.c_str(), 0, KEY_SET_VALUE | KEY_WOW64_64KEY, &registro) != ERROR_SUCCESS) return false;
        const DWORD bytes = static_cast<DWORD>((valor.size() + 1) * sizeof(wchar_t));
        const LSTATUS estado = RegSetValueExW(registro, clave.c_str(), 0, REG_SZ,
                                               reinterpret_cast<const BYTE*>(valor.c_str()), bytes);
        RegCloseKey(registro);
        return estado == ERROR_SUCCESS;
    }

    static bool GuardarEntero(const Backend& backend, const std::wstring& clave, int valor) {
        if (backend.tipo == TipoBackend::Ini) {
            return GuardarCadena(backend, clave, std::to_wstring(valor));
        }
        HKEY registro = nullptr;
        if (RegOpenKeyExW(backend.raiz, backend.subclave.c_str(), 0, KEY_SET_VALUE | KEY_WOW64_64KEY, &registro) != ERROR_SUCCESS) return false;
        const DWORD valorDword = static_cast<DWORD>(std::max(0, valor));
        const LSTATUS estado = RegSetValueExW(registro, clave.c_str(), 0, REG_DWORD,
                                               reinterpret_cast<const BYTE*>(&valorDword), sizeof(valorDword));
        RegCloseKey(registro);
        return estado == ERROR_SUCCESS;
    }

    static void Eliminar(const Backend& backend, const std::wstring& clave) {
        if (backend.tipo == TipoBackend::Ini) {
            WritePrivateProfileStringW(L"LocalStorage", clave.c_str(), nullptr, backend.archivoIni.c_str());
            return;
        }
        HKEY registro = nullptr;
        if (RegOpenKeyExW(backend.raiz, backend.subclave.c_str(), 0, KEY_SET_VALUE | KEY_WOW64_64KEY, &registro) == ERROR_SUCCESS) {
            RegDeleteValueW(registro, clave.c_str());
            RegCloseKey(registro);
        }
    }

    static void VaciarCacheIni(const Backend& backend) {
        if (backend.tipo == TipoBackend::Ini) {
            WritePrivateProfileStringW(nullptr, nullptr, nullptr, backend.archivoIni.c_str());
        }
    }

    static bool LeerGrande(const Backend& backend, const std::wstring& clave, std::wstring& salida) {
        salida.clear();
        int cantidad = 0;
        LeerEntero(backend, clave + L"_n", cantidad);
        if (cantidad < 0 || cantidad > kMaximoFragmentos) return false;
        if (cantidad == 0) return LeerCadena(backend, clave, salida);
        for (int i = 0; i < cantidad; ++i) {
            std::wstring parte;
            if (!LeerCadena(backend, clave + L"_" + std::to_wstring(i), parte)) return false;
            salida += parte;
        }
        return true;
    }

    static bool GuardarGrande(const Backend& backend, const std::wstring& clave, const std::wstring& valor) {
        int anterior = 0;
        LeerEntero(backend, clave + L"_n", anterior);
        int cantidad = 0;
        for (std::size_t inicio = 0; inicio < valor.size(); inicio += kTamanoFragmento) {
            const std::wstring parte = valor.substr(inicio, kTamanoFragmento);
            if (!GuardarCadena(backend, clave + L"_" + std::to_wstring(cantidad), parte)) return false;
            ++cantidad;
        }
        if (cantidad == 0) {
            if (!GuardarCadena(backend, clave + L"_0", L"")) return false;
            cantidad = 1;
        }
        for (int i = cantidad; i < anterior; ++i) Eliminar(backend, clave + L"_" + std::to_wstring(i));
        if (!GuardarEntero(backend, clave + L"_n", cantidad)) return false;
        Eliminar(backend, clave);
        return true;
    }

    static void EliminarGrande(const Backend& backend, const std::wstring& clave) {
        int cantidad = 0;
        LeerEntero(backend, clave + L"_n", cantidad);
        if (cantidad >= 0 && cantidad <= kMaximoFragmentos) {
            for (int i = 0; i < cantidad; ++i) Eliminar(backend, clave + L"_" + std::to_wstring(i));
        }
        Eliminar(backend, clave + L"_n");
        Eliminar(backend, clave);
    }

    static std::vector<std::wstring> Separar(const std::wstring& valor, wchar_t separador) {
        std::vector<std::wstring> partes;
        std::size_t inicio = 0;
        while (inicio <= valor.size()) {
            const std::size_t fin = valor.find(separador, inicio);
            partes.push_back(valor.substr(inicio, fin == std::wstring::npos ? fin : fin - inicio));
            if (fin == std::wstring::npos) break;
            inicio = fin + 1;
        }
        return partes;
    }

    static int AEntero(const std::wstring& valor, int predeterminado) {
        if (valor.empty()) return predeterminado;
        wchar_t* fin = nullptr;
        const long resultado = wcstol(valor.c_str(), &fin, 10);
        if (!fin || *fin != L'\0' || resultado < 0 || resultado > 2147483647L) return predeterminado;
        return static_cast<int>(resultado);
    }

    static std::wstring Escapar(const std::wstring& valor) {
        std::wstring salida;
        salida.reserve(valor.size() + 16);
        for (wchar_t caracter : valor) {
            switch (caracter) {
                case L'\\': salida += L"\\\\"; break;
                case L'|': salida += L"\\p"; break;
                case L';': salida += L"\\s"; break;
                case L'\n': salida += L"\\n"; break;
                case L'\r': break;
                default: salida += caracter; break;
            }
        }
        return salida;
    }

    static std::wstring Desescapar(const std::wstring& valor) {
        std::wstring salida;
        salida.reserve(valor.size());
        for (std::size_t i = 0; i < valor.size(); ++i) {
            if (valor[i] != L'\\' || i + 1 >= valor.size()) {
                salida += valor[i];
                continue;
            }
            const wchar_t codigo = valor[++i];
            if (codigo == L'n') salida += L'\n';
            else if (codigo == L'p') salida += L'|';
            else if (codigo == L's') salida += L';';
            else if (codigo == L'\\') salida += L'\\';
            else { salida += L'\\'; salida += codigo; }
        }
        return salida;
    }

    static bool DeserializarNota(const std::wstring& valor, NotaWinKlokV2& nota) {
        const std::size_t primero = valor.find(L'|');
        if (primero == std::wstring::npos) return false;
        const std::size_t segundo = valor.find(L'|', primero + 1);
        if (segundo == std::wstring::npos) return false;
        nota.modificado = static_cast<long long>(AEntero(valor.substr(0, primero), 0));
        nota.titulo = Desescapar(valor.substr(primero + 1, segundo - primero - 1));
        nota.cuerpo = Desescapar(valor.substr(segundo + 1));
        return nota.modificado >= 0;
    }

    static std::wstring SerializarNota(const NotaWinKlokV2& nota) {
        return std::to_wstring(std::max(0LL, nota.modificado)) + L"|" +
               Escapar(nota.titulo) + L"|" + Escapar(nota.cuerpo);
    }
};

class ServicioNotasV2 final {
public:
    explicit ServicioNotasV2(
        std::shared_ptr<ServicioPersistenciaWindhawkV2> persistencia,
        std::function<bool()> politicaSincronizacion = {})
        : m_persistencia(std::move(persistencia)),
          m_politicaSincronizacion(std::move(politicaSincronizacion)) {}

    void Cargar() {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_cargado) return;
        CargarNativas();
        m_cargado = true;
        ActualizarPuenteWinKlokBloqueado();
    }

    void Recargar() {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (!m_cargado) {
            CargarNativas();
            m_cargado = true;
        } else {
            CargarNativas();
        }
        ActualizarPuenteWinKlokBloqueado();
    }

    const std::vector<NotaV2>& Notas() const { return m_notas; }

    const NotaV2* Buscar(int id) const {
        const auto it = std::find_if(m_notas.begin(), m_notas.end(), [id](const auto& nota) { return nota.id == id; });
        return it == m_notas.end() ? nullptr : &*it;
    }

    int Crear(const std::wstring& titulo, const std::wstring& cuerpo) {
        std::lock_guard<std::mutex> lock(m_mutex);
        CargarSiHaceFaltaBloqueado();
        NotaV2 nota{};
        nota.id = m_siguienteId++;
        nota.titulo = titulo.empty() ? L"Nueva nota" : titulo;
        nota.cuerpo = cuerpo;
        nota.modificado = Ahora();
        m_notas.insert(m_notas.begin(), nota);
        GuardarNativasBloqueado();
        if (SincronizacionActiva()) SincronizarBloqueado(false);
        return nota.id;
    }

    bool Actualizar(int id, const std::wstring& titulo, const std::wstring& cuerpo) {
        std::lock_guard<std::mutex> lock(m_mutex);
        CargarSiHaceFaltaBloqueado();
        NotaV2* nota = BuscarMutable(id);
        if (!nota) return false;
        nota->titulo = titulo.empty() ? L"Sin título" : titulo;
        nota->cuerpo = cuerpo;
        nota->modificado = Ahora();
        OrdenarBloqueado();
        GuardarNativasBloqueado();
        if (SincronizacionActiva()) SincronizarBloqueado(false);
        return true;
    }

    bool Eliminar(int id) {
        std::lock_guard<std::mutex> lock(m_mutex);
        CargarSiHaceFaltaBloqueado();
        const auto it = std::find_if(m_notas.begin(), m_notas.end(), [id](const auto& nota) { return nota.id == id; });
        if (it == m_notas.end()) return false;
        if (it->idWinKlok > 0) m_eliminadosWinKlok.push_back(it->idWinKlok);
        m_notas.erase(it);
        GuardarNativasBloqueado();
        if (SincronizacionActiva()) SincronizarBloqueado(false);
        return true;
    }

    bool Copiar(HWND hwnd, int id) const {
        std::lock_guard<std::mutex> lock(m_mutex);
        const NotaV2* nota = Buscar(id);
        if (!nota || !OpenClipboard(hwnd)) return false;
        const std::wstring contenido = nota->cuerpo;
        EmptyClipboard();
        const SIZE_T bytes = (contenido.size() + 1) * sizeof(wchar_t);
        HGLOBAL memoria = GlobalAlloc(GMEM_MOVEABLE, bytes);
        if (!memoria) { CloseClipboard(); return false; }
        void* destino = GlobalLock(memoria);
        if (!destino) { GlobalFree(memoria); CloseClipboard(); return false; }
        memcpy(destino, contenido.c_str(), bytes);
        GlobalUnlock(memoria);
        if (!SetClipboardData(CF_UNICODETEXT, memoria)) {
            GlobalFree(memoria);
            CloseClipboard();
            return false;
        }
        CloseClipboard();
        return true;
    }

    ResultadoSincronizacionNotasV2 ForzarSincronizacion() {
        std::lock_guard<std::mutex> lock(m_mutex);
        CargarSiHaceFaltaBloqueado();
        if (!SincronizacionActiva()) {
            return {false, false, 0, 0,
                    L"Activa \"Sincronización de notas con WinKlok (experimental)\" en Ajustes para usar esta función."};
        }
        return SincronizarBloqueado(true);
    }

    bool WinKlokDisponible() const { return m_winKlokDisponible; }

    bool EditarConPrompt(HWND propietario, int id, bool esNueva) {
        NotaV2 inicial{};
        if (!esNueva) {
            std::lock_guard<std::mutex> lock(m_mutex);
            const NotaV2* existente = Buscar(id);
            if (!existente) return false;
            inicial = *existente;
        }

        PromptNotaV2 prompt{propietario, inicial.titulo, inicial.cuerpo};
        if (!MostrarPromptNota(prompt)) return false;
        if (esNueva) {
            Crear(prompt.titulo, prompt.cuerpo);
        } else {
            Actualizar(id, prompt.titulo, prompt.cuerpo);
        }
        return true;
    }

private:
    struct PromptNotaV2 {
        HWND propietario{nullptr};
        std::wstring titulo;
        std::wstring cuerpo;
        bool aceptado{false};
        HWND campoTitulo{nullptr};
        HWND campoCuerpo{nullptr};
    };

    static constexpr int kControlTitulo = 101;
    static constexpr int kControlCuerpo = 102;

    static LRESULT CALLBACK ProcedimientoPromptNota(HWND hwnd, UINT mensaje, WPARAM wParam, LPARAM lParam) {
        auto* estado = reinterpret_cast<PromptNotaV2*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
        if (mensaje == WM_NCCREATE) {
            auto* crear = reinterpret_cast<CREATESTRUCTW*>(lParam);
            estado = reinterpret_cast<PromptNotaV2*>(crear->lpCreateParams);
            SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(estado));
        }
        if (!estado) return DefWindowProcW(hwnd, mensaje, wParam, lParam);
        if (mensaje == WM_CREATE) {
            CreateWindowW(L"STATIC", L"Título", WS_CHILD | WS_VISIBLE, 14, 12, 70, 20, hwnd, nullptr, GetModuleHandleW(nullptr), nullptr);
            estado->campoTitulo = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", estado->titulo.c_str(), WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
                                                  82, 10, 390, 24, hwnd, reinterpret_cast<HMENU>(kControlTitulo), GetModuleHandleW(nullptr), nullptr);
            CreateWindowW(L"STATIC", L"Contenido (Markdown opcional)", WS_CHILD | WS_VISIBLE, 14, 44, 240, 20, hwnd, nullptr, GetModuleHandleW(nullptr), nullptr);
            estado->campoCuerpo = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", estado->cuerpo.c_str(), WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_AUTOVSCROLL | ES_WANTRETURN | WS_VSCROLL,
                                                  14, 66, 458, 250, hwnd, reinterpret_cast<HMENU>(kControlCuerpo), GetModuleHandleW(nullptr), nullptr);
            CreateWindowW(L"BUTTON", L"Guardar", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON, 296, 328, 84, 28, hwnd, reinterpret_cast<HMENU>(IDOK), GetModuleHandleW(nullptr), nullptr);
            CreateWindowW(L"BUTTON", L"Cancelar", WS_CHILD | WS_VISIBLE, 388, 328, 84, 28, hwnd, reinterpret_cast<HMENU>(IDCANCEL), GetModuleHandleW(nullptr), nullptr);
            SendMessageW(estado->campoTitulo, EM_SETSEL, 0, -1);
            SetFocus(estado->campoTitulo);
            return 0;
        }
        if (mensaje == WM_COMMAND && (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)) {
            if (LOWORD(wParam) == IDOK) {
                const int tituloLargo = GetWindowTextLengthW(estado->campoTitulo);
                const int cuerpoLargo = GetWindowTextLengthW(estado->campoCuerpo);
                std::vector<wchar_t> titulo(static_cast<std::size_t>(tituloLargo) + 1, L'\0');
                std::vector<wchar_t> cuerpo(static_cast<std::size_t>(cuerpoLargo) + 1, L'\0');
                GetWindowTextW(estado->campoTitulo, titulo.data(), static_cast<int>(titulo.size()));
                GetWindowTextW(estado->campoCuerpo, cuerpo.data(), static_cast<int>(cuerpo.size()));
                estado->titulo.assign(titulo.data());
                estado->cuerpo.assign(cuerpo.data());
                if (estado->titulo.empty()) estado->titulo = L"Sin título";
                estado->aceptado = true;
            }
            EnableWindow(estado->propietario, TRUE);
            DestroyWindow(hwnd);
            return 0;
        }
        if (mensaje == WM_CLOSE) {
            EnableWindow(estado->propietario, TRUE);
            DestroyWindow(hwnd);
            return 0;
        }
        return DefWindowProcW(hwnd, mensaje, wParam, lParam);
    }

    static bool MostrarPromptNota(PromptNotaV2& estado) {
        static std::once_flag registroClase;
        std::call_once(registroClase, [] {
            WNDCLASSW clase{};
            clase.lpfnWndProc = ProcedimientoPromptNota;
            clase.hInstance = GetModuleHandleW(nullptr);
            clase.hCursor = LoadCursorW(nullptr, IDC_ARROW);
            clase.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
            clase.lpszClassName = L"UsefulSideView_PromptNota_V2";
            RegisterClassW(&clase);
        });

        EnableWindow(estado.propietario, FALSE);
        HWND ventana = CreateWindowExW(WS_EX_DLGMODALFRAME, L"UsefulSideView_PromptNota_V2", L"Nota",
                                       WS_CAPTION | WS_SYSMENU, CW_USEDEFAULT, CW_USEDEFAULT, 500, 395,
                                       estado.propietario, nullptr, GetModuleHandleW(nullptr), &estado);
        if (!ventana) { EnableWindow(estado.propietario, TRUE); return false; }
        RECT padre{}, dialogo{};
        GetWindowRect(estado.propietario, &padre);
        GetWindowRect(ventana, &dialogo);
        SetWindowPos(ventana, nullptr,
                     padre.left + ((padre.right - padre.left) - (dialogo.right - dialogo.left)) / 2,
                     padre.top + ((padre.bottom - padre.top) - (dialogo.bottom - dialogo.top)) / 2,
                     0, 0, SWP_NOSIZE | SWP_NOZORDER);
        ShowWindow(ventana, SW_SHOW);
        UpdateWindow(ventana);
        MSG mensaje{};
        while (IsWindow(ventana) && GetMessageW(&mensaje, nullptr, 0, 0) > 0) {
            if (!IsDialogMessageW(ventana, &mensaje)) {
                TranslateMessage(&mensaje);
                DispatchMessageW(&mensaje);
            }
        }
        EnableWindow(estado.propietario, TRUE);
        SetForegroundWindow(estado.propietario);
        return estado.aceptado;
    }

    static long long Ahora() {
        FILETIME ahora{};
        GetSystemTimeAsFileTime(&ahora);
        ULARGE_INTEGER ticks{};
        ticks.LowPart = ahora.dwLowDateTime;
        ticks.HighPart = ahora.dwHighDateTime;
        constexpr ULONGLONG kEpochUnix = 116444736000000000ULL;
        return static_cast<long long>((ticks.QuadPart > kEpochUnix ? ticks.QuadPart - kEpochUnix : 0) / 10000000ULL);
    }

    static std::wstring Escapar(const std::wstring& valor) {
        std::wstring salida;
        salida.reserve(valor.size() + 16);
        for (wchar_t caracter : valor) {
            switch (caracter) {
                case L'\\': salida += L"\\\\"; break;
                case L'|': salida += L"\\p"; break;
                case L';': salida += L"\\s"; break;
                case L'\n': salida += L"\\n"; break;
                case L'\r': break;
                default: salida += caracter; break;
            }
        }
        return salida;
    }

    static std::wstring Desescapar(const std::wstring& valor) {
        std::wstring salida;
        for (std::size_t i = 0; i < valor.size(); ++i) {
            if (valor[i] != L'\\' || i + 1 >= valor.size()) { salida += valor[i]; continue; }
            const wchar_t codigo = valor[++i];
            if (codigo == L'n') salida += L'\n';
            else if (codigo == L'p') salida += L'|';
            else if (codigo == L's') salida += L';';
            else if (codigo == L'\\') salida += L'\\';
            else { salida += L'\\'; salida += codigo; }
        }
        return salida;
    }

    static std::vector<std::wstring> Separar(const std::wstring& valor, wchar_t separador) {
        std::vector<std::wstring> partes;
        std::size_t inicio = 0;
        while (inicio <= valor.size()) {
            const std::size_t fin = valor.find(separador, inicio);
            partes.push_back(valor.substr(inicio, fin == std::wstring::npos ? fin : fin - inicio));
            if (fin == std::wstring::npos) break;
            inicio = fin + 1;
        }
        return partes;
    }

    static int AEntero(const std::wstring& valor, int predeterminado) {
        if (valor.empty()) return predeterminado;
        wchar_t* fin = nullptr;
        const long resultado = wcstol(valor.c_str(), &fin, 10);
        if (!fin || *fin != L'\0' || resultado < 0 || resultado > 2147483647L) return predeterminado;
        return static_cast<int>(resultado);
    }

    static std::wstring ClaveNota(int id) { return L"nota." + std::to_wstring(id); }

    static std::wstring Serializar(const NotaV2& nota) {
        return std::to_wstring(std::max(0LL, nota.modificado)) + L"|" +
               (nota.fuente == FuenteNotaV2::WinKlok ? L"w" : L"n") + L"|" +
               std::to_wstring(std::max(0, nota.idWinKlok)) + L"|" + Escapar(nota.titulo) + L"|" + Escapar(nota.cuerpo);
    }

    static bool Deserializar(const std::wstring& valor, int id, NotaV2& nota) {
        const auto campos = Separar(valor, L'|');
        if (campos.size() < 3) return false;
        nota = {};
        nota.id = id;
        nota.modificado = static_cast<long long>(AEntero(campos[0], 0));
        if (campos.size() >= 5) {
            nota.fuente = campos[1] == L"w" ? FuenteNotaV2::WinKlok : FuenteNotaV2::Nativa;
            nota.idWinKlok = AEntero(campos[2], 0);
            nota.titulo = Desescapar(campos[3]);
            nota.cuerpo = Desescapar(campos[4]);
        } else {
            nota.fuente = FuenteNotaV2::Nativa;
            nota.titulo = Desescapar(campos[1]);
            nota.cuerpo = Desescapar(campos[2]);
        }
        return nota.modificado >= 0;
    }

    NotaV2* BuscarMutable(int id) {
        const auto it = std::find_if(m_notas.begin(), m_notas.end(), [id](const auto& nota) { return nota.id == id; });
        return it == m_notas.end() ? nullptr : &*it;
    }

    void OrdenarBloqueado() {
        std::stable_sort(m_notas.begin(), m_notas.end(), [](const auto& a, const auto& b) {
            return a.modificado > b.modificado;
        });
    }

    void CargarSiHaceFaltaBloqueado() {
        if (m_cargado) return;
        CargarNativas();
        m_cargado = true;
        ActualizarPuenteWinKlokBloqueado();
    }

    bool SincronizacionActiva() const {
        return m_politicaSincronizacion && m_politicaSincronizacion();
    }

    void ActualizarPuenteWinKlokBloqueado() {
        if (SincronizacionActiva()) {
            SincronizarBloqueado(false);
            return;
        }

        // Al desactivar la opción, las notas importadas dejan de formar parte
        // del modelo local y las nativas pierden el vínculo externo. Así el
        // modo predeterminado queda completamente atómico y reversible sin
        // escribir ni borrar datos del otro mod.
        bool cambio = !m_eliminadosWinKlok.empty();
        for (auto it = m_notas.begin(); it != m_notas.end();) {
            if (it->fuente == FuenteNotaV2::WinKlok) {
                BorrarCadenaFragmentada(ClaveNota(it->id));
                it = m_notas.erase(it);
                cambio = true;
                continue;
            }
            if (it->idWinKlok != 0) {
                it->idWinKlok = 0;
                cambio = true;
            }
            ++it;
        }
        m_eliminadosWinKlok.clear();
        m_winKlokDisponible = false;
        if (cambio) {
            OrdenarBloqueado();
            GuardarNativasBloqueado();
        }
    }

    void CargarNativas() {
        m_notas.clear();
        m_eliminadosWinKlok.clear();
        if (!m_persistencia) return;

        const std::wstring indice = m_persistencia->LeerCadena(m_persistencia->CrearClave(L"notas", L"index"));
        int maxId = 0;
        for (const std::wstring& token : Separar(indice, L',')) {
            const int id = AEntero(token, 0);
            if (id <= 0) continue;
            const std::wstring valor = LeerCadenaFragmentada(ClaveNota(id));
            NotaV2 nota{};
            if (!Deserializar(valor, id, nota)) continue;
            m_notas.push_back(std::move(nota));
            maxId = std::max(maxId, id);
        }
        m_siguienteId = std::max(maxId + 1, m_persistencia->LeerEntero(m_persistencia->CrearClave(L"notas", L"next"), 1));
        const std::wstring eliminados = m_persistencia->LeerCadena(m_persistencia->CrearClave(L"notas", L"deleted-winklok"));
        for (const std::wstring& token : Separar(eliminados, L',')) {
            const int id = AEntero(token, 0);
            if (id > 0) m_eliminadosWinKlok.push_back(id);
        }
        OrdenarBloqueado();
    }

    void GuardarNativasBloqueado() const {
        if (!m_persistencia) return;
        std::wstring indice;
        for (const auto& nota : m_notas) {
            if (!indice.empty()) indice += L',';
            indice += std::to_wstring(nota.id);
            GuardarCadenaFragmentada(ClaveNota(nota.id), Serializar(nota));
        }
        m_persistencia->GuardarCadena(m_persistencia->CrearClave(L"notas", L"index"), indice);
        m_persistencia->GuardarEntero(m_persistencia->CrearClave(L"notas", L"next"), std::max(1, m_siguienteId));
        std::wstring eliminados;
        for (int id : m_eliminadosWinKlok) {
            if (!eliminados.empty()) eliminados += L',';
            eliminados += std::to_wstring(id);
        }
        m_persistencia->GuardarCadena(m_persistencia->CrearClave(L"notas", L"deleted-winklok"), eliminados);
    }

    void BorrarCadenaFragmentada(const std::wstring& clave) const {
        if (!m_persistencia) return;
        const int cantidad = m_persistencia->LeerEntero(
            m_persistencia->CrearClave(L"notas", (clave + L".n").c_str()), 0);
        if (cantidad > 0 && cantidad <= AlmacenNotasWinKlokV2::MaximoFragmentos()) {
            for (int i = 0; i < cantidad; ++i) {
                m_persistencia->Eliminar(m_persistencia->CrearClave(
                    L"notas", (clave + L"." + std::to_wstring(i)).c_str()));
            }
        }
        m_persistencia->Eliminar(m_persistencia->CrearClave(L"notas", (clave + L".n").c_str()));
        m_persistencia->Eliminar(m_persistencia->CrearClave(L"notas", clave.c_str()));
    }

    std::wstring LeerCadenaFragmentada(const std::wstring& clave) const {
        if (!m_persistencia) return {};
        const int cantidad = m_persistencia->LeerEntero(m_persistencia->CrearClave(L"notas", (clave + L".n").c_str()), 0);
        if (cantidad <= 0 || cantidad > AlmacenNotasWinKlokV2::MaximoFragmentos()) {
            return m_persistencia->LeerCadena(m_persistencia->CrearClave(L"notas", clave.c_str()));
        }
        std::wstring salida;
        for (int i = 0; i < cantidad; ++i) {
            salida += m_persistencia->LeerCadena(m_persistencia->CrearClave(L"notas", (clave + L"." + std::to_wstring(i)).c_str()));
        }
        return salida;
    }

    void GuardarCadenaFragmentada(const std::wstring& clave, const std::wstring& valor) const {
        if (!m_persistencia) return;
        const int anterior = m_persistencia->LeerEntero(m_persistencia->CrearClave(L"notas", (clave + L".n").c_str()), 0);
        int cantidad = 0;
        if (valor.empty()) {
            m_persistencia->GuardarCadena(m_persistencia->CrearClave(L"notas", (clave + L".0").c_str()), L"");
        }
        for (std::size_t inicio = 0; inicio < valor.size(); inicio += AlmacenNotasWinKlokV2::TamanoFragmento()) {
            m_persistencia->GuardarCadena(m_persistencia->CrearClave(L"notas", (clave + L"." + std::to_wstring(cantidad)).c_str()),
                                           valor.substr(inicio, AlmacenNotasWinKlokV2::TamanoFragmento()));
            ++cantidad;
        }
        if (cantidad == 0) cantidad = 1;
        for (int i = cantidad; i < anterior; ++i) {
            m_persistencia->Eliminar(m_persistencia->CrearClave(L"notas", (clave + L"." + std::to_wstring(i)).c_str()));
        }
        m_persistencia->GuardarEntero(m_persistencia->CrearClave(L"notas", (clave + L".n").c_str()), cantidad);
        m_persistencia->Eliminar(m_persistencia->CrearClave(L"notas", clave.c_str()));
    }

    ResultadoSincronizacionNotasV2 SincronizarBloqueado(bool forzar) {
        ResultadoSincronizacionNotasV2 resultado{};
        std::vector<NotaWinKlokV2> externas;
        int siguienteExterno = 1;
        if (!m_almacenWinKlok.Leer(externas, siguienteExterno, forzar)) {
            m_winKlokDisponible = false;
            if (!forzar) {
                resultado.mensaje = L"WinKlok no está disponible; las notas nativas quedaron guardadas.";
                return resultado;
            }
            resultado.mensaje = L"WinKlok no está instalado o su almacén no está disponible; las notas nativas quedaron guardadas.";
            return resultado;
        }
        resultado.disponible = true;
        m_winKlokDisponible = true;

        std::vector<int> idsEliminar = m_eliminadosWinKlok;
        auto externa = [&externas](int id) -> NotaWinKlokV2* {
            const auto it = std::find_if(externas.begin(), externas.end(), [id](const auto& nota) { return nota.id == id; });
            return it == externas.end() ? nullptr : &*it;
        };

        for (auto it = m_notas.begin(); it != m_notas.end();) {
            auto& nota = *it;
            if (nota.idWinKlok <= 0) {
                nota.idWinKlok = siguienteExterno++;
            }
            NotaWinKlokV2* espejo = externa(nota.idWinKlok);
            if (!espejo) {
                if (nota.fuente == FuenteNotaV2::WinKlok) {
                    BorrarCadenaFragmentada(ClaveNota(nota.id));
                    it = m_notas.erase(it);
                    continue;
                }
                externas.push_back({nota.idWinKlok, nota.titulo, nota.cuerpo, nota.modificado});
                ++resultado.exportadas;
            } else if (espejo->modificado > nota.modificado) {
                nota.titulo = espejo->titulo;
                nota.cuerpo = espejo->cuerpo;
                nota.modificado = espejo->modificado;
                ++resultado.importadas;
            } else if (nota.modificado > espejo->modificado || nota.fuente == FuenteNotaV2::Nativa) {
                espejo->titulo = nota.titulo;
                espejo->cuerpo = nota.cuerpo;
                espejo->modificado = nota.modificado;
                ++resultado.exportadas;
            }
            ++it;
        }

        for (const auto& externaNota : externas) {
            if (std::find(idsEliminar.begin(), idsEliminar.end(), externaNota.id) != idsEliminar.end()) continue;
            const bool conocida = std::any_of(m_notas.begin(), m_notas.end(), [&](const auto& nota) {
                return nota.idWinKlok == externaNota.id;
            });
            if (conocida) continue;
            NotaV2 importada{};
            importada.id = m_siguienteId++;
            importada.titulo = externaNota.titulo;
            importada.cuerpo = externaNota.cuerpo;
            importada.modificado = externaNota.modificado;
            importada.fuente = FuenteNotaV2::WinKlok;
            importada.idWinKlok = externaNota.id;
            m_notas.push_back(std::move(importada));
            ++resultado.importadas;
        }

        const bool guardadoExterno = m_almacenWinKlok.Guardar(externas, siguienteExterno, idsEliminar);
        if (!guardadoExterno) {
            resultado.mensaje = L"No se pudo escribir el almacén de WinKlok; las notas nativas quedaron guardadas.";
            GuardarNativasBloqueado();
            return resultado;
        }
        m_eliminadosWinKlok.clear();
        OrdenarBloqueado();
        GuardarNativasBloqueado();
        resultado.realizada = true;
        resultado.mensaje = resultado.importadas || resultado.exportadas
            ? L"Sincronización completada con WinKlok."
            : L"Las notas ya estaban sincronizadas con WinKlok.";
        return resultado;
    }

    std::shared_ptr<ServicioPersistenciaWindhawkV2> m_persistencia;
    std::function<bool()> m_politicaSincronizacion;
    mutable std::mutex m_mutex;
    mutable AlmacenNotasWinKlokV2 m_almacenWinKlok;
    std::vector<NotaV2> m_notas;
    std::vector<int> m_eliminadosWinKlok;
    int m_siguienteId{1};
    bool m_cargado{false};
    bool m_winKlokDisponible{false};
};

} // namespace SideViewV2
/**** ended inlining v2/nucleo/servicio_notas.hpp ****/
/**** start inlining v2/nucleo/configuracion_v2.hpp ****/

#include <algorithm>
#include <atomic>

namespace SideViewV2 {

// Interruptor de seguridad para la funcionalidad experimental. Mantenerlo
// aislado permite desactivar el flujo completo sin acoplarlo a las vistas.
inline std::atomic_bool g_comparadorVideosExperimentalV2{true};
inline std::atomic_bool g_comparadorVideosPantallaCompletaV2{true};
inline std::atomic_bool g_mostrarVersionModV2{true};
inline std::atomic_bool g_modoSemanticoDiffViewV2{true};
inline std::atomic_bool g_sincronizacionNotasWinKlokV2{false};
inline std::atomic<int> g_tamanoFuenteTextoV2{10};
inline std::atomic<int> g_tamanoFuenteCodigoV2{10};
inline std::atomic_bool g_ajusteLineaTextoCodigoV2{true};
inline std::atomic_bool g_reproductorVideoEnVistaV2{false};

inline int NormalizarTamanoFuenteV2(int valor) {
    // Un valor 0 puede aparecer si se instala una versión que todavía no
    // tenía esta opción; conservar el valor inicial evita una fuente
    // accidentalmente diminuta en esa migración.
    return valor <= 0 ? 10 : std::clamp(valor, 8, 18);
}

inline bool ComparadorVideosExperimentalDisponibleV2() {
    return g_comparadorVideosExperimentalV2.load(std::memory_order_acquire);
}

inline bool ComparadorVideosPantallaCompletaDisponibleV2() {
    return g_comparadorVideosPantallaCompletaV2.load(std::memory_order_acquire);
}

inline bool MostrarVersionModDisponibleV2() {
    return g_mostrarVersionModV2.load(std::memory_order_acquire);
}

inline bool ModoSemanticoDiffViewDisponibleV2() {
    return g_modoSemanticoDiffViewV2.load(std::memory_order_acquire);
}

inline bool SincronizacionNotasWinKlokDisponibleV2() {
    return g_sincronizacionNotasWinKlokV2.load(std::memory_order_acquire);
}

inline int TamanoFuenteTextoV2() {
    return NormalizarTamanoFuenteV2(g_tamanoFuenteTextoV2.load(std::memory_order_acquire));
}

inline int TamanoFuenteCodigoV2() {
    return NormalizarTamanoFuenteV2(g_tamanoFuenteCodigoV2.load(std::memory_order_acquire));
}

inline bool AjusteLineaTextoCodigoDisponibleV2() {
    return g_ajusteLineaTextoCodigoV2.load(std::memory_order_acquire);
}

inline bool ReproductorVideoEnVistaDisponibleV2() {
    return g_reproductorVideoEnVistaV2.load(std::memory_order_acquire);
}

} // namespace SideViewV2
/**** ended inlining v2/nucleo/configuracion_v2.hpp ****/
#include <memory>
namespace SideViewV2 {
struct DependenciasVistaV2 {
    std::shared_ptr<ServicioNavegacionV2> navegacion{std::make_shared<ServicioNavegacionV2>()};
    std::shared_ptr<ServicioExtensionV2> extensiones{std::make_shared<ServicioExtensionV2>()};
    std::shared_ptr<ServicioPersistenciaWindhawkV2> persistencia{std::make_shared<ServicioPersistenciaWindhawkV2>()};
    std::shared_ptr<ServicioVersionModV2> versionMod{std::make_shared<ServicioVersionModV2>()};
    std::shared_ptr<ServicioCompatibilidadGpuV2> compatibilidadGpu{std::make_shared<ServicioCompatibilidadGpuV2>()};
    std::shared_ptr<ServicioNotasV2> notas{std::make_shared<ServicioNotasV2>(
        persistencia, [] { return SincronizacionNotasWinKlokDisponibleV2(); })};
    std::shared_ptr<ServicioAbrirConV2> abrirConCarpetas{std::make_shared<ServicioAbrirConV2>(persistencia, L"carpetas", std::vector<std::wstring>{L"code.exe",L"explorer.exe",L"cmd.exe",L"powershell.exe"})};
    std::shared_ptr<ServicioAbrirConV2> abrirConComprimidos{std::make_shared<ServicioAbrirConV2>(persistencia, L"comprimidos")};
    std::shared_ptr<ServicioAbrirConV2> abrirConImagenes{std::make_shared<ServicioAbrirConV2>(persistencia, L"imagenes")};
    std::shared_ptr<ServicioAbrirConV2> abrirConVideos{std::make_shared<ServicioAbrirConV2>(persistencia, L"videos")};
    std::shared_ptr<ServicioTextoV2> texto{std::make_shared<ServicioTextoV2>()};
    std::shared_ptr<ServicioComparadorArchivosV2> comparadorArchivos{std::make_shared<ServicioComparadorArchivosV2>()};
    std::shared_ptr<ServicioComparadorTextoV2> comparadorTexto{std::make_shared<ServicioComparadorTextoV2>(texto)};
    std::shared_ptr<ServicioComparadorDocumentosV2> comparadorDocumentos{std::make_shared<ServicioComparadorDocumentosV2>()};
    std::shared_ptr<ServicioContenidoComprimidoV2> contenidoComprimido{std::make_shared<ServicioContenidoComprimidoV2>()};
    std::shared_ptr<ServicioEnlacesV2> enlaces{std::make_shared<ServicioEnlacesV2>()};
    std::shared_ptr<ServicioPrevisualizacionMediaV2> previsualizacionMedia{std::make_shared<ServicioPrevisualizacionMediaV2>()};
    std::shared_ptr<ServicioFusionCarpetasV2> fusionCarpetas{std::make_shared<ServicioFusionCarpetasV2>()};
    std::shared_ptr<ServicioInspeccionCarpetasV2> inspeccionCarpetas{std::make_shared<ServicioInspeccionCarpetasV2>()};
    std::shared_ptr<ServicioReproductorVideoV2> reproductorVideo{std::make_shared<ServicioReproductorVideoV2>()};
    std::shared_ptr<ServicioComparadorVideosV2> comparadorVideos{std::make_shared<ServicioComparadorVideosV2>(reproductorVideo)};
};
}
/**** ended inlining v2/nucleo/servicios_vistas.hpp ****/
#include <windows.h>
#include <string>
#include <vector>
#include <algorithm>

namespace SideViewV2 {

class VistaComprimidos : public IPanelContenido {
public:
    explicit VistaComprimidos(std::shared_ptr<ServicioAbrirConV2> servicio,
                              std::shared_ptr<ServicioNavegacionV2> navegacion,
                              std::shared_ptr<ServicioExtensionV2> extensiones,
                              std::shared_ptr<ServicioContenidoComprimidoV2> contenido)
        : m_servicio(std::move(servicio)), m_navegacion(std::move(navegacion)),
          m_extensiones(std::move(extensiones)), m_contenido(std::move(contenido)) {
        m_formatosDisponibles = { L"zip", L"rar", L"7z", L"tar", L"cbz", L"cbr" };
        m_formatoSeleccionado = L"zip";
    }

    void AlCambiarSeleccion(const std::vector<std::wstring>& archivos, const std::wstring&) override {
        m_archivos = archivos;
        m_indiceArchivo = 0;
        m_desplazamientoScrollY = 0;
        CargarArchivoActual();
    }

    bool ProcesarMensajeRaton(UINT msg, WPARAM wParam, LPARAM lParam, HWND hwndPanel) override {
        POINT pt{GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};

        if (msg == WM_MOUSEWHEEL) {
            short delta = GET_WHEEL_DELTA_WPARAM(wParam);
            int incremento = (delta > 0 ? -40 : 40);
            int maxScroll = std::max(0, static_cast<int>(m_entradas.size() * 26) - 200);
            m_desplazamientoScrollY = std::max(0, std::min(m_desplazamientoScrollY + incremento, maxScroll));
            InvalidateRect(hwndPanel, nullptr, FALSE);
            return true;
        }

        if (msg == WM_LBUTTONDOWN) {
            // Botón Anterior
            if (PtInRect(&m_rcBotonAnterior, pt)) {
                if (m_archivos.size() > 1) {
                    m_indiceArchivo = m_navegacion->Anterior(m_indiceArchivo, m_archivos.size());
                    CargarArchivoActual();
                    InvalidateRect(hwndPanel, nullptr, FALSE);
                    return true;
                }
            }
            // Botón Siguiente
            if (PtInRect(&m_rcBotonSiguiente, pt)) {
                if (m_archivos.size() > 1) {
                    m_indiceArchivo = m_navegacion->Siguiente(m_indiceArchivo, m_archivos.size());
                    CargarArchivoActual();
                    InvalidateRect(hwndPanel, nullptr, FALSE);
                    return true;
                }
            }
            // Botón Abrir Con
            if (PtInRect(&m_rcBotonAbrirCon, pt)) {
                EjecutarAbrirCon(hwndPanel);
                return true;
            }
            if (PtInRect(&m_rcBotonComparar, pt)) {
                if (PuedeCompararSeleccionV2(m_archivos)) {
                    PostMessageW(hwndPanel, WM_SIDEVIEW_REQUEST_COMPARADOR_ARCHIVOS, 0, 0);
                }
                return true;
            }
            // Botón Desplegable Formatos
            if (PtInRect(&m_rcBotonDesplegableFormato, pt)) {
                MostrarMenuFormatos(hwndPanel);
                return true;
            }
            // Botón Cambiar extensión
            if (PtInRect(&m_rcBotonCambiarExtension, pt)) {
                EjecutarConversion(hwndPanel);
                return true;
            }
        }

        return false;
    }

    void Renderizar(HDC hdc, const RECT& rect, const TemaVisual& tema) override {
        int ancho = rect.right - rect.left;
        int alto = rect.bottom - rect.top;
        if (ancho <= 0 || alto <= 0) return;

        // Fondo
        HBRUSH brochaFondo = CreateSolidBrush(tema.fondo);
        FillRect(hdc, &rect, brochaFondo);
        DeleteObject(brochaFondo);

        // Cabecera Fluent
        int altoCabecera = 54;
        RECT rectCabecera{rect.left, rect.top, rect.right, rect.top + altoCabecera};
        HBRUSH brochaCabecera = CreateSolidBrush(tema.fondoCabecera);
        FillRect(hdc, &rectCabecera, brochaCabecera);
        DeleteObject(brochaCabecera);

        // Bordes separadores
        HPEN plumaBorde = CreatePen(PS_SOLID, 1, tema.borde);
        HPEN plumaVieja = static_cast<HPEN>(SelectObject(hdc, plumaBorde));
        MoveToEx(hdc, rect.left, rect.top + altoCabecera, nullptr);
        LineTo(hdc, rect.right, rect.top + altoCabecera);
        MoveToEx(hdc, rect.left, rect.top, nullptr);
        LineTo(hdc, rect.left, rect.bottom);
        SelectObject(hdc, plumaVieja);
        DeleteObject(plumaBorde);

        SetBkMode(hdc, TRANSPARENT);

        HFONT fuenteTitulo = CreateFontW(-16, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE,
                                         DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                         CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI Variable Display");
        HFONT fuenteSubtitulo = CreateFontW(-12, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                                            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                            CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI Variable Text");
        HFONT fuenteBold = CreateFontW(-13, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                                       DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                       CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI Variable Text");
        HFONT fuenteVieja = static_cast<HFONT>(SelectObject(hdc, fuenteTitulo));

        // Título de vista
        SetTextColor(hdc, tema.textoPrincipal);
        std::wstring nombreArchivoActual = L"Sin archivos comprimidos";
        if (!m_archivos.empty() && m_indiceArchivo < m_archivos.size()) {
            const wchar_t* pName = PathFindFileNameW(m_archivos[m_indiceArchivo].c_str());
            nombreArchivoActual = pName ? pName : m_archivos[m_indiceArchivo];
        }

        RECT rectTextoTitulo{rect.left + 16, rect.top + 8, rect.right - 16, rect.top + 28};
        std::wstring strCabecera = L"📦 " + nombreArchivoActual;
        DrawTextW(hdc, strCabecera.c_str(), -1, &rectTextoTitulo,
                  DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS | DT_NOPREFIX);

        // Subtítulo con contador
        SelectObject(hdc, fuenteSubtitulo);
        SetTextColor(hdc, tema.textoSecundario);
        std::wstring strSub = L"Archivo " + std::to_wstring(m_archivos.empty() ? 0 : m_indiceArchivo + 1) +
                              L" de " + std::to_wstring(m_archivos.size()) + L" • Suite de Comprimidos & Cómics";
        RECT rectTextoSub{rect.left + 16, rect.top + 28, rect.right - 16, rect.top + 46};
        DrawTextW(hdc, strSub.c_str(), -1, &rectTextoSub,
                  DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS | DT_NOPREFIX);

        // Barra superior: navegación y conversión.
        int margen = 14;
        int yBarra = rect.top + altoCabecera + 12;
        int altoBotones = 30;

        // Botón Anterior
        m_rcBotonAnterior = {rect.left + margen, yBarra, rect.left + margen + 32, yBarra + altoBotones};
        DibujarBoton(hdc, m_rcBotonAnterior, L"◀", tema, m_archivos.size() > 1);

        // Botón Siguiente
        m_rcBotonSiguiente = {m_rcBotonAnterior.right + 6, yBarra, m_rcBotonAnterior.right + 38, yBarra + altoBotones};
        DibujarBoton(hdc, m_rcBotonSiguiente, L"▶", tema, m_archivos.size() > 1);

        // Selector de Formato (Drop)
        m_rcBotonDesplegableFormato = {m_rcBotonSiguiente.right + 10, yBarra, m_rcBotonSiguiente.right + 85, yBarra + altoBotones};
        DibujarBoton(hdc, m_rcBotonDesplegableFormato, (m_formatoSeleccionado + L" ▾").c_str(), tema, true);

        // Operación de cambio de extensión
        m_rcBotonCambiarExtension = {m_rcBotonDesplegableFormato.right + 6, yBarra, rect.right - margen, yBarra + altoBotones};
        DibujarBoton(hdc, m_rcBotonCambiarExtension, L"Cambiar extensión", tema, true);

        int yAbrir = rect.bottom - margen - altoBotones;
        m_rcBotonAbrirCon = {rect.left + margen, yAbrir, rect.right - margen, yAbrir + altoBotones};
        std::wstring programa = m_servicio->ProgramaActual();
        std::wstring etiqueta = L"Abrir con: ";
        etiqueta += programa.empty() ? L"(click para seleccionar programa) ▾" : PathFindFileNameW(programa.c_str());
        DibujarBoton(hdc, m_rcBotonAbrirCon, etiqueta.c_str(), tema, true);

        int yComparar = yAbrir - altoBotones - 8;
        m_rcBotonComparar = {rect.left + margen, yComparar,
                             rect.left + margen + 126, yComparar + altoBotones};
        DibujarBoton(hdc, m_rcBotonComparar, L"Comparar archivos", tema,
                     PuedeCompararSeleccionV2(m_archivos));

        // Área de Contenido: Árbol de archivos comprimidos
        int topLista = yBarra + altoBotones + 14;
        RECT rcContenedorLista{rect.left + margen, topLista, rect.right - margen, yComparar - 12};
        DibujarContenedor(hdc, rcContenedorLista, tema);

        // Header de la lista
        RECT rcHeaderLista{rcContenedorLista.left, rcContenedorLista.top, rcContenedorLista.right, rcContenedorLista.top + 28};
        HBRUSH brHeaderL = CreateSolidBrush(tema.fondoCabecera);
        FillRect(hdc, &rcHeaderLista, brHeaderL);
        DeleteObject(brHeaderL);

        SelectObject(hdc, fuenteBold);
        SetTextColor(hdc, tema.acento);
        RECT rcColNombre{rcHeaderLista.left + 10, rcHeaderLista.top, rcHeaderLista.right - 90, rcHeaderLista.bottom};
        DrawTextW(hdc, L"Nombre del elemento", -1, &rcColNombre, DT_SINGLELINE | DT_VCENTER | DT_NOPREFIX);

        RECT rcColTamano{rcHeaderLista.right - 85, rcHeaderLista.top, rcHeaderLista.right - 10, rcHeaderLista.bottom};
        DrawTextW(hdc, L"Tamaño", -1, &rcColTamano, DT_SINGLELINE | DT_RIGHT | DT_VCENTER | DT_NOPREFIX);

        // Renderizado de las filas
        SelectObject(hdc, fuenteSubtitulo);
        SetTextColor(hdc, tema.textoPrincipal);

        int yFila = rcHeaderLista.bottom + 4 - m_desplazamientoScrollY;
        int altoFila = 24;

        for (const auto& entrada : m_entradas) {
            if (yFila + altoFila >= rcHeaderLista.bottom && yFila <= rcContenedorLista.bottom - 4) {
                std::wstring iconoItem = entrada.esDirectorio ? L"📁 " : L"📄 ";
                std::wstring textoFila = iconoItem + entrada.nombre;

                RECT rcFilaNom{rcContenedorLista.left + 10, yFila, rcContenedorLista.right - 90, yFila + altoFila};
                DrawTextW(hdc, textoFila.c_str(), -1, &rcFilaNom, DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS | DT_NOPREFIX);

                std::wstring strTam = entrada.esDirectorio ? L"-" : FormatearTamano(entrada.tamanoBytes);
                RECT rcFilaTam{rcContenedorLista.right - 85, yFila, rcContenedorLista.right - 10, yFila + altoFila};
                DrawTextW(hdc, strTam.c_str(), -1, &rcFilaTam, DT_SINGLELINE | DT_RIGHT | DT_VCENTER | DT_NOPREFIX);
            }
            yFila += altoFila;
        }

        if (m_entradas.empty()) {
            RECT rcVacio{rcContenedorLista.left, rcHeaderLista.bottom, rcContenedorLista.right, rcContenedorLista.bottom};
            SetTextColor(hdc, tema.textoSecundario);
            DrawTextW(hdc, L"(Estructura no indexada o cómic con visor de portada)", -1, &rcVacio,
                      DT_SINGLELINE | DT_CENTER | DT_VCENTER | DT_NOPREFIX);
        }

        SelectObject(hdc, fuenteVieja);
        DeleteObject(fuenteTitulo);
        DeleteObject(fuenteSubtitulo);
        DeleteObject(fuenteBold);
    }

private:
    void CargarArchivoActual() {
        m_entradas.clear();
        if (m_archivos.empty() || m_indiceArchivo >= m_archivos.size()) return;
        m_entradas = m_contenido->Cargar(m_archivos[m_indiceArchivo]);
    }

    void DibujarBoton(HDC hdc, const RECT& rc, const wchar_t* texto, const TemaVisual& tema, bool habilitado) {
        HBRUSH br = CreateSolidBrush(habilitado ? tema.botonFondo : tema.fondoCabecera);
        FillRect(hdc, &rc, br);
        DeleteObject(br);

        HPEN pen = CreatePen(PS_SOLID, 1, tema.borde);
        HPEN oldPen = static_cast<HPEN>(SelectObject(hdc, pen));
        HBRUSH nullBr = static_cast<HBRUSH>(GetStockObject(NULL_BRUSH));
        HBRUSH oldBr = static_cast<HBRUSH>(SelectObject(hdc, nullBr));
        RoundRect(hdc, rc.left, rc.top, rc.right, rc.bottom, 6, 6);
        SelectObject(hdc, oldBr);
        SelectObject(hdc, oldPen);
        DeleteObject(pen);

        SetTextColor(hdc, habilitado ? tema.textoPrincipal : tema.textoSecundario);
        RECT rcTexto = rc;
        DrawTextW(hdc, texto, -1, &rcTexto, DT_SINGLELINE | DT_CENTER | DT_VCENTER | DT_NOPREFIX);
    }

    void DibujarContenedor(HDC hdc, const RECT& rc, const TemaVisual& tema) {
        HBRUSH br = CreateSolidBrush(tema.fondoTarjeta);
        FillRect(hdc, &rc, br);
        DeleteObject(br);

        HPEN pen = CreatePen(PS_SOLID, 1, tema.borde);
        HPEN oldPen = static_cast<HPEN>(SelectObject(hdc, pen));
        HBRUSH nullBr = static_cast<HBRUSH>(GetStockObject(NULL_BRUSH));
        HBRUSH oldBr = static_cast<HBRUSH>(SelectObject(hdc, nullBr));
        RoundRect(hdc, rc.left, rc.top, rc.right, rc.bottom, 8, 8);
        SelectObject(hdc, oldBr);
        SelectObject(hdc, oldPen);
        DeleteObject(pen);
    }

    std::wstring FormatearTamano(uintmax_t bytes) {
        if (bytes < 1024) return std::to_wstring(bytes) + L" B";
        if (bytes < 1024 * 1024) {
            wchar_t b[32];
            swprintf_s(b, L"%.1f KB", static_cast<double>(bytes) / 1024.0);
            return b;
        }
        wchar_t b[32];
        swprintf_s(b, L"%.1f MB", static_cast<double>(bytes) / (1024.0 * 1024.0));
        return b;
    }

    void EjecutarAbrirCon(HWND hwndPadre) {
        std::wstring programa = m_servicio->ProgramaActual();
        if (!programa.empty()) {
            m_servicio->Seleccionar(programa);
            m_servicio->Ejecutar(m_archivos);
            return;
        }
        if (m_servicio->AbrirDialogo(hwndPadre)) {
            m_servicio->Ejecutar(m_archivos);
        }
    }

    void MostrarMenuFormatos(HWND hwndPadre) {
        HMENU hMenu = CreatePopupMenu();
        for (size_t i = 0; i < m_formatosDisponibles.size(); ++i) {
            UINT flags = MF_STRING;
            if (m_formatosDisponibles[i] == m_formatoSeleccionado) {
                flags |= MF_CHECKED;
            }
            AppendMenuW(hMenu, flags, i + 1, m_formatosDisponibles[i].c_str());
        }

        POINT ptMenu{m_rcBotonDesplegableFormato.left, m_rcBotonDesplegableFormato.bottom};
        ClientToScreen(hwndPadre, &ptMenu);
        int cmd = TrackPopupMenu(hMenu, TPM_LEFTALIGN | TPM_TOPALIGN | TPM_RETURNCMD, ptMenu.x, ptMenu.y, 0, hwndPadre, nullptr);
        DestroyMenu(hMenu);

        if (cmd > 0 && cmd <= static_cast<int>(m_formatosDisponibles.size())) {
            m_formatoSeleccionado = m_formatosDisponibles[static_cast<std::size_t>(cmd - 1)];
            InvalidateRect(hwndPadre, nullptr, FALSE);
        }
    }

    void EjecutarConversion(HWND hwndPadre) {
        size_t cantidad = m_extensiones->Cambiar(m_archivos, m_formatoSeleccionado);
        std::wstring mensaje = L"Se cambiaron " + std::to_wstring(cantidad) + L" archivo(s) a ." + m_formatoSeleccionado + L".";
        MessageBoxW(hwndPadre, mensaje.c_str(), L"Cambiar extensión", MB_OK | MB_ICONINFORMATION);
    }

    std::vector<std::wstring> m_archivos;
    size_t m_indiceArchivo{0};
    int m_desplazamientoScrollY{0};
    std::vector<EntradaComprimidoV2> m_entradas;

    std::wstring m_formatoSeleccionado;
    std::vector<std::wstring> m_formatosDisponibles;
    std::shared_ptr<ServicioAbrirConV2> m_servicio;
    std::shared_ptr<ServicioNavegacionV2> m_navegacion;
    std::shared_ptr<ServicioExtensionV2> m_extensiones;
    std::shared_ptr<ServicioContenidoComprimidoV2> m_contenido;

    RECT m_rcBotonAnterior{};
    RECT m_rcBotonSiguiente{};
    RECT m_rcBotonAbrirCon{};
    RECT m_rcBotonDesplegableFormato{};
    RECT m_rcBotonCambiarExtension{};
    RECT m_rcBotonComparar{};
};

} // namespace SideViewV2
/**** ended inlining v2/vistas/vista_comprimidos.hpp ****/
/**** start inlining v2/vistas/vista_carpetas.hpp ****/

/**** skipping file: v2/ui/panel_interfaz.hpp ****/
/**** skipping file: v2/ui/ventana_base.hpp ****/
/**** skipping file: v2/nucleo/servicios_vistas.hpp ****/
/**** skipping file: v2/nucleo/servicio_com.hpp ****/
/**** skipping file: v2/nucleo/configuracion_v2.hpp ****/
#include <windows.h>
#include <shellapi.h>
#include <shlobj.h>
#include <shobjidl.h>
#include <commdlg.h>
#include <string>
#include <vector>
#include <algorithm>
#include <atomic>
#include <mutex>

namespace SideViewV2 {

inline std::atomic<int> g_limiteCarpetasPesadasConfig{3};

struct PromptNombreCarpetaV2 {
    HWND ventanaPadre{nullptr};
    HWND campoTexto{nullptr};
    std::wstring resultado;
    bool terminado{false};
};

inline LRESULT CALLBACK ProcedimientoPromptNombreCarpetaV2(HWND hwnd, UINT mensaje,
                                                           WPARAM wParam, LPARAM lParam) {
    auto* estado = reinterpret_cast<PromptNombreCarpetaV2*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    if (mensaje == WM_NCCREATE) {
        auto* crear = reinterpret_cast<CREATESTRUCTW*>(lParam);
        estado = reinterpret_cast<PromptNombreCarpetaV2*>(crear->lpCreateParams);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(estado));
    }
    if (!estado) return DefWindowProcW(hwnd, mensaje, wParam, lParam);
    if (mensaje == WM_CREATE) {
        CreateWindowW(L"STATIC", L"Nombre de la carpeta unificada:", WS_CHILD | WS_VISIBLE,
                      14, 12, 280, 20, hwnd, nullptr, GetModuleHandleW(nullptr), nullptr);
        estado->campoTexto = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"Carpeta unificada",
                                             WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
                                             14, 36, 280, 24, hwnd, reinterpret_cast<HMENU>(101),
                                             GetModuleHandleW(nullptr), nullptr);
        CreateWindowW(L"BUTTON", L"Aceptar", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
                      134, 68, 76, 26, hwnd, reinterpret_cast<HMENU>(IDOK),
                      GetModuleHandleW(nullptr), nullptr);
        CreateWindowW(L"BUTTON", L"Cancelar", WS_CHILD | WS_VISIBLE,
                      218, 68, 76, 26, hwnd, reinterpret_cast<HMENU>(IDCANCEL),
                      GetModuleHandleW(nullptr), nullptr);
        SendMessageW(estado->campoTexto, EM_SETSEL, 0, -1);
        SetFocus(estado->campoTexto);
        return 0;
    }
    if (mensaje == WM_COMMAND && (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)) {
        if (LOWORD(wParam) == IDOK) {
            wchar_t texto[MAX_PATH]{};
            GetWindowTextW(estado->campoTexto, texto, ARRAYSIZE(texto));
            estado->resultado = texto;
            if (estado->resultado.empty()) return 0;
        }
        estado->terminado = true;
        EnableWindow(estado->ventanaPadre, TRUE);
        DestroyWindow(hwnd);
        return 0;
    }
    if (mensaje == WM_CLOSE) {
        estado->terminado = true;
        EnableWindow(estado->ventanaPadre, TRUE);
        DestroyWindow(hwnd);
        return 0;
    }
    return DefWindowProcW(hwnd, mensaje, wParam, lParam);
}

inline std::wstring SolicitarNombreCarpetaV2(HWND padre) {
    static std::once_flag registroClase;
    std::call_once(registroClase, [] {
        WNDCLASSW clase{};
        clase.lpfnWndProc = ProcedimientoPromptNombreCarpetaV2;
        clase.hInstance = GetModuleHandleW(nullptr);
        clase.hCursor = LoadCursorW(nullptr, IDC_ARROW);
        clase.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
        clase.lpszClassName = L"UsefulSideView_PromptNombreCarpeta_V2";
        RegisterClassW(&clase);
    });
    PromptNombreCarpetaV2 estado{padre};
    EnableWindow(padre, FALSE);
    HWND ventana = CreateWindowExW(WS_EX_DLGMODALFRAME, L"UsefulSideView_PromptNombreCarpeta_V2",
                                   L"Fusionar carpetas", WS_CAPTION | WS_SYSMENU,
                                   CW_USEDEFAULT, CW_USEDEFAULT, 320, 132, padre, nullptr,
                                   GetModuleHandleW(nullptr), &estado);
    if (!ventana) { EnableWindow(padre, TRUE); return {}; }
    RECT rcPadre{}, rcVentana{};
    GetWindowRect(padre, &rcPadre); GetWindowRect(ventana, &rcVentana);
    SetWindowPos(ventana, nullptr,
                 rcPadre.left + ((rcPadre.right - rcPadre.left) - (rcVentana.right - rcVentana.left)) / 2,
                 rcPadre.top + ((rcPadre.bottom - rcPadre.top) - (rcVentana.bottom - rcVentana.top)) / 2,
                 0, 0, SWP_NOSIZE | SWP_NOZORDER);
    ShowWindow(ventana, SW_SHOW); UpdateWindow(ventana);
    MSG mensaje{};
    while (!estado.terminado && GetMessageW(&mensaje, nullptr, 0, 0) > 0) {
        if (!IsDialogMessageW(ventana, &mensaje)) { TranslateMessage(&mensaje); DispatchMessageW(&mensaje); }
    }
    EnableWindow(padre, TRUE); SetForegroundWindow(padre);
    return estado.resultado;
}

class VistaCarpetas : public IPanelContenido {
public:
    explicit VistaCarpetas(std::shared_ptr<ServicioAbrirConV2> servicio,
                           std::shared_ptr<ServicioNavegacionV2> navegacion,
                           std::shared_ptr<ServicioFusionCarpetasV2> fusion,
                           std::shared_ptr<ServicioInspeccionCarpetasV2> inspeccion,
                           std::shared_ptr<ServicioVersionModV2> versionMod,
                           std::shared_ptr<ServicioNotasV2> notas)
        : m_servicio(std::move(servicio)), m_navegacion(std::move(navegacion)),
          m_fusion(std::move(fusion)), m_inspeccion(std::move(inspeccion)),
          m_versionMod(std::move(versionMod)), m_notas(std::move(notas)) {
        m_programasRecordados = m_servicio ? m_servicio->Programas()
                                           : std::vector<std::wstring>{};
        m_limiteCarpetasPesadas = std::max(1, std::min(g_limiteCarpetasPesadasConfig.load(), 100));
        if (m_notas) m_notas->Cargar();
    }

    void AlCambiarSeleccion(const std::vector<std::wstring>& archivos, const std::wstring&) override {
        m_carpetasSeleccionadas = archivos;
        m_indiceCarpeta = 0;
        m_desplazamientoScrollY = 0;
        m_scrollPesadasY = 0;
        m_nivelExpansionMax = 1; // 1 = Raíz, 2 = Nivel 1, 3 = Nivel 2, 4 = Todo
        m_limiteCarpetasPesadas = std::max(1, std::min(g_limiteCarpetasPesadasConfig.load(), 100));

        if (m_carpetasSeleccionadas.empty() && m_notas) {
            m_notas->Recargar();
            m_notaSeleccionada = 0;
            m_mostrarDetalleNota = false;
        }
        CargarCarpetaActual();
    }

    void ActualizarConfiguracion() override {
        int limiteAnterior = m_limiteCarpetasPesadas;
        m_limiteCarpetasPesadas = std::max(1, std::min(g_limiteCarpetasPesadasConfig.load(), 100));
        if (limiteAnterior != m_limiteCarpetasPesadas) CargarCarpetaActual();
        if (m_carpetasSeleccionadas.empty() && m_notas) m_notas->Recargar();
    }

    bool ProcesarMensajeRaton(UINT msg, WPARAM wParam, LPARAM lParam, HWND hwndPanel) override {
        POINT pt{GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};

        if (msg == WM_MOUSEWHEEL) {
            short delta = GET_WHEEL_DELTA_WPARAM(wParam);
            int incremento = (delta > 0 ? -36 : 36);

            // Si el cursor está sobre la caja de carpetas pesadas
            if (PtInRect(&m_rcCajaPesadas, pt)) {
                int totalMostrar = std::min<int>(m_limiteCarpetasPesadas, static_cast<int>(m_topCarpetasPesadas.size()));
                int altoTotalContenido = totalMostrar * 40;
                int altoVisible = m_rcCajaPesadas.bottom - m_rcCajaPesadas.top - 8;
                int maxScrollPesadas = std::max(0, altoTotalContenido - altoVisible);

                int nuevoScroll = std::max(0, std::min(m_scrollPesadasY + incremento, maxScrollPesadas));
                if (nuevoScroll != m_scrollPesadasY) {
                    m_scrollPesadasY = nuevoScroll;
                    InvalidateRect(hwndPanel, nullptr, FALSE);
                }
                return true;
            }

            // Scroll exacto y acotado sobre la estructura de archivos
            int elementosVisibles = 0;
            for (const auto& item : m_elementosArbol) {
                if (item.nivelJerarquia < m_nivelExpansionMax) {
                    elementosVisibles++;
                }
            }

            int altoTotalArbol = elementosVisibles * 22 + 12;
            int altoVisibleArbol = m_rcCajaArbol.bottom - m_rcCajaArbol.top - 12;
            int maxScrollArbol = std::max(0, altoTotalArbol - altoVisibleArbol);

            int nuevoScrollArbol = std::max(0, std::min(m_desplazamientoScrollY + incremento, maxScrollArbol));
            if (nuevoScrollArbol != m_desplazamientoScrollY) {
                m_desplazamientoScrollY = nuevoScrollArbol;
                InvalidateRect(hwndPanel, nullptr, FALSE);
            }
            return true;
        }

        if (msg == WM_LBUTTONDOWN) {
            if (m_fusionEnCurso &&
                (PtInRect(&m_rcBotonFusionCarpetas, pt) || PtInRect(&m_rcBotonEliminarVacias, pt))) {
                return true;
            }
            // Botón Anterior
            if (PtInRect(&m_rcBotonAnterior, pt)) {
                if (m_carpetasSeleccionadas.size() > 1) {
                    m_indiceCarpeta = m_navegacion->Anterior(m_indiceCarpeta, m_carpetasSeleccionadas.size());
                    CargarCarpetaActual();
                    InvalidateRect(hwndPanel, nullptr, FALSE);
                    return true;
                }
            }
            // Botón Siguiente
            if (PtInRect(&m_rcBotonSiguiente, pt)) {
                if (m_carpetasSeleccionadas.size() > 1) {
                    m_indiceCarpeta = m_navegacion->Siguiente(m_indiceCarpeta, m_carpetasSeleccionadas.size());
                    CargarCarpetaActual();
                    InvalidateRect(hwndPanel, nullptr, FALSE);
                    return true;
                }
            }
            // Botón Alternar Expansión de Niveles
            if (PtInRect(&m_rcBotonExpansion, pt)) {
                m_nivelExpansionMax = (m_nivelExpansionMax >= 4) ? 1 : (m_nivelExpansionMax + 1);
                m_desplazamientoScrollY = 0;
                InvalidateRect(hwndPanel, nullptr, FALSE);
                return true;
            }
            // Botón Selector de Cantidad de Carpetas Pesadas
            if (PtInRect(&m_rcBotonLimitePesadas, pt)) {
                MostrarMenuLimitePesadas(hwndPanel);
                return true;
            }
            // Selector Desplegable de Programas
            if (PtInRect(&m_rcSelectorPrograma, pt)) {
                MostrarMenuDesplegableProgramas(hwndPanel);
                return true;
            }
            // Botón Ejecutar Abrir con
            if (PtInRect(&m_rcBotonEjecutarAbrir, pt)) {
                EjecutarAbrirCon(hwndPanel);
                return true;
            }
            // Botón Crear Enlace Simbólico
            if (PtInRect(&m_rcBotonCrearEnlace, pt)) {
                CrearEnlaceSimbolico(hwndPanel);
                return true;
            }
            // Botón Copiar Contenido al Portapapeles
            if (PtInRect(&m_rcBotonCopiarContenido, pt)) {
                CopiarListaAlPortapapeles(hwndPanel);
                return true;
            }
            if (m_carpetasSeleccionadas.empty() && m_notas) {
                if (PtInRect(&m_rcBotonNuevaNota, pt)) {
                    if (m_notas->EditarConPrompt(hwndPanel, 0, true)) {
                        m_notas->Recargar();
                        m_mostrarDetalleNota = false;
                        m_notaSeleccionada = m_notas->Notas().empty() ? 0 : m_notas->Notas().front().id;
                        InvalidateRect(hwndPanel, nullptr, FALSE);
                    }
                    return true;
                }
                if (PtInRect(&m_rcBotonSincronizarNotas, pt)) {
                    const ResultadoSincronizacionNotasV2 resultado = m_notas->ForzarSincronizacion();
                    m_notaSeleccionada = 0;
                    m_mostrarDetalleNota = false;
                    MessageBoxW(hwndPanel, resultado.mensaje.c_str(), L"Useful Side View - Notas", MB_OK |
                                (resultado.realizada ? MB_ICONINFORMATION : MB_ICONWARNING));
                    InvalidateRect(hwndPanel, nullptr, FALSE);
                    return true;
                }
                if (m_mostrarDetalleNota) {
                    if (PtInRect(&m_rcBotonVolverNotas, pt)) {
                        m_mostrarDetalleNota = false;
                        InvalidateRect(hwndPanel, nullptr, FALSE);
                        return true;
                    }
                    if (PtInRect(&m_rcBotonEditarNota, pt) && m_notaSeleccionada > 0) {
                        if (m_notas->EditarConPrompt(hwndPanel, m_notaSeleccionada, false)) {
                            m_notas->Recargar();
                            InvalidateRect(hwndPanel, nullptr, FALSE);
                        }
                        return true;
                    }
                    if (PtInRect(&m_rcBotonCopiarNota, pt) && m_notaSeleccionada > 0) {
                        m_notas->Copiar(hwndPanel, m_notaSeleccionada);
                        return true;
                    }
                    if (PtInRect(&m_rcBotonEliminarNota, pt) && m_notaSeleccionada > 0) {
                        if (MessageBoxW(hwndPanel, L"¿Eliminar esta nota?", L"Useful Side View - Notas",
                                        MB_YESNO | MB_ICONQUESTION) == IDYES) {
                            m_notas->Eliminar(m_notaSeleccionada);
                            m_notaSeleccionada = 0;
                            m_mostrarDetalleNota = false;
                            InvalidateRect(hwndPanel, nullptr, FALSE);
                        }
                        return true;
                    }
                } else if (PtInRect(&m_rcCajaNotas, pt)) {
                    const int indice = (pt.y - m_rcCajaNotas.top) / 42;
                    if (indice >= 0 && indice < static_cast<int>(m_notas->Notas().size())) {
                        m_notaSeleccionada = m_notas->Notas()[static_cast<std::size_t>(indice)].id;
                        m_mostrarDetalleNota = true;
                        InvalidateRect(hwndPanel, nullptr, FALSE);
                    }
                    return true;
                }
            }
            // El acceso permanece visible en la vista general, pero el
            // comparador sólo se activa al seleccionar exactamente dos videos.
            if (m_carpetasSeleccionadas.empty() &&
                PtInRect(&m_rcBotonCompararVideos, pt)) {
                return true;
            }
            // Botón Fusionar Carpetas (si hay múltiples seleccionadas)
            if (m_carpetasSeleccionadas.size() > 1 && PtInRect(&m_rcBotonFusionCarpetas, pt)) {
                EjecutarFusionCarpetas(hwndPanel);
                return true;
            }
            // Botón Limpiar Carpetas Vacías
            if (!m_carpetasVaciasDetectadas.empty() && PtInRect(&m_rcBotonEliminarVacias, pt)) {
                EliminarCarpetasVacias(hwndPanel);
                return true;
            }
        }

        return false;
    }

    bool ProcesarMensajeVentana(UINT msg, WPARAM, LPARAM, HWND hwndPanel) override {
        if (msg != ServicioFusionCarpetasV2::WM_FUSION_COMPLETADA) return false;
        m_fusion->FinalizarTrabajo();
        ResultadoFusionCarpetasV2 resultado = m_fusion->UltimoResultado();
        m_fusionEnCurso = false;
        if (resultado.errores == 0 && !m_destinoFusion.empty()) {
            m_carpetasSeleccionadas = {m_destinoFusion};
            m_indiceCarpeta = 0;
            CargarCarpetaActual();
            std::wstring mensaje = L"La fusión terminó correctamente. Se transfirieron " +
                std::to_wstring(resultado.elementosTransferidos) + L" elementos.";
            MessageBoxW(hwndPanel, mensaje.c_str(), L"Fusión completada", MB_OK | MB_ICONINFORMATION);
        } else {
            std::wstring mensaje = L"La fusión terminó con " + std::to_wstring(resultado.errores) +
                L" error(es) y transfirió " + std::to_wstring(resultado.elementosTransferidos) +
                L" elemento(s). Las carpetas de origen se conservaron cuando no fue posible completar una transferencia.";
            MessageBoxW(hwndPanel, mensaje.c_str(), L"Fusión incompleta", MB_OK | MB_ICONWARNING);
        }
        InvalidateRect(hwndPanel, nullptr, FALSE);
        return true;
    }

    void Renderizar(HDC hdc, const RECT& rect, const TemaVisual& tema) override {
        int ancho = rect.right - rect.left;
        int alto = rect.bottom - rect.top;
        if (ancho <= 0 || alto <= 0) return;

        // 1. Fondo principal
        HBRUSH brochaFondo = CreateSolidBrush(tema.fondo);
        FillRect(hdc, &rect, brochaFondo);
        DeleteObject(brochaFondo);

        // 2. Cabecera Fluent
        int altoCabecera = 54;
        RECT rectCabecera{rect.left, rect.top, rect.right, rect.top + altoCabecera};
        HBRUSH brochaCabecera = CreateSolidBrush(tema.fondoCabecera);
        FillRect(hdc, &rectCabecera, brochaCabecera);
        DeleteObject(brochaCabecera);

        // Bordes separadores
        HPEN plumaBorde = CreatePen(PS_SOLID, 1, tema.borde);
        HPEN plumaVieja = static_cast<HPEN>(SelectObject(hdc, plumaBorde));
        MoveToEx(hdc, rect.left, rect.top + altoCabecera, nullptr);
        LineTo(hdc, rect.right, rect.top + altoCabecera);
        MoveToEx(hdc, rect.left, rect.top, nullptr);
        LineTo(hdc, rect.left, rect.bottom);
        SelectObject(hdc, plumaVieja);
        DeleteObject(plumaBorde);

        SetBkMode(hdc, TRANSPARENT);

        HFONT fuenteTitulo = CreateFontW(-16, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE,
                                         DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                         CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI Variable Display");
        HFONT fuenteSubtitulo = CreateFontW(-12, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                                            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                            CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI Variable Text");
        HFONT fuenteBold = CreateFontW(-13, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                                       DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                       CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI Variable Text");
        HFONT fuentePequena = CreateFontW(-11, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                                          DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                          CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI Variable Text");
        HFONT fuenteVieja = static_cast<HFONT>(SelectObject(hdc, fuenteTitulo));

        std::wstring nombreCarpetaActual = L"Sin carpeta seleccionada";
        std::wstring rutaCompletaActual;
        if (!m_carpetasSeleccionadas.empty() && m_indiceCarpeta < m_carpetasSeleccionadas.size()) {
            rutaCompletaActual = m_carpetasSeleccionadas[m_indiceCarpeta];
            const wchar_t* pNom = PathFindFileNameW(rutaCompletaActual.c_str());
            nombreCarpetaActual = pNom ? pNom : rutaCompletaActual;
        }

        // Título de la cabecera
        SetTextColor(hdc, tema.textoPrincipal);
        std::wstring strTitulo = L"📁 " + nombreCarpetaActual;
        if (m_carpetasSeleccionadas.size() > 1) {
            strTitulo += L" (" + std::to_wstring(m_indiceCarpeta + 1) + L"/" +
                         std::to_wstring(m_carpetasSeleccionadas.size()) + L")";
        }
        RECT rectTextoTitulo{rect.left + 16, rect.top + 8, rect.right - 90, rect.top + 28};
        DrawTextW(hdc, strTitulo.c_str(), -1, &rectTextoTitulo,
                  DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS | DT_NOPREFIX);

        // Subtítulo con peso total
        SelectObject(hdc, fuenteSubtitulo);
        SetTextColor(hdc, tema.textoSecundario);
        std::wstring strSub = L"Peso total: " + FormatearTamano(m_tamanoTotalBytes) +
                              L" • " + std::to_wstring(m_totalSubcarpetas) + L" carpetas, " +
                              std::to_wstring(m_totalArchivos) + L" archivos";
        if (m_carpetasSeleccionadas.empty() && MostrarVersionModDisponibleV2() &&
            m_versionMod) {
            strSub += L" • " + m_versionMod->TextoVersion();
        }
        RECT rectTextoSub{rect.left + 16, rect.top + 28, rect.right - 90, rect.top + 46};
        DrawTextW(hdc, strSub.c_str(), -1, &rectTextoSub,
                  DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS | DT_NOPREFIX);

        // Botones de navegación en la cabecera (◀ y ▶)
        int altoBotonesNav = 28;
        int yNav = rect.top + 13;
        m_rcBotonAnterior = {rect.right - 78, yNav, rect.right - 46, yNav + altoBotonesNav};
        m_rcBotonSiguiente = {rect.right - 42, yNav, rect.right - 10, yNav + altoBotonesNav};
        DibujarBoton(hdc, m_rcBotonAnterior, L"◀", tema, m_carpetasSeleccionadas.size() > 1);
        DibujarBoton(hdc, m_rcBotonSiguiente, L"▶", tema, m_carpetasSeleccionadas.size() > 1);

        int margen = 16;
        int yCursor = rect.top + altoCabecera + 14;
        m_rcBotonCompararVideos = {};
        m_rcBotonFusionCarpetas = {};
        m_rcBotonEliminarVacias = {};
        m_rcBotonNuevaNota = {};
        m_rcBotonSincronizarNotas = {};
        m_rcCajaNotas = {};
        m_rcBotonVolverNotas = {};
        m_rcBotonEditarNota = {};
        m_rcBotonCopiarNota = {};
        m_rcBotonEliminarNota = {};

        // 3. SECCIÓN ESTRUCTURA DE ARCHIVOS (Árbol Jerárquico)
        SelectObject(hdc, fuentePequena);
        SetTextColor(hdc, tema.textoSecundario);
        RECT rcEtiquetaArbol{rect.left + margen, yCursor, rect.right - 130, yCursor + 18};
        DrawTextW(hdc, L"ESTRUCTURA DE ARCHIVOS (VISTA DE CARPETAS)", -1, &rcEtiquetaArbol,
                  DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

        // Botón Alternar Expansión
        std::wstring txtExpansion = L"▶ Raíz";
        if (m_nivelExpansionMax == 2) txtExpansion = L"▶ Nivel 1";
        else if (m_nivelExpansionMax == 3) txtExpansion = L"▶ Nivel 2";
        else if (m_nivelExpansionMax >= 4) txtExpansion = L"🔽 Todo";

        m_rcBotonExpansion = {rect.right - margen - 95, yCursor - 2, rect.right - margen, yCursor + 20};
        DibujarBoton(hdc, m_rcBotonExpansion, txtExpansion.c_str(), tema, true);

        yCursor += 24;

        // Contenedor del Árbol (Calculado proporcionalmente)
        int altoCajaArbol = std::max(100, std::min<int>(180, alto - 450));
        m_rcCajaArbol = {rect.left + margen, yCursor, rect.right - margen, yCursor + altoCajaArbol};
        DibujarContenedor(hdc, m_rcCajaArbol, tema);

        // Clamp estricto del scroll para evitar scroll infinito en vacío
        int elementosVisibles = 0;
        for (const auto& item : m_elementosArbol) {
            if (item.nivelJerarquia < m_nivelExpansionMax) {
                elementosVisibles++;
            }
        }
        int altoTotalArbol = elementosVisibles * 22 + 12;
        int altoVisibleArbol = m_rcCajaArbol.bottom - m_rcCajaArbol.top - 12;
        int maxScrollArbol = std::max(0, altoTotalArbol - altoVisibleArbol);
        m_desplazamientoScrollY = std::max(0, std::min(m_desplazamientoScrollY, maxScrollArbol));

        // Recorte y renderizado de filas del árbol
        HRGN rgnClip = CreateRectRgn(m_rcCajaArbol.left + 2, m_rcCajaArbol.top + 2,
                                     m_rcCajaArbol.right - 2, m_rcCajaArbol.bottom - 2);
        SelectClipRgn(hdc, rgnClip);

        int yFila = m_rcCajaArbol.top + 6 - m_desplazamientoScrollY;
        int altoFila = 22;

        if (m_elementosArbol.empty()) {
            SelectObject(hdc, fuenteSubtitulo);
            SetTextColor(hdc, tema.textoSecundario);
            RECT rcVacio = m_rcCajaArbol;
            DrawTextW(hdc, L"(Carpeta vacía o sin subelementos)", -1, &rcVacio,
                      DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
        } else {
            SelectObject(hdc, fuenteSubtitulo);
            for (const auto& item : m_elementosArbol) {
                if (item.nivelJerarquia < m_nivelExpansionMax) {
                    if (yFila + altoFila >= m_rcCajaArbol.top && yFila <= m_rcCajaArbol.bottom) {
                        int sangria = m_rcCajaArbol.left + 10 + (item.nivelJerarquia * 16);
                        std::wstring icono = item.esDirectorio ? L"📁 " : L"📄 ";
                        std::wstring linea = icono + item.nombre;

                        SetTextColor(hdc, item.esDirectorio ? tema.acento : tema.textoPrincipal);
                        RECT rcNom{sangria, yFila, m_rcCajaArbol.right - 85, yFila + altoFila};
                        DrawTextW(hdc, linea.c_str(), -1, &rcNom,
                                  DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);

                        SetTextColor(hdc, tema.textoSecundario);
                        std::wstring strTam = item.esDirectorio ? L"-" : FormatearTamano(item.tamanoBytes);
                        RECT rcTam{m_rcCajaArbol.right - 80, yFila, m_rcCajaArbol.right - 10, yFila + altoFila};
                        DrawTextW(hdc, strTam.c_str(), -1, &rcTam,
                                  DT_RIGHT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
                    }
                    yFila += altoFila;
                }
            }
        }

        SelectClipRgn(hdc, nullptr);
        DeleteObject(rgnClip);

        yCursor += altoCajaArbol + 12;

        // 4. SECCIÓN ABRIR CARPETA CON
        SelectObject(hdc, fuentePequena);
        SetTextColor(hdc, tema.textoSecundario);
        RECT rcEtiquetaAbrir{rect.left + margen, yCursor, rect.right - margen, yCursor + 16};
        DrawTextW(hdc, L"ABRIR CARPETA CON", -1, &rcEtiquetaAbrir, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

        yCursor += 20;

        int anchoBotonAbrir = 80;
        int anchoSelector = (rect.right - rect.left) - (margen * 2) - anchoBotonAbrir - 10;
        m_rcSelectorPrograma = {rect.left + margen, yCursor, rect.left + margen + anchoSelector, yCursor + 30};
        m_rcBotonEjecutarAbrir = {m_rcSelectorPrograma.right + 10, yCursor, rect.right - margen, yCursor + 30};

        std::wstring seleccionado = m_servicio->ProgramaActual();
        std::wstring strProg = seleccionado.empty() ? L"(click para seleccionar programa)" : PathFindFileNameW(seleccionado.c_str());
        DibujarBoton(hdc, m_rcSelectorPrograma, (strProg + L"  ▾").c_str(), tema, true);

        // Botón Verde / Fluent Abrir
        HBRUSH brAbrir = CreateSolidBrush(RGB(46, 160, 67));
        FillRect(hdc, &m_rcBotonEjecutarAbrir, brAbrir);
        DeleteObject(brAbrir);
        SelectObject(hdc, fuenteBold);
        SetTextColor(hdc, RGB(255, 255, 255));
        RECT rcTxtAbrir = m_rcBotonEjecutarAbrir;
        DrawTextW(hdc, L"Abrir", -1, &rcTxtAbrir, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

        yCursor += 40;

        // 5. SECCIÓN ACCIONES RÁPIDAS
        SelectObject(hdc, fuentePequena);
        SetTextColor(hdc, tema.textoSecundario);
        RECT rcEtiquetaAcciones{rect.left + margen, yCursor, rect.right - margen, yCursor + 16};
        DrawTextW(hdc, L"ACCIONES RÁPIDAS", -1, &rcEtiquetaAcciones, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

        yCursor += 20;

        int anchoMitad = ((rect.right - rect.left) - (margen * 2) - 8) / 2;
        m_rcBotonCrearEnlace = {rect.left + margen, yCursor, rect.left + margen + anchoMitad, yCursor + 30};
        m_rcBotonCopiarContenido = {m_rcBotonCrearEnlace.right + 8, yCursor, rect.right - margen, yCursor + 30};

        DibujarBoton(hdc, m_rcBotonCrearEnlace, L"🔗 Crear enlace", tema, true);
        DibujarBoton(hdc, m_rcBotonCopiarContenido, L"📋 Copiar contenido", tema, true);

        yCursor += 38;

        if (m_carpetasSeleccionadas.empty() && m_notas) {
            DibujarNotas(hdc, rect, tema, fuentePequena, fuenteSubtitulo, fuenteBold, yCursor);
        }

        // Acceso fijo del comparador experimental. El flujo real se activa
        // automáticamente cuando Explorer entrega dos videos seleccionados.
        if (m_carpetasSeleccionadas.empty()) {
            m_rcBotonCompararVideos = {rect.left + margen, yCursor,
                                       rect.right - margen, yCursor + 32};
            DibujarBoton(hdc, m_rcBotonCompararVideos,
                         L"Comparador de videos · selecciona 2 videos", tema, false);
            yCursor += 40;
        }

        // 6. OPERACIONES CON CARPETA (Fusión o Eliminación de vacías)
        if (m_carpetasSeleccionadas.size() > 1) {
            SelectObject(hdc, fuentePequena);
            SetTextColor(hdc, tema.textoSecundario);
            RECT rcEtiquetaOp{rect.left + margen, yCursor, rect.right - margen, yCursor + 16};
            DrawTextW(hdc, L"OPERACIONES CON MÚLTIPLES CARPETAS", -1, &rcEtiquetaOp, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

            yCursor += 20;
            m_rcBotonFusionCarpetas = {rect.left + margen, yCursor, rect.right - margen, yCursor + 32};
            DibujarBoton(hdc, m_rcBotonFusionCarpetas, L"🔀 Fusionar carpetas seleccionadas...", tema, true);
            yCursor += 40;
        } else if (!m_carpetasVaciasDetectadas.empty()) {
            SelectObject(hdc, fuentePequena);
            SetTextColor(hdc, RGB(220, 100, 30));
            RECT rcEtiquetaVacias{rect.left + margen, yCursor, rect.right - margen, yCursor + 16};
            std::wstring strVacias = L"CARPETAS VACÍAS DETECTADAS (" + std::to_wstring(m_carpetasVaciasDetectadas.size()) + L")";
            DrawTextW(hdc, strVacias.c_str(), -1, &rcEtiquetaVacias, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

            yCursor += 20;
            m_rcBotonEliminarVacias = {rect.left + margen, yCursor, rect.right - margen, yCursor + 30};
            DibujarBoton(hdc, m_rcBotonEliminarVacias, L"🗑️ Eliminar carpetas vacías", tema, true);
            yCursor += 38;
        }

        // 7. SUBCARPETAS MÁS PESADAS (Con límite configurable y scroll independiente)
        if (!m_topCarpetasPesadas.empty()) {
            SelectObject(hdc, fuentePequena);
            SetTextColor(hdc, tema.textoSecundario);
            RECT rcEtiquetaTop{rect.left + margen, yCursor, rect.right - 100, yCursor + 16};
            DrawTextW(hdc, L"SUBCARPETAS MÁS PESADAS", -1, &rcEtiquetaTop, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

            // Botón selector de cantidad: "Mostrar (N) ▾"
            std::wstring strBotonLimite = L"Mostrar (" + std::to_wstring(m_limiteCarpetasPesadas) + L") ▾";
            m_rcBotonLimitePesadas = {rect.right - margen - 95, yCursor - 2, rect.right - margen, yCursor + 18};
            DibujarBoton(hdc, m_rcBotonLimitePesadas, strBotonLimite.c_str(), tema, true);

            yCursor += 22;

            int altoDisponiblePesadas = rect.bottom - margen - yCursor;
            int altoCajaPesadas = std::max(60, altoDisponiblePesadas);
            m_rcCajaPesadas = {rect.left + margen, yCursor, rect.right - margen, yCursor + altoCajaPesadas};

            int totalMostrar = std::min<int>(m_limiteCarpetasPesadas, static_cast<int>(m_topCarpetasPesadas.size()));
            int altoTotalContenidoPesadas = totalMostrar * 40;
            int altoVisiblePesadas = m_rcCajaPesadas.bottom - m_rcCajaPesadas.top - 8;
            int maxScrollPesadas = std::max(0, altoTotalContenidoPesadas - altoVisiblePesadas);
            m_scrollPesadasY = std::max(0, std::min(m_scrollPesadasY, maxScrollPesadas));

            HRGN rgnClipPesadas = CreateRectRgn(m_rcCajaPesadas.left, m_rcCajaPesadas.top,
                                                m_rcCajaPesadas.right, m_rcCajaPesadas.bottom);
            SelectClipRgn(hdc, rgnClipPesadas);

            int yPesada = m_rcCajaPesadas.top + 2 - m_scrollPesadasY;
            int altoTarjetaPesada = 34;

            for (int i = 0; i < totalMostrar; ++i) {
                if (yPesada + altoTarjetaPesada >= m_rcCajaPesadas.top && yPesada <= m_rcCajaPesadas.bottom) {
                    RECT rcPesada{m_rcCajaPesadas.left, yPesada, m_rcCajaPesadas.right, yPesada + altoTarjetaPesada};
                    DibujarContenedor(hdc, rcPesada, tema);

                    SelectObject(hdc, fuenteSubtitulo);
                    SetTextColor(hdc, tema.textoPrincipal);
                    RECT rcNomP{rcPesada.left + 10, rcPesada.top + 2, rcPesada.right - 90, rcPesada.top + 18};
                    const std::size_t indicePesada = static_cast<std::size_t>(i);
                    DrawTextW(hdc, m_topCarpetasPesadas[indicePesada].nombre.c_str(), -1, &rcNomP,
                              DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);

                    SetTextColor(hdc, tema.acento);
                    RECT rcTamP{rcPesada.right - 85, rcPesada.top + 2, rcPesada.right - 10, rcPesada.top + 18};
                    std::wstring strPesadaTam = FormatearTamano(m_topCarpetasPesadas[indicePesada].tamanoBytes);
                    DrawTextW(hdc, strPesadaTam.c_str(), -1, &rcTamP,
                              DT_RIGHT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

                    // Barra de progreso de peso
                    RECT rcBarraFondo{rcPesada.left + 10, rcPesada.top + 22, rcPesada.right - 10, rcPesada.top + 26};
                    HBRUSH brBkgBar = CreateSolidBrush(tema.fondoCabecera);
                    FillRect(hdc, &rcBarraFondo, brBkgBar);
                    DeleteObject(brBkgBar);

                    int anchoLleno = static_cast<int>(static_cast<double>(rcBarraFondo.right - rcBarraFondo.left) * (m_topCarpetasPesadas[indicePesada].porcentaje / 100.0));
                    RECT rcBarraProg{rcBarraFondo.left, rcBarraFondo.top, rcBarraFondo.left + anchoLleno, rcBarraFondo.bottom};
                    HBRUSH brFillBar = CreateSolidBrush(tema.acento);
                    FillRect(hdc, &rcBarraProg, brFillBar);
                    DeleteObject(brFillBar);
                }
                yPesada += altoTarjetaPesada + 6;
            }

            SelectClipRgn(hdc, nullptr);
            DeleteObject(rgnClipPesadas);
        }

        SelectObject(hdc, fuenteVieja);
        DeleteObject(fuenteTitulo);
        DeleteObject(fuenteSubtitulo);
        DeleteObject(fuenteBold);
        DeleteObject(fuentePequena);
    }

private:
    void DibujarNotas(HDC hdc, const RECT& rect, const TemaVisual& tema,
                      HFONT fuentePequena, HFONT fuenteSubtitulo, HFONT fuenteBold,
                      int& yCursor) {
        const int margen = 16;
        SelectObject(hdc, fuentePequena);
        SetTextColor(hdc, tema.textoSecundario);
        RECT etiqueta{rect.left + margen, yCursor, rect.right - margen, yCursor + 16};
        const auto& notas = m_notas->Notas();
        int nativas = 0;
        int winklok = 0;
        for (const auto& nota : notas) {
            if (nota.fuente == FuenteNotaV2::WinKlok) ++winklok;
            else ++nativas;
        }
        std::wstring textoEtiqueta = L"NOTAS  ·  " + std::to_wstring(nativas) + L" nativas";
        if (winklok > 0) textoEtiqueta += L"  ·  " + std::to_wstring(winklok) + L" WinKlok";
        DrawTextW(hdc, textoEtiqueta.c_str(), -1, &etiqueta, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

        yCursor += 20;
        const int anchoMitad = ((rect.right - rect.left) - (margen * 2) - 8) / 2;
        m_rcBotonNuevaNota = {rect.left + margen, yCursor, rect.left + margen + anchoMitad, yCursor + 30};
        m_rcBotonSincronizarNotas = {m_rcBotonNuevaNota.right + 8, yCursor, rect.right - margen, yCursor + 30};
        DibujarBoton(hdc, m_rcBotonNuevaNota, L"📝 Nueva nota", tema, true);
        DibujarBoton(hdc, m_rcBotonSincronizarNotas, L"⟳ Forzar sincronización", tema, m_notas->WinKlokDisponible());
        yCursor += 38;

        const int altoDisponible = std::max(90, static_cast<int>(rect.bottom - yCursor - 92));
        const int altoCaja = std::min(190, altoDisponible);
        m_rcCajaNotas = {rect.left + margen, yCursor, rect.right - margen, yCursor + altoCaja};
        DibujarContenedor(hdc, m_rcCajaNotas, tema);

        if (m_mostrarDetalleNota) {
            const NotaV2* nota = m_notas->Buscar(m_notaSeleccionada);
            if (!nota) {
                m_mostrarDetalleNota = false;
                m_notaSeleccionada = 0;
            } else {
                const int anchoBoton = std::max(70, static_cast<int>((m_rcCajaNotas.right - m_rcCajaNotas.left - 42) / 4));
                int xBoton = m_rcCajaNotas.left + 8;
                m_rcBotonVolverNotas = {xBoton, m_rcCajaNotas.top + 8, xBoton + anchoBoton, m_rcCajaNotas.top + 32};
                xBoton += anchoBoton + 6;
                m_rcBotonEditarNota = {xBoton, m_rcCajaNotas.top + 8, xBoton + anchoBoton, m_rcCajaNotas.top + 32};
                xBoton += anchoBoton + 6;
                m_rcBotonCopiarNota = {xBoton, m_rcCajaNotas.top + 8, xBoton + anchoBoton, m_rcCajaNotas.top + 32};
                xBoton += anchoBoton + 6;
                m_rcBotonEliminarNota = {xBoton, m_rcCajaNotas.top + 8, m_rcCajaNotas.right - 8, m_rcCajaNotas.top + 32};
                DibujarBoton(hdc, m_rcBotonVolverNotas, L"Volver", tema, true);
                DibujarBoton(hdc, m_rcBotonEditarNota, L"Editar", tema, true);
                DibujarBoton(hdc, m_rcBotonCopiarNota, L"Copiar", tema, true);
                DibujarBoton(hdc, m_rcBotonEliminarNota, L"Eliminar", tema, true);

                SelectObject(hdc, fuenteBold);
                SetTextColor(hdc, tema.textoPrincipal);
                RECT titulo{m_rcCajaNotas.left + 10, m_rcCajaNotas.top + 38, m_rcCajaNotas.right - 10, m_rcCajaNotas.top + 60};
                DrawTextW(hdc, nota->titulo.c_str(), -1, &titulo, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);
                SelectObject(hdc, fuentePequena);
                SetTextColor(hdc, nota->fuente == FuenteNotaV2::WinKlok ? tema.acento : tema.textoSecundario);
                RECT origen{m_rcCajaNotas.left + 10, m_rcCajaNotas.top + 60, m_rcCajaNotas.right - 10, m_rcCajaNotas.top + 78};
                DrawTextW(hdc, nota->fuente == FuenteNotaV2::WinKlok ? L"Origen: WinKlok" :
                          (nota->idWinKlok > 0 ? L"Origen: Useful Side View · sincronizada" : L"Origen: Useful Side View"),
                          -1, &origen, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
                SelectObject(hdc, fuenteSubtitulo);
                SetTextColor(hdc, tema.textoPrincipal);
                RECT cuerpo{m_rcCajaNotas.left + 10, m_rcCajaNotas.top + 82, m_rcCajaNotas.right - 10, m_rcCajaNotas.bottom - 8};
                DrawTextW(hdc, nota->cuerpo.empty() ? L"(Nota vacía)" : nota->cuerpo.c_str(), -1, &cuerpo,
                          DT_LEFT | DT_TOP | DT_WORDBREAK | DT_NOPREFIX | DT_END_ELLIPSIS);
            }
        }

        if (!m_mostrarDetalleNota) {
            if (notas.empty()) {
                SelectObject(hdc, fuenteSubtitulo);
                SetTextColor(hdc, tema.textoSecundario);
                RECT vacio = m_rcCajaNotas;
                DrawTextW(hdc, L"(No hay notas. Crea una para comenzar.)", -1, &vacio,
                          DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
            } else {
                HRGN clip = CreateRectRgn(m_rcCajaNotas.left + 2, m_rcCajaNotas.top + 2,
                                          m_rcCajaNotas.right - 2, m_rcCajaNotas.bottom - 2);
                SelectClipRgn(hdc, clip);
                int yNota = m_rcCajaNotas.top + 6;
                for (const auto& nota : notas) {
                    RECT tarjeta{m_rcCajaNotas.left + 6, yNota, m_rcCajaNotas.right - 6, yNota + 36};
                    SelectObject(hdc, fuenteBold);
                    SetTextColor(hdc, tema.textoPrincipal);
                    RECT titulo{tarjeta.left + 8, tarjeta.top + 2, tarjeta.right - 112, tarjeta.top + 19};
                    DrawTextW(hdc, nota.titulo.empty() ? L"Sin título" : nota.titulo.c_str(), -1, &titulo,
                              DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);
                    SelectObject(hdc, fuentePequena);
                    SetTextColor(hdc, nota.fuente == FuenteNotaV2::WinKlok ? tema.acento : tema.textoSecundario);
                    RECT origen{tarjeta.right - 106, tarjeta.top + 2, tarjeta.right - 8, tarjeta.top + 19};
                    DrawTextW(hdc, nota.fuente == FuenteNotaV2::WinKlok ? L"WinKlok" :
                              (nota.idWinKlok > 0 ? L"Nativa · sync" : L"Nativa"), -1, &origen,
                              DT_RIGHT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
                    SetTextColor(hdc, tema.textoSecundario);
                    RECT resumen{tarjeta.left + 8, tarjeta.top + 19, tarjeta.right - 8, tarjeta.bottom - 2};
                    DrawTextW(hdc, nota.cuerpo.empty() ? L"(vacía)" : nota.cuerpo.c_str(), -1, &resumen,
                              DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);
                    yNota += 42;
                    if (yNota > m_rcCajaNotas.bottom) break;
                }
                SelectClipRgn(hdc, nullptr);
                DeleteObject(clip);
            }
        }
        yCursor += altoCaja + 12;
    }

    void CargarCarpetaActual() {
        m_elementosArbol.clear();
        m_tamanoTotalBytes = 0;
        m_totalSubcarpetas = 0;
        m_totalArchivos = 0;
        m_topCarpetasPesadas.clear();
        m_carpetasVaciasDetectadas.clear();

        if (m_carpetasSeleccionadas.empty() || m_indiceCarpeta >= m_carpetasSeleccionadas.size()) return;

        const std::wstring& ruta = m_carpetasSeleccionadas[m_indiceCarpeta];
        ResultadoInspeccionCarpetaV2 resultado = m_inspeccion->Ejecutar(ruta, static_cast<size_t>(m_limiteCarpetasPesadas));
        m_elementosArbol = std::move(resultado.elementos);
        m_topCarpetasPesadas = std::move(resultado.pesadas);
        m_carpetasVaciasDetectadas = std::move(resultado.vacias);
        m_tamanoTotalBytes = resultado.tamanoTotalBytes;
        m_totalSubcarpetas = resultado.totalSubcarpetas;
        m_totalArchivos = resultado.totalArchivos;
    }

    void DibujarBoton(HDC hdc, const RECT& rc, const wchar_t* texto, const TemaVisual& tema, bool habilitado) {
        HBRUSH br = CreateSolidBrush(habilitado ? tema.botonFondo : tema.fondoCabecera);
        FillRect(hdc, &rc, br);
        DeleteObject(br);

        HPEN pen = CreatePen(PS_SOLID, 1, tema.borde);
        HPEN oldPen = static_cast<HPEN>(SelectObject(hdc, pen));
        HBRUSH nullBr = static_cast<HBRUSH>(GetStockObject(NULL_BRUSH));
        HBRUSH oldBr = static_cast<HBRUSH>(SelectObject(hdc, nullBr));
        RoundRect(hdc, rc.left, rc.top, rc.right, rc.bottom, 6, 6);
        SelectObject(hdc, oldBr);
        SelectObject(hdc, oldPen);
        DeleteObject(pen);

        SetTextColor(hdc, habilitado ? tema.textoPrincipal : tema.textoSecundario);
        RECT rcTexto = rc;
        DrawTextW(hdc, texto, -1, &rcTexto, DT_SINGLELINE | DT_CENTER | DT_VCENTER | DT_NOPREFIX);
    }

    void DibujarContenedor(HDC hdc, const RECT& rc, const TemaVisual& tema) {
        HBRUSH br = CreateSolidBrush(tema.fondoTarjeta);
        FillRect(hdc, &rc, br);
        DeleteObject(br);

        HPEN pen = CreatePen(PS_SOLID, 1, tema.borde);
        HPEN oldPen = static_cast<HPEN>(SelectObject(hdc, pen));
        HBRUSH nullBr = static_cast<HBRUSH>(GetStockObject(NULL_BRUSH));
        HBRUSH oldBr = static_cast<HBRUSH>(SelectObject(hdc, nullBr));
        RoundRect(hdc, rc.left, rc.top, rc.right, rc.bottom, 8, 8);
        SelectObject(hdc, oldBr);
        SelectObject(hdc, oldPen);
        DeleteObject(pen);
    }

    std::wstring FormatearTamano(uintmax_t bytes) {
        if (bytes < 1024) return std::to_wstring(bytes) + L" B";
        if (bytes < 1024 * 1024) {
            wchar_t b[32];
            swprintf_s(b, L"%.1f KB", static_cast<double>(bytes) / 1024.0);
            return b;
        }
        if (bytes < 1024ULL * 1024 * 1024) {
            wchar_t b[32];
            swprintf_s(b, L"%.1f MB", static_cast<double>(bytes) / (1024.0 * 1024.0));
            return b;
        }
        wchar_t b[32];
        swprintf_s(b, L"%.2f GB", static_cast<double>(bytes) / (1024.0 * 1024.0 * 1024.0));
        return b;
    }

    void MostrarMenuLimitePesadas(HWND hwndPadre) {
        HMENU hMenu = CreatePopupMenu();
        AppendMenuW(hMenu, (m_limiteCarpetasPesadas == 3 ? MF_CHECKED : 0) | MF_STRING, 1, L"Mostrar 3 carpetas (Por defecto)");
        AppendMenuW(hMenu, (m_limiteCarpetasPesadas == 5 ? MF_CHECKED : 0) | MF_STRING, 2, L"Mostrar 5 carpetas");
        AppendMenuW(hMenu, (m_limiteCarpetasPesadas == 10 ? MF_CHECKED : 0) | MF_STRING, 3, L"Mostrar 10 carpetas");
        AppendMenuW(hMenu, (m_limiteCarpetasPesadas == 20 ? MF_CHECKED : 0) | MF_STRING, 4, L"Mostrar 20 carpetas");
        AppendMenuW(hMenu, (m_limiteCarpetasPesadas >= 999 ? MF_CHECKED : 0) | MF_STRING, 5, L"Mostrar todas");

        POINT ptMenu{m_rcBotonLimitePesadas.left, m_rcBotonLimitePesadas.bottom};
        ClientToScreen(hwndPadre, &ptMenu);
        int cmd = TrackPopupMenu(hMenu, TPM_LEFTALIGN | TPM_TOPALIGN | TPM_RETURNCMD, ptMenu.x, ptMenu.y, 0, hwndPadre, nullptr);
        DestroyMenu(hMenu);

        if (cmd == 1) m_limiteCarpetasPesadas = 3;
        else if (cmd == 2) m_limiteCarpetasPesadas = 5;
        else if (cmd == 3) m_limiteCarpetasPesadas = 10;
        else if (cmd == 4) m_limiteCarpetasPesadas = 20;
        else if (cmd == 5) m_limiteCarpetasPesadas = 999;

        if (cmd > 0) {
            g_limiteCarpetasPesadasConfig.store(m_limiteCarpetasPesadas);
            m_scrollPesadasY = 0;
            InvalidateRect(hwndPadre, nullptr, FALSE);
        }
    }

    void MostrarMenuDesplegableProgramas(HWND hwndPadre) {
        m_programasRecordados = m_servicio->Programas();
        HMENU hMenu = CreatePopupMenu();
        for (size_t i = 0; i < m_programasRecordados.size(); ++i) {
            AppendMenuW(hMenu, MF_STRING, i + 1, m_programasRecordados[i].c_str());
        }
        AppendMenuW(hMenu, MF_SEPARATOR, 0, nullptr);
        AppendMenuW(hMenu, MF_STRING, 100, L"Examinar otro ejecutable...");

        POINT ptMenu{m_rcSelectorPrograma.left, m_rcSelectorPrograma.bottom};
        ClientToScreen(hwndPadre, &ptMenu);
        int cmd = TrackPopupMenu(hMenu, TPM_LEFTALIGN | TPM_TOPALIGN | TPM_RETURNCMD, ptMenu.x, ptMenu.y, 0, hwndPadre, nullptr);
        DestroyMenu(hMenu);

        if (cmd == 100) {
            if (m_servicio->AbrirDialogo(hwndPadre)) {
                m_programasRecordados = m_servicio->Programas();
                InvalidateRect(hwndPadre, nullptr, FALSE);
            }
        } else if (cmd > 0 && cmd <= static_cast<int>(m_programasRecordados.size())) {
            m_servicio->Seleccionar(m_programasRecordados[static_cast<std::size_t>(cmd - 1)]);
            InvalidateRect(hwndPadre, nullptr, FALSE);
        }
    }

    void EjecutarAbrirCon(HWND hwndPadre) {
        std::wstring programa = m_servicio->ProgramaActual();
        if (programa.empty()) {
            MostrarMenuDesplegableProgramas(hwndPadre);
            programa = m_servicio->ProgramaActual();
            if (programa.empty()) return;
        }
        m_servicio->Seleccionar(programa);
        m_servicio->Ejecutar(m_carpetasSeleccionadas);
    }

    void CopiarListaAlPortapapeles(HWND hwndPadre) {
        if (m_carpetasSeleccionadas.empty() || m_indiceCarpeta >= m_carpetasSeleccionadas.size()) return;
        const std::wstring& ruta = m_carpetasSeleccionadas[m_indiceCarpeta];

        std::wstring textoAcumulado = L"Contenido de: " + ruta + L"\r\n\r\n";
        for (const auto& item : m_elementosArbol) {
            std::wstring sangria(static_cast<std::size_t>(item.nivelJerarquia * 2), L' ');
            textoAcumulado += sangria + (item.esDirectorio ? L"[CARPETA] " : L"  ") + item.nombre;
            if (!item.esDirectorio) {
                textoAcumulado += L" (" + FormatearTamano(item.tamanoBytes) + L")";
            }
            textoAcumulado += L"\r\n";
        }

        if (OpenClipboard(hwndPadre)) {
            bool copiado = EmptyClipboard();
            size_t bytes = (textoAcumulado.size() + 1) * sizeof(wchar_t);
            HGLOBAL hMem = copiado ? GlobalAlloc(GMEM_MOVEABLE, bytes) : nullptr;
            if (copiado && !hMem) copiado = false;
            if (hMem) {
                void* pMem = GlobalLock(hMem);
                if (pMem) {
                    memcpy(pMem, textoAcumulado.c_str(), bytes);
                    GlobalUnlock(hMem);
                    copiado = SetClipboardData(CF_UNICODETEXT, hMem) != nullptr;
                    if (!copiado) GlobalFree(hMem);
                } else {
                    GlobalFree(hMem);
                    copiado = false;
                }
            }
            CloseClipboard();
            MessageBoxW(hwndPadre, copiado ? L"¡Estructura de la carpeta copiada al portapapeles!" : L"No fue posible copiar la estructura al portapapeles.", L"Useful Side View", MB_OK | (copiado ? MB_ICONINFORMATION : MB_ICONWARNING));
        }
    }

    void CrearEnlaceSimbolico(HWND hwndPadre) {
        if (m_carpetasSeleccionadas.empty() || m_indiceCarpeta >= m_carpetasSeleccionadas.size()) return;
        AmbitoComV2 com;
        if (!com.Disponible()) return;
        const std::wstring& rutaOrigen = m_carpetasSeleccionadas[m_indiceCarpeta];

        wchar_t rutaDestino[MAX_PATH] = L"";
        OPENFILENAMEW ofn{};
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = hwndPadre;
        ofn.lpstrFilter = L"Accesos Directos (*.lnk)\0*.lnk\0Todos los archivos (*.*)\0*.*\0";
        ofn.lpstrFile = rutaDestino;
        ofn.nMaxFile = MAX_PATH;
        ofn.lpstrTitle = L"Crear enlace o acceso directo hacia esta carpeta";
        ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;

        if (GetSaveFileNameW(&ofn)) {
            std::wstring destinoStr = rutaDestino;
            if (destinoStr.size() < 4 || _wcsicmp(destinoStr.substr(destinoStr.size() - 4).c_str(), L".lnk") != 0) {
                destinoStr += L".lnk";
            }

            IShellLinkW* psl = nullptr;
            if (SUCCEEDED(CoCreateInstance(CLSID_ShellLink, nullptr, CLSCTX_INPROC_SERVER, IID_IShellLinkW, reinterpret_cast<void**>(&psl))) &&
                SUCCEEDED(psl->SetPath(rutaOrigen.c_str()))) {
                IPersistFile* ppf = nullptr;
                if (SUCCEEDED(psl->QueryInterface(IID_IPersistFile, reinterpret_cast<void**>(&ppf))) && ppf) {
                    HRESULT guardado = ppf->Save(destinoStr.c_str(), TRUE);
                    ppf->Release();
                    if (SUCCEEDED(guardado)) {
                    MessageBoxW(hwndPadre, L"Enlace creado exitosamente.", L"Useful Side View", MB_OK | MB_ICONINFORMATION);
                    }
                }
                psl->Release();
            }
        }
    }

    void EjecutarFusionCarpetas(HWND hwndPadre) {
        if (m_carpetasSeleccionadas.size() < 2) return;
        if (m_fusionEnCurso) return;
        std::wstring confirmacion = L"Se fusionarán " + std::to_wstring(m_carpetasSeleccionadas.size()) +
            L" carpetas.\n\nLa operación combinará todo su contenido en una sola carpeta y eliminará las carpetas de origen cuando la transferencia termine correctamente.\n\n¿Deseas continuar?";
        if (MessageBoxW(hwndPadre, confirmacion.c_str(), L"Useful Side View - Fusión", MB_YESNO | MB_ICONQUESTION) != IDYES) return;

        wchar_t padreBuffer[MAX_PATH]{};
        wcsncpy_s(padreBuffer, m_carpetasSeleccionadas.front().c_str(), _TRUNCATE);
        PathRemoveFileSpecW(padreBuffer);
        std::wstring nombre = SolicitarNombreCarpetaV2(hwndPadre);
        if (nombre.empty()) return;
        if (nombre.find_first_of(L"\\/:*?\"<>|") != std::wstring::npos || nombre == L"." || nombre == L"..") {
            MessageBoxW(hwndPadre, L"El nombre contiene caracteres no válidos.", L"Fusión no válida", MB_OK | MB_ICONWARNING);
            return;
        }
        std::wstring destino = std::wstring(padreBuffer) + L"\\" + nombre;
        for (const auto& origen : m_carpetasSeleccionadas) {
            std::wstring prefijo = origen;
            if (!prefijo.empty() && prefijo.back() != L'\\') prefijo += L'\\';
            if (_wcsicmp(destino.c_str(), origen.c_str()) == 0 ||
                (destino.size() > prefijo.size() && _wcsnicmp(destino.c_str(), prefijo.c_str(), prefijo.size()) == 0)) {
                MessageBoxW(hwndPadre, L"La carpeta destino no puede ser una carpeta de origen ni estar dentro de ella.", L"Fusión no válida", MB_OK | MB_ICONWARNING);
                return;
            }
        }

        DWORD atributosDestino = GetFileAttributesW(destino.c_str());
        if (atributosDestino != INVALID_FILE_ATTRIBUTES && !(atributosDestino & FILE_ATTRIBUTE_DIRECTORY)) {
            MessageBoxW(hwndPadre, L"La ruta elegida ya corresponde a un archivo.", L"Fusión no válida", MB_OK | MB_ICONWARNING);
            return;
        }
        if (atributosDestino == INVALID_FILE_ATTRIBUTES && !CreateDirectoryW(destino.c_str(), nullptr)) {
            MessageBoxW(hwndPadre, L"No fue posible crear la carpeta unificada.", L"Fusión fallida", MB_OK | MB_ICONERROR);
            return;
        }

        m_fusionEnCurso = true;
        m_destinoFusion = destino;
        if (!m_fusion->EjecutarEnSegundoPlano(m_carpetasSeleccionadas, destino, hwndPadre)) {
            m_fusionEnCurso = false;
            MessageBoxW(hwndPadre, L"No fue posible iniciar la fusión en segundo plano.", L"Fusión fallida", MB_OK | MB_ICONERROR);
            return;
        }
        MessageBoxW(hwndPadre,
            L"La fusión se ejecutará en segundo plano para evitar congelar Explorer.\n\nPuedes seguir usando la ventana; el contenido se actualizará al finalizar.",
            L"Fusión iniciada", MB_OK | MB_ICONINFORMATION);
    }

    void EliminarCarpetasVacias(HWND hwndPadre) {
        int eliminadas = 0;
        for (const auto& rVacia : m_carpetasVaciasDetectadas) {
            if (RemoveDirectoryW(rVacia.c_str())) {
                eliminadas++;
            }
        }
        m_carpetasVaciasDetectadas.clear();
        CargarCarpetaActual();
        InvalidateRect(hwndPadre, nullptr, FALSE);
        std::wstring msg = L"Se eliminaron " + std::to_wstring(eliminadas) + L" carpetas vacías.";
        MessageBoxW(hwndPadre, msg.c_str(), L"Useful Side View - Limpieza", MB_OK | MB_ICONINFORMATION);
    }

    std::vector<std::wstring> m_carpetasSeleccionadas;
    size_t m_indiceCarpeta{0};
    int m_desplazamientoScrollY{0};
    int m_scrollPesadasY{0};
    int m_nivelExpansionMax{1};
    int m_limiteCarpetasPesadas{3};

    uintmax_t m_tamanoTotalBytes{0};
    size_t m_totalSubcarpetas{0};
    size_t m_totalArchivos{0};

    std::vector<ItemArbolCarpetaV2> m_elementosArbol;
    std::vector<CarpetaPesadaV2> m_topCarpetasPesadas;
    std::vector<std::wstring> m_carpetasVaciasDetectadas;

    std::vector<std::wstring> m_programasRecordados;
    std::shared_ptr<ServicioAbrirConV2> m_servicio;
    std::shared_ptr<ServicioNavegacionV2> m_navegacion;
    std::shared_ptr<ServicioFusionCarpetasV2> m_fusion;
    std::shared_ptr<ServicioInspeccionCarpetasV2> m_inspeccion;
    std::shared_ptr<ServicioVersionModV2> m_versionMod;
    std::shared_ptr<ServicioNotasV2> m_notas;
    int m_notaSeleccionada{0};
    bool m_mostrarDetalleNota{false};
    bool m_fusionEnCurso{false};
    std::wstring m_destinoFusion;

    RECT m_rcBotonAnterior{};
    RECT m_rcBotonSiguiente{};
    RECT m_rcBotonExpansion{};
    RECT m_rcCajaArbol{};
    RECT m_rcSelectorPrograma{};
    RECT m_rcBotonEjecutarAbrir{};
    RECT m_rcBotonCrearEnlace{};
    RECT m_rcBotonCopiarContenido{};
    RECT m_rcBotonCompararVideos{};
    RECT m_rcBotonFusionCarpetas{};
    RECT m_rcBotonEliminarVacias{};
    RECT m_rcBotonLimitePesadas{};
    RECT m_rcCajaPesadas{};
    RECT m_rcBotonNuevaNota{};
    RECT m_rcBotonSincronizarNotas{};
    RECT m_rcCajaNotas{};
    RECT m_rcBotonVolverNotas{};
    RECT m_rcBotonEditarNota{};
    RECT m_rcBotonCopiarNota{};
    RECT m_rcBotonEliminarNota{};
};

} // namespace SideViewV2
/**** ended inlining v2/vistas/vista_carpetas.hpp ****/
/**** start inlining v2/vistas/vista_en_desarrollo.hpp ****/

/**** skipping file: v2/ui/panel_interfaz.hpp ****/
/**** skipping file: v2/ui/ventana_base.hpp ****/
/**** skipping file: v2/nucleo/enrutador_vistas.hpp ****/
#include <windowsx.h>

namespace SideViewV2 {

class VistaEnDesarrollo : public IPanelContenido {
public:
    VistaEnDesarrollo(const std::wstring& nombreVista, const std::wstring& iconoVista, const std::wstring& descripcion)
        : m_nombreVista(nombreVista), m_iconoVista(iconoVista), m_descripcion(descripcion) {}

    void AlCambiarSeleccion(const std::vector<std::wstring>& archivos, const std::wstring& carpeta) override {
        m_archivos = archivos;
        m_carpeta = carpeta;
    }

    bool ProcesarMensajeRaton(UINT msg, WPARAM, LPARAM lParam, HWND hwnd) override {
        if (msg != WM_LBUTTONDOWN) return false;
        POINT punto{GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
        if (!PtInRect(&m_rcComparar, punto)) return false;
        if (PuedeCompararSeleccionV2(m_archivos)) {
            PostMessageW(hwnd, WM_SIDEVIEW_REQUEST_COMPARADOR_ARCHIVOS, 0, 0);
        }
        return true;
    }

    void Renderizar(HDC hdc, const RECT& rect, const TemaVisual& tema) override {
        int ancho = rect.right - rect.left;
        int alto = rect.bottom - rect.top;
        if (ancho <= 0 || alto <= 0) return;

        // Fondo
        HBRUSH brochaFondo = CreateSolidBrush(tema.fondo);
        FillRect(hdc, &rect, brochaFondo);
        DeleteObject(brochaFondo);

        // Cabecera Fluent
        int altoCabecera = 54;
        RECT rectCabecera{rect.left, rect.top, rect.right, rect.top + altoCabecera};
        HBRUSH brochaCabecera = CreateSolidBrush(tema.fondoCabecera);
        FillRect(hdc, &rectCabecera, brochaCabecera);
        DeleteObject(brochaCabecera);

        // Bordes separadores
        HPEN plumaBorde = CreatePen(PS_SOLID, 1, tema.borde);
        HPEN plumaVieja = static_cast<HPEN>(SelectObject(hdc, plumaBorde));
        MoveToEx(hdc, rect.left, rect.top + altoCabecera, nullptr);
        LineTo(hdc, rect.right, rect.top + altoCabecera);
        MoveToEx(hdc, rect.left, rect.top, nullptr);
        LineTo(hdc, rect.left, rect.bottom);
        SelectObject(hdc, plumaVieja);
        DeleteObject(plumaBorde);

        SetBkMode(hdc, TRANSPARENT);

        HFONT fuenteTitulo = CreateFontW(-16, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE,
                                         DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                         CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI Variable Display");
        HFONT fuenteSubtitulo = CreateFontW(-12, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                                            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                            CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI Variable Text");
        HFONT fuenteGrande = CreateFontW(-28, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                                         DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                         CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI Variable Display");
        HFONT fuenteInsignia = CreateFontW(-11, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                                           DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                           CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI Variable Text");

        HFONT fuenteVieja = static_cast<HFONT>(SelectObject(hdc, fuenteTitulo));

        // Título de vista
        SetTextColor(hdc, tema.textoPrincipal);
        std::wstring headerText = m_iconoVista + L"  " + m_nombreVista;
        RECT rectTextoTitulo{rect.left + 16, rect.top + 8, rect.right - 16, rect.top + 28};
        DrawTextW(hdc, headerText.c_str(), -1, &rectTextoTitulo,
                  DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS | DT_NOPREFIX);

        // Subtítulo
        SelectObject(hdc, fuenteSubtitulo);
        SetTextColor(hdc, tema.textoSecundario);
        RECT rectTextoSub{rect.left + 16, rect.top + 28, rect.right - 16, rect.top + 46};
        DrawTextW(hdc, L"Módulo de Previsualización Especializada", -1, &rectTextoSub,
                  DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS | DT_NOPREFIX);

        // Tarjeta Central informativa
        int margen = 20;
        int topTarjeta = rect.top + altoCabecera + 24;
        RECT rcTarjeta{rect.left + margen, topTarjeta, rect.right - margen, topTarjeta + 260};

        HBRUSH brTarjeta = CreateSolidBrush(tema.fondoTarjeta);
        FillRect(hdc, &rcTarjeta, brTarjeta);
        DeleteObject(brTarjeta);

        plumaBorde = CreatePen(PS_SOLID, 1, tema.borde);
        plumaVieja = static_cast<HPEN>(SelectObject(hdc, plumaBorde));
        HBRUSH nullBr = static_cast<HBRUSH>(GetStockObject(NULL_BRUSH));
        HBRUSH oldBr = static_cast<HBRUSH>(SelectObject(hdc, nullBr));
        RoundRect(hdc, rcTarjeta.left, rcTarjeta.top, rcTarjeta.right, rcTarjeta.bottom, 12, 12);
        SelectObject(hdc, oldBr);
        SelectObject(hdc, plumaVieja);
        DeleteObject(plumaBorde);

        // Insignia "(En desarrollo)"
        RECT rcBadge{rcTarjeta.left + 16, rcTarjeta.top + 16, rcTarjeta.left + 130, rcTarjeta.top + 40};
        HBRUSH brBadge = CreateSolidBrush(RGB(220, 100, 30)); // Naranja de advertencia amigable
        FillRect(hdc, &rcBadge, brBadge);
        DeleteObject(brBadge);

        SelectObject(hdc, fuenteInsignia);
        SetTextColor(hdc, RGB(255, 255, 255));
        DrawTextW(hdc, L"EN DESARROLLO", -1, &rcBadge, DT_SINGLELINE | DT_CENTER | DT_VCENTER | DT_NOPREFIX);

        // Icono gigante
        SelectObject(hdc, fuenteGrande);
        SetTextColor(hdc, tema.textoPrincipal);
        RECT rcIcono{rcTarjeta.left + 16, rcTarjeta.top + 48, rcTarjeta.right - 16, rcTarjeta.top + 88};
        DrawTextW(hdc, m_iconoVista.c_str(), -1, &rcIcono, DT_SINGLELINE | DT_VCENTER | DT_NOPREFIX);

        // Título del módulo
        SelectObject(hdc, fuenteTitulo);
        SetTextColor(hdc, tema.textoPrincipal);
        RECT rcModTitle{rcTarjeta.left + 16, rcTarjeta.top + 92, rcTarjeta.right - 16, rcTarjeta.top + 116};
        DrawTextW(hdc, m_nombreVista.c_str(), -1, &rcModTitle, DT_SINGLELINE | DT_VCENTER | DT_NOPREFIX);

        // Descripción del módulo
        SelectObject(hdc, fuenteSubtitulo);
        SetTextColor(hdc, tema.textoSecundario);
        RECT rcModDesc{rcTarjeta.left + 16, rcTarjeta.top + 120, rcTarjeta.right - 16, rcTarjeta.top + 175};
        DrawTextW(hdc, m_descripcion.c_str(), -1, &rcModDesc, DT_WORDBREAK | DT_NOPREFIX);

        // Elementos seleccionados
        std::wstring infoSeleccion;
        if (m_archivos.empty()) {
            infoSeleccion = L"Sin archivos seleccionados.";
        } else if (m_archivos.size() == 1) {
            infoSeleccion = L"Archivo: " + m_archivos.front();
        } else {
            infoSeleccion = L"Selección múltiple: " + std::to_wstring(m_archivos.size()) + L" elementos";
        }
        RECT rcModSel{rcTarjeta.left + 16, rcTarjeta.top + 180, rcTarjeta.right - 176, rcTarjeta.bottom - 48};
        DrawTextW(hdc, infoSeleccion.c_str(), -1, &rcModSel, DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);

        m_rcComparar = {rcTarjeta.right - 158, rcTarjeta.bottom - 42,
                        rcTarjeta.right - 16, rcTarjeta.bottom - 14};
        const bool puedeComparar = PuedeCompararSeleccionV2(m_archivos);
        HBRUSH fondoComparar = CreateSolidBrush(puedeComparar
            ? tema.botonFondo : tema.fondoCabecera);
        FillRect(hdc, &m_rcComparar, fondoComparar);
        DeleteObject(fondoComparar);
        SetTextColor(hdc, puedeComparar
            ? tema.textoPrincipal : tema.textoSecundario);
        DrawTextW(hdc, L"Comparar archivos", -1, &m_rcComparar,
                  DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

        SelectObject(hdc, fuenteVieja);
        DeleteObject(fuenteTitulo);
        DeleteObject(fuenteSubtitulo);
        DeleteObject(fuenteGrande);
        DeleteObject(fuenteInsignia);
    }

private:
    std::wstring m_nombreVista;
    std::wstring m_iconoVista;
    std::wstring m_descripcion;
    std::vector<std::wstring> m_archivos;
    std::wstring m_carpeta;
    RECT m_rcComparar{};
};

} // namespace SideViewV2
/**** ended inlining v2/vistas/vista_en_desarrollo.hpp ****/
/**** start inlining v2/nucleo/fabrica_vistas.hpp ****/

/**** skipping file: v2/ui/panel_interfaz.hpp ****/
/**** skipping file: v2/nucleo/enrutador_vistas.hpp ****/
/**** skipping file: v2/vistas/vista_walkthrough.hpp ****/
/**** skipping file: v2/vistas/vista_comprimidos.hpp ****/
/**** start inlining v2/vistas/vista_comics.hpp ****/

/**** skipping file: v2/ui/panel_interfaz.hpp ****/
/**** skipping file: v2/ui/ventana_base.hpp ****/
/**** skipping file: v2/nucleo/enrutador_vistas.hpp ****/
/**** skipping file: v2/nucleo/servicios_vistas.hpp ****/
#include <windows.h>
#include <windowsx.h>
#include <shlwapi.h>
#include <string>
#include <vector>
#include <algorithm>

namespace SideViewV2 {

class VistaComics : public IPanelContenido {
public:
    explicit VistaComics(std::shared_ptr<ServicioExtensionV2> extensiones,
                         std::shared_ptr<ServicioNavegacionV2> navegacion,
                         std::shared_ptr<ServicioPrevisualizacionMediaV2> previsualizacion)
        : m_extensiones(std::move(extensiones)), m_navegacion(std::move(navegacion)), m_previsualizacion(std::move(previsualizacion)) {}

    ~VistaComics() override { Liberar(); }

    void AlCambiarSeleccion(const std::vector<std::wstring>& archivos,
                            const std::wstring&) override {
        m_archivos = archivos;
        m_indice = 0;
        CargarPortadas();
    }

    void Liberar() override {
        for (HBITMAP portada : m_portadas) if (portada) DeleteObject(portada);
        m_portadas.clear();
    }

    bool ProcesarMensajeRaton(UINT msg, WPARAM, LPARAM lParam, HWND hwnd) override {
        if (msg != WM_LBUTTONDOWN) return false;
        POINT pt{GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
        if (PtInRect(&m_rcAnterior, pt) && m_archivos.size() > 1) {
            m_indice = m_navegacion->Anterior(m_indice, m_archivos.size());
            InvalidateRect(hwnd, nullptr, FALSE); return true;
        }
        if (PtInRect(&m_rcSiguiente, pt) && m_archivos.size() > 1) {
            m_indice = m_navegacion->Siguiente(m_indice, m_archivos.size());
            InvalidateRect(hwnd, nullptr, FALSE); return true;
        }
        if (PtInRect(&m_rcCambiarExtension, pt)) {
            MostrarMenuExtensiones(hwnd); return true;
        }
        if (PtInRect(&m_rcComparar, pt)) {
            if (PuedeCompararSeleccionV2(m_archivos)) {
                PostMessageW(hwnd, WM_SIDEVIEW_REQUEST_COMPARADOR_ARCHIVOS, 0, 0);
            }
            return true;
        }
        return false;
    }

    void Renderizar(HDC hdc, const RECT& rect, const TemaVisual& tema) override {
        if (!hdc || rect.right <= rect.left || rect.bottom <= rect.top) return;
        if (m_indice >= m_archivos.size()) m_indice = 0;
        HBRUSH fondo = CreateSolidBrush(tema.fondo); FillRect(hdc, &rect, fondo); DeleteObject(fondo);
        SetBkMode(hdc, TRANSPARENT);
        HFONT titulo = CreateFontW(-17, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE,
                                   DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                   CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI Variable Display");
        HFONT texto = CreateFontW(-12, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                                  DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                  CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI Variable Text");
        HFONT anterior = static_cast<HFONT>(SelectObject(hdc, titulo));
        SetTextColor(hdc, tema.textoPrincipal);
        RECT rcTitulo{rect.left + 16, rect.top + 10, rect.right - 16, rect.top + 35};
        std::wstring nombre = m_archivos.empty() ? L"Sin comics seleccionados" :
            L"📚 " + std::wstring(PathFindFileNameW(m_archivos[m_indice].c_str()));
        DrawTextW(hdc, nombre.c_str(), -1, &rcTitulo, DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS | DT_NOPREFIX);
        SelectObject(hdc, texto); SetTextColor(hdc, tema.textoSecundario);
        RECT rcSub{rect.left + 16, rect.top + 36, rect.right - 16, rect.top + 58};
        std::wstring sub = std::to_wstring(m_archivos.size()) + L" comic(s) · Cover de la primera página";
        DrawTextW(hdc, sub.c_str(), -1, &rcSub, DT_SINGLELINE | DT_VCENTER | DT_NOPREFIX);

        int yBoton = rect.top + 72;
        m_rcAnterior = {rect.left + 16, yBoton, rect.left + 50, yBoton + 30};
        m_rcSiguiente = {rect.left + 56, yBoton, rect.left + 90, yBoton + 30};
        DibujarBoton(hdc, m_rcAnterior, L"◀", tema, m_archivos.size() > 1);
        DibujarBoton(hdc, m_rcSiguiente, L"▶", tema, m_archivos.size() > 1);
        m_rcCambiarExtension = {rect.right - 170, yBoton, rect.right - 16, yBoton + 30};
        DibujarBoton(hdc, m_rcCambiarExtension, L"Cambiar extensión", tema, true);
        m_rcComparar = {rect.right - 296, yBoton, rect.right - 178, yBoton + 30};
        DibujarBoton(hdc, m_rcComparar, L"Comparar archivos", tema,
                     PuedeCompararSeleccionV2(m_archivos));

        // Contenedor cuadrado con grid fijo 2x2. Evita que los covers se
        // conviertan en una fila horizontal y mantiene una composición
        // estable con una, dos, tres o cuatro selecciones.
        int top = yBoton + 48;
        int bottom = rect.bottom - 18;
        int anchoDisponible = rect.right - rect.left - 32;
        int altoDisponible = bottom - top - 30;
        int ladoContenedor = std::max(160, std::min(anchoDisponible, altoDisponible));
        ladoContenedor = std::min(ladoContenedor, std::max(160, anchoDisponible));
        int gridLeft = rect.left + ((rect.right - rect.left) - ladoContenedor) / 2;
        int gridTop = top + std::max(0, (altoDisponible - ladoContenedor) / 2);
        RECT contenedorGrid{gridLeft, gridTop, gridLeft + ladoContenedor, gridTop + ladoContenedor};
        HBRUSH fondoGrid = CreateSolidBrush(tema.fondoTarjeta);
        FillRect(hdc, &contenedorGrid, fondoGrid);
        DeleteObject(fondoGrid);
        HPEN bordeGrid = CreatePen(PS_SOLID, 1, tema.borde);
        HPEN anteriorGrid = static_cast<HPEN>(SelectObject(hdc, bordeGrid));
        HBRUSH sinRelleno = static_cast<HBRUSH>(GetStockObject(NULL_BRUSH));
        HBRUSH anteriorRelleno = static_cast<HBRUSH>(SelectObject(hdc, sinRelleno));
        RoundRect(hdc, contenedorGrid.left, contenedorGrid.top, contenedorGrid.right,
                  contenedorGrid.bottom, 10, 10);
        SelectObject(hdc, anteriorRelleno);
        SelectObject(hdc, anteriorGrid);
        DeleteObject(bordeGrid);

        int separacion = 8;
        int ladoCelda = (ladoContenedor - separacion * 3) / 2;
        size_t cantidad = std::min<size_t>(4, m_archivos.size());
        for (size_t i = 0; i < cantidad; ++i) {
            int fila = static_cast<int>(i / 2);
            int columna = static_cast<int>(i % 2);
            int tileLeft = contenedorGrid.left + separacion + columna * (ladoCelda + separacion);
            int tileTop = contenedorGrid.top + separacion + fila * (ladoCelda + separacion);
            RECT tile{tileLeft, tileTop, tileLeft + ladoCelda, tileTop + ladoCelda};
            HBRUSH tarjeta = CreateSolidBrush(tema.fondoCabecera); FillRect(hdc, &tile, tarjeta); DeleteObject(tarjeta);
            if (i < m_portadas.size() && m_portadas[i]) {
                BITMAP bm{};
                if (GetObjectW(m_portadas[i], sizeof(bm), &bm) != sizeof(bm) ||
                    bm.bmWidth <= 0 || bm.bmHeight == 0) {
                    continue;
                }
                HDC mem = CreateCompatibleDC(hdc); if (!mem) continue; HGDIOBJ old = SelectObject(mem, m_portadas[i]); if (!old || old == HGDI_ERROR) { DeleteDC(mem); continue; }
                int margen = 8; int destinoAncho = ladoCelda - margen * 2;
                int anchoBitmap = std::max(1, static_cast<int>(bm.bmWidth));
                int altoDisponiblePortada = ladoCelda - margen * 2;
                int altoPortada = MulDiv(static_cast<int>(bm.bmHeight), destinoAncho, anchoBitmap);
                int destinoAlto = std::min(altoDisponiblePortada, altoPortada);
                int destinoX = tile.left + (ladoCelda - destinoAncho) / 2;
                int destinoY = tile.top + (ladoCelda - destinoAlto) / 2;
                int modoAnterior = SetStretchBltMode(hdc, HALFTONE);
                SetBrushOrgEx(hdc, 0, 0, nullptr);
                StretchBlt(hdc, destinoX, destinoY, destinoAncho, destinoAlto,
                           mem, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY);
                SetStretchBltMode(hdc, modoAnterior);
                SelectObject(mem, old); DeleteDC(mem);
            } else {
                SetTextColor(hdc, tema.acento);
                DrawTextW(hdc, L"COVER\nno disponible", -1, &tile, DT_CENTER | DT_VCENTER | DT_NOPREFIX);
            }
        }
        if (m_archivos.size() > 4) {
            RECT contador{contenedorGrid.left, contenedorGrid.bottom + 4,
                          contenedorGrid.right, bottom};
            SetTextColor(hdc, tema.textoSecundario);
            std::wstring extra = L"+" + std::to_wstring(m_archivos.size() - 4) + L" comics adicionales";
            DrawTextW(hdc, extra.c_str(), -1, &contador,
                      DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
        }
        SelectObject(hdc, anterior); DeleteObject(titulo); DeleteObject(texto);
    }

private:
    void DibujarBoton(HDC hdc, const RECT& rc, const wchar_t* texto,
                      const TemaVisual& tema, bool habilitado) {
        HBRUSH fondo = CreateSolidBrush(habilitado ? tema.botonFondo : tema.fondoCabecera);
        FillRect(hdc, &rc, fondo); DeleteObject(fondo);
        HPEN borde = CreatePen(PS_SOLID, 1, tema.borde);
        HPEN anteriorPen = static_cast<HPEN>(SelectObject(hdc, borde));
        HBRUSH transparente = static_cast<HBRUSH>(GetStockObject(NULL_BRUSH));
        HBRUSH anteriorBrush = static_cast<HBRUSH>(SelectObject(hdc, transparente));
        RoundRect(hdc, rc.left, rc.top, rc.right, rc.bottom, 6, 6);
        SelectObject(hdc, anteriorBrush); SelectObject(hdc, anteriorPen); DeleteObject(borde);
        SetTextColor(hdc, habilitado ? tema.textoPrincipal : tema.textoSecundario);
        RECT textoRect = rc;
        DrawTextW(hdc, texto, -1, &textoRect, DT_SINGLELINE | DT_CENTER | DT_VCENTER | DT_NOPREFIX);
    }

    void CargarPortadas() {
        Liberar();
        size_t cantidad = std::min<size_t>(4, m_archivos.size());
        for (size_t i = 0; i < cantidad; ++i) {
            HBITMAP bitmap = m_previsualizacion->CargarMiniatura(m_archivos[i], SIZE{420, 620});
            // Conservamos una entrada por celda; un thumbnail ausente se
            // representa como fallback visual y nunca se usa como bitmap.
            m_portadas.push_back(bitmap);
        }
    }

    void MostrarMenuExtensiones(HWND hwnd) {
        HMENU menu = CreatePopupMenu();
        const wchar_t* opciones[] = {L".cbz", L".cbr", L".cb7", L".cbt", L".cba", L".zip", L".rar"};
        for (UINT i = 0; i < ARRAYSIZE(opciones); ++i) AppendMenuW(menu, MF_STRING, i + 1, opciones[i]);
        POINT p{m_rcCambiarExtension.left, m_rcCambiarExtension.bottom}; ClientToScreen(hwnd, &p);
        UINT cmd = static_cast<UINT>(TrackPopupMenu(menu, TPM_LEFTALIGN | TPM_TOPALIGN | TPM_RETURNCMD, p.x, p.y, 0, hwnd, nullptr));
        DestroyMenu(menu);
        if (cmd >= 1 && cmd <= ARRAYSIZE(opciones)) {
            size_t cantidad = m_extensiones->Cambiar(m_archivos, opciones[cmd - 1]);
            MessageBoxW(hwnd, (L"Se cambiaron " + std::to_wstring(cantidad) + L" archivo(s)." ).c_str(), L"Cambiar extensión", MB_OK | MB_ICONINFORMATION);
        }
    }

    std::vector<std::wstring> m_archivos;
    std::vector<HBITMAP> m_portadas;
    std::shared_ptr<ServicioExtensionV2> m_extensiones;
    std::shared_ptr<ServicioNavegacionV2> m_navegacion;
    std::shared_ptr<ServicioPrevisualizacionMediaV2> m_previsualizacion;
    size_t m_indice{0};
    RECT m_rcAnterior{}, m_rcSiguiente{}, m_rcCambiarExtension{}, m_rcComparar{};
};
}
/**** ended inlining v2/vistas/vista_comics.hpp ****/
/**** skipping file: v2/vistas/vista_carpetas.hpp ****/
/**** skipping file: v2/vistas/vista_en_desarrollo.hpp ****/
/**** start inlining v2/vistas/vista_medios.hpp ****/

/**** skipping file: v2/ui/panel_interfaz.hpp ****/
/**** skipping file: v2/nucleo/configuracion_v2.hpp ****/
/**** skipping file: v2/nucleo/enrutador_vistas.hpp ****/
/**** skipping file: v2/nucleo/servicios_vistas.hpp ****/
#include <windows.h>
#include <windowsx.h>
#include <shlwapi.h>
#include <string>
#include <vector>
#include <memory>

namespace SideViewV2 {

inline void DibujarBotonMediaV2(HDC hdc, const RECT& rc, const wchar_t* texto,
                                const TemaVisual& tema, bool activo = true) {
    HBRUSH fondo = CreateSolidBrush(activo ? tema.botonFondo : tema.fondoCabecera);
    FillRect(hdc, &rc, fondo); DeleteObject(fondo);
    SetBkMode(hdc, TRANSPARENT); SetTextColor(hdc, activo ? tema.textoPrincipal : tema.textoSecundario);
    RECT textoRect = rc;
    DrawTextW(hdc, texto, -1, &textoRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
}

inline std::wstring TamanoMediaV2(const std::wstring& ruta) {
    WIN32_FILE_ATTRIBUTE_DATA datos{};
    if (!GetFileAttributesExW(ruta.c_str(), GetFileExInfoStandard, &datos)) return L"Tamaño no disponible";
    ULONGLONG bytes = (static_cast<ULONGLONG>(datos.nFileSizeHigh) << 32) | datos.nFileSizeLow;
    if (bytes < 1024) return std::to_wstring(bytes) + L" B";
    if (bytes < 1024 * 1024) return std::to_wstring(bytes / 1024) + L" KB";
    if (bytes < 1024ULL * 1024 * 1024) return std::to_wstring(bytes / (1024 * 1024)) + L" MB";
    return std::to_wstring(bytes / (1024ULL * 1024 * 1024)) + L" GB";
}

class VistaMediaV2 : public IPanelContenido {
protected:
    VistaMediaV2(std::shared_ptr<ServicioNavegacionV2> navegacion,
                 std::shared_ptr<ServicioAbrirConV2> abrirCon,
                 std::shared_ptr<ServicioPrevisualizacionMediaV2> previsualizacion)
        : m_navegacion(std::move(navegacion)), m_abrirCon(std::move(abrirCon)),
          m_previsualizacion(std::move(previsualizacion)) {}

    bool ProcesarNavegacion(UINT msg, WPARAM wParam, LPARAM lParam, HWND hwnd) {
        if (msg == WM_KEYDOWN && (wParam == VK_LEFT || wParam == VK_RIGHT)) {
            CambiarIndice(wParam == VK_LEFT ? -1 : 1); InvalidateRect(hwnd, nullptr, FALSE); return true;
        }
        if (msg != WM_LBUTTONDOWN) return false;
        POINT pt{GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
        if (PtInRect(&m_rcAnterior, pt)) { CambiarIndice(-1); InvalidateRect(hwnd, nullptr, FALSE); return true; }
        if (PtInRect(&m_rcSiguiente, pt)) { CambiarIndice(1); InvalidateRect(hwnd, nullptr, FALSE); return true; }
        if (PtInRect(&m_rcAbrirCon, pt)) {
            if (m_archivos.empty()) return true;
            if (m_abrirCon->ProgramaActual().empty()) m_abrirCon->AbrirDialogo(hwnd);
            m_abrirCon->Ejecutar({m_archivos[m_indice]}); return true;
        }
        if (MostrarComparador() && PtInRect(&m_rcComparar, pt)) {
            if (PuedeCompararSeleccionV2(m_archivos)) {
                PostMessageW(hwnd, WM_SIDEVIEW_REQUEST_COMPARADOR_ARCHIVOS, 0, 0);
            }
            return true;
        }
        return false;
    }

    void DibujarCabecera(HDC hdc, const RECT& rect, const TemaVisual& tema,
                         const wchar_t* titulo, const std::wstring& nombre) {
        SetBkMode(hdc, TRANSPARENT); SetTextColor(hdc, tema.textoPrincipal);
        RECT rTitulo{rect.left + 16, rect.top + 10, rect.right - 16, rect.top + 36};
        DrawTextW(hdc, titulo, -1, &rTitulo, DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS | DT_NOPREFIX);
        SetTextColor(hdc, tema.textoSecundario);
        RECT rNombre{rect.left + 16, rect.top + 38, rect.right - 16, rect.top + 60};
        std::wstring detalle = nombre;
        if (!m_archivos.empty()) detalle += L"  ·  " + TamanoMediaV2(m_archivos[m_indice]);
        DrawTextW(hdc, detalle.c_str(), -1, &rNombre, DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS | DT_NOPREFIX);
        int y = rect.top + 72;
        m_rcAnterior = {rect.left + 16, y, rect.left + 52, y + 30};
        m_rcSiguiente = {rect.left + 58, y, rect.left + 94, y + 30};
        m_rcComparar = {rect.right - 250, y, rect.right - 148, y + 30};
        m_rcAbrirCon = {rect.right - 140, y, rect.right - 16, y + 30};
        DibujarBotonMediaV2(hdc, m_rcAnterior, L"◀", tema, m_archivos.size() > 1);
        DibujarBotonMediaV2(hdc, m_rcSiguiente, L"▶", tema, m_archivos.size() > 1);
        if (MostrarComparador()) {
            DibujarBotonMediaV2(hdc, m_rcComparar, L"Comparar", tema,
                                PuedeCompararSeleccionV2(m_archivos));
        }
        DibujarBotonMediaV2(hdc, m_rcAbrirCon, L"Abrir con", tema);
    }

    void DibujarPreview(HDC hdc, const RECT& rect, const TemaVisual& tema, HBITMAP bitmap) {
        RECT contenedor{rect.left + 16, rect.top + 118, rect.right - 16, rect.bottom - 16};
        HBRUSH fondo = CreateSolidBrush(tema.fondoTarjeta); FillRect(hdc, &contenedor, fondo); DeleteObject(fondo);
        if (!bitmap) { SetTextColor(hdc, tema.textoSecundario); DrawTextW(hdc, L"Vista previa no disponible", -1, &contenedor, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX); return; }
        BITMAP bm{};
        if (GetObjectW(bitmap, sizeof(bm), &bm) != sizeof(bm) || bm.bmWidth <= 0 || bm.bmHeight == 0) {
            SetTextColor(hdc, tema.textoSecundario);
            DrawTextW(hdc, L"Vista previa no disponible", -1, &contenedor,
                      DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
            return;
        }
        int w = contenedor.right - contenedor.left - 24, h = contenedor.bottom - contenedor.top - 24;
        int pw = w, ph = MulDiv(static_cast<int>(bm.bmHeight), w, std::max(1, static_cast<int>(bm.bmWidth)));
        if (ph > h) { ph = h; pw = MulDiv(static_cast<int>(bm.bmWidth), h, std::max(1, static_cast<int>(bm.bmHeight))); }
        HDC mem = CreateCompatibleDC(hdc); if (!mem) return; HGDIOBJ old = SelectObject(mem, bitmap); if (!old || old == HGDI_ERROR) { DeleteDC(mem); return; }
        int modoAnterior = SetStretchBltMode(hdc, HALFTONE);
        SetBrushOrgEx(hdc, 0, 0, nullptr);
        StretchBlt(hdc, contenedor.left + (w - pw) / 2 + 12, contenedor.top + (h - ph) / 2 + 12, pw, ph, mem, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY);
        SetStretchBltMode(hdc, modoAnterior);
        SelectObject(mem, old); DeleteDC(mem);
    }

    void CambiarIndice(int delta) {
        if (m_archivos.size() < 2) return;
        m_indice = delta < 0 ? m_navegacion->Anterior(m_indice, m_archivos.size()) : m_navegacion->Siguiente(m_indice, m_archivos.size());
        CargarActual();
    }

    void ReemplazarPreview(HBITMAP nuevo) {
        if (m_preview && m_preview != nuevo) DeleteObject(m_preview);
        m_preview = nuevo;
    }

    void Liberar() override {
        if (m_preview) DeleteObject(m_preview);
        m_preview = nullptr;
        m_archivos.clear();
        m_indice = 0;
        m_rcAnterior = {};
        m_rcSiguiente = {};
        m_rcAbrirCon = {};
        m_rcComparar = {};
    }

    virtual void CargarActual() = 0;
    virtual bool MostrarComparador() const { return true; }
    std::vector<std::wstring> m_archivos; size_t m_indice{0}; HBITMAP m_preview{nullptr};
    std::shared_ptr<ServicioNavegacionV2> m_navegacion;
    std::shared_ptr<ServicioAbrirConV2> m_abrirCon;
    std::shared_ptr<ServicioPrevisualizacionMediaV2> m_previsualizacion;
    RECT m_rcAnterior{}, m_rcSiguiente{}, m_rcAbrirCon{}, m_rcComparar{};
};

class VistaImagenes final : public VistaMediaV2 {
public:
    VistaImagenes(std::shared_ptr<ServicioNavegacionV2> n, std::shared_ptr<ServicioAbrirConV2> a, std::shared_ptr<ServicioPrevisualizacionMediaV2> p) : VistaMediaV2(std::move(n), std::move(a), std::move(p)) {}
    ~VistaImagenes() override { Liberar(); }
    void AlCambiarSeleccion(const std::vector<std::wstring>& archivos, const std::wstring&) override { Liberar(); m_archivos = archivos; m_indice = 0; CargarActual(); }
    bool ProcesarMensajeRaton(UINT m, WPARAM w, LPARAM l, HWND h) override { return ProcesarNavegacion(m, w, l, h); }
    bool ProcesarMensajeTeclado(UINT m, WPARAM w, LPARAM l, HWND h) override { return ProcesarNavegacion(m, w, l, h); }
    void Renderizar(HDC hdc, const RECT& rect, const TemaVisual& tema) override { HBRUSH b=CreateSolidBrush(tema.fondo); FillRect(hdc,&rect,b); DeleteObject(b); std::wstring n=m_archivos.empty()?L"Sin imágenes":PathFindFileNameW(m_archivos[m_indice].c_str()); DibujarCabecera(hdc,rect,tema,L"🖼 Imágenes",n); DibujarPreview(hdc,rect,tema,m_preview); }
private: void CargarActual() override { ReemplazarPreview(m_archivos.empty() ? nullptr : m_previsualizacion->CargarImagen(m_archivos[m_indice])); }
};

class VistaVideos final : public VistaMediaV2 {
public:
    VistaVideos(std::shared_ptr<ServicioNavegacionV2> n,
                std::shared_ptr<ServicioAbrirConV2> a,
                std::shared_ptr<ServicioPrevisualizacionMediaV2> p,
                std::shared_ptr<ServicioReproductorVideoV2> r)
        : VistaMediaV2(std::move(n), std::move(a), std::move(p)),
          m_servicioReproductor(std::move(r)) {}
    ~VistaVideos() override { Liberar(); }
    void AlActivar(HWND panel) override {
        m_panel = panel;
        CargarActual();
    }
    void AlCambiarSeleccion(const std::vector<std::wstring>& archivos,
                            const std::wstring&) override {
        HWND panel = m_panel;
        Liberar();
        m_panel = panel;
        m_archivos = archivos;
        m_indice = 0;
        CargarActual();
    }
    void ActualizarConfiguracion() override {
        const bool debeUsarReproductor = ReproductorVideoEnVistaDisponibleV2();
        if (debeUsarReproductor == (m_reproductor != nullptr)) return;
        DesmontarReproductor();
        ReemplazarPreview(nullptr);
        CargarActual();
    }
    bool ProcesarMensajeRaton(UINT m, WPARAM w, LPARAM l, HWND h) override { return ProcesarNavegacion(m, w, l, h); }
    bool ProcesarMensajeTeclado(UINT m, WPARAM w, LPARAM l, HWND h) override { return ProcesarNavegacion(m, w, l, h); }
    void Renderizar(HDC hdc, const RECT& rect, const TemaVisual& tema) override {
        HBRUSH b = CreateSolidBrush(tema.fondo);
        FillRect(hdc, &rect, b);
        DeleteObject(b);

        const std::wstring nombre = m_archivos.empty()
            ? L"Sin videos" : PathFindFileNameW(m_archivos[m_indice].c_str());
        DibujarCabecera(
            hdc, rect, tema,
            ReproductorVideoEnVistaDisponibleV2()
                ? L"🎬 Videos · Reproductor"
                : L"🎬 Videos · Vista previa",
            nombre);

        const RECT contenedor{rect.left + 16, rect.top + 118,
                              rect.right - 16, rect.bottom - 16};
        if (m_reproductor) {
            // El reproductor reserva internamente la franja superior del
            // marco para que el título permanezca dibujado por la vista.
            RECT ranura = contenedor;
            ranura.top -= 30;
            m_reproductor->ActualizarRect(ranura, false);
        } else {
            DibujarPreview(hdc, rect, tema, m_preview);
        }
    }

    void Liberar() override {
        DesmontarReproductor();
        VistaMediaV2::Liberar();
        m_panel = nullptr;
    }

private:
    void CargarActual() override {
        if (m_archivos.empty() || m_indice >= m_archivos.size()) {
            DesmontarReproductor();
            ReemplazarPreview(nullptr);
            return;
        }

        if (ReproductorVideoEnVistaDisponibleV2() && MontarReproductorActual()) {
            ReemplazarPreview(nullptr);
            return;
        }

        DesmontarReproductor();
        ReemplazarPreview(m_previsualizacion
            ? m_previsualizacion->CargarMiniatura(m_archivos[m_indice])
            : nullptr);
    }

    bool MontarReproductorActual() {
        if (!m_panel || !m_servicioReproductor) return false;
        DesmontarReproductor();
        std::unique_ptr<IReproductorVideoV2> reproductor =
            m_servicioReproductor->Crear();
        if (!reproductor || !reproductor->Montar(m_panel, 0, m_archivos[m_indice])) {
            return false;
        }
        m_reproductor = std::move(reproductor);
        return true;
    }

    void DesmontarReproductor() {
        if (m_reproductor) {
            m_reproductor->Liberar();
            m_reproductor.reset();
        }
    }

    HWND m_panel{nullptr};
    std::shared_ptr<ServicioReproductorVideoV2> m_servicioReproductor;
    std::unique_ptr<IReproductorVideoV2> m_reproductor;
};
}
/**** ended inlining v2/vistas/vista_medios.hpp ****/
/**** start inlining v2/vistas/vista_texto_codigo.hpp ****/

/**** skipping file: v2/ui/panel_interfaz.hpp ****/
/**** skipping file: v2/nucleo/configuracion_v2.hpp ****/
/**** skipping file: v2/nucleo/enrutador_vistas.hpp ****/
/**** skipping file: v2/nucleo/servicio_texto.hpp ****/
/**** skipping file: v2/nucleo/servicio_navegacion.hpp ****/
#include <windowsx.h>
#include <algorithm>
#include <memory>
#include <string>

namespace SideViewV2 {

class VistaTextoCodigo : public IPanelContenido {
public:
    VistaTextoCodigo(std::shared_ptr<ServicioTextoV2> texto,
                     std::shared_ptr<ServicioNavegacionV2> nav,
                     bool codigo)
        : m_texto(std::move(texto)), m_nav(std::move(nav)), m_codigo(codigo) {}

    void Liberar() override {
        m_documento = {};
        m_archivos.clear();
        m_indiceArchivo = 0;
        m_lineaInicial = 0;
        m_rcAnterior = {};
        m_rcSiguiente = {};
        m_rcComparar = {};
        m_rcCopiar = {};
        m_rcRenombrar = {};
        m_rcIndice = {};
    }

    void AlCambiarSeleccion(const std::vector<std::wstring>& archivos,
                            const std::wstring&) override {
        Liberar();
        m_archivos = archivos;
        Cargar();
    }

    bool ProcesarMensajeRaton(UINT msg, WPARAM, LPARAM lParam,
                              HWND hwnd) override {
        if (msg != WM_LBUTTONDOWN) return false;
        POINT punto{GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};

        if (PtInRect(&m_rcAnterior, punto)) {
            Cambiar(-1, hwnd);
            return true;
        }
        if (PtInRect(&m_rcSiguiente, punto)) {
            Cambiar(1, hwnd);
            return true;
        }
        if (PtInRect(&m_rcCopiar, punto)) {
            if (m_texto) m_texto->Copiar(Unir());
            return true;
        }
        if (PtInRect(&m_rcRenombrar, punto) && !m_documento.ruta.empty()) {
            if (m_texto && m_texto->Renombrar(hwnd, m_documento.ruta)) Cargar();
            return true;
        }
        if (PtInRect(&m_rcComparar, punto)) {
            if (PuedeCompararSeleccionV2(m_archivos)) {
                PostMessageW(hwnd, WM_SIDEVIEW_REQUEST_COMPARADOR_ARCHIVOS, 0, 0);
            }
            return true;
        }

        for (size_t i = 0; i < m_documento.indice.size(); ++i) {
            RECT elemento{
                m_rcIndice.left,
                m_rcIndice.top + static_cast<int>(i * kAltoEntradaIndice),
                m_rcIndice.right,
                m_rcIndice.top + static_cast<int>((i + 1) * kAltoEntradaIndice)
            };
            if (PtInRect(&elemento, punto)) {
                const size_t numeroLinea = m_documento.indice[i];
                if (numeroLinea < m_documento.lineas.size()) {
                    m_lineaInicial = numeroLinea;
                    InvalidateRect(hwnd, nullptr, FALSE);
                }
                return true;
            }
        }
        return PtInRect(&m_rcIndice, punto);
    }

    bool ProcesarMensajeTeclado(UINT msg, WPARAM wParam, LPARAM,
                                HWND hwnd) override {
        if (msg != WM_KEYDOWN) return false;
        if (wParam == VK_LEFT) {
            Cambiar(-1, hwnd);
            return true;
        }
        if (wParam == VK_RIGHT) {
            Cambiar(1, hwnd);
            return true;
        }
        if (wParam == VK_HOME) {
            m_lineaInicial = 0;
            InvalidateRect(hwnd, nullptr, FALSE);
            return true;
        }
        return false;
    }

    void Renderizar(HDC hdc, const RECT& rect, const TemaVisual& tema) override {
        if (!hdc || rect.right <= rect.left || rect.bottom <= rect.top) return;

        HBRUSH fondo = CreateSolidBrush(tema.fondo);
        FillRect(hdc, &rect, fondo);
        DeleteObject(fondo);
        SetBkMode(hdc, TRANSPARENT);

        const int tamanoFuente = m_codigo ? TamanoFuenteCodigoV2() : TamanoFuenteTextoV2();
        HFONT fuente = CrearFuenteDocumento(tamanoFuente, m_codigo);
        HGDIOBJ fuenteAnterior = nullptr;
        if (fuente) fuenteAnterior = SelectObject(hdc, fuente);

        const int top = 8;
        m_rcAnterior = {rect.left + 8, rect.top + top,
                        rect.left + 72, rect.top + top + 28};
        m_rcSiguiente = {rect.left + 78, rect.top + top,
                         rect.left + 142, rect.top + top + 28};
        m_rcComparar = {rect.right - 300, rect.top + top,
                        rect.right - 178, rect.top + top + 28};
        m_rcCopiar = {rect.right - 170, rect.top + top,
                      rect.right - 88, rect.top + top + 28};
        m_rcRenombrar = {rect.right - 82, rect.top + top,
                         rect.right - 8, rect.top + top + 28};

        DibujarBoton(hdc, m_rcAnterior, L"◀", tema);
        DibujarBoton(hdc, m_rcSiguiente, L"▶", tema);
        DibujarBoton(hdc, m_rcComparar, L"Comparar", tema,
                     PuedeCompararSeleccionV2(m_archivos));
        DibujarBoton(hdc, m_rcCopiar, L"Copiar", tema);
        DibujarBoton(hdc, m_rcRenombrar, L"Renombrar", tema);

        SetTextColor(hdc, tema.textoPrincipal);
        RECT titulo{rect.left + 154, rect.top + top,
                    rect.right - 180, rect.top + top + 28};
        std::wstring nombre = m_documento.ruta;
        const size_t separador = nombre.find_last_of(L"\\/");
        if (separador != std::wstring::npos) nombre = nombre.substr(separador + 1);
        DrawTextW(hdc, nombre.c_str(), -1, &titulo,
                  DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS | DT_NOPREFIX);

        m_rcIndice = {rect.left + 8, rect.top + 48,
                      rect.left + 190, rect.bottom - 8};
        HBRUSH fondoIndice = CreateSolidBrush(tema.fondoCabecera);
        FillRect(hdc, &m_rcIndice, fondoIndice);
        DeleteObject(fondoIndice);
        DibujarIndice(hdc, tema);

        const bool mostrarNumeros = g_mostrarNumerosLineaV2.load(std::memory_order_acquire);
        const bool ajustarLinea = AjusteLineaTextoCodigoDisponibleV2();
        const int xContenido = rect.left + 202 + (mostrarNumeros ? 52 : 4);
        const int anchoContenido = std::max(
            1, static_cast<int>(rect.right - 8) - xContenido);
        const int altoBase = std::max(16, tamanoFuente + 7);
        int y = rect.top + 54;

        SetTextColor(hdc, tema.textoPrincipal);
        for (size_t i = m_lineaInicial;
             i < m_documento.lineas.size() && y < rect.bottom - 8; ++i) {
            const std::wstring& linea = m_documento.lineas[i];
            const int altoDisponible = rect.bottom - 8 - y;
            if (altoDisponible <= 0) break;

            int altoLinea = altoBase;
            if (ajustarLinea) {
                RECT calculo{xContenido, 0, xContenido + anchoContenido, 0};
                const int altoCalculado = DrawTextW(
                    hdc, linea.c_str(), -1, &calculo,
                    DT_CALCRECT | DT_WORDBREAK | DT_NOPREFIX);
                if (altoCalculado > 0) {
                    altoLinea = std::max(
                        altoBase, static_cast<int>(calculo.bottom - calculo.top));
                }
            }
            altoLinea = std::min(altoLinea, altoDisponible);
            DibujarLinea(hdc, linea, tema, rect, xContenido, y,
                         anchoContenido, altoLinea, mostrarNumeros, i,
                         ajustarLinea,
                         m_codigo && EsClave(linea) ? RGB(180, 100, 220)
                                                    : ColorLinea(linea, tema));
            y += altoLinea;
        }

        if (fuente) {
            if (fuenteAnterior && fuenteAnterior != HGDI_ERROR) {
                SelectObject(hdc, fuenteAnterior);
            }
            DeleteObject(fuente);
        }
    }

private:
    static constexpr int kAltoEntradaIndice = 22;

    void Cargar() {
        if (!m_texto || m_archivos.empty() || m_indiceArchivo >= m_archivos.size()) {
            m_documento = {};
            m_lineaInicial = 0;
            return;
        }
        m_documento = m_texto->Cargar(m_archivos[m_indiceArchivo]);
        m_lineaInicial = 0;
    }

    void Cambiar(int delta, HWND hwnd) {
        if (m_archivos.size() < 2 || !m_nav) return;
        m_indiceArchivo = delta < 0
            ? m_nav->Anterior(m_indiceArchivo, m_archivos.size())
            : m_nav->Siguiente(m_indiceArchivo, m_archivos.size());
        Cargar();
        InvalidateRect(hwnd, nullptr, FALSE);
    }

    std::wstring Unir() const {
        size_t capacidad = 0;
        for (const auto& linea : m_documento.lineas) capacidad += linea.size() + 2;
        std::wstring resultado;
        resultado.reserve(capacidad);
        for (const auto& linea : m_documento.lineas) {
            resultado += linea;
            resultado += L"\r\n";
        }
        return resultado;
    }

    void DibujarIndice(HDC hdc, const TemaVisual& tema) const {
        SetTextColor(hdc, tema.textoSecundario);
        const size_t filasDisponibles = m_rcIndice.bottom > m_rcIndice.top
            ? static_cast<size_t>((m_rcIndice.bottom - m_rcIndice.top) /
                                   kAltoEntradaIndice)
            : 0;
        const size_t entradas = std::min(filasDisponibles, m_documento.indice.size());
        for (size_t i = 0; i < entradas; ++i) {
            const size_t numeroLinea = m_documento.indice[i];
            if (numeroLinea >= m_documento.lineas.size()) continue;
            RECT elemento{
                m_rcIndice.left + 8,
                m_rcIndice.top + static_cast<int>(i * kAltoEntradaIndice),
                m_rcIndice.right - 4,
                m_rcIndice.top + static_cast<int>((i + 1) * kAltoEntradaIndice)
            };
            DrawTextW(hdc, m_documento.lineas[numeroLinea].c_str(), -1, &elemento,
                      DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);
        }
    }

    static COLORREF ColorLinea(const std::wstring& linea, const TemaVisual& tema) {
        const bool encabezado = !linea.empty() && linea[0] == L'#';
        const bool lista = linea.size() > 1 &&
            (linea[0] == L'-' || linea[0] == L'*' || linea[0] == L'>');
        if (encabezado) return RGB(70, 150, 230);
        if (lista) return RGB(90, 180, 120);
        return tema.textoPrincipal;
    }

    static bool EsClave(const std::wstring& linea) {
        static const wchar_t* palabrasClave[] = {
            L"class ", L"struct ", L"void ", L"int ", L"return ",
            L"#include", L"function ", L"const ", L"public:", L"private:"
        };
        for (const wchar_t* palabra : palabrasClave) {
            if (linea.find(palabra) != std::wstring::npos) return true;
        }
        return false;
    }

    static HFONT CrearFuenteDocumento(int tamano, bool codigo) {
        return CreateFontW(
            -tamano, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
            codigo ? L"Consolas" : L"Segoe UI");
    }

    void DibujarLinea(HDC hdc, const std::wstring& linea,
                      const TemaVisual& tema, const RECT& rect,
                      int x, int y, int ancho, int alto,
                      bool mostrarNumeros, size_t numeroLinea,
                      bool ajustarLinea, COLORREF color) const {
        if (mostrarNumeros) {
            SetTextColor(hdc, tema.textoSecundario);
            wchar_t numero[32]{};
            swprintf_s(numero, L"%5zu", numeroLinea + 1);
            TextOutW(hdc, rect.left + 202, y, numero,
                     static_cast<int>(wcslen(numero)));
        }

        SetTextColor(hdc, color);
        RECT destino{x, y, x + ancho, y + alto};
        const UINT flags = ajustarLinea
            ? DT_WORDBREAK | DT_NOPREFIX
            : DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS | DT_NOPREFIX;
        DrawTextW(hdc, linea.c_str(), -1, &destino, flags);
    }

    void DibujarBoton(HDC hdc, const RECT& rect, const wchar_t* texto,
                      const TemaVisual& tema, bool habilitado = true) const {
        HBRUSH fondo = CreateSolidBrush(habilitado ? tema.botonFondo : tema.fondoCabecera);
        FillRect(hdc, &rect, fondo);
        DeleteObject(fondo);
        SetTextColor(hdc, habilitado ? tema.textoPrincipal : tema.textoSecundario);
        RECT textoRect = rect;
        DrawTextW(hdc, texto, -1, &textoRect,
                  DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
    }

    std::shared_ptr<ServicioTextoV2> m_texto;
    std::shared_ptr<ServicioNavegacionV2> m_nav;
    bool m_codigo{};
    std::vector<std::wstring> m_archivos;
    size_t m_indiceArchivo{};
    size_t m_lineaInicial{};
    DocumentoTextoV2 m_documento;
    RECT m_rcAnterior{}, m_rcSiguiente{}, m_rcComparar{}, m_rcCopiar{},
        m_rcRenombrar{}, m_rcIndice{};
};

} // namespace SideViewV2
/**** ended inlining v2/vistas/vista_texto_codigo.hpp ****/
/**** start inlining v2/vistas/vista_comparador_videos.hpp ****/

/**** skipping file: v2/ui/panel_interfaz.hpp ****/
/**** skipping file: v2/nucleo/configuracion_v2.hpp ****/
/**** skipping file: v2/nucleo/servicio_comparador_videos.hpp ****/
/**** start inlining v2/vistas/vista_comparador_archivos.hpp ****/

/**** skipping file: v2/ui/panel_interfaz.hpp ****/
/**** skipping file: v2/nucleo/servicio_comparador_archivos.hpp ****/
#include <windowsx.h>
#include <algorithm>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace SideViewV2 {

// Chrome común del comparador. La vista dibuja y enruta eventos; las reglas
// de comparación y navegación permanecen en los servicios inyectados.
class VistaComparadorArchivosV2 : public IPanelContenido {
public:
    explicit VistaComparadorArchivosV2(
        std::shared_ptr<ServicioComparadorArchivosV2> servicio)
        : m_servicio(std::move(servicio)) {}

    void Liberar() override {
        if (m_panel && GetCapture() == m_panel) ReleaseCapture();
        m_estado = {};
        m_modoDiffView = false;
        m_panel = nullptr;
        m_rcVolver = {};
        m_rcBotonDiffView = {};
        m_rcAreaEspecifica = {};
        m_rcResumen = {};
    }

    void AlCambiarSeleccion(const std::vector<std::wstring>& archivos,
                            const std::wstring&) override {
        m_modoDiffView = false;
        CargarComparacionComun(archivos);
    }

    void AlActivar(HWND panel) override { m_panel = panel; }

    bool ProcesarMensajeRaton(UINT msg, WPARAM wParam, LPARAM lParam,
                              HWND hwndPanel) override {
        if (msg == WM_LBUTTONDOWN) {
            POINT punto{GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
            if (PtInRect(&m_rcVolver, punto)) {
                PostMessageW(hwndPanel, WM_SIDEVIEW_CLOSE_COMPARADOR_ARCHIVOS, 0, 0);
                return true;
            }
            if (TieneDiffView() && PtInRect(&m_rcBotonDiffView, punto)) {
                m_modoDiffView = !m_modoDiffView;
                AlCambiarModoDiff(m_modoDiffView);
                InvalidateRect(hwndPanel, nullptr, FALSE);
                return true;
            }
        }
        if (msg == WM_KEYDOWN && wParam == VK_ESCAPE) {
            PostMessageW(hwndPanel, WM_SIDEVIEW_CLOSE_COMPARADOR_ARCHIVOS, 0, 0);
            return true;
        }
        return ProcesarMensajeContenido(msg, wParam, lParam, hwndPanel);
    }

    bool ProcesarMensajeTeclado(UINT msg, WPARAM wParam, LPARAM lParam,
                                HWND hwndPanel) override {
        return ProcesarMensajeRaton(msg, wParam, lParam, hwndPanel);
    }

    void Renderizar(HDC hdc, const RECT& rect, const TemaVisual& tema) override {
        if (!hdc || rect.right <= rect.left || rect.bottom <= rect.top) return;
        HBRUSH fondo = CreateSolidBrush(tema.fondo);
        FillRect(hdc, &rect, fondo);
        DeleteObject(fondo);
        SetBkMode(hdc, TRANSPARENT);

        HFONT titulo = CrearFuente(-17, FW_SEMIBOLD);
        HFONT texto = CrearFuente(-12, FW_NORMAL);
        HFONT anterior = static_cast<HFONT>(SelectObject(hdc, titulo));
        const int margen = 16;
        m_rcVolver = {rect.left + margen, rect.top + 12,
                      rect.left + 124, rect.top + 40};
        DibujarBoton(hdc, m_rcVolver, L"←  Volver", tema);

        SetTextColor(hdc, tema.textoPrincipal);
        RECT encabezado{rect.left + 142, rect.top + 12, rect.right - margen, rect.top + 40};
        DrawTextW(hdc, NombreVista(), -1, &encabezado,
                  DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);

        m_rcBotonDiffView = {};
        if (TieneDiffView()) {
            m_rcBotonDiffView = {rect.right - 178, rect.top + 12,
                                 rect.right - margen, rect.top + 40};
            DibujarBoton(hdc, m_rcBotonDiffView,
                         m_modoDiffView ? L"Vista lado a lado" : L"Diff View", tema);
        }

        const int altoResumen = 154;
        m_rcAreaEspecifica = {rect.left + margen, rect.top + 54,
                              rect.right - margen,
                              std::max(rect.top + 56, rect.bottom - altoResumen - 24)};
        m_rcResumen = {rect.left + margen, m_rcAreaEspecifica.bottom + 12,
                       rect.right - margen, rect.bottom - margen};

        SelectObject(hdc, texto);
        if (m_modoDiffView && TieneDiffView()) {
            RenderizarDiffView(hdc, m_rcAreaEspecifica, tema, texto);
        } else {
            RenderizarContenido(hdc, m_rcAreaEspecifica, tema, texto);
        }
        DibujarResumenTresColumnas(hdc, m_rcResumen, tema, texto);

        SelectObject(hdc, anterior);
        DeleteObject(titulo);
        DeleteObject(texto);
    }

protected:
    virtual const wchar_t* NombreVista() const { return L"Comparar archivos"; }
    virtual bool TieneDiffView() const { return false; }
    virtual void AlCambiarModoDiff(bool) {}
    virtual bool ProcesarMensajeContenido(UINT, WPARAM, LPARAM, HWND) { return false; }

    virtual void RenderizarContenido(HDC hdc, const RECT& rect,
                                     const TemaVisual& tema, HFONT fuente) {
        DibujarContenedor(hdc, rect, tema);
        SelectObject(hdc, fuente);
        SetTextColor(hdc, tema.textoSecundario);
        RECT mensaje = rect;
        DrawTextW(hdc, m_estado.mensaje.c_str(), -1, &mensaje,
                  DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);
    }

    virtual void RenderizarDiffView(HDC hdc, const RECT& rect,
                                    const TemaVisual& tema, HFONT fuente) {
        RenderizarContenido(hdc, rect, tema, fuente);
    }

    void CargarComparacionComun(const std::vector<std::wstring>& archivos) {
        m_estado = m_servicio ? m_servicio->CrearComparacion(archivos)
                               : EstadoComparadorArchivosV2{};
    }

    const RECT& AreaEspecifica() const { return m_rcAreaEspecifica; }
    bool EstaEnDiffView() const { return m_modoDiffView; }

    void DibujarContenedor(HDC hdc, const RECT& rect, const TemaVisual& tema) const {
        if (rect.right <= rect.left || rect.bottom <= rect.top) return;
        HBRUSH fondo = CreateSolidBrush(tema.fondoTarjeta);
        FillRect(hdc, &rect, fondo);
        DeleteObject(fondo);
        HPEN borde = CreatePen(PS_SOLID, 1, tema.borde);
        HPEN anterior = static_cast<HPEN>(SelectObject(hdc, borde));
        HBRUSH nulo = static_cast<HBRUSH>(GetStockObject(NULL_BRUSH));
        HBRUSH anteriorBrocha = static_cast<HBRUSH>(SelectObject(hdc, nulo));
        RoundRect(hdc, rect.left, rect.top, rect.right, rect.bottom, 8, 8);
        SelectObject(hdc, anteriorBrocha);
        SelectObject(hdc, anterior);
        DeleteObject(borde);
    }

    void DibujarBoton(HDC hdc, const RECT& rect, const wchar_t* texto,
                      const TemaVisual& tema) const {
        HBRUSH fondo = CreateSolidBrush(tema.botonFondo);
        FillRect(hdc, &rect, fondo);
        DeleteObject(fondo);
        HPEN borde = CreatePen(PS_SOLID, 1, tema.borde);
        HPEN anterior = static_cast<HPEN>(SelectObject(hdc, borde));
        HBRUSH nulo = static_cast<HBRUSH>(GetStockObject(NULL_BRUSH));
        HBRUSH anteriorBrocha = static_cast<HBRUSH>(SelectObject(hdc, nulo));
        RoundRect(hdc, rect.left, rect.top, rect.right, rect.bottom, 6, 6);
        SelectObject(hdc, anteriorBrocha);
        SelectObject(hdc, anterior);
        DeleteObject(borde);
        SetTextColor(hdc, tema.textoPrincipal);
        RECT textoRect = rect;
        DrawTextW(hdc, texto, -1, &textoRect,
                  DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);
    }

    static HFONT CrearFuente(int altura, int peso) {
        return CreateFontW(altura, 0, 0, 0, peso, FALSE, FALSE, FALSE,
                           DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                           CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
                           L"Segoe UI Variable Text");
    }

    static std::wstring TextoConteo(std::uintmax_t valor) {
        return valor == 0 ? L"No aplica" : std::to_wstring(valor);
    }

    void DibujarResumenTresColumnas(HDC hdc, const RECT& rect,
                                    const TemaVisual& tema, HFONT fuente) const {
        DibujarContenedor(hdc, rect, tema);
        SelectObject(hdc, fuente);
        SetTextColor(hdc, tema.textoPrincipal);
        RECT titulo{rect.left + 12, rect.top + 8, rect.right - 12, rect.top + 28};
        DrawTextW(hdc, L"RESUMEN DE LA COMPARACIÓN", -1, &titulo,
                  DT_LEFT | DT_SINGLELINE | DT_NOPREFIX);

        const int anchoColumna = (rect.right - rect.left - 24) / 3;
        const int yEncabezado = rect.top + 30;
        const int yDatos = rect.top + 50;
        const int altoFila = 18;
        const RECT columnas[3] = {
            {rect.left + 12, rect.left + 12 + anchoColumna, 0, 0},
            {rect.left + 12 + anchoColumna, rect.left + 12 + anchoColumna * 2, 0, 0},
            {rect.left + 12 + anchoColumna * 2, rect.right - 12, 0, 0}};
        const wchar_t* encabezados[] = {L"ARCHIVO 1", L"ARCHIVO 2", L"DIFERENCIAS"};
        for (int i = 0; i < 3; ++i) {
            RECT cab{columnas[i].left, yEncabezado, columnas[i].right, yEncabezado + 18};
            SetTextColor(hdc, tema.acento);
            DrawTextW(hdc, encabezados[i], -1, &cab,
                      DT_LEFT | DT_SINGLELINE | DT_NOPREFIX);
        }

        const std::wstring diferenciasSha =
            m_estado.izquierda.sha256 == L"No disponible" ||
            m_estado.derecha.sha256 == L"No disponible"
                ? L"No aplica"
                : (m_estado.izquierda.sha256 == m_estado.derecha.sha256 ? L"Iguales" : L"Diferentes");
        const std::wstring diferenciaLineas =
            m_estado.izquierda.lineas && m_estado.derecha.lineas
                ? std::to_wstring(static_cast<long long>(m_estado.derecha.lineas) -
                                  static_cast<long long>(m_estado.izquierda.lineas))
                : L"No aplica";
        const std::wstring diferenciaPaginas =
            m_estado.izquierda.paginas && m_estado.derecha.paginas
                ? std::to_wstring(static_cast<long long>(m_estado.derecha.paginas) -
                                  static_cast<long long>(m_estado.izquierda.paginas))
                : L"No aplica";
        const std::wstring filas[5][3] = {
            {m_estado.izquierda.nombre, m_estado.derecha.nombre, L"No aplica"},
            {m_estado.izquierda.sha256, m_estado.derecha.sha256, diferenciasSha},
            {m_servicio ? m_servicio->FormatearTamano(m_estado.izquierda.bytes) : L"0 B",
             m_servicio ? m_servicio->FormatearTamano(m_estado.derecha.bytes) : L"0 B",
             m_servicio ? m_servicio->FormatearDiferencia(m_estado.izquierda.bytes, m_estado.derecha.bytes) : L"+0 B"},
            {TextoConteo(m_estado.izquierda.lineas), TextoConteo(m_estado.derecha.lineas), diferenciaLineas},
            {TextoConteo(m_estado.izquierda.paginas), TextoConteo(m_estado.derecha.paginas), diferenciaPaginas}
        };
        const wchar_t* etiquetas[] = {L"Nombre: ", L"SHA-256: ", L"Peso: ", L"Líneas: ", L"Páginas: "};
        for (int fila = 0; fila < 5; ++fila) {
            for (int columna = 0; columna < 3; ++columna) {
                RECT dato{columnas[columna].left, yDatos + fila * altoFila,
                          columnas[columna].right, yDatos + fila * altoFila + altoFila};
                SetTextColor(hdc, columna == 2 ? tema.acento : tema.textoSecundario);
                std::wstring valor = (columna == 0 ? etiquetas[fila] : L"") + filas[fila][columna];
                DrawTextW(hdc, valor.c_str(), -1, &dato,
                          DT_LEFT | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);
            }
        }
    }

    void DibujarControlDesplazamiento(HDC hdc, const RECT& rect,
                                       std::size_t maximo, std::size_t actual,
                                       const TemaVisual& tema) const {
        if (rect.right <= rect.left || rect.bottom <= rect.top) return;
        const int y = rect.top + (rect.bottom - rect.top) / 2;
        RECT linea{rect.left + 8, y - 2, rect.right - 8, y + 2};
        HBRUSH brocha = CreateSolidBrush(tema.borde);
        FillRect(hdc, &linea, brocha);
        DeleteObject(brocha);
        const double proporcion = maximo == 0 ? 0.0 :
            static_cast<double>(std::min(actual, maximo)) / static_cast<double>(maximo);
        const int x = linea.left + static_cast<int>((linea.right - linea.left) * proporcion);
        brocha = CreateSolidBrush(tema.acento);
        Ellipse(hdc, x - 6, y - 6, x + 6, y + 6);
        DeleteObject(brocha);
    }

    std::size_t DesplazamientoDesdePunto(const RECT& rect, POINT punto,
                                         std::size_t maximo) const {
        const int inicio = rect.left + 8;
        const int fin = rect.right - 8;
        if (fin <= inicio || maximo == 0) return 0;
        const double proporcion = std::clamp(
            static_cast<double>(punto.x - inicio) / static_cast<double>(fin - inicio), 0.0, 1.0);
        return static_cast<std::size_t>(proporcion * static_cast<double>(maximo));
    }

    HWND m_panel{nullptr};
    RECT m_rcVolver{};
    RECT m_rcBotonDiffView{};
    RECT m_rcAreaEspecifica{};
    RECT m_rcResumen{};
    bool m_modoDiffView{false};
    EstadoComparadorArchivosV2 m_estado;
    std::shared_ptr<ServicioComparadorArchivosV2> m_servicio;
};

} // namespace SideViewV2
/**** ended inlining v2/vistas/vista_comparador_archivos.hpp ****/
#include <windows.h>
#include <windowsx.h>
#include <algorithm>
#include <memory>
#include <string>
#include <vector>

namespace SideViewV2 {

class VistaComparadorVideos final : public VistaComparadorArchivosV2 {
public:
    explicit VistaComparadorVideos(std::shared_ptr<ServicioComparadorVideosV2> servicio)
        : VistaComparadorArchivosV2(servicio), m_servicio(std::move(servicio)) {}

    ~VistaComparadorVideos() override { Liberar(); }

    void AlCambiarSeleccion(const std::vector<std::wstring>& archivos,
                            const std::wstring&) override {
        CargarComparacionComun(archivos);
        LiberarReproductores();
        m_estado = m_servicio ? m_servicio->CrearFlujo(archivos)
                               : EstadoComparadorVideosV2{};
        m_arrastrandoTiempo = false;
        m_hoverVideo = 0;
        if (m_panel) MontarReproductores();
    }

    void AlActivar(HWND panel) override {
        m_panel = panel;
        MontarReproductores();
    }

    void ActualizarConfiguracion() override {
        if (!ComparadorVideosPantallaCompletaDisponibleV2()) {
            m_estado.modoPantallaCompleta = false;
        }
    }

    void Liberar() override {
        if (m_panel && GetCapture() == m_panel) ReleaseCapture();
        LiberarReproductores();
        m_estado = {};
        m_arrastrandoTiempo = false;
        m_hoverVideo = 0;
        m_panel = nullptr;
        VistaComparadorArchivosV2::Liberar();
    }

    bool ProcesarMensajeVentana(UINT msg, WPARAM wParam, LPARAM,
                                HWND hwndPanel) override {
        if (msg == WM_SIDEVIEW_COMPARADOR_TICK) {
            ActualizarEstadoDesdeReproductores();
            InvalidateRect(hwndPanel, nullptr, FALSE);
            return true;
        }
        if (msg == WM_SIDEVIEW_COMPARADOR_PIP_CLICK &&
            m_estado.modoPantallaCompleta &&
            static_cast<int>(wParam) != m_estado.indicePantallaCompleta) {
            // El propio reproductor informa que su superficie es el PiP. El
            // swap sólo cambia geometría/rol; nunca pausa ni busca el video.
            if (m_servicio) m_servicio->IntercambiarPantallaCompleta(m_estado);
            InvalidateRect(hwndPanel, nullptr, FALSE);
            return true;
        }
        return false;
    }

    bool ProcesarMensajeRaton(UINT msg, WPARAM, LPARAM lParam,
                              HWND hwndPanel) override {
        POINT punto{GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};

        if (msg == WM_MOUSEMOVE) {
            int hoverNuevo = 0;
            if (PtInRect(&m_rcBotonFullIzquierda, punto)) hoverNuevo = 1;
            else if (PtInRect(&m_rcBotonFullDerecha, punto)) hoverNuevo = 2;
            else if (PtInRect(&m_rcVideoIzquierda, punto)) hoverNuevo = 3;
            else if (PtInRect(&m_rcVideoDerecha, punto)) hoverNuevo = 4;
            if (hoverNuevo != m_hoverVideo) {
                m_hoverVideo = hoverNuevo;
                InvalidateRect(hwndPanel, nullptr, FALSE);
            }
            if (m_arrastrandoTiempo && m_servicio) {
                m_servicio->BuscarAmbos(m_estado, PosicionDesdePunto(punto),
                                        m_reproductorIzquierda.get(),
                                        m_reproductorDerecha.get());
                InvalidateRect(hwndPanel, nullptr, FALSE);
            }
            return m_arrastrandoTiempo || hoverNuevo != 0;
        }

        if (msg == WM_LBUTTONUP) {
            m_arrastrandoTiempo = false;
            ReleaseCapture();
            return true;
        }

        if (msg != WM_LBUTTONDOWN) return false;
        SetFocus(hwndPanel);

        if (PtInRect(&m_rcVolver, punto)) {
            PostMessageW(hwndPanel, WM_SIDEVIEW_CLOSE_COMPARADOR_ARCHIVOS, 0, 0);
            return true;
        }
        if (ComparadorVideosPantallaCompletaDisponibleV2()) {
            if (PtInRect(&m_rcBotonFullIzquierda, punto)) {
                if (m_servicio) m_servicio->AlternarPantallaCompleta(m_estado, 0);
                InvalidateRect(hwndPanel, nullptr, FALSE);
                return true;
            }
            if (PtInRect(&m_rcBotonFullDerecha, punto)) {
                if (m_servicio) m_servicio->AlternarPantallaCompleta(m_estado, 1);
                InvalidateRect(hwndPanel, nullptr, FALSE);
                return true;
            }
        }
        if (PtInRect(&m_rcLineaTiempo, punto)) {
            m_arrastrandoTiempo = true;
            SetCapture(hwndPanel);
            if (m_servicio) m_servicio->BuscarAmbos(
                m_estado, PosicionDesdePunto(punto), m_reproductorIzquierda.get(),
                m_reproductorDerecha.get());
            InvalidateRect(hwndPanel, nullptr, FALSE);
            return true;
        }
        if (PtInRect(&m_rcReproducir, punto)) {
            if (m_servicio) m_servicio->AlternarEstadoReproduccion(
                m_estado, m_reproductorIzquierda.get(), m_reproductorDerecha.get());
            InvalidateRect(hwndPanel, nullptr, FALSE);
            return true;
        }
        if (PtInRect(&m_rcRetroceder, punto)) {
            BuscarRelativo(-10.0);
            InvalidateRect(hwndPanel, nullptr, FALSE);
            return true;
        }
        if (PtInRect(&m_rcAvanzar, punto)) {
            BuscarRelativo(10.0);
            InvalidateRect(hwndPanel, nullptr, FALSE);
            return true;
        }
        if (PtInRect(&m_rcConfirmarSincronizacion, punto)) {
            if (m_servicio) {
                m_servicio->ActualizarDuraciones(
                    m_estado,
                    m_reproductorIzquierda ? m_reproductorIzquierda->Duracion() : 0.0,
                    m_reproductorDerecha ? m_reproductorDerecha->Duracion() : 0.0);
                m_servicio->ConfirmarSincronizacion(m_estado);
            }
            InvalidateRect(hwndPanel, nullptr, FALSE);
            return true;
        }
        return false;
    }

    bool ProcesarMensajeTeclado(UINT msg, WPARAM wParam, LPARAM,
                                HWND hwndPanel) override {
        if (msg != WM_KEYDOWN || !m_servicio) return false;
        switch (wParam) {
            case VK_SPACE:
                m_servicio->AlternarEstadoReproduccion(
                    m_estado, m_reproductorIzquierda.get(), m_reproductorDerecha.get());
                break;
            case VK_LEFT:
                BuscarRelativo(-10.0);
                break;
            case VK_RIGHT:
                BuscarRelativo(10.0);
                break;
            default:
                return false;
        }
        InvalidateRect(hwndPanel, nullptr, FALSE);
        return true;
    }

    void Renderizar(HDC hdc, const RECT& rect, const TemaVisual& tema) override {
        HBRUSH fondo = CreateSolidBrush(tema.fondo);
        FillRect(hdc, &rect, fondo);
        DeleteObject(fondo);
        SetBkMode(hdc, TRANSPARENT);

        HFONT titulo = CrearFuente(-18, FW_SEMIBOLD);
        HFONT texto = CrearFuente(-13, FW_NORMAL);
        HFONT negrita = CrearFuente(-13, FW_BOLD);
        HFONT anterior = static_cast<HFONT>(SelectObject(hdc, titulo));

        const int margen = 16;
        const int espacio = 12;
        const int anchoCliente = static_cast<int>(rect.right - rect.left);
        const int altoCliente = static_cast<int>(rect.bottom - rect.top);
        const int ancho = std::max(1, anchoCliente);
        const int anchoVideo = std::max(1, (ancho - margen * 2 - espacio) / 2);
        const int altoVideo = std::max(220, std::min(520, altoCliente / 2));
        const int yVideo = rect.top + 52;

        m_rcVolver = {rect.left + margen, rect.top + 12,
                      rect.left + 124, rect.top + 40};
        DibujarBoton(hdc, m_rcVolver, L"←  Volver", tema, true);

        if (m_estado.modoPantallaCompleta &&
            ComparadorVideosPantallaCompletaDisponibleV2()) {
            ConstruirLayoutPantallaCompleta(hdc, rect, margen, yVideo, altoVideo, tema);
        } else {
            m_estado.modoPantallaCompleta = false;
            m_rcVideoIzquierda = {rect.left + margen, yVideo,
                                  rect.left + margen + anchoVideo, yVideo + altoVideo};
            m_rcVideoDerecha = {m_rcVideoIzquierda.right + espacio, yVideo,
                                rect.right - margen, yVideo + altoVideo};
            ActualizarRectReproductores();
            DibujarMarcoVideo(hdc, m_rcVideoIzquierda, m_estado.izquierda,
                              L"Video 1", false, 0, tema);
            DibujarMarcoVideo(hdc, m_rcVideoDerecha, m_estado.derecha,
                              L"Video 2", false, 1, tema);
        }

        const int yControles = std::max(m_rcVideoIzquierda.bottom,
                                        m_rcVideoDerecha.bottom) + 18;
        m_rcLineaTiempo = {rect.left + margen, yControles,
                           rect.right - margen, yControles + 34};
        DibujarContenedor(hdc, m_rcLineaTiempo, tema);
        DibujarLineaTiempo(hdc, tema);

        const int yBotones = m_rcLineaTiempo.bottom + 8;
        m_rcRetroceder = {rect.left + margen, yBotones,
                          rect.left + margen + 92, yBotones + 30};
        m_rcReproducir = {m_rcRetroceder.right + 8, yBotones,
                          m_rcRetroceder.right + 116, yBotones + 30};
        m_rcAvanzar = {m_rcReproducir.right + 8, yBotones,
                       m_rcReproducir.right + 100, yBotones + 30};
        m_rcConfirmarSincronizacion = {m_rcAvanzar.right + 8, yBotones,
                                       m_rcAvanzar.right + 190, yBotones + 30};
        DibujarBoton(hdc, m_rcRetroceder, L"-10 s", tema, true);
        DibujarBoton(hdc, m_rcReproducir,
                     m_estado.reproduciendo ? L"Pausar" : L"Reproducir", tema, true);
        DibujarBoton(hdc, m_rcAvanzar, L"+10 s", tema, true);
        DibujarBoton(hdc, m_rcConfirmarSincronizacion,
                     m_estado.sincronizacionConfirmada ? L"Sincronización confirmada" : L"Confirmar sincronización",
                     tema, m_estado.sincronizacionConfirmada);

        SelectObject(hdc, texto);
        SetTextColor(hdc, tema.textoSecundario);
        RECT tiempo{m_rcConfirmarSincronizacion.right + 12, yBotones,
                    rect.right - margen, yBotones + 30};
        std::wstring etiquetaTiempo = m_servicio
            ? m_servicio->FormatearTiempo(m_estado.posicionSegundos) + L" / " +
              m_servicio->FormatearTiempo(m_estado.duracionSegundos)
            : L"00:00 / 10:00";
        DrawTextW(hdc, etiquetaTiempo.c_str(), -1, &tiempo,
                  DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

        const int yInfo = yBotones + 46;
        RECT info{rect.left + margen, yInfo, rect.right - margen,
                  std::max(yInfo + 148, static_cast<int>(rect.bottom - margen))};
        DibujarContenedor(hdc, info, tema);
        SelectObject(hdc, negrita);
        SetTextColor(hdc, tema.textoPrincipal);
        RECT tituloInfo{info.left + 12, info.top + 10,
                        info.right - 12, info.top + 32};
        DrawTextW(hdc, L"INFORMACIÓN DE LA COMPARACIÓN", -1, &tituloInfo,
                  DT_LEFT | DT_SINGLELINE | DT_NOPREFIX);
        SelectObject(hdc, texto);
        DibujarDato(hdc, info, m_estado.izquierda, L"Video 1", 0, tema);
        DibujarDato(hdc, info, m_estado.derecha, L"Video 2", 1, tema);
        SetTextColor(hdc, tema.acento);
        RECT diferencia{info.left + 12, info.bottom - 30,
                        info.right - 12, info.bottom - 10};
        std::wstring diferenciaTexto = L"Diferencia de peso: " +
            (m_servicio ? m_servicio->FormatearDiferencia(
                m_estado.izquierda.bytes, m_estado.derecha.bytes) : L"+0 B");
        DrawTextW(hdc, diferenciaTexto.c_str(), -1, &diferencia,
                  DT_LEFT | DT_SINGLELINE | DT_NOPREFIX);

        SetTextColor(hdc, tema.textoSecundario);
        RECT hashIzquierda{info.left + 12, info.top + 124,
                           info.left + (info.right - info.left) / 2 - 12,
                           info.top + 144};
        RECT hashDerecha{info.left + (info.right - info.left) / 2 + 12,
                         info.top + 124, info.right - 12, info.top + 144};
        std::wstring shaIzquierda = L"SHA-256: " + m_estado.izquierda.sha256;
        std::wstring shaDerecha = L"SHA-256: " + m_estado.derecha.sha256;
        DrawTextW(hdc, shaIzquierda.c_str(), -1, &hashIzquierda,
                  DT_LEFT | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);
        DrawTextW(hdc, shaDerecha.c_str(), -1, &hashDerecha,
                  DT_LEFT | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);

        SetTextColor(hdc, tema.textoSecundario);
        RECT mensaje{info.left + 12, info.top + 34,
                     info.right - 12, info.top + 54};
        DrawTextW(hdc, m_estado.mensaje.c_str(), -1, &mensaje,
                  DT_LEFT | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);

        if (m_hoverVideo >= 3 && m_hoverVideo <= 4) {
            const DatosVideoComparacionV2& video = m_hoverVideo == 3
                ? m_estado.izquierda : m_estado.derecha;
            RECT base = m_hoverVideo == 3 ? m_rcVideoIzquierda : m_rcVideoDerecha;
            RECT tooltip{base.left + 10, base.top + 34,
                         base.right - 10, base.top + 62};
            HBRUSH fondoTooltip = CreateSolidBrush(tema.fondoCabecera);
            FillRect(hdc, &tooltip, fondoTooltip);
            DeleteObject(fondoTooltip);
            SetTextColor(hdc, tema.textoPrincipal);
            DrawTextW(hdc, video.nombre.c_str(), -1, &tooltip,
                      DT_LEFT | DT_VCENTER | DT_SINGLELINE |
                      DT_END_ELLIPSIS | DT_NOPREFIX);
        }

        SelectObject(hdc, anterior);
        DeleteObject(titulo);
        DeleteObject(texto);
        DeleteObject(negrita);
    }

private:
    static HFONT CrearFuente(int altura, int peso) {
        return CreateFontW(altura, 0, 0, 0, peso, FALSE, FALSE, FALSE,
                           DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                           CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
                           L"Segoe UI Variable Text");
    }

    void MontarReproductores() {
        if (!m_panel || !m_servicio) return;
        LiberarReproductores();
        m_reproductorIzquierda = m_servicio->CrearReproductor();
        m_reproductorDerecha = m_servicio->CrearReproductor();
        if (m_reproductorIzquierda) {
            m_reproductorIzquierda->Montar(m_panel, 0, m_estado.izquierda.ruta);
        }
        if (m_reproductorDerecha) {
            m_reproductorDerecha->Montar(m_panel, 1, m_estado.derecha.ruta);
        }
    }

    void LiberarReproductores() {
        if (m_reproductorIzquierda) m_reproductorIzquierda->Liberar();
        if (m_reproductorDerecha) m_reproductorDerecha->Liberar();
        m_reproductorIzquierda.reset();
        m_reproductorDerecha.reset();
    }

    void ActualizarEstadoDesdeReproductores() {
        const double duracionIzquierda = m_reproductorIzquierda
            ? m_reproductorIzquierda->Duracion() : 0.0;
        const double duracionDerecha = m_reproductorDerecha
            ? m_reproductorDerecha->Duracion() : 0.0;
        if (m_servicio) {
            m_servicio->ActualizarDuraciones(m_estado, duracionIzquierda, duracionDerecha);
        }

        IReproductorVideoV2* reloj = m_reproductorIzquierda &&
            m_reproductorIzquierda->EstaListo() ? m_reproductorIzquierda.get() :
            m_reproductorDerecha.get();
        if (reloj) {
            m_estado.posicionSegundos = std::clamp(
                reloj->Posicion(), 0.0, m_estado.duracionSegundos);
        }
    }

    void BuscarRelativo(double desplazamiento) {
        if (!m_servicio) return;
        m_servicio->BuscarAmbos(m_estado,
                                m_estado.posicionSegundos + desplazamiento,
                                m_reproductorIzquierda.get(),
                                m_reproductorDerecha.get());
    }

    double PosicionDesdePunto(POINT punto) const {
        const int inicio = m_rcLineaTiempo.left + 14;
        const int fin = m_rcLineaTiempo.right - 14;
        if (fin <= inicio) return 0.0;
        const double porcentaje = std::clamp(
            static_cast<double>(punto.x - inicio) / (fin - inicio), 0.0, 1.0);
        return porcentaje * m_estado.duracionSegundos;
    }

    RECT BotonPantallaCompleta(const RECT& video) const {
        return {video.right - 34, video.top + 4, video.right - 6, video.top + 28};
    }

    void ConstruirLayoutPantallaCompleta(HDC hdc, const RECT& rect, int margen,
                                         int yVideo, int altoVideo,
                                         const TemaVisual& tema) {
        RECT completa{rect.left + margen, yVideo, rect.right - margen, yVideo + altoVideo};
        const int anchoCompleta = static_cast<int>(completa.right - completa.left);
        const int altoCompleta = static_cast<int>(completa.bottom - completa.top);
        const int anchoPip = std::max(190, anchoCompleta / 4);
        const int altoPip = std::max(130, altoCompleta / 4);
        RECT pip{completa.right - anchoPip - 18, completa.bottom - altoPip - 18,
                 completa.right - 18, completa.bottom - 18};
        if (m_estado.indicePantallaCompleta == 0) {
            m_rcVideoIzquierda = completa;
            m_rcVideoDerecha = pip;
        } else {
            m_rcVideoDerecha = completa;
            m_rcVideoIzquierda = pip;
        }
        ActualizarRectReproductores();
        if (m_estado.indicePantallaCompleta == 0) {
            DibujarMarcoVideo(hdc, m_rcVideoIzquierda, m_estado.izquierda,
                              L"Video 1", false, 0, tema);
            DibujarMarcoVideo(hdc, m_rcVideoDerecha, m_estado.derecha,
                              L"Video 2", true, 1, tema);
        } else {
            DibujarMarcoVideo(hdc, m_rcVideoIzquierda, m_estado.izquierda,
                              L"Video 1", true, 0, tema);
            DibujarMarcoVideo(hdc, m_rcVideoDerecha, m_estado.derecha,
                              L"Video 2", false, 1, tema);
        }
    }

    void ActualizarRectReproductores() {
        if (!m_reproductorIzquierda || !m_reproductorDerecha) return;
        if (m_estado.modoPantallaCompleta) {
            const bool izquierdaFull = m_estado.indicePantallaCompleta == 0;
            if (izquierdaFull) {
                m_reproductorIzquierda->ActualizarRect(m_rcVideoIzquierda, false);
                m_reproductorDerecha->ActualizarRect(m_rcVideoDerecha, true);
            } else {
                m_reproductorDerecha->ActualizarRect(m_rcVideoDerecha, false);
                m_reproductorIzquierda->ActualizarRect(m_rcVideoIzquierda, true);
            }
        } else {
            m_reproductorIzquierda->ActualizarRect(m_rcVideoIzquierda, false);
            m_reproductorDerecha->ActualizarRect(m_rcVideoDerecha, false);
        }
    }

    void DibujarLineaTiempo(HDC hdc, const TemaVisual& tema) {
        RECT linea{m_rcLineaTiempo.left + 14, m_rcLineaTiempo.top + 15,
                   m_rcLineaTiempo.right - 14, m_rcLineaTiempo.top + 19};
        HBRUSH fondo = CreateSolidBrush(tema.borde);
        FillRect(hdc, &linea, fondo);
        DeleteObject(fondo);
        const double progreso = m_estado.duracionSegundos > 0.0
            ? std::clamp(m_estado.posicionSegundos / m_estado.duracionSegundos, 0.0, 1.0)
            : 0.0;
        const int x = linea.left + static_cast<int>((linea.right - linea.left) * progreso);
        HBRUSH relleno = CreateSolidBrush(tema.acento);
        RECT tramo{linea.left, linea.top, x, linea.bottom};
        FillRect(hdc, &tramo, relleno);
        DeleteObject(relleno);
        Ellipse(hdc, x - 6, linea.top - 5, x + 6, linea.bottom + 5);
    }

    void DibujarMarcoVideo(HDC hdc, const RECT& rect,
                           const DatosVideoComparacionV2& video,
                           const wchar_t* etiqueta, bool esPip, int indice,
                           const TemaVisual& tema) {
        DibujarContenedor(hdc, rect, tema);
        RECT cabecera{rect.left + 1, rect.top + 1, rect.right - 1, rect.top + 31};
        HBRUSH fondo = CreateSolidBrush(esPip ? tema.fondoCabecera : tema.fondoTarjeta);
        FillRect(hdc, &cabecera, fondo);
        DeleteObject(fondo);
        SetTextColor(hdc, tema.textoPrincipal);
        RECT titulo{rect.left + 10, rect.top + 4, rect.right - 42, rect.top + 28};
        std::wstring texto = esPip ? L"PIP · " : L"FULL SCREEN · ";
        texto += etiqueta;
        texto += L" — ";
        texto += video.nombre;
        DrawTextW(hdc, texto.c_str(), -1, &titulo,
                  DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);

        if (ComparadorVideosPantallaCompletaDisponibleV2()) {
            RECT boton = BotonPantallaCompleta(rect);
            if (indice == 0) m_rcBotonFullIzquierda = boton;
            else m_rcBotonFullDerecha = boton;
            DibujarBoton(hdc, boton, L"⛶", tema,
                         m_hoverVideo == (indice == 0 ? 1 : 2));
        }
    }

    void DibujarDato(HDC hdc, const RECT& info,
                     const DatosVideoComparacionV2& video,
                     const wchar_t* etiqueta, int columna,
                     const TemaVisual& tema) {
        const int mitad = (info.right - info.left) / 2;
        const int izquierda = info.left + 12 + columna * mitad;
        const int derecha = info.left + (columna + 1) * mitad - 12;
        RECT fila{izquierda, info.top + 60, derecha, info.top + 82};
        SetTextColor(hdc, tema.textoPrincipal);
        std::wstring texto = std::wstring(etiqueta) + L": " + video.nombre;
        DrawTextW(hdc, texto.c_str(), -1, &fila,
                  DT_LEFT | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);
        RECT meta{izquierda, info.top + 82, derecha, info.top + 102};
        SetTextColor(hdc, tema.textoSecundario);
        std::wstring datos = L"FPS: " + video.fps + L"  ·  Tamaño: " +
            (m_servicio ? m_servicio->FormatearTamano(video.bytes) : L"0 B");
        DrawTextW(hdc, datos.c_str(), -1, &meta,
                  DT_LEFT | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);
    }

    void DibujarBoton(HDC hdc, const RECT& rc, const wchar_t* texto,
                      const TemaVisual& tema, bool resaltado) {
        HBRUSH fondo = CreateSolidBrush(resaltado ? tema.acento : tema.botonFondo);
        FillRect(hdc, &rc, fondo);
        DeleteObject(fondo);
        HPEN borde = CreatePen(PS_SOLID, 1, tema.borde);
        HPEN anterior = static_cast<HPEN>(SelectObject(hdc, borde));
        HBRUSH nulo = static_cast<HBRUSH>(GetStockObject(NULL_BRUSH));
        HBRUSH anteriorBrocha = static_cast<HBRUSH>(SelectObject(hdc, nulo));
        RoundRect(hdc, rc.left, rc.top, rc.right, rc.bottom, 6, 6);
        SelectObject(hdc, anteriorBrocha);
        SelectObject(hdc, anterior);
        DeleteObject(borde);
        SetTextColor(hdc, resaltado ? RGB(255, 255, 255) : tema.textoPrincipal);
        RECT textoRect = rc;
        DrawTextW(hdc, texto, -1, &textoRect,
                  DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
    }

    void DibujarContenedor(HDC hdc, const RECT& rc, const TemaVisual& tema) {
        if (!hdc || rc.right <= rc.left || rc.bottom <= rc.top) return;
        HBRUSH fondo = CreateSolidBrush(tema.fondoTarjeta);
        FillRect(hdc, &rc, fondo);
        DeleteObject(fondo);
        HPEN borde = CreatePen(PS_SOLID, 1, tema.borde);
        HPEN anterior = static_cast<HPEN>(SelectObject(hdc, borde));
        HBRUSH nulo = static_cast<HBRUSH>(GetStockObject(NULL_BRUSH));
        HBRUSH anteriorBrocha = static_cast<HBRUSH>(SelectObject(hdc, nulo));
        RoundRect(hdc, rc.left, rc.top, rc.right, rc.bottom, 8, 8);
        SelectObject(hdc, anteriorBrocha);
        SelectObject(hdc, anterior);
        DeleteObject(borde);
    }

    std::shared_ptr<ServicioComparadorVideosV2> m_servicio;
    std::unique_ptr<IReproductorVideoV2> m_reproductorIzquierda;
    std::unique_ptr<IReproductorVideoV2> m_reproductorDerecha;
    EstadoComparadorVideosV2 m_estado;
    HWND m_panel{nullptr};
    bool m_arrastrandoTiempo{false};
    int m_hoverVideo{0};

    RECT m_rcVolver{};
    RECT m_rcVideoIzquierda{};
    RECT m_rcVideoDerecha{};
    RECT m_rcBotonFullIzquierda{};
    RECT m_rcBotonFullDerecha{};
    RECT m_rcLineaTiempo{};
    RECT m_rcRetroceder{};
    RECT m_rcReproducir{};
    RECT m_rcAvanzar{};
    RECT m_rcConfirmarSincronizacion{};
};

} // namespace SideViewV2
/**** ended inlining v2/vistas/vista_comparador_videos.hpp ****/
/**** skipping file: v2/vistas/vista_comparador_archivos.hpp ****/
/**** start inlining v2/vistas/vista_comparador_texto.hpp ****/

/**** skipping file: v2/vistas/vista_comparador_archivos.hpp ****/
/**** skipping file: v2/nucleo/configuracion_v2.hpp ****/
/**** skipping file: v2/nucleo/servicio_diff.hpp ****/
#include <windowsx.h>
#include <algorithm>
#include <memory>

namespace SideViewV2 {

class VistaComparadorTextoV2 final : public VistaComparadorArchivosV2 {
public:
    explicit VistaComparadorTextoV2(std::shared_ptr<ServicioComparadorTextoV2> servicio)
        : VistaComparadorArchivosV2(servicio), m_diff(std::move(servicio)) {}

    void Liberar() override {
        m_estadoDiff = {};
        m_scrollIzquierda = 0;
        m_scrollDerecha = 0;
        m_scrollGeneral = 0;
        m_sliderActivo = 0;
        VistaComparadorArchivosV2::Liberar();
    }

    void AlCambiarSeleccion(const std::vector<std::wstring>& archivos,
                            const std::wstring& carpeta) override {
        VistaComparadorArchivosV2::AlCambiarSeleccion(archivos, carpeta);
        m_estadoDiff = m_diff ? m_diff->CrearDiff(archivos) : EstadoDiffV2{};
        m_scrollIzquierda = 0;
        m_scrollDerecha = 0;
        m_scrollGeneral = 0;
    }

protected:
    const wchar_t* NombreVista() const override { return L"Comparar texto"; }
    bool TieneDiffView() const override { return true; }

    void AlCambiarModoDiff(bool) override {
        m_scrollIzquierda = 0;
        m_scrollDerecha = 0;
        m_scrollGeneral = 0;
        m_sliderActivo = 0;
    }

    bool ProcesarMensajeContenido(UINT msg, WPARAM wParam, LPARAM lParam,
                                  HWND hwndPanel) override {
        const int maximo = MaximoDesplazamiento();
        if (msg == WM_MOUSEWHEEL) {
            const int delta = GET_WHEEL_DELTA_WPARAM(wParam) > 0 ? -3 : 3;
            if (EstaEnDiffView()) {
                m_scrollGeneral = std::clamp(m_scrollGeneral + delta, 0, maximo);
            } else {
                m_scrollIzquierda = std::clamp(m_scrollIzquierda + delta, 0, maximoIzquierda());
                m_scrollDerecha = std::clamp(m_scrollDerecha + delta, 0, maximoDerecha());
            }
            InvalidateRect(hwndPanel, nullptr, FALSE);
            return true;
        }
        if (msg == WM_KEYDOWN) {
            if (wParam == VK_UP || wParam == VK_DOWN) {
                const int delta = wParam == VK_UP ? -1 : 1;
                if (EstaEnDiffView()) m_scrollGeneral = std::clamp(m_scrollGeneral + delta, 0, maximo);
                else {
                    m_scrollIzquierda = std::clamp(m_scrollIzquierda + delta, 0, maximoIzquierda());
                    m_scrollDerecha = std::clamp(m_scrollDerecha + delta, 0, maximoDerecha());
                }
                InvalidateRect(hwndPanel, nullptr, FALSE);
                return true;
            }
        }
        if (msg == WM_LBUTTONDOWN) {
            POINT punto{GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
            if (EstaEnDiffView() && PtInRect(&m_rcSliderGeneral, punto)) m_sliderActivo = 3;
            else if (!EstaEnDiffView() && PtInRect(&m_rcSliderIzquierda, punto)) m_sliderActivo = 1;
            else if (!EstaEnDiffView() && PtInRect(&m_rcSliderDerecha, punto)) m_sliderActivo = 2;
            else if (!EstaEnDiffView() && PtInRect(&m_rcSliderGeneral, punto)) m_sliderActivo = 3;
            else return false;
            ActualizarSlider(punto, hwndPanel);
            SetCapture(hwndPanel);
            return true;
        }
        if (msg == WM_MOUSEMOVE && m_sliderActivo != 0) {
            ActualizarSlider(POINT{GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)}, hwndPanel);
            return true;
        }
        if (msg == WM_LBUTTONUP && m_sliderActivo != 0) {
            m_sliderActivo = 0;
            ReleaseCapture();
            return true;
        }
        return false;
    }

    void RenderizarContenido(HDC hdc, const RECT& rect, const TemaVisual& tema,
                             HFONT fuente) override {
        DibujarContenedor(hdc, rect, tema);
        const int mitad = (rect.right - rect.left) / 2;
        const int altoFila = 22;
        const int inicio = rect.top + 34;
        const int pie = rect.bottom - 58;
        const int visibles = std::max(0, (pie - inicio) / altoFila);
        RECT izquierda{rect.left + 8, rect.top + 8, rect.left + mitad - 4, rect.top + 28};
        RECT derecha{rect.left + mitad + 4, rect.top + 8, rect.right - 8, rect.top + 28};
        SetTextColor(hdc, tema.textoPrincipal);
        DrawTextW(hdc, L"Archivo 1", -1, &izquierda, DT_LEFT | DT_SINGLELINE | DT_NOPREFIX);
        DrawTextW(hdc, L"Archivo 2", -1, &derecha, DT_LEFT | DT_SINGLELINE | DT_NOPREFIX);

        SelectObject(hdc, fuente);
        for (int visible = 0; visible < visibles; ++visible) {
            const int y = inicio + visible * altoFila;
            const std::size_t indiceIzquierda = static_cast<std::size_t>(m_scrollIzquierda + visible);
            const std::size_t indiceDerecha = static_cast<std::size_t>(m_scrollDerecha + visible);
            DibujarLinea(hdc, rect, mitad, y, altoFila,
                         indiceIzquierda < m_estadoDiff.lineas.size()
                             ? &m_estadoDiff.lineas[indiceIzquierda] : nullptr,
                         true, tema);
            DibujarLinea(hdc, rect, mitad, y, altoFila,
                         indiceDerecha < m_estadoDiff.lineas.size()
                             ? &m_estadoDiff.lineas[indiceDerecha] : nullptr,
                         false, tema);
        }

        m_rcSliderIzquierda = {rect.left + 10, rect.bottom - 38,
                               rect.left + mitad - 8, rect.bottom - 24};
        m_rcSliderDerecha = {rect.left + mitad + 8, rect.bottom - 38,
                             rect.right - 10, rect.bottom - 24};
        m_rcSliderGeneral = {rect.left + 10, rect.bottom - 20,
                             rect.right - 10, rect.bottom - 6};
        DibujarControlDesplazamiento(hdc, m_rcSliderIzquierda,
                                     static_cast<std::size_t>(maximoIzquierda()),
                                     static_cast<std::size_t>(m_scrollIzquierda), tema);
        DibujarControlDesplazamiento(hdc, m_rcSliderDerecha,
                                     static_cast<std::size_t>(maximoDerecha()),
                                     static_cast<std::size_t>(m_scrollDerecha), tema);
        SetTextColor(hdc, tema.textoSecundario);
        RECT estado{rect.left + 10, rect.bottom - 54, rect.right - 10, rect.bottom - 40};
        DrawTextW(hdc, L"Arriba: desplazamiento independiente · Abajo: sincronizado",
                  -1, &estado, DT_CENTER | DT_SINGLELINE | DT_NOPREFIX);
        DibujarControlDesplazamiento(hdc, m_rcSliderGeneral,
                                     static_cast<std::size_t>(std::min(maximoIzquierda(), maximoDerecha())),
                                     static_cast<std::size_t>(std::min(m_scrollIzquierda, m_scrollDerecha)), tema);
    }

    void RenderizarDiffView(HDC hdc, const RECT& rect, const TemaVisual& tema,
                            HFONT fuente) override {
        DibujarContenedor(hdc, rect, tema);
        const int altoFila = 24;
        const int inicio = rect.top + 34;
        const int pie = rect.bottom - 30;
        const int visibles = std::max(0, (pie - inicio) / altoFila);
        const bool semantico = ModoSemanticoDiffViewDisponibleV2();
        RECT encabezado{rect.left + 12, rect.top + 8, rect.right - 12, rect.top + 30};
        SetTextColor(hdc, tema.acento);
        DrawTextW(hdc, semantico ? L"Diff semántico · lectura guiada" : L"Diff clásico",
                  -1, &encabezado, DT_LEFT | DT_SINGLELINE | DT_NOPREFIX);
        SelectObject(hdc, fuente);
        for (int visible = 0; visible < visibles; ++visible) {
            const std::size_t indice = static_cast<std::size_t>(m_scrollGeneral + visible);
            if (indice >= m_estadoDiff.lineas.size()) break;
            const LineaDiffV2& linea = m_estadoDiff.lineas[indice];
            const int y = inicio + visible * altoFila;
            COLORREF color = tema.fondoTarjeta;
            if (linea.tipo == TipoLineaDiffV2::Agregada) color = RGB(218, 245, 220);
            else if (linea.tipo == TipoLineaDiffV2::Eliminada) color = RGB(250, 220, 220);
            else if (linea.tipo == TipoLineaDiffV2::Modificada) color = RGB(250, 240, 204);
            HBRUSH fila = CreateSolidBrush(tema.modoOscuro ? Mezclar(color, tema.fondo, 35) : color);
            RECT fondoFila{rect.left + 4, y, rect.right - 4, y + altoFila};
            FillRect(hdc, &fondoFila, fila);
            DeleteObject(fila);
            std::wstring texto = semantico ? TextoSemantico(linea) : TextoClasico(linea);
            SetTextColor(hdc, linea.tipo == TipoLineaDiffV2::Igual
                                   ? tema.textoPrincipal : tema.acento);
            RECT lineaRect{fondoFila.left + 8, y, fondoFila.right - 8, y + altoFila};
            DrawTextW(hdc, texto.c_str(), -1, &lineaRect,
                      DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);
        }
        m_rcSliderGeneral = {rect.left + 12, rect.bottom - 24, rect.right - 12, rect.bottom - 8};
        m_rcSliderIzquierda = {};
        m_rcSliderDerecha = {};
        DibujarControlDesplazamiento(hdc, m_rcSliderGeneral,
                                     static_cast<std::size_t>(MaximoDesplazamiento()),
                                     static_cast<std::size_t>(m_scrollGeneral), tema);
    }

private:
    static COLORREF Mezclar(COLORREF frente, COLORREF fondo, int porcentaje) {
        const int p = std::clamp(porcentaje, 0, 100);
        return RGB((GetRValue(frente) * p + GetRValue(fondo) * (100 - p)) / 100,
                   (GetGValue(frente) * p + GetGValue(fondo) * (100 - p)) / 100,
                   (GetBValue(frente) * p + GetBValue(fondo) * (100 - p)) / 100);
    }

    static std::wstring TextoClasico(const LineaDiffV2& linea) {
        return L"[" + std::to_wstring(linea.lineaIzquierda) + L" | " +
               std::to_wstring(linea.lineaDerecha) + L"] " + linea.izquierda +
               (linea.tipo == TipoLineaDiffV2::Modificada ? L"  →  " : L"") + linea.derecha;
    }

    static std::wstring TextoSemantico(const LineaDiffV2& linea) {
        switch (linea.tipo) {
            case TipoLineaDiffV2::Agregada: return L"Añadido en el archivo 2: " + linea.derecha;
            case TipoLineaDiffV2::Eliminada: return L"Eliminado del archivo 1: " + linea.izquierda;
            case TipoLineaDiffV2::Modificada: return L"Cambio: " + linea.izquierda + L"  →  " + linea.derecha;
            case TipoLineaDiffV2::Igual: return L"Sin cambios: " + linea.izquierda;
        }
        return {};
    }

    void DibujarLinea(HDC hdc, const RECT& rect, int mitad, int y, int alto,
                      const LineaDiffV2* linea, bool izquierda,
                      const TemaVisual& tema) const {
        const int inicio = izquierda ? rect.left + 2 : rect.left + mitad + 2;
        const int fin = izquierda ? rect.left + mitad - 2 : rect.right - 2;
        RECT fila{inicio, y, fin, y + alto};
        if (!linea) return;
        COLORREF color = tema.fondoTarjeta;
        if (linea->tipo == TipoLineaDiffV2::Agregada) color = RGB(218, 245, 220);
        else if (linea->tipo == TipoLineaDiffV2::Eliminada) color = RGB(250, 220, 220);
        else if (linea->tipo == TipoLineaDiffV2::Modificada) color = RGB(250, 240, 204);
        HBRUSH brocha = CreateSolidBrush(tema.modoOscuro ? Mezclar(color, tema.fondo, 35) : color);
        FillRect(hdc, &fila, brocha);
        DeleteObject(brocha);
        const std::wstring& texto = izquierda ? linea->izquierda : linea->derecha;
        const std::size_t numero = izquierda ? linea->lineaIzquierda : linea->lineaDerecha;
        wchar_t numeroTexto[24]{};
        swprintf_s(numeroTexto, L"%4zu", numero);
        SetTextColor(hdc, tema.textoSecundario);
        RECT numeroRect{fila.left + 4, y, fila.left + 42, y + alto};
        DrawTextW(hdc, numeroTexto, -1, &numeroRect,
                  DT_RIGHT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
        SetTextColor(hdc, tema.textoPrincipal);
        RECT contenido{fila.left + 50, y, fila.right - 6, y + alto};
        DrawTextW(hdc, texto.c_str(), -1, &contenido,
                  DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);
    }

    int MaximoDesplazamiento() const {
        const int alto = static_cast<int>(AreaEspecifica().bottom - AreaEspecifica().top);
        const int visibles = std::max(1, (alto - 64) / 24);
        return std::max(0, static_cast<int>(m_estadoDiff.lineas.size()) - visibles);
    }

    int maximoIzquierda() const { return MaximoDesplazamiento(); }
    int maximoDerecha() const { return MaximoDesplazamiento(); }

    void ActualizarSlider(POINT punto, HWND hwndPanel) {
        if (m_sliderActivo == 3) {
            m_scrollGeneral = static_cast<int>(DesplazamientoDesdePunto(
                m_rcSliderGeneral, punto, static_cast<std::size_t>(MaximoDesplazamiento())));
            if (!EstaEnDiffView()) {
                const int limite = std::min(maximoIzquierda(), maximoDerecha());
                m_scrollIzquierda = std::min(m_scrollGeneral, maximoIzquierda());
                m_scrollDerecha = std::min(m_scrollGeneral, maximoDerecha());
                m_scrollGeneral = std::min(m_scrollGeneral, limite);
            }
        } else if (m_sliderActivo == 1) {
            m_scrollIzquierda = static_cast<int>(DesplazamientoDesdePunto(
                m_rcSliderIzquierda, punto, static_cast<std::size_t>(maximoIzquierda())));
        } else if (m_sliderActivo == 2) {
            m_scrollDerecha = static_cast<int>(DesplazamientoDesdePunto(
                m_rcSliderDerecha, punto, static_cast<std::size_t>(maximoDerecha())));
        }
        InvalidateRect(hwndPanel, nullptr, FALSE);
    }

    std::shared_ptr<ServicioComparadorTextoV2> m_diff;
    EstadoDiffV2 m_estadoDiff;
    int m_scrollIzquierda{0};
    int m_scrollDerecha{0};
    int m_scrollGeneral{0};
    int m_sliderActivo{0};
    RECT m_rcSliderIzquierda{};
    RECT m_rcSliderDerecha{};
    RECT m_rcSliderGeneral{};
};

} // namespace SideViewV2
/**** ended inlining v2/vistas/vista_comparador_texto.hpp ****/
/**** start inlining v2/vistas/vista_comparador_documentos.hpp ****/

/**** skipping file: v2/vistas/vista_comparador_archivos.hpp ****/
/**** skipping file: v2/nucleo/configuracion_v2.hpp ****/
/**** skipping file: v2/nucleo/servicio_diff.hpp ****/
#include <windowsx.h>
#include <algorithm>
#include <memory>

namespace SideViewV2 {

class VistaComparadorDocumentosV2 final : public VistaComparadorArchivosV2 {
public:
    explicit VistaComparadorDocumentosV2(
        std::shared_ptr<ServicioComparadorDocumentosV2> servicio)
        : VistaComparadorArchivosV2(servicio), m_diff(std::move(servicio)) {}

    void Liberar() override {
        m_estadoDiff = {};
        m_scrollIzquierda = 0;
        m_scrollDerecha = 0;
        m_scrollGeneral = 0;
        m_sliderActivo = 0;
        VistaComparadorArchivosV2::Liberar();
    }

    void AlCambiarSeleccion(const std::vector<std::wstring>& archivos,
                            const std::wstring& carpeta) override {
        VistaComparadorArchivosV2::AlCambiarSeleccion(archivos, carpeta);
        m_estadoDiff = m_diff ? m_diff->CrearDiff(archivos) : EstadoDiffV2{};
        m_scrollIzquierda = 0;
        m_scrollDerecha = 0;
        m_scrollGeneral = 0;
    }

protected:
    const wchar_t* NombreVista() const override { return L"Comparar documentos"; }
    bool TieneDiffView() const override { return true; }

    void AlCambiarModoDiff(bool) override {
        m_scrollIzquierda = 0;
        m_scrollDerecha = 0;
        m_scrollGeneral = 0;
        m_sliderActivo = 0;
    }

    bool ProcesarMensajeContenido(UINT msg, WPARAM wParam, LPARAM lParam,
                                  HWND hwndPanel) override {
        if (msg == WM_MOUSEWHEEL) {
            const int delta = GET_WHEEL_DELTA_WPARAM(wParam) > 0 ? -3 : 3;
            if (EstaEnDiffView()) {
                m_scrollGeneral = std::clamp(m_scrollGeneral + delta, 0, MaximoGeneral());
            } else {
                m_scrollIzquierda = std::clamp(m_scrollIzquierda + delta, 0, MaximoIzquierda());
                m_scrollDerecha = std::clamp(m_scrollDerecha + delta, 0, MaximoDerecha());
            }
            InvalidateRect(hwndPanel, nullptr, FALSE);
            return true;
        }
        if (msg == WM_KEYDOWN && (wParam == VK_UP || wParam == VK_DOWN)) {
            const int delta = wParam == VK_UP ? -1 : 1;
            if (EstaEnDiffView()) m_scrollGeneral = std::clamp(m_scrollGeneral + delta, 0, MaximoGeneral());
            else {
                m_scrollIzquierda = std::clamp(m_scrollIzquierda + delta, 0, MaximoIzquierda());
                m_scrollDerecha = std::clamp(m_scrollDerecha + delta, 0, MaximoDerecha());
            }
            InvalidateRect(hwndPanel, nullptr, FALSE);
            return true;
        }
        if (msg == WM_LBUTTONDOWN) {
            const POINT punto{GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
            if (EstaEnDiffView() && PtInRect(&m_rcSliderGeneral, punto)) m_sliderActivo = 3;
            else if (!EstaEnDiffView() && PtInRect(&m_rcSliderIzquierda, punto)) m_sliderActivo = 1;
            else if (!EstaEnDiffView() && PtInRect(&m_rcSliderDerecha, punto)) m_sliderActivo = 2;
            else if (!EstaEnDiffView() && PtInRect(&m_rcSliderGeneral, punto)) m_sliderActivo = 3;
            else return false;
            ActualizarSlider({GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)}, hwndPanel);
            SetCapture(hwndPanel);
            return true;
        }
        if (msg == WM_MOUSEMOVE && m_sliderActivo != 0) {
            ActualizarSlider({GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)}, hwndPanel);
            return true;
        }
        if (msg == WM_LBUTTONUP && m_sliderActivo != 0) {
            m_sliderActivo = 0;
            ReleaseCapture();
            return true;
        }
        return false;
    }

    void RenderizarContenido(HDC hdc, const RECT& rect, const TemaVisual& tema,
                             HFONT fuente) override {
        DibujarContenedor(hdc, rect, tema);
        const int mitad = (rect.right - rect.left) / 2;
        const int altoFila = 22;
        const int inicio = rect.top + 34;
        const int pie = rect.bottom - 58;
        const int visibles = std::max(0, (pie - inicio) / altoFila);
        RECT cabeceraIzquierda{rect.left + 8, rect.top + 8, rect.left + mitad - 4, rect.top + 28};
        RECT cabeceraDerecha{rect.left + mitad + 4, rect.top + 8, rect.right - 8, rect.top + 28};
        SetTextColor(hdc, tema.textoPrincipal);
        DrawTextW(hdc, L"Documento 1", -1, &cabeceraIzquierda,
                  DT_LEFT | DT_SINGLELINE | DT_NOPREFIX);
        DrawTextW(hdc, L"Documento 2", -1, &cabeceraDerecha,
                  DT_LEFT | DT_SINGLELINE | DT_NOPREFIX);
        SelectObject(hdc, fuente);
        for (int visible = 0; visible < visibles; ++visible) {
            const int y = inicio + visible * altoFila;
            const std::size_t izquierda = static_cast<std::size_t>(m_scrollIzquierda + visible);
            const std::size_t derecha = static_cast<std::size_t>(m_scrollDerecha + visible);
            DibujarLinea(hdc, rect, mitad, y, altoFila,
                         izquierda < m_estadoDiff.lineas.size() ? &m_estadoDiff.lineas[izquierda] : nullptr,
                         true, tema);
            DibujarLinea(hdc, rect, mitad, y, altoFila,
                         derecha < m_estadoDiff.lineas.size() ? &m_estadoDiff.lineas[derecha] : nullptr,
                         false, tema);
        }
        DibujarSlidersNormales(hdc, rect, mitad, tema);
    }

    void RenderizarDiffView(HDC hdc, const RECT& rect, const TemaVisual& tema,
                            HFONT fuente) override {
        DibujarContenedor(hdc, rect, tema);
        const int altoFila = 24;
        const int inicio = rect.top + 34;
        const int visibles = std::max(0, (static_cast<int>(rect.bottom) - 30 - inicio) / altoFila);
        const bool semantico = ModoSemanticoDiffViewDisponibleV2();
        RECT encabezado{rect.left + 12, rect.top + 8, rect.right - 12, rect.top + 30};
        SetTextColor(hdc, tema.acento);
        DrawTextW(hdc, semantico ? L"Diff semántico · explicación del documento" : L"Diff clásico del documento",
                  -1, &encabezado, DT_LEFT | DT_SINGLELINE | DT_NOPREFIX);
        SelectObject(hdc, fuente);
        for (int visible = 0; visible < visibles; ++visible) {
            const std::size_t indice = static_cast<std::size_t>(m_scrollGeneral + visible);
            if (indice >= m_estadoDiff.lineas.size()) break;
            const LineaDiffV2& linea = m_estadoDiff.lineas[indice];
            const int y = inicio + visible * altoFila;
            COLORREF color = tema.fondoTarjeta;
            if (linea.tipo == TipoLineaDiffV2::Agregada) color = RGB(218, 245, 220);
            else if (linea.tipo == TipoLineaDiffV2::Eliminada) color = RGB(250, 220, 220);
            else if (linea.tipo == TipoLineaDiffV2::Modificada) color = RGB(250, 240, 204);
            HBRUSH brocha = CreateSolidBrush(tema.modoOscuro ? Mezclar(color, tema.fondo, 35) : color);
            RECT fila{rect.left + 4, y, rect.right - 4, y + altoFila};
            FillRect(hdc, &fila, brocha);
            DeleteObject(brocha);
            const std::wstring texto = semantico ? TextoSemantico(linea) : TextoClasico(linea);
            SetTextColor(hdc, linea.tipo == TipoLineaDiffV2::Igual ? tema.textoPrincipal : tema.acento);
            RECT contenido{fila.left + 8, y, fila.right - 8, y + altoFila};
            DrawTextW(hdc, texto.c_str(), -1, &contenido,
                      DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);
        }
        m_rcSliderGeneral = {rect.left + 12, rect.bottom - 24, rect.right - 12, rect.bottom - 8};
        m_rcSliderIzquierda = {};
        m_rcSliderDerecha = {};
        DibujarControlDesplazamiento(hdc, m_rcSliderGeneral,
                                     static_cast<std::size_t>(MaximoGeneral()),
                                     static_cast<std::size_t>(m_scrollGeneral), tema);
    }

private:
    static COLORREF Mezclar(COLORREF frente, COLORREF fondo, int porcentaje) {
        const int p = std::clamp(porcentaje, 0, 100);
        return RGB((GetRValue(frente) * p + GetRValue(fondo) * (100 - p)) / 100,
                   (GetGValue(frente) * p + GetGValue(fondo) * (100 - p)) / 100,
                   (GetBValue(frente) * p + GetBValue(fondo) * (100 - p)) / 100);
    }

    static std::wstring TextoClasico(const LineaDiffV2& linea) {
        return L"[" + std::to_wstring(linea.lineaIzquierda) + L" | " +
               std::to_wstring(linea.lineaDerecha) + L"] " + linea.izquierda +
               (linea.tipo == TipoLineaDiffV2::Modificada ? L"  →  " : L"") + linea.derecha;
    }

    static std::wstring TextoSemantico(const LineaDiffV2& linea) {
        switch (linea.tipo) {
            case TipoLineaDiffV2::Agregada: return L"Añadido en el documento 2: " + linea.derecha;
            case TipoLineaDiffV2::Eliminada: return L"Eliminado del documento 1: " + linea.izquierda;
            case TipoLineaDiffV2::Modificada: return L"Cambio: " + linea.izquierda + L"  →  " + linea.derecha;
            case TipoLineaDiffV2::Igual: return L"Sin cambios: " + linea.izquierda;
        }
        return {};
    }

    void DibujarLinea(HDC hdc, const RECT& rect, int mitad, int y, int alto,
                      const LineaDiffV2* linea, bool izquierda,
                      const TemaVisual& tema) const {
        if (!linea) return;
        const int inicio = izquierda ? rect.left + 2 : rect.left + mitad + 2;
        const int fin = izquierda ? rect.left + mitad - 2 : rect.right - 2;
        RECT fila{inicio, y, fin, y + alto};
        COLORREF color = tema.fondoTarjeta;
        if (linea->tipo == TipoLineaDiffV2::Agregada) color = RGB(218, 245, 220);
        else if (linea->tipo == TipoLineaDiffV2::Eliminada) color = RGB(250, 220, 220);
        else if (linea->tipo == TipoLineaDiffV2::Modificada) color = RGB(250, 240, 204);
        HBRUSH brocha = CreateSolidBrush(tema.modoOscuro ? Mezclar(color, tema.fondo, 35) : color);
        FillRect(hdc, &fila, brocha);
        DeleteObject(brocha);
        const std::wstring& texto = izquierda ? linea->izquierda : linea->derecha;
        const std::size_t numero = izquierda ? linea->lineaIzquierda : linea->lineaDerecha;
        wchar_t numeroTexto[24]{};
        swprintf_s(numeroTexto, L"%4zu", numero);
        SetTextColor(hdc, tema.textoSecundario);
        RECT numeroRect{fila.left + 4, y, fila.left + 42, y + alto};
        DrawTextW(hdc, numeroTexto, -1, &numeroRect,
                  DT_RIGHT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
        SetTextColor(hdc, tema.textoPrincipal);
        RECT contenido{fila.left + 50, y, fila.right - 6, y + alto};
        DrawTextW(hdc, texto.c_str(), -1, &contenido,
                  DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);
    }

    void DibujarSlidersNormales(HDC hdc, const RECT& rect, int mitad,
                                const TemaVisual& tema) {
        m_rcSliderIzquierda = {rect.left + 10, rect.bottom - 38,
                               rect.left + mitad - 8, rect.bottom - 24};
        m_rcSliderDerecha = {rect.left + mitad + 8, rect.bottom - 38,
                             rect.right - 10, rect.bottom - 24};
        m_rcSliderGeneral = {rect.left + 10, rect.bottom - 20,
                             rect.right - 10, rect.bottom - 6};
        DibujarControlDesplazamiento(hdc, m_rcSliderIzquierda,
                                     static_cast<std::size_t>(MaximoIzquierda()),
                                     static_cast<std::size_t>(m_scrollIzquierda), tema);
        DibujarControlDesplazamiento(hdc, m_rcSliderDerecha,
                                     static_cast<std::size_t>(MaximoDerecha()),
                                     static_cast<std::size_t>(m_scrollDerecha), tema);
        DibujarControlDesplazamiento(hdc, m_rcSliderGeneral,
                                     static_cast<std::size_t>(std::min(MaximoIzquierda(), MaximoDerecha())),
                                     static_cast<std::size_t>(std::min(m_scrollIzquierda, m_scrollDerecha)), tema);
        SetTextColor(hdc, tema.textoSecundario);
        RECT etiqueta{rect.left + 10, rect.bottom - 54, rect.right - 10, rect.bottom - 40};
        DrawTextW(hdc, L"Arriba: desplazamiento independiente · Abajo: sincronizado",
                  -1, &etiqueta, DT_CENTER | DT_SINGLELINE | DT_NOPREFIX);
    }

    int MaximoGeneral() const { return MaximoContenido(); }
    int MaximoIzquierda() const { return MaximoContenido(); }
    int MaximoDerecha() const { return MaximoContenido(); }

    int MaximoContenido() const {
        const int alto = static_cast<int>(AreaEspecifica().bottom - AreaEspecifica().top);
        const int visibles = std::max(1, (alto - 64) / 24);
        return std::max(0, static_cast<int>(m_estadoDiff.lineas.size()) - visibles);
    }

    void ActualizarSlider(POINT punto, HWND hwndPanel) {
        if (m_sliderActivo == 3) {
            const int limite = std::min(MaximoIzquierda(), MaximoDerecha());
            const int valor = static_cast<int>(DesplazamientoDesdePunto(
                m_rcSliderGeneral, punto, static_cast<std::size_t>(EstaEnDiffView() ? MaximoGeneral() : limite)));
            m_scrollGeneral = std::min(valor, MaximoGeneral());
            if (!EstaEnDiffView()) {
                m_scrollIzquierda = std::min(valor, MaximoIzquierda());
                m_scrollDerecha = std::min(valor, MaximoDerecha());
            }
        } else if (m_sliderActivo == 1) {
            m_scrollIzquierda = static_cast<int>(DesplazamientoDesdePunto(
                m_rcSliderIzquierda, punto, static_cast<std::size_t>(MaximoIzquierda())));
        } else if (m_sliderActivo == 2) {
            m_scrollDerecha = static_cast<int>(DesplazamientoDesdePunto(
                m_rcSliderDerecha, punto, static_cast<std::size_t>(MaximoDerecha())));
        }
        InvalidateRect(hwndPanel, nullptr, FALSE);
    }

    std::shared_ptr<ServicioComparadorDocumentosV2> m_diff;
    EstadoDiffV2 m_estadoDiff;
    int m_scrollIzquierda{0};
    int m_scrollDerecha{0};
    int m_scrollGeneral{0};
    int m_sliderActivo{0};
    RECT m_rcSliderIzquierda{};
    RECT m_rcSliderDerecha{};
    RECT m_rcSliderGeneral{};
};

} // namespace SideViewV2
/**** ended inlining v2/vistas/vista_comparador_documentos.hpp ****/
/**** skipping file: v2/nucleo/servicios_vistas.hpp ****/
/**** skipping file: v2/nucleo/configuracion_v2.hpp ****/
#include <memory>
#include <vector>
#include <string>

namespace SideViewV2 {

class FabricaVistas {
public:
    static DependenciasVistaV2& Dependencias() {
        static DependenciasVistaV2 dependencias;
        return dependencias;
    }

    static std::shared_ptr<IPanelContenido> CrearVistaComparadorVideos(
        const std::vector<std::wstring>& seleccion = {}) {
        auto vista = std::make_shared<VistaComparadorVideos>(Dependencias().comparadorVideos);
        vista->AlCambiarSeleccion(seleccion, L"");
        return vista;
    }

    static bool SeleccionaDosDelMismoTipo(
        const std::vector<std::wstring>& seleccion,
        TipoVistaContenido* tipoResultado = nullptr) {
        return PuedeCompararSeleccionV2(seleccion, tipoResultado);
    }

    static bool DebeCrearComparador(
        const std::vector<std::wstring>& seleccion) {
        TipoVistaContenido tipo{};
        if (!SeleccionaDosDelMismoTipo(seleccion, &tipo)) return false;
        return tipo != TipoVistaContenido::Video ||
               ComparadorVideosExperimentalDisponibleV2();
    }

    static std::shared_ptr<IPanelContenido> CrearComparadorParaSeleccion(
        const std::vector<std::wstring>& seleccion,
        const std::wstring& carpeta) {
        if (!DebeCrearComparador(seleccion)) return nullptr;

        TipoVistaContenido tipo{};
        if (!SeleccionaDosDelMismoTipo(seleccion, &tipo)) return nullptr;
        if (tipo == TipoVistaContenido::Video) {
            return CrearVistaComparadorVideos(seleccion);
        }
        if (tipo == TipoVistaContenido::Texto ||
            tipo == TipoVistaContenido::Codigo) {
            auto vista = std::make_shared<VistaComparadorTextoV2>(
                Dependencias().comparadorTexto);
            vista->AlCambiarSeleccion(seleccion, carpeta);
            return vista;
        }
        if (tipo == TipoVistaContenido::Documentos) {
            auto vista = std::make_shared<VistaComparadorDocumentosV2>(
                Dependencias().comparadorDocumentos);
            vista->AlCambiarSeleccion(seleccion, carpeta);
            return vista;
        }
        auto vista = std::make_shared<VistaComparadorArchivosV2>(
            Dependencias().comparadorArchivos);
        vista->AlCambiarSeleccion(seleccion, carpeta);
        return vista;
    }

    static std::shared_ptr<IPanelContenido> CrearVistaParaSeleccion(
        const std::vector<std::wstring>& seleccion,
        const std::wstring& carpeta) {

        TipoVistaContenido tipo = ClasificarSeleccion(seleccion, carpeta);

        std::shared_ptr<IPanelContenido> vista;

        switch (tipo) {
            case TipoVistaContenido::Tutorial:
                vista = std::make_shared<VistaWalkthrough>(Dependencias().enlaces);
                break;

            case TipoVistaContenido::Comprimidos:
                vista = std::make_shared<VistaComprimidos>(Dependencias().abrirConComprimidos, Dependencias().navegacion, Dependencias().extensiones, Dependencias().contenidoComprimido);
                break;

            case TipoVistaContenido::Comics:
                vista = std::make_shared<VistaComics>(Dependencias().extensiones, Dependencias().navegacion, Dependencias().previsualizacionMedia);
                break;

            case TipoVistaContenido::Carpetas:
                vista = std::make_shared<VistaCarpetas>(Dependencias().abrirConCarpetas,
                    Dependencias().navegacion, Dependencias().fusionCarpetas,
                    Dependencias().inspeccionCarpetas, Dependencias().versionMod,
                    Dependencias().notas);
                break;

            case TipoVistaContenido::Texto:
                vista = std::make_shared<VistaTextoCodigo>(Dependencias().texto, Dependencias().navegacion, false);
                break;

            case TipoVistaContenido::Imagen:
                vista = std::make_shared<VistaImagenes>(Dependencias().navegacion, Dependencias().abrirConImagenes, Dependencias().previsualizacionMedia);
                break;

            case TipoVistaContenido::Video:
                vista = std::make_shared<VistaVideos>(
                    Dependencias().navegacion,
                    Dependencias().abrirConVideos,
                    Dependencias().previsualizacionMedia,
                    Dependencias().reproductorVideo);
                break;

            case TipoVistaContenido::Documentos:
                vista = std::make_shared<VistaEnDesarrollo>(
                    L"Visor de Documentos de Oficina",
                    L"📑",
                    L"Visualización embebida de archivos PDF, Word (DOCX), Excel (XLSX) y presentaciones (PPTX)."
                );
                break;

            case TipoVistaContenido::Codigo:
                vista = std::make_shared<VistaTextoCodigo>(Dependencias().texto, Dependencias().navegacion, true);
                break;

            default:
                vista = std::make_shared<VistaEnDesarrollo>(
                    L"Previsualizador Universal",
                    L"🔍",
                    L"Inspección de metadatos de archivos del sistema y delegado IPreviewHandler."
                );
                break;
        }

        if (vista) {
            vista->AlCambiarSeleccion(seleccion, carpeta);
        }
        return vista;
    }

};

} // namespace SideViewV2
/**** ended inlining v2/nucleo/fabrica_vistas.hpp ****/
/**** start inlining v2/nucleo/shell_extractor.hpp ****/

#include <windows.h>
#include <shobjidl.h>
#include <exdisp.h>
#include <shlobj.h>
#include <shlguid.h>
#include <string>
#include <vector>
/**** skipping file: v2/nucleo/servicio_com.hpp ****/

namespace SideViewV2 {

inline IShellView* ObtenerShellViewActivo(HWND top, HWND* outTab) {
    if (outTab) *outTab = nullptr;
    if (!top) return nullptr;

    IShellWindows* shellWindows = nullptr;
    HRESULT hr = CoCreateInstance(CLSID_ShellWindows, nullptr, CLSCTX_ALL,
                                  IID_IShellWindows, reinterpret_cast<void**>(&shellWindows));
    if (FAILED(hr) || !shellWindows) return nullptr;

    HWND tabActiva = FindWindowExW(top, nullptr, L"ShellTabWindowClass", nullptr);
    long cantidad = 0;
    shellWindows->get_Count(&cantidad);

    IShellView* resultado = nullptr;

    for (long i = 0; i < cantidad && !resultado; ++i) {
        VARIANT indice;
        VariantInit(&indice);
        indice.vt = VT_I4;
        indice.lVal = i;

        IDispatch* despacho = nullptr;
        if (FAILED(shellWindows->Item(indice, &despacho)) || !despacho) {
            VariantClear(&indice);
            continue;
        }
        VariantClear(&indice);

        IWebBrowserApp* app = nullptr;
        if (FAILED(despacho->QueryInterface(IID_IWebBrowserApp, reinterpret_cast<void**>(&app))) || !app) {
            despacho->Release();
            continue;
        }

        SHANDLE_PTR hwndItem = 0;
        if (SUCCEEDED(app->get_HWND(&hwndItem)) && hwndItem) {
            HWND hwndVentana = reinterpret_cast<HWND>(hwndItem);
            bool coincide = (hwndVentana == top);
            if (!coincide) {
                HWND ancestro = GetAncestor(hwndVentana, GA_ROOT);
                if (ancestro == top) coincide = true;
            }

            if (coincide) {
                IServiceProvider* sp = nullptr;
                if (SUCCEEDED(app->QueryInterface(IID_IServiceProvider, reinterpret_cast<void**>(&sp))) && sp) {
                    IShellBrowser* sb = nullptr;
                    if (SUCCEEDED(sp->QueryService(SID_STopLevelBrowser, IID_IShellBrowser,
                                                   reinterpret_cast<void**>(&sb))) && sb) {
                        IShellView* sv = nullptr;
                        if (SUCCEEDED(sb->QueryActiveShellView(&sv)) && sv) {
                            HWND hwndVista = nullptr;
                            sv->GetWindow(&hwndVista);
                            if (hwndVista) {
                                bool vistaCoincide = true;
                                if (tabActiva && IsWindow(tabActiva)) {
                                    vistaCoincide = IsChild(tabActiva, hwndVista);
                                }
                                if (vistaCoincide) {
                                    resultado = sv;
                                    if (outTab) {
                                        HWND p = hwndVista;
                                        while (p && p != top) {
                                            wchar_t clase[64]{};
                                            GetClassNameW(p, clase, ARRAYSIZE(clase));
                                            if (_wcsicmp(clase, L"ShellTabWindowClass") == 0) {
                                                *outTab = p;
                                                break;
                                            }
                                            p = GetParent(p);
                                        }
                                        if (!*outTab) *outTab = tabActiva ? tabActiva : top;
                                    }
                                }
                            }
                            if (!resultado) sv->Release();
                        }
                        sb->Release();
                    }
                    sp->Release();
                }
            }
        }
        app->Release();
        despacho->Release();
    }

    shellWindows->Release();
    return resultado;
}

inline std::vector<std::wstring> ObtenerRutasSeleccionadasDeVista(IShellView* vista, std::wstring* outCarpeta = nullptr) {
    if (outCarpeta) outCarpeta->clear();
    if (!vista) return {};
    AmbitoComV2 com;
    if (!com.Disponible()) return {};

    IFolderView2* fv = nullptr;
    if (FAILED(vista->QueryInterface(IID_IFolderView2, reinterpret_cast<void**>(&fv))) || !fv) {
        return {};
    }

    if (outCarpeta) {
        IShellItem* carpetaItem = nullptr;
        if (SUCCEEDED(fv->GetFolder(IID_IShellItem, reinterpret_cast<void**>(&carpetaItem))) && carpetaItem) {
            PWSTR rutaDirectorio = nullptr;
            if (SUCCEEDED(carpetaItem->GetDisplayName(SIGDN_FILESYSPATH, &rutaDirectorio)) && rutaDirectorio) {
                *outCarpeta = rutaDirectorio;
                CoTaskMemFree(rutaDirectorio);
            }
            carpetaItem->Release();
        }
    }

    std::vector<std::wstring> rutas;
    IShellItemArray* items = nullptr;
    if (SUCCEEDED(fv->GetSelection(TRUE, &items)) && items) {
        DWORD conteo = 0;
        items->GetCount(&conteo);
        for (DWORD i = 0; i < conteo; ++i) {
            IShellItem* item = nullptr;
            if (SUCCEEDED(items->GetItemAt(i, &item)) && item) {
                PWSTR ruta = nullptr;
                if (SUCCEEDED(item->GetDisplayName(SIGDN_FILESYSPATH, &ruta)) && ruta) {
                    rutas.push_back(ruta);
                    CoTaskMemFree(ruta);
                }
                item->Release();
            }
        }
        items->Release();
    }

    fv->Release();
    return rutas;
}

} // namespace SideViewV2
/**** ended inlining v2/nucleo/shell_extractor.hpp ****/
/**** start inlining v2/nucleo/gestor_ventanas.hpp ****/

#include <windows.h>
#include <commctrl.h>
#include <map>
#include <mutex>
#include <vector>
#include <windhawk_api.h>
#include <windhawk_utils.h>
/**** skipping file: v2/ui/ventana_base.hpp ****/
/**** skipping file: v2/ui/tema_visual.hpp ****/
/**** skipping file: v2/nucleo/shell_extractor.hpp ****/
/**** skipping file: v2/nucleo/enrutador_vistas.hpp ****/
/**** skipping file: v2/nucleo/fabrica_vistas.hpp ****/
/**** skipping file: v2/nucleo/configuracion_v2.hpp ****/

namespace SideViewV2 {

inline UINT WM_SIDEVIEW_TOGGLE = 0;
inline UINT WM_SIDEVIEW_SETTINGS = 0;

inline std::mutex g_bloqueoEstados;
inline std::map<HWND, EstadoPanelV2*> g_estadosPorVentana;

inline bool PuedeCompararArchivosPorSeleccionV2(
    const std::vector<std::wstring>& seleccion) {
    return FabricaVistas::DebeCrearComparador(seleccion);
}

LRESULT CALLBACK ProcedimientoVentanaPanel(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK ProcedimientoSubclasePadre(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam,
                                            UINT_PTR uIdSubclass, DWORD_PTR dwRefData);
LRESULT CALLBACK ProcedimientoSubclaseTop(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam,
                                          DWORD_PTR dwRefData);

inline void LiberarVistaContenidoV2(EstadoPanelV2* estado) {
    if (!estado || !estado->vistaContenido) return;
    // Liberar es parte del contrato de la vista: desmonta reproductores,
    // timers, capturas de ratón y cualquier backend que pueda mantener abierto
    // un archivo. El shared_ptr se corta inmediatamente después para impedir
    // que una vista contraída siga recibiendo eventos.
    estado->vistaContenido->Liberar();
    estado->vistaContenido.reset();
}

inline void DestruirEstado(EstadoPanelV2* estado, bool quitarDelMapa = true) {
    if (!estado) return;
    bool esperado = false;
    if (!estado->destruyendo.compare_exchange_strong(esperado, true,
                                                     std::memory_order_acq_rel)) {
        return;
    }

    // Cortar primero la entrada del estado evita que un timer, una reentrada
    // de DestroyWindow o una notificación pendiente use recursos en proceso
    // de liberación.
    estado->abierto = false;
    if (estado->panel && IsWindow(estado->panel)) {
        KillTimer(estado->panel, 101);
    }

    if (quitarDelMapa) {
        std::lock_guard<std::mutex> lock(g_bloqueoEstados);
        for (auto it = g_estadosPorVentana.begin(); it != g_estadosPorVentana.end(); ) {
            if (it->second == estado) {
                it = g_estadosPorVentana.erase(it);
            } else {
                ++it;
            }
        }
    }

    LiberarBufferMemoria(estado);

    LiberarVistaContenidoV2(estado);

    if (estado->panel && IsWindow(estado->panel)) {
        DestroyWindow(estado->panel);
        estado->panel = nullptr;
    }

    if (estado->parent && IsWindow(estado->parent)) {
        RemoveWindowSubclass(estado->parent, ProcedimientoSubclasePadre, 1);
    }

    if (estado->tab && IsWindow(estado->tab)) {
        RemoveWindowSubclass(estado->tab, ProcedimientoSubclasePadre, 2);
    }

    delete estado;
}

inline void DisponerPanel(EstadoPanelV2* estado) {
    if (!estado || !estado->abierto || !estado->panel || !estado->view || !estado->parent) {
        return;
    }
    if (!IsWindow(estado->panel) || !IsWindow(estado->view) || !IsWindow(estado->parent)) {
        return;
    }

    UINT dpi = ObtenerDpiVentana(estado->panel);
    RECT rectCliente{};
    GetClientRect(estado->parent, &rectCliente);

    // El renderer GPU es un hijo directo del frame y el mod de referencia
    // conserva SHELLDLL_DefView como superficie COM oculta. No se debe
    // redimensionar el overlay GPU: sus hooks de layout y DirectComposition
    // volverían a ocupar el área completa y podrían producir una carrera de
    // geometría. El panel se dibuja encima del sector derecho y el overlay
    // permanece intacto como superficie de Explorer.
    if (estado->superficie.gpuAcelerada) {
        if (!IsWindowVisible(estado->view)) {
            ShowWindow(estado->panel, SW_HIDE);
            return;
        }
        auto compatibilidad = FabricaVistas::Dependencias().compatibilidadGpu;
        if (!compatibilidad || !compatibilidad->ActualizarLimites(estado->superficie)) {
            return;
        }
        estado->view = estado->superficie.superficieVisual;
        estado->parent = estado->superficie.contenedor;
        estado->viewOriginal = estado->superficie.limites;
        estado->parentCliente = rectCliente;

        const int anchoVista = estado->viewOriginal.right - estado->viewOriginal.left;
        const int altoVista = estado->viewOriginal.bottom - estado->viewOriginal.top;
        if (anchoVista < 1 || altoVista < 1) return;

        if (estado->comparadorArchivosActivo) {
            SetWindowPos(estado->panel, HWND_TOP, estado->viewOriginal.left,
                         estado->viewOriginal.top, anchoVista, altoVista,
                         SWP_NOACTIVATE | SWP_SHOWWINDOW);
        } else {
            const int anchoPanelSolicitado = EscalarDpi(estado->anchoDIP, dpi);
            const int anchoPanelMaximo = std::max(1, anchoVista - EscalarDpi(200, dpi));
            const int anchoPanelMinimo = std::min(EscalarDpi(220, dpi), anchoPanelMaximo);
            const int anchoPanel = std::min(
                std::max(anchoPanelSolicitado, anchoPanelMinimo), anchoPanelMaximo);
            const RECT rectPanel{
                estado->viewOriginal.right - anchoPanel,
                estado->viewOriginal.top,
                estado->viewOriginal.right,
                estado->viewOriginal.bottom
            };
            SetWindowPos(estado->panel, HWND_TOP, rectPanel.left, rectPanel.top,
                         rectPanel.right - rectPanel.left,
                         rectPanel.bottom - rectPanel.top,
                         SWP_NOACTIVATE | SWP_SHOWWINDOW);
        }
        InvalidateRect(estado->panel, nullptr, FALSE);
        return;
    }

    int deltaAncho = rectCliente.right - estado->parentCliente.right;
    int deltaAlto = rectCliente.bottom - estado->parentCliente.bottom;
    if (deltaAncho != 0 || deltaAlto != 0) {
        estado->viewOriginal.right += deltaAncho;
        estado->viewOriginal.bottom += deltaAlto;
        estado->parentCliente = rectCliente;
    }

    estado->viewOriginal.left = std::max(0L, estado->viewOriginal.left);
    estado->viewOriginal.top = std::max(0L, estado->viewOriginal.top);
    estado->viewOriginal.right = std::min(rectCliente.right, estado->viewOriginal.right);
    estado->viewOriginal.bottom = std::min(rectCliente.bottom, estado->viewOriginal.bottom);

    int anchoVista = estado->viewOriginal.right - estado->viewOriginal.left;
    int altoVista = estado->viewOriginal.bottom - estado->viewOriginal.top;
    if (anchoVista < 1 || altoVista < 1) return;

    // El comparador es una experiencia temporal a pantalla completa dentro
    // del contenedor de Explorer. Al salir se vuelve al layout lateral normal.
    if (estado->comparadorArchivosActivo) {
        SetWindowPos(estado->view, nullptr, estado->viewOriginal.left,
                     estado->viewOriginal.top, 1, altoVista,
                     SWP_NOZORDER | SWP_NOACTIVATE);
        SetWindowPos(estado->panel, HWND_TOP, estado->viewOriginal.left,
                     estado->viewOriginal.top, anchoVista, altoVista,
                     SWP_NOACTIVATE | SWP_SHOWWINDOW);
        InvalidateRect(estado->panel, nullptr, FALSE);
        return;
    }

    int anchoPanelSolicitado = EscalarDpi(estado->anchoDIP, dpi);
    int anchoPanelMaximo = std::max(1, anchoVista - EscalarDpi(200, dpi));
    int anchoPanelMinimo = std::min(EscalarDpi(220, dpi), anchoPanelMaximo);
    int anchoPanel = std::min(std::max(anchoPanelSolicitado, anchoPanelMinimo), anchoPanelMaximo);

    int nuevoAnchoVista = std::max(1, anchoVista - anchoPanel);

    RECT rectPanel = {
        estado->viewOriginal.left + anchoVista - anchoPanel,
        estado->viewOriginal.top,
        estado->viewOriginal.right,
        estado->viewOriginal.bottom
    };

    // Redimensionar la vista nativa de archivos para reservar espacio sin distorsionar DirectUI
    SetWindowPos(estado->view, nullptr, estado->viewOriginal.left, estado->viewOriginal.top,
                 nuevoAnchoVista, altoVista, SWP_NOZORDER | SWP_NOACTIVATE);

    // Posicionar nuestro panel cooperativo a la derecha
    SetWindowPos(estado->panel, HWND_TOP, rectPanel.left, rectPanel.top,
                 rectPanel.right - rectPanel.left, rectPanel.bottom - rectPanel.top,
                 SWP_NOACTIVATE | SWP_SHOWWINDOW);

    InvalidateRect(estado->panel, nullptr, FALSE);
}

inline void CerrarEstado(EstadoPanelV2* estado) {
    if (!estado || !estado->abierto) return;
    estado->abierto = false;

    // Contraer el panel no es sólo una operación visual. El contenido deja
    // de estar activo y debe desmontarse para liberar handles de archivos,
    // Media Foundation, callbacks y timers antes de ocultar la ventana.
    LiberarVistaContenidoV2(estado);

    if (estado->panel && IsWindow(estado->panel)) {
        KillTimer(estado->panel, 101);
        ShowWindow(estado->panel, SW_HIDE);
    }

    // El overlay GPU nunca se redimensionó: sólo se ocultó nuestro panel.
    // Restaurarlo aquí provocaría una carrera con el layout del mod GPU.
    if (estado->superficie.gpuAcelerada) return;

    // Restaurar el ancho completo de la vista de archivos
    if (estado->view && IsWindow(estado->view)) {
        int anchoTotal = estado->viewOriginal.right - estado->viewOriginal.left;
        int altoTotal = estado->viewOriginal.bottom - estado->viewOriginal.top;
        SetWindowPos(estado->view, nullptr, estado->viewOriginal.left, estado->viewOriginal.top,
                     anchoTotal, altoTotal, SWP_NOZORDER | SWP_NOACTIVATE);
    }
}

inline EstadoPanelV2* AsegurarEstado(HWND top, IShellView** outVista = nullptr) {
    if (outVista) *outVista = nullptr;
    if (!IsWindow(top)) return nullptr;
    AmbitoComV2 com;
    if (!com.Disponible()) return nullptr;

    HWND tab = nullptr;
    IShellView* vistaShell = ObtenerShellViewActivo(top, &tab);
    if (!vistaShell) return nullptr;

    HWND ventanaVista = nullptr;
    vistaShell->GetWindow(&ventanaVista);
    if (!ventanaVista) {
        vistaShell->Release();
        return nullptr;
    }

    auto compatibilidad = FabricaVistas::Dependencias().compatibilidadGpu;
    const SuperficieExploradorV2 superficie = compatibilidad
        ? compatibilidad->Resolver(top, tab, ventanaVista)
        : SuperficieExploradorV2{};
    if (!superficie.EsValida()) {
        vistaShell->Release();
        return nullptr;
    }

    HWND clave = tab ? tab : top;
    EstadoPanelV2* estado = nullptr;

    {
        std::lock_guard<std::mutex> lock(g_bloqueoEstados);
        auto it = g_estadosPorVentana.find(clave);
        if (it != g_estadosPorVentana.end() &&
            it->second && !it->second->destruyendo.load(std::memory_order_acquire)) {
            estado = it->second;
        }
    }

    if (estado && (estado->vistaShell != ventanaVista ||
                   estado->view != superficie.superficieVisual ||
                   estado->parent != superficie.contenedor)) {
        DestruirEstado(estado, true);
        estado = nullptr;
    }

    if (!estado) {
        estado = new EstadoPanelV2();
        estado->top = top;
        estado->tab = clave;
        estado->vistaShell = ventanaVista;
        estado->superficie = superficie;
        estado->view = superficie.superficieVisual;
        estado->parent = superficie.contenedor;
        estado->tema = ObtenerTemaActual();

        estado->viewOriginal = superficie.limites;
        GetClientRect(estado->parent, &estado->parentCliente);

        HINSTANCE hInst = GetModuleHandleW(nullptr);
        estado->panel = CreateWindowExW(
            0, CLASE_PANEL_V2, L"",
            WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
            0, 0, 10, 10, estado->parent, nullptr, hInst, estado);

        if (!estado->panel) {
            delete estado;
            vistaShell->Release();
            return nullptr;
        }

        SetWindowSubclass(estado->parent, ProcedimientoSubclasePadre, 1,
                          reinterpret_cast<DWORD_PTR>(estado));
        if (tab && tab != estado->parent) {
            SetWindowSubclass(tab, ProcedimientoSubclasePadre, 2, reinterpret_cast<DWORD_PTR>(estado));
        }

        std::lock_guard<std::mutex> lock(g_bloqueoEstados);
        g_estadosPorVentana[clave] = estado;
    }

    if (outVista) {
        *outVista = vistaShell;
    } else {
        vistaShell->Release();
    }

    return estado;
}

inline void AlternarSideView(HWND top) {
    if (!top || !IsWindow(top)) return;

    HRESULT hrCom = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    bool desinicializarCom = (hrCom == S_OK);

    IShellView* vista = nullptr;
    EstadoPanelV2* estado = AsegurarEstado(top, &vista);
    if (!estado) {
        if (vista) vista->Release();
        if (desinicializarCom) CoUninitialize();
        return;
    }

    if (estado->abierto) {
        CerrarEstado(estado);
    } else {
        estado->tema = ObtenerTemaActual();
        estado->abierto = true;

        // Extraer selección de archivos actual de Explorer
        if (vista) {
            std::wstring carpeta;
            auto archivos = ObtenerRutasSeleccionadasDeVista(vista, &carpeta);
            estado->archivosSeleccionados = archivos;
            estado->carpetaContenedora = carpeta;

            // Inyectar dinámicamente la vista correspondiente mediante la Fábrica
            estado->vistaContenido = FabricaVistas::CrearVistaParaSeleccion(archivos, carpeta);
            // La selección sólo habilita el botón. El comparador se crea
            // exclusivamente después de recibir su acción explícita.
            estado->comparadorArchivosActivo = false;
            if (estado->vistaContenido) estado->vistaContenido->AlActivar(estado->panel);
        } else {
            estado->vistaContenido = FabricaVistas::CrearVistaParaSeleccion({}, L"");
        }

        DisponerPanel(estado);
        SetTimer(estado->panel, 101, 500, nullptr); // Polling moderado para no cargar Explorer
    }

    if (vista) vista->Release();
    if (desinicializarCom) CoUninitialize();
}

inline LRESULT CALLBACK ProcedimientoVentanaPanel(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    EstadoPanelV2* estado = reinterpret_cast<EstadoPanelV2*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));

    if (estado && estado->vistaContenido) {
        try {
        if (msg == WM_SIDEVIEW_CLOSE_COMPARADOR_ARCHIVOS &&
            estado->comparadorArchivosActivo) {
            LiberarVistaContenidoV2(estado);
            estado->vistaContenido = FabricaVistas::CrearVistaParaSeleccion(
                estado->archivosSeleccionados, estado->carpetaContenedora);
            estado->comparadorArchivosActivo = false;
            if (estado->vistaContenido) estado->vistaContenido->AlActivar(hwnd);
            DisponerPanel(estado);
            InvalidateRect(hwnd, nullptr, FALSE);
            return 0;
        }

        if (msg == WM_SIDEVIEW_SETTINGS) {
            if (estado->comparadorArchivosActivo &&
                !PuedeCompararArchivosPorSeleccionV2(estado->archivosSeleccionados)) {
                LiberarVistaContenidoV2(estado);
                estado->vistaContenido = FabricaVistas::CrearVistaParaSeleccion(
                    estado->archivosSeleccionados, estado->carpetaContenedora);
                estado->comparadorArchivosActivo = false;
                if (estado->vistaContenido) estado->vistaContenido->AlActivar(hwnd);
                DisponerPanel(estado);
            }
            if (estado->vistaContenido) estado->vistaContenido->ActualizarConfiguracion();
            InvalidateRect(hwnd, nullptr, FALSE);
            return 0;
        }

        if (msg == WM_SIDEVIEW_REQUEST_COMPARADOR_ARCHIVOS &&
            !estado->comparadorArchivosActivo &&
            PuedeCompararArchivosPorSeleccionV2(estado->archivosSeleccionados)) {
            LiberarVistaContenidoV2(estado);
            estado->vistaContenido = FabricaVistas::CrearComparadorParaSeleccion(
                estado->archivosSeleccionados, estado->carpetaContenedora);
            estado->comparadorArchivosActivo = estado->vistaContenido != nullptr;
            if (estado->vistaContenido) estado->vistaContenido->AlActivar(hwnd);
            DisponerPanel(estado);
            InvalidateRect(hwnd, nullptr, FALSE);
            return 0;
        }
        if (estado->vistaContenido->ProcesarMensajeVentana(msg, wParam, lParam, hwnd)) {
            return 0;
        }
        if (msg >= WM_MOUSEFIRST && msg <= WM_MOUSELAST) {
            if (estado->vistaContenido->ProcesarMensajeRaton(msg, wParam, lParam, hwnd)) {
                return 0;
            }
        } else if (msg >= WM_KEYFIRST && msg <= WM_KEYLAST) {
            if (estado->vistaContenido->ProcesarMensajeTeclado(msg, wParam, lParam, hwnd)) {
                return 0;
            }
        }
        } catch (...) {
            Wh_Log(L"[UsefulSideView] Excepción contenida en el procedimiento del panel");
            return 0;
        }
    }

    switch (msg) {
        case WM_NCCREATE: {
            CREATESTRUCTW* cs = reinterpret_cast<CREATESTRUCTW*>(lParam);
            SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(cs->lpCreateParams));
            return TRUE;
        }

        case WM_ERASEBKGND:
            return 1; // Manejado completamente por el doble buffer

        case WM_TIMER: {
            IShellView* vistaPendiente = nullptr;
            try {
            if (wParam == 101 && estado && estado->abierto) {
                if (estado->superficie.gpuAcelerada) {
                    // El mod GPU puede rehacer su posición después de un
                    // cambio de pestaña o tamaño. Reubicar sólo nuestro panel
                    // mantiene la cooperación sin tocar su superficie.
                    DisponerPanel(estado);
                }
                HWND frente = GetForegroundWindow();
                if (frente && frente != estado->top && GetAncestor(frente, GA_ROOT) != estado->top) {
                    return 0;
                }
                // Actualizar selección periódicamente si el panel está visible
                AmbitoComV2 com;
                if (!com.Disponible()) return 0;
                HWND tab = nullptr;
                vistaPendiente = ObtenerShellViewActivo(estado->top, &tab);
                if (vistaPendiente) {
                    std::wstring carpeta;
                    auto seleccion = ObtenerRutasSeleccionadasDeVista(vistaPendiente, &carpeta);
                    if (seleccion != estado->archivosSeleccionados || carpeta != estado->carpetaContenedora) {
                        TipoVistaContenido tipoPrevio = ClasificarSeleccion(
                            estado->archivosSeleccionados, estado->carpetaContenedora);
                        TipoVistaContenido tipoNuevo = ClasificarSeleccion(seleccion, carpeta);

                        estado->archivosSeleccionados = seleccion;
                        estado->carpetaContenedora = carpeta;

                        const bool mantenerComparador =
                            estado->comparadorArchivosActivo &&
                            PuedeCompararArchivosPorSeleccionV2(seleccion);

                        // Si el comparador sigue recibiendo exactamente dos
                        // videos, sólo renueva sus fuentes; no desmonta la UX.
                        if (mantenerComparador) {
                            estado->vistaContenido->AlCambiarSeleccion(seleccion, carpeta);
                        } else if (estado->comparadorArchivosActivo ||
                                   tipoPrevio != tipoNuevo ||
                                   !estado->vistaContenido) {
                            LiberarVistaContenidoV2(estado);
                            estado->vistaContenido = FabricaVistas::CrearVistaParaSeleccion(seleccion, carpeta);
                            estado->comparadorArchivosActivo = false;
                            if (estado->vistaContenido) estado->vistaContenido->AlActivar(hwnd);
                            DisponerPanel(estado);
                        } else {
                            estado->vistaContenido->AlCambiarSeleccion(seleccion, carpeta);
                        }

                        InvalidateRect(hwnd, nullptr, FALSE);
                    }
                    vistaPendiente->Release();
                    vistaPendiente = nullptr;
                }
            }
            } catch (...) {
                if (vistaPendiente) vistaPendiente->Release();
                Wh_Log(L"[UsefulSideView] Excepción contenida durante WM_TIMER");
            }
            return 0;
        }

        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdcPantalla = BeginPaint(hwnd, &ps);
            RECT rectCliente;
            GetClientRect(hwnd, &rectCliente);
            int ancho = rectCliente.right - rectCliente.left;
            int alto = rectCliente.bottom - rectCliente.top;

            if (ancho > 0 && alto > 0 && estado) {
                if (AsegurarBufferMemoria(estado, hdcPantalla, ancho, alto)) {
                    if (estado->vistaContenido) {
                        try {
                            estado->vistaContenido->Renderizar(
                                estado->dcMemoria, rectCliente, estado->tema);
                        } catch (...) {
                            Wh_Log(L"[UsefulSideView] Excepción contenida durante WM_PAINT");
                            HBRUSH fondo = CreateSolidBrush(estado->tema.fondo);
                            FillRect(estado->dcMemoria, &rectCliente, fondo);
                            DeleteObject(fondo);
                            SetBkMode(estado->dcMemoria, TRANSPARENT);
                            SetTextColor(estado->dcMemoria, estado->tema.textoSecundario);
                            DrawTextW(estado->dcMemoria,
                                      L"Useful Side View no pudo dibujar esta vista.", -1,
                                      &rectCliente,
                                      DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
                        }
                    }
                    BitBlt(hdcPantalla, 0, 0, ancho, alto, estado->dcMemoria, 0, 0, SRCCOPY);
                }
            }

            EndPaint(hwnd, &ps);
            return 0;
        }

        case WM_DESTROY: {
            if (estado) {
                KillTimer(hwnd, 101);
                LiberarVistaContenidoV2(estado);
                LiberarBufferMemoria(estado);
            }
            return 0;
        }
    }

    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

inline LRESULT CALLBACK ProcedimientoSubclasePadre(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam,
                                                  UINT_PTR uIdSubclass, DWORD_PTR dwRefData) {
    EstadoPanelV2* estado = reinterpret_cast<EstadoPanelV2*>(dwRefData);

    if (msg == WM_SIZE) {
        LRESULT res = DefSubclassProc(hwnd, msg, wParam, lParam);
        if (estado && estado->abierto) {
            DisponerPanel(estado);
        }
        return res;
    }

    if (msg == WM_CLOSE && estado && hwnd == estado->tab && estado->abierto) {
        CerrarEstado(estado);
    }

    // En Explorer con pestañas el contenedor de la pestaña puede destruirse
    // antes que la ventana principal. En Explorer clásico, en cambio, sólo
    // recibimos el ciclo de vida del top-level. Cubrir ambos evita estados
    // colgantes que bloqueaban la segunda ventana de Explorer.
    if (msg == WM_DESTROY && estado && (hwnd == estado->tab || hwnd == estado->top)) {
        DestruirEstado(estado, true);
        return DefSubclassProc(hwnd, msg, wParam, lParam);
    }

    if (msg == WM_NCDESTROY) {
        RemoveWindowSubclass(hwnd, ProcedimientoSubclasePadre, uIdSubclass);
    }

    return DefSubclassProc(hwnd, msg, wParam, lParam);
}

inline LRESULT CALLBACK ProcedimientoSubclaseTop(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam,
                                                DWORD_PTR) {
    if (msg == WM_SIDEVIEW_TOGGLE) {
        try {
            AlternarSideView(hwnd);
        } catch (...) {
            Wh_Log(L"[UsefulSideView] Excepción contenida al alternar el panel");
        }
        return 0;
    }

    if (msg == WM_SETTINGCHANGE || msg == WM_THEMECHANGED || msg == WM_SYSCOLORCHANGE) {
        std::lock_guard<std::mutex> lock(g_bloqueoEstados);
        for (auto& par : g_estadosPorVentana) {
            if (par.second && par.second->top == hwnd) {
                par.second->tema = ObtenerTemaActual();
                if (par.second->panel && IsWindow(par.second->panel)) {
                    InvalidateRect(par.second->panel, nullptr, FALSE);
                }
            }
        }
    }

    if (msg == WM_NCDESTROY) {
        WindhawkUtils::RemoveWindowSubclassFromAnyThread(hwnd, ProcedimientoSubclaseTop);
    }

    return DefSubclassProc(hwnd, msg, wParam, lParam);
}

inline bool RegistrarClasePanelV2(HINSTANCE hInstance) {
    WNDCLASSEXW wc{};
    wc.cbSize = sizeof(wc);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = ProcedimientoVentanaPanel;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wc.lpszClassName = CLASE_PANEL_V2;

    if (!RegisterClassExW(&wc) && GetLastError() != ERROR_CLASS_ALREADY_EXISTS) {
        return false;
    }
    return true;
}

} // namespace SideViewV2
/**** ended inlining v2/nucleo/gestor_ventanas.hpp ****/
/**** start inlining v2/nucleo/hook_teclado.hpp ****/

#include <windows.h>
#include <algorithm>
#include <cwctype>
#include <vector>
#include <atomic>
#include <string>
/**** skipping file: v2/nucleo/gestor_ventanas.hpp ****/

namespace SideViewV2 {

inline HANDLE g_hiloTeclado = nullptr;
inline DWORD g_idHiloTeclado = 0;
inline HHOOK g_hookTeclado = nullptr;
inline std::atomic_bool g_descargandoMod{false};

// Configuración de atajo dinámica
inline std::atomic<DWORD> g_teclaAtajo{'K'};
inline std::atomic_bool g_requiereCtrl{true};
inline std::atomic_bool g_requiereAlt{false};
inline std::atomic_bool g_requiereShift{false};

inline void ConfigurarAtajoDesdeString(const wchar_t* strAtajo) {
    if (!strAtajo || *strAtajo == L'\0') {
        g_teclaAtajo.store('K');
        g_requiereCtrl.store(true);
        g_requiereAlt.store(false);
        g_requiereShift.store(false);
        return;
    }

    std::wstring s = strAtajo;
    std::transform(s.begin(), s.end(), s.begin(), [](wchar_t caracter) {
        return static_cast<wchar_t>(std::towupper(caracter));
    });

    bool c = (s.find(L"CTRL") != std::wstring::npos);
    bool a = (s.find(L"ALT") != std::wstring::npos);
    bool sh = (s.find(L"SHIFT") != std::wstring::npos);

    // Buscar la letra o tecla principal
    DWORD vk = 'K';
    for (wchar_t ch : s) {
        if (ch >= L'A' && ch <= L'Z') {
            // Verificar que no sea parte de CTRL, ALT o SHIFT
            if (s.find(L"CTRL") != std::wstring::npos && (ch == 'C' || ch == 'T' || ch == 'R' || ch == 'L')) continue;
            if (s.find(L"ALT") != std::wstring::npos && (ch == 'A' || ch == 'L' || ch == 'T')) continue;
            if (s.find(L"SHIFT") != std::wstring::npos && (ch == 'S' || ch == 'H' || ch == 'I' || ch == 'F' || ch == 'T')) continue;
            vk = static_cast<DWORD>(ch);
            break;
        } else if (ch >= L'0' && ch <= L'9') {
            vk = static_cast<DWORD>(ch);
            break;
        }
    }

    if (s.find(L"SPACE") != std::wstring::npos) vk = VK_SPACE;
    for (int numero = 12; numero >= 1; --numero) {
        if (s.find(L"F" + std::to_wstring(numero)) != std::wstring::npos) {
            vk = static_cast<DWORD>(VK_F1 + numero - 1);
            break;
        }
    }

    g_teclaAtajo.store(vk);
    g_requiereCtrl.store(c);
    g_requiereAlt.store(a);
    g_requiereShift.store(sh);
}

inline bool EsClaseExplorer(HWND hwnd) {
    if (!hwnd) return false;
    wchar_t nombreClase[64]{};
    GetClassNameW(hwnd, nombreClase, ARRAYSIZE(nombreClase));
    return (_wcsicmp(nombreClase, L"CabinetWClass") == 0 ||
            _wcsicmp(nombreClase, L"ExploreWClass") == 0);
}

inline bool EsFocoEnVistaArchivos(HWND top) {
    if (!top) return false;
    DWORD idHilo = GetWindowThreadProcessId(top, nullptr);
    GUITHREADINFO info{};
    info.cbSize = sizeof(info);
    if (!GetGUIThreadInfo(idHilo, &info)) return false;

    HWND foco = info.hwndFocus ? info.hwndFocus : top;
    wchar_t nombreClase[64]{};
    GetClassNameW(foco, nombreClase, ARRAYSIZE(nombreClase));

    // Si el usuario está escribiendo texto en un cuadro de edición o árbol, no interceptamos
    if (_wcsicmp(nombreClase, L"Edit") == 0 ||
        _wcsicmp(nombreClase, L"RichEdit") == 0 ||
        _wcsicmp(nombreClase, L"RICHEDIT50W") == 0 ||
        _wcsicmp(nombreClase, L"ComboBox") == 0 ||
        _wcsicmp(nombreClase, L"ComboBoxEx32") == 0 ||
        _wcsicmp(nombreClase, L"SysTreeView32") == 0) {
        return false;
    }

    if (_wcsicmp(nombreClase, L"DirectUIHWND") == 0 ||
        _wcsicmp(nombreClase, L"DUIViewWndClassName") == 0 ||
        _wcsicmp(nombreClase, L"SysListView32") == 0 ||
        _wcsicmp(nombreClase, L"SHELLDLL_DefView") == 0 ||
        _wcsicmp(nombreClase, L"ShellTabWindowClass") == 0 ||
        _wcsicmp(nombreClase, L"CtrlNotifySink") == 0 ||
        _wcsicmp(nombreClase, CLASE_PANEL_V2) == 0 ||
        foco == top) {
        return true;
    }

    return false;
}

inline LRESULT CALLBACK ProcedimientoHookTeclado(int code, WPARAM wParam, LPARAM lParam) {
    if (code < 0 || g_descargandoMod.load()) {
        return CallNextHookEx(nullptr, code, wParam, lParam);
    }

    auto* kbd = reinterpret_cast<KBDLLHOOKSTRUCT*>(lParam);

    if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) {
        DWORD targetKey = g_teclaAtajo.load();
        if (kbd->vkCode == targetKey) {
            bool ctrlPresionado = (GetAsyncKeyState(VK_CONTROL) & 0x8000) != 0;
            bool altPresionado = (GetAsyncKeyState(VK_MENU) & 0x8000) != 0;
            bool shiftPresionado = (GetAsyncKeyState(VK_SHIFT) & 0x8000) != 0;

            if (ctrlPresionado == g_requiereCtrl.load() &&
                altPresionado == g_requiereAlt.load() &&
                shiftPresionado == g_requiereShift.load()) {

                HWND ventanaFrente = GetForegroundWindow();
                if (EsClaseExplorer(ventanaFrente)) {
                    DWORD idProcesoFrente = 0;
                    GetWindowThreadProcessId(ventanaFrente, &idProcesoFrente);
                    if (idProcesoFrente == GetCurrentProcessId()) {
                        if (EsFocoEnVistaArchivos(ventanaFrente)) {
                            PostMessageW(ventanaFrente, WM_SIDEVIEW_TOGGLE, 0, 0);
                            return 1; // Consumir atajo
                        }
                    }
                }
            }
        }
    }

    return CallNextHookEx(nullptr, code, wParam, lParam);
}

inline DWORD WINAPI HiloHookProc(LPVOID) {
    HINSTANCE hInst = GetModuleHandleW(nullptr);
    g_hookTeclado = SetWindowsHookExW(WH_KEYBOARD_LL, ProcedimientoHookTeclado, hInst, 0);

    MSG msg;
    while (GetMessageW(&msg, nullptr, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    if (g_hookTeclado) {
        UnhookWindowsHookEx(g_hookTeclado);
        g_hookTeclado = nullptr;
    }
    return 0;
}

inline bool IniciarHookTeclado() {
    DWORD idHilo = 0;
    g_hiloTeclado = CreateThread(nullptr, 0, HiloHookProc, nullptr, 0, &idHilo);
    if (!g_hiloTeclado) return false;
    g_idHiloTeclado = idHilo;
    return true;
}

inline void DetenerHookTeclado() {
    if (!g_hiloTeclado) return;
    while (WaitForSingleObject(g_hiloTeclado, 50) == WAIT_TIMEOUT) {
        PostThreadMessageW(g_idHiloTeclado, WM_QUIT, 0, 0);
    }
    CloseHandle(g_hiloTeclado);
    g_hiloTeclado = nullptr;
    g_idHiloTeclado = 0;
}

} // namespace SideViewV2
/**** ended inlining v2/nucleo/hook_teclado.hpp ****/
/**** start inlining v2/nucleo/mod_core.hpp ****/

#include <windows.h>
#include <algorithm>
#include <vector>
#include <windhawk_api.h>
#include <windhawk_utils.h>
/**** skipping file: v2/nucleo/gestor_ventanas.hpp ****/
/**** skipping file: v2/nucleo/hook_teclado.hpp ****/
/**** skipping file: v2/nucleo/configuracion_v2.hpp ****/

namespace SideViewV2 {

using CreateWindowExW_t = decltype(&CreateWindowExW);
inline CreateWindowExW_t CreateWindowExW_Original = nullptr;
inline thread_local bool g_dentroHookCreateWindowV2{false};

inline HWND WINAPI CreateWindowExW_Hook(DWORD dwExStyle, LPCWSTR lpClassName,
                                        LPCWSTR lpWindowName, DWORD dwStyle,
                                        int X, int Y, int nWidth, int nHeight,
                                        HWND hWndParent, HMENU hMenu,
                                        HINSTANCE hInstance, LPVOID lpParam) {
    HWND hwnd = CreateWindowExW_Original(dwExStyle, lpClassName, lpWindowName,
                                         dwStyle, X, Y, nWidth, nHeight,
                                         hWndParent, hMenu, hInstance, lpParam);
    if (!hwnd || g_descargandoMod.load()) {
        return hwnd;
    }

    // La instalación de una subclase puede provocar creación de ventanas
    // auxiliares. Evitar reentrada aquí impide encadenar el hook sobre sí
    // mismo durante la construcción de Explorer.
    if (g_dentroHookCreateWindowV2) return hwnd;
    g_dentroHookCreateWindowV2 = true;

    if (!IS_INTRESOURCE(lpClassName) && (_wcsicmp(lpClassName, L"CabinetWClass") == 0 ||
                                         _wcsicmp(lpClassName, L"ExploreWClass") == 0)) {
        WindhawkUtils::SetWindowSubclassFromAnyThread(hwnd, ProcedimientoSubclaseTop, 0);
    }

    g_dentroHookCreateWindowV2 = false;

    return hwnd;
}

inline BOOL CALLBACK EnumVentanasExplorerCallback(HWND hwnd, LPARAM lParam) {
    DWORD idProceso = 0;
    GetWindowThreadProcessId(hwnd, &idProceso);
    if (idProceso != GetCurrentProcessId()) return TRUE;

    if (EsClaseExplorer(hwnd)) {
        reinterpret_cast<std::vector<HWND>*>(lParam)->push_back(hwnd);
    }
    return TRUE;
}

inline std::vector<HWND> ObtenerVentanasExplorerActuales() {
    std::vector<HWND> ventanas;
    EnumWindows(EnumVentanasExplorerCallback, reinterpret_cast<LPARAM>(&ventanas));
    return ventanas;
}

inline void CargarConfiguracionesV2() {
    PCWSTR atajoStr = Wh_GetStringSetting(L"atajoTeclado");
    if (atajoStr && *atajoStr != L'\0') {
        ConfigurarAtajoDesdeString(atajoStr);
    } else {
        ConfigurarAtajoDesdeString(L"Ctrl+K");
    }
    if (atajoStr) {
        Wh_FreeStringSetting(atajoStr);
    }

    int limite = Wh_GetIntSetting(L"limiteCarpetasPesadas");
    g_limiteCarpetasPesadasConfig.store(std::max(1, std::min(limite, 100)));
    g_mostrarNumerosLineaV2.store(Wh_GetIntSetting(L"mostrarNumerosLinea") != 0);
    g_tamanoFuenteTextoV2.store(
        NormalizarTamanoFuenteV2(Wh_GetIntSetting(L"tamanoFuenteTexto")),
        std::memory_order_release);
    g_tamanoFuenteCodigoV2.store(
        NormalizarTamanoFuenteV2(Wh_GetIntSetting(L"tamanoFuenteCodigo")),
        std::memory_order_release);
    g_ajusteLineaTextoCodigoV2.store(
        Wh_GetIntSetting(L"ajusteLineaTextoCodigo") != 0,
        std::memory_order_release);
    g_reproductorVideoEnVistaV2.store(
        Wh_GetIntSetting(L"reproductorVideoEnVista") != 0,
        std::memory_order_release);
    g_comparadorVideosExperimentalV2.store(
        Wh_GetIntSetting(L"comparadorVideosExperimental") != 0,
        std::memory_order_release);
    g_comparadorVideosPantallaCompletaV2.store(
        Wh_GetIntSetting(L"comparadorVideosPantallaCompleta") != 0,
        std::memory_order_release);
    g_mostrarVersionModV2.store(
        Wh_GetIntSetting(L"mostrarVersionMod") != 0,
        std::memory_order_release);
    g_modoSemanticoDiffViewV2.store(
        Wh_GetIntSetting(L"modoSemanticoDiffView") != 0,
        std::memory_order_release);
    g_sincronizacionNotasWinKlokV2.store(
        Wh_GetIntSetting(L"sincronizacionNotasWinKlok") != 0,
        std::memory_order_release);
}

inline bool InicializarModV2() {
    g_descargandoMod.store(false, std::memory_order_release);
    WM_SIDEVIEW_TOGGLE = RegisterWindowMessageW(L"UsefulSideView_WM_Toggle_V2");
    WM_SIDEVIEW_SETTINGS = RegisterWindowMessageW(L"UsefulSideView_WM_Settings_V2");

    if (!WM_SIDEVIEW_TOGGLE || !WM_SIDEVIEW_SETTINGS) {
        return false;
    }

    CargarConfiguracionesV2();

    HINSTANCE hInst = GetModuleHandleW(nullptr);
    if (!RegistrarClasePanelV2(hInst)) {
        return false;
    }

    if (!WindhawkUtils::SetFunctionHook(CreateWindowExW, CreateWindowExW_Hook,
                                        &CreateWindowExW_Original)) {
        UnregisterClassW(CLASE_PANEL_V2, hInst);
        return false;
    }

    return true;
}

inline void DespuesDeIniciarModV2() {
    // Subclasear las ventanas existentes de Explorer
    auto ventanas = ObtenerVentanasExplorerActuales();
    for (HWND w : ventanas) {
        WindhawkUtils::SetWindowSubclassFromAnyThread(w, ProcedimientoSubclaseTop, 0);
    }

    // Iniciar el hook de teclado en su propio hilo
    IniciarHookTeclado();
}

inline void AlCambiarConfiguracionesV2() {
    CargarConfiguracionesV2();
    std::lock_guard<std::mutex> lock(g_bloqueoEstados);
    for (auto& par : g_estadosPorVentana) {
        if (par.second && par.second->panel && IsWindow(par.second->panel)) {
            PostMessageW(par.second->panel, WM_SIDEVIEW_SETTINGS, 0, 0);
        }
    }
}

inline void DesinicializarModV2() {
    g_descargandoMod = true;
    DetenerHookTeclado();

    // Extraer el registro antes de destruir ventanas. DestroyWindow puede
    // provocar reentrada y nunca debe ejecutarse sosteniendo este mutex.
    std::vector<EstadoPanelV2*> estados;
    {
        std::lock_guard<std::mutex> lock(g_bloqueoEstados);
        estados.reserve(g_estadosPorVentana.size());
        for (auto& par : g_estadosPorVentana) if (par.second) estados.push_back(par.second);
        g_estadosPorVentana.clear();
    }
    for (EstadoPanelV2* estado : estados) DestruirEstado(estado, false);

    HINSTANCE hInst = GetModuleHandleW(nullptr);
    UnregisterClassW(CLASE_PANEL_V2, hInst);
}

} // namespace SideViewV2
/**** ended inlining v2/nucleo/mod_core.hpp ****/

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

void Wh_ModSettingsChanged() {
    Wh_Log(L"[UsefulSideView-v2] Configuración cambiada");
    SideViewV2::AlCambiarConfiguracionesV2();
}

void Wh_ModUninit() {
    Wh_Log(L"[UsefulSideView-v2] Descargando mod de forma segura");
    SideViewV2::DesinicializarModV2();
}
/**** ended inlining useful-sideview-v2.cpp ****/
