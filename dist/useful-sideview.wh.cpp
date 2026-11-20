/**** start inlining useful-sideview-v2.cpp ****/
// ==WindhawkMod==
// @id              useful-sideview
// @name            Useful Side View
// @description     Transforma el explorador de Windows 11 con un panel lateral inyectado de alta fidelidad, fluido y cooperativo. Alterna con el atajo configurable (por defecto Ctrl + K).
// @version         4.0.2
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
*/
// ==/WindhawkModSettings==

// ==WindhawkModReadme==
/*
# Useful Side View V2 (4.0.2)

Prototipo modular de nueva generación con arquitectura canónica de Windows 11:
- Inyección cooperativa con `CreateWindowExW` y subclaseo de `CabinetWClass` / `ShellTabWindowClass`.
- Renderizado de doble buffer GDI sin parpadeo (Anti-Flicker BitBlt).
- **Atajo interactivo configurable**: Por defecto **[ Ctrl + K ]**.
- **Pantalla Tutorial (Walkthrough)**: Se activa automáticamente al presionar el atajo sin archivos seleccionados.
- **Suite Funcional de Comprimidos & Cómics**: Portadas, árbol de archivos, navegación anterior/siguiente, "Abrir con" y selector de conversión.
- **Suite Funcional de Inspección de Carpetas**: Árbol jerárquico recursivo, cálculo de peso, Top 4 subcarpetas pesadas, enlaces simbólicos, copia de estructura y limpieza de carpetas vacías.
- **Vistas Especializadas Modulares**: Texto, Markdown, Imágenes, Video y Código con servicios inyectados.
- **Comparadores polimórficos**: Comparador general de nombres, peso y SHA-256; comparadores especializados de texto/documentos y video mediante servicios inyectables.
- **Comparador de texto**: Diff clásico de dos columnas, estilo GitHub, con navegación y metadatos comunes.
- **Comparador de videos experimental**: Dos reproductores Media Foundation 50/50, sincronización estricta, pantalla completa con PiP, swap de roles e información de archivos, activable desde Ajustes y únicamente para dos videos.
- **Persistencia nativa**: Historial de programas de "Abrir con" por categoría, migrado y validado con valores persistentes de Windhawk.
- **Versión visible**: La vista general puede mostrar la versión actual desde el ajuste correspondiente.
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
    HWND view{nullptr};          // SHELLDLL_DefView
    HWND parent{nullptr};        // Contenedor padre de la vista
    HWND panel{nullptr};         // Ventana del panel inyectado

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
        HANDLE archivo = CreateFileW(ruta.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (archivo == INVALID_HANDLE_VALUE) return documento;
        LARGE_INTEGER tam{}; if (!GetFileSizeEx(archivo, &tam) || tam.QuadPart > 16 * 1024 * 1024) { CloseHandle(archivo); return documento; }
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
        UINT ancho=0, alto=0; converter->GetSize(&ancho,&alto);
        if (ancho == 0 || alto == 0 || ancho > 8192 || alto > 8192 || static_cast<ULONGLONG>(ancho) * alto > 16ULL * 1024 * 1024) { converter->Release(); frame->Release(); decoder->Release(); fabrica->Release(); return nullptr; }
        BITMAPINFO info{}; info.bmiHeader.biSize=sizeof(BITMAPINFOHEADER); info.bmiHeader.biWidth=static_cast<LONG>(ancho); info.bmiHeader.biHeight=-static_cast<LONG>(alto); info.bmiHeader.biPlanes=1; info.bmiHeader.biBitCount=24; info.bmiHeader.biCompression=BI_RGB;
        void* bits=nullptr; HDC dc=GetDC(nullptr); resultado=CreateDIBSection(dc,&info,DIB_RGB_COLORS,&bits,nullptr,0); ReleaseDC(nullptr,dc);
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
                ? (static_cast<double>(carpeta.tamanoBytes) / totalCompleto) * 100.0
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
                                     nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
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
    virtual bool EstaReproduciendo() const = 0;
    virtual double Posicion() const = 0;
    virtual double Duracion() const = 0;
    virtual const std::wstring& Mensaje() const = 0;
    virtual void Liberar() = 0;
};

class MediaEngineNotifyV2 final : public IMFMediaEngineNotify {
public:
    explicit MediaEngineNotifyV2(HWND destino) : m_destino(destino) {}

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
                                           DWORD) override {
        if (evento == MF_MEDIA_ENGINE_EVENT_NOTIFYSTABLESTATE) {
            if (param1) SetEvent(reinterpret_cast<HANDLE>(param1));
            return S_OK;
        }
        HWND destino = m_destino.load(std::memory_order_acquire);
        if (destino) {
            PostMessageW(destino, WM_SIDEVIEW_VIDEO_EVENT_V2,
                         static_cast<WPARAM>(evento), static_cast<LPARAM>(param1));
        }
        return S_OK;
    }

private:
    ~MediaEngineNotifyV2() = default;
    LONG m_referencias{1};
    std::atomic<HWND> m_destino{nullptr};
};

class ReproductorMediaFoundationV2 final : public IReproductorVideoV2 {
public:
    ~ReproductorMediaFoundationV2() override { Liberar(); }

    bool Montar(HWND panel, int ranura, const std::wstring& ruta) override {
        Liberar();
        m_panel = panel;
        m_ranura = ranura;
        m_ruta = ruta;
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
            return false;
        }

        DWORD atributos = GetFileAttributesW(ruta.c_str());
        if (atributos == INVALID_FILE_ATTRIBUTES ||
            (atributos & FILE_ATTRIBUTE_DIRECTORY) != 0) {
            m_mensaje = L"No se pudo abrir el archivo de video.";
            return false;
        }

        HRESULT hr = MFStartup(MF_VERSION);
        if (FAILED(hr)) {
            m_mensaje = L"Media Foundation no está disponible en este sistema.";
            return false;
        }
        m_mfIniciado = true;

        MediaEngineNotifyV2* callback = new (std::nothrow) MediaEngineNotifyV2(m_ventana);
        if (!callback) {
            m_mensaje = L"No hubo memoria suficiente para iniciar el reproductor.";
            LiberarMediaFoundation();
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
            LiberarBackend();
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
        SetWindowPos(m_ventana, HWND_TOP, render.left, render.top,
                     render.right - render.left, render.bottom - render.top,
                     SWP_NOACTIVATE | SWP_SHOWWINDOW);
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
    bool EstaReproduciendo() const override { return m_reproduciendo; }

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

    const std::wstring& Mensaje() const override { return m_mensaje; }

    void Liberar() override {
        LiberarBackend();
        if (IsWindow(m_ventana)) DestroyWindow(m_ventana);
        m_ventana = nullptr;
        m_panel = nullptr;
        m_listo = false;
        m_reproduciendo = false;
        m_reproduccionSolicitada = false;
        m_posicionPendiente = 0.0;
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
                    reproductor->ActualizarFrame();
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
                reproductor->ProcesarEvento(static_cast<DWORD>(wParam),
                                             static_cast<HRESULT>(lParam));
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
                m_mensaje.clear();
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
    std::wstring m_ruta;
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
    bool reproduciendo{false};
    bool simulado{true};
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
        estado.simulado = seleccion.size() != 2;
        estado.mensaje = seleccion.size() == 2
            ? L"Sincronización estricta activa."
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

    std::wstring FormatearTiempo(double segundos) const {
        const int total = std::max(0, static_cast<int>(std::lround(segundos)));
        wchar_t buffer[32]{};
        swprintf_s(buffer, L"%02d:%02d", (total % 3600) / 60, total % 60);
        return buffer;
    }

private:
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

// Contrato general de diff. Cada tipo de archivo puede aportar otra estrategia
// sin modificar la vista ni el comparador padre.
class ServicioDiffViewV2 : public ServicioComparadorArchivosV2 {
public:
    ~ServicioDiffViewV2() override = default;
    virtual EstadoDiffV2 CrearDiff(const std::vector<std::wstring>& seleccion) const = 0;
};

class ServicioComparadorTextoV2 final : public ServicioDiffViewV2 {
public:
    explicit ServicioComparadorTextoV2(std::shared_ptr<ServicioTextoV2> texto)
        : m_texto(std::move(texto)) {}

    EstadoDiffV2 CrearDiff(const std::vector<std::wstring>& seleccion) const override {
        EstadoDiffV2 resultado;
        if (seleccion.size() != 2 || !m_texto) {
            resultado.mensaje = L"Selecciona exactamente dos archivos de texto.";
            return resultado;
        }
        DocumentoTextoV2 izquierda = m_texto->Cargar(seleccion[0]);
        DocumentoTextoV2 derecha = m_texto->Cargar(seleccion[1]);
        resultado = CompararLineas(izquierda.lineas, derecha.lineas);
        resultado.mensaje = L"Diff de texto listo.";
        return resultado;
    }

private:
    static EstadoDiffV2 CompararLineas(const std::vector<std::wstring>& izquierda,
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

    std::shared_ptr<ServicioTextoV2> m_texto;
};

class ServicioComparadorDocumentosV2 final : public ServicioComparadorArchivosV2 {
public:
    EstadoComparadorArchivosV2 CrearComparacion(
        const std::vector<std::wstring>& seleccion) const override {
        EstadoComparadorArchivosV2 estado =
            ServicioComparadorArchivosV2::CrearComparacion(seleccion);
        estado.mensaje = L"Comparación documental preparada; el contenido se mostrará por tipo de documento.";
        return estado;
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
inline constexpr wchar_t VERSION_MOD_V2[] = L"4.0.2";

class ServicioVersionModV2 final {
public:
    std::wstring TextoVersion() const {
        return L"Useful Side View v" + std::wstring(VERSION_MOD_V2);
    }
};

} // namespace SideViewV2
/**** ended inlining v2/nucleo/servicio_version_mod.hpp ****/
#include <memory>
namespace SideViewV2 {
struct DependenciasVistaV2 {
    std::shared_ptr<ServicioNavegacionV2> navegacion{std::make_shared<ServicioNavegacionV2>()};
    std::shared_ptr<ServicioExtensionV2> extensiones{std::make_shared<ServicioExtensionV2>()};
    std::shared_ptr<ServicioPersistenciaWindhawkV2> persistencia{std::make_shared<ServicioPersistenciaWindhawkV2>()};
    std::shared_ptr<ServicioVersionModV2> versionMod{std::make_shared<ServicioVersionModV2>()};
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
                if (m_archivos.size() == 2) {
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
                     m_archivos.size() == 2);

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
            m_formatoSeleccionado = m_formatosDisponibles[cmd - 1];
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
/**** start inlining v2/nucleo/configuracion_v2.hpp ****/

#include <atomic>

namespace SideViewV2 {

// Interruptor de seguridad para la funcionalidad experimental. Mantenerlo
// aislado permite desactivar el flujo completo sin acoplarlo a las vistas.
inline std::atomic_bool g_comparadorVideosExperimentalV2{true};
inline std::atomic_bool g_comparadorVideosPantallaCompletaV2{true};
inline std::atomic_bool g_mostrarVersionModV2{true};

inline bool ComparadorVideosExperimentalDisponibleV2() {
    return g_comparadorVideosExperimentalV2.load(std::memory_order_acquire);
}

inline bool ComparadorVideosPantallaCompletaDisponibleV2() {
    return g_comparadorVideosPantallaCompletaV2.load(std::memory_order_acquire);
}

inline bool MostrarVersionModDisponibleV2() {
    return g_mostrarVersionModV2.load(std::memory_order_acquire);
}

} // namespace SideViewV2
/**** ended inlining v2/nucleo/configuracion_v2.hpp ****/
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
                           std::shared_ptr<ServicioVersionModV2> versionMod)
        : m_servicio(std::move(servicio)), m_navegacion(std::move(navegacion)),
          m_fusion(std::move(fusion)), m_inspeccion(std::move(inspeccion)),
          m_versionMod(std::move(versionMod)) {
        m_programasRecordados = m_servicio ? m_servicio->Programas()
                                           : std::vector<std::wstring>{};
        m_limiteCarpetasPesadas = std::max(1, std::min(g_limiteCarpetasPesadasConfig.load(), 100));
    }

    void AlCambiarSeleccion(const std::vector<std::wstring>& archivos, const std::wstring&) override {
        m_carpetasSeleccionadas = archivos;
        m_indiceCarpeta = 0;
        m_desplazamientoScrollY = 0;
        m_scrollPesadasY = 0;
        m_nivelExpansionMax = 1; // 1 = Raíz, 2 = Nivel 1, 3 = Nivel 2, 4 = Todo
        m_limiteCarpetasPesadas = std::max(1, std::min(g_limiteCarpetasPesadasConfig.load(), 100));

        CargarCarpetaActual();
    }

    void ActualizarConfiguracion() override {
        int limiteAnterior = m_limiteCarpetasPesadas;
        m_limiteCarpetasPesadas = std::max(1, std::min(g_limiteCarpetasPesadasConfig.load(), 100));
        if (limiteAnterior != m_limiteCarpetasPesadas) CargarCarpetaActual();
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
                    DrawTextW(hdc, m_topCarpetasPesadas[i].nombre.c_str(), -1, &rcNomP,
                              DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);

                    SetTextColor(hdc, tema.acento);
                    RECT rcTamP{rcPesada.right - 85, rcPesada.top + 2, rcPesada.right - 10, rcPesada.top + 18};
                    std::wstring strPesadaTam = FormatearTamano(m_topCarpetasPesadas[i].tamanoBytes);
                    DrawTextW(hdc, strPesadaTam.c_str(), -1, &rcTamP,
                              DT_RIGHT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

                    // Barra de progreso de peso
                    RECT rcBarraFondo{rcPesada.left + 10, rcPesada.top + 22, rcPesada.right - 10, rcPesada.top + 26};
                    HBRUSH brBkgBar = CreateSolidBrush(tema.fondoCabecera);
                    FillRect(hdc, &rcBarraFondo, brBkgBar);
                    DeleteObject(brBkgBar);

                    int anchoLleno = static_cast<int>((rcBarraFondo.right - rcBarraFondo.left) * (m_topCarpetasPesadas[i].porcentaje / 100.0));
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
            m_servicio->Seleccionar(m_programasRecordados[cmd - 1]);
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
            std::wstring sangria(item.nivelJerarquia * 2, L' ');
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
};

} // namespace SideViewV2
/**** ended inlining v2/vistas/vista_carpetas.hpp ****/
/**** start inlining v2/vistas/vista_en_desarrollo.hpp ****/

/**** skipping file: v2/ui/panel_interfaz.hpp ****/
/**** skipping file: v2/ui/ventana_base.hpp ****/
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
        if (m_archivos.size() == 2) {
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
        HBRUSH fondoComparar = CreateSolidBrush(m_archivos.size() == 2
            ? tema.botonFondo : tema.fondoCabecera);
        FillRect(hdc, &m_rcComparar, fondoComparar);
        DeleteObject(fondoComparar);
        SetTextColor(hdc, m_archivos.size() == 2
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
            if (m_archivos.size() == 2) {
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
                     m_archivos.size() == 2);

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
        UINT cmd = TrackPopupMenu(menu, TPM_LEFTALIGN | TPM_TOPALIGN | TPM_RETURNCMD, p.x, p.y, 0, hwnd, nullptr);
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
            if (m_archivos.size() == 2) {
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
                                m_archivos.size() == 2);
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
    void Liberar() override { if (m_preview) DeleteObject(m_preview); m_preview = nullptr; }
    void Renderizar(HDC hdc, const RECT& rect, const TemaVisual& tema) override { HBRUSH b=CreateSolidBrush(tema.fondo); FillRect(hdc,&rect,b); DeleteObject(b); std::wstring n=m_archivos.empty()?L"Sin imágenes":PathFindFileNameW(m_archivos[m_indice].c_str()); DibujarCabecera(hdc,rect,tema,L"🖼 Imágenes",n); DibujarPreview(hdc,rect,tema,m_preview); }
private: void CargarActual() override { ReemplazarPreview(m_archivos.empty() ? nullptr : m_previsualizacion->CargarImagen(m_archivos[m_indice])); }
};

class VistaVideos final : public VistaMediaV2 {
public:
    VistaVideos(std::shared_ptr<ServicioNavegacionV2> n, std::shared_ptr<ServicioAbrirConV2> a, std::shared_ptr<ServicioPrevisualizacionMediaV2> p) : VistaMediaV2(std::move(n), std::move(a), std::move(p)) {}
    ~VistaVideos() override { Liberar(); }
    void AlCambiarSeleccion(const std::vector<std::wstring>& archivos, const std::wstring&) override { Liberar(); m_archivos = archivos; m_indice = 0; CargarActual(); }
    bool ProcesarMensajeRaton(UINT m, WPARAM w, LPARAM l, HWND h) override { return ProcesarNavegacion(m, w, l, h); }
    bool ProcesarMensajeTeclado(UINT m, WPARAM w, LPARAM l, HWND h) override { return ProcesarNavegacion(m, w, l, h); }
    void Liberar() override { if (m_preview) DeleteObject(m_preview); m_preview = nullptr; }
    void Renderizar(HDC hdc, const RECT& rect, const TemaVisual& tema) override { HBRUSH b=CreateSolidBrush(tema.fondo); FillRect(hdc,&rect,b); DeleteObject(b); std::wstring n=m_archivos.empty()?L"Sin videos":PathFindFileNameW(m_archivos[m_indice].c_str()); DibujarCabecera(hdc,rect,tema,L"🎬 Videos · Vista previa",n); DibujarPreview(hdc,rect,tema,m_preview); }
private: void CargarActual() override { ReemplazarPreview(m_archivos.empty() ? nullptr : m_previsualizacion->CargarMiniatura(m_archivos[m_indice])); }
protected: bool MostrarComparador() const override { return false; }
};
}
/**** ended inlining v2/vistas/vista_medios.hpp ****/
/**** start inlining v2/vistas/vista_texto_codigo.hpp ****/
/**** skipping file: v2/ui/panel_interfaz.hpp ****/
/**** skipping file: v2/nucleo/servicio_texto.hpp ****/
/**** skipping file: v2/nucleo/servicio_navegacion.hpp ****/
#include <windowsx.h>
#include <algorithm>
#include <memory>

namespace SideViewV2 {
class VistaTextoCodigo : public IPanelContenido {
public:
    VistaTextoCodigo(std::shared_ptr<ServicioTextoV2> texto, std::shared_ptr<ServicioNavegacionV2> nav, bool codigo)
        : m_texto(std::move(texto)), m_nav(std::move(nav)), m_codigo(codigo) {}
    void AlCambiarSeleccion(const std::vector<std::wstring>& archivos, const std::wstring&) override {
        m_archivos = archivos; m_indiceArchivo = 0; Cargar();
    }
    bool ProcesarMensajeRaton(UINT msg, WPARAM, LPARAM lParam, HWND hwnd) override {
        if (msg != WM_LBUTTONDOWN) return false; POINT p{GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
        if (PtInRect(&m_rcAnterior,p)) { Cambiar(-1,hwnd); return true; }
        if (PtInRect(&m_rcSiguiente,p)) { Cambiar(1,hwnd); return true; }
        if (PtInRect(&m_rcCopiar,p)) { m_texto->Copiar(Unir()); return true; }
        if (PtInRect(&m_rcRenombrar,p) && !m_documento.ruta.empty()) { if(m_texto->Renombrar(hwnd,m_documento.ruta)) Cargar(); return true; }
        if (PtInRect(&m_rcComparar, p)) {
            if (m_archivos.size() == 2) {
                PostMessageW(hwnd, WM_SIDEVIEW_REQUEST_COMPARADOR_ARCHIVOS, 0, 0);
            }
            return true;
        }
        for (size_t i=0;i<m_documento.indice.size();++i) { RECT r{m_rcIndice.left,m_rcIndice.top+static_cast<int>(i*22),m_rcIndice.right,m_rcIndice.top+static_cast<int>((i+1)*22)}; if(PtInRect(&r,p)){m_lineaInicial=m_documento.indice[i];InvalidateRect(hwnd,nullptr,FALSE);return true;} }
        return PtInRect(&m_rcIndice,p);
    }
    bool ProcesarMensajeTeclado(UINT msg, WPARAM wParam, LPARAM, HWND hwnd) override {
        if(msg != WM_KEYDOWN) return false; if(wParam==VK_LEFT){Cambiar(-1,hwnd);return true;} if(wParam==VK_RIGHT){Cambiar(1,hwnd);return true;} if(wParam==VK_HOME){m_lineaInicial=0;InvalidateRect(hwnd,nullptr,FALSE);return true;} return false;
    }
    void Renderizar(HDC hdc, const RECT& rect, const TemaVisual& tema) override {
        HBRUSH b=CreateSolidBrush(tema.fondo); FillRect(hdc,&rect,b); DeleteObject(b); SetBkMode(hdc,TRANSPARENT);
        int top=8; m_rcAnterior={rect.left+8,rect.top+top,rect.left+72,rect.top+top+28}; m_rcSiguiente={rect.left+78,rect.top+top,rect.left+142,rect.top+top+28}; m_rcComparar={rect.right-300,rect.top+top,rect.right-178,rect.top+top+28}; m_rcCopiar={rect.right-170,rect.top+top,rect.right-88,rect.top+top+28}; m_rcRenombrar={rect.right-82,rect.top+top,rect.right-8,rect.top+top+28};
        DibujarBoton(hdc,m_rcAnterior,L"◀",tema); DibujarBoton(hdc,m_rcSiguiente,L"▶",tema); DibujarBoton(hdc,m_rcComparar,L"Comparar",tema); DibujarBoton(hdc,m_rcCopiar,L"Copiar",tema); DibujarBoton(hdc,m_rcRenombrar,L"Renombrar",tema);
        SetTextColor(hdc,tema.textoPrincipal); RECT titulo{rect.left+154,rect.top+top,rect.right-180,rect.top+top+28}; std::wstring nombre=m_documento.ruta; size_t p=nombre.find_last_of(L"\\/"); if(p!=std::wstring::npos) nombre=nombre.substr(p+1); DrawTextW(hdc,nombre.c_str(),-1,&titulo,DT_SINGLELINE|DT_VCENTER|DT_END_ELLIPSIS|DT_NOPREFIX);
        m_rcIndice={rect.left+8,rect.top+48,rect.left+190,rect.bottom-8}; HBRUSH bi=CreateSolidBrush(tema.fondoCabecera); FillRect(hdc,&m_rcIndice,bi); DeleteObject(bi); SetTextColor(hdc,tema.textoSecundario); size_t filasIndice = m_rcIndice.bottom > m_rcIndice.top ? static_cast<size_t>((m_rcIndice.bottom-m_rcIndice.top)/22) : 0; size_t entradasIndice = std::min(filasIndice,m_documento.indice.size()); for(size_t i=0;i<entradasIndice;++i){size_t n=m_documento.indice[i];RECT r{m_rcIndice.left+8,m_rcIndice.top+static_cast<int>(i*22),m_rcIndice.right-4,m_rcIndice.top+static_cast<int>((i+1)*22)}; DrawTextW(hdc,m_documento.lineas[n].c_str(),-1,&r,DT_SINGLELINE|DT_END_ELLIPSIS|DT_NOPREFIX);}
        bool mostrarNumeros=g_mostrarNumerosLineaV2.load(); int x=rect.left+202,y=rect.top+54; SetTextColor(hdc,tema.textoPrincipal); for(size_t i=m_lineaInicial;i<m_documento.lineas.size() && y<rect.bottom-8;++i,y+=20){if(mostrarNumeros){SetTextColor(hdc,tema.textoSecundario); wchar_t no[32]; swprintf_s(no,L"%5zu",i+1); TextOutW(hdc,x,y,no,static_cast<int>(wcslen(no)));} const std::wstring& linea=m_documento.lineas[i]; bool encabezado=!linea.empty() && linea[0]==L'#'; bool lista=linea.size()>1 && (linea[0]==L'-'||linea[0]==L'*'||linea[0]==L'>'); COLORREF color=m_codigo&&EsClave(linea)?RGB(180,100,220):(encabezado?RGB(70,150,230):(lista?RGB(90,180,120):tema.textoPrincipal)); SetTextColor(hdc,color); RECT r{x+ (mostrarNumeros?52:4),y,rect.right-8,y+20}; DrawTextW(hdc,linea.c_str(),-1,&r,DT_SINGLELINE|DT_END_ELLIPSIS|DT_NOPREFIX);}
    }
private:
    void Cargar(){m_documento=m_archivos.empty()?DocumentoTextoV2{}:m_texto->Cargar(m_archivos[m_indiceArchivo]);m_lineaInicial=0;}
    void Cambiar(int delta,HWND hwnd){if(m_archivos.size()<2)return;m_indiceArchivo=delta<0?m_nav->Anterior(m_indiceArchivo,m_archivos.size()):m_nav->Siguiente(m_indiceArchivo,m_archivos.size());Cargar();InvalidateRect(hwnd,nullptr,FALSE);}
    std::wstring Unir()const{size_t capacidad=0;for(const auto& l:m_documento.lineas) capacidad+=l.size()+2;std::wstring r;r.reserve(capacidad);for(const auto& l:m_documento.lineas){r+=l;r+=L"\r\n";}return r;}
    static bool EsClave(const std::wstring& l){static const wchar_t* k[]={L"class ",L"struct ",L"void ",L"int ",L"return ",L"#include",L"function ",L"const ",L"public:",L"private:"};for(auto s:k)if(l.find(s)!=std::wstring::npos)return true;return false;}
    void DibujarBoton(HDC hdc,const RECT& r,const wchar_t* t,const TemaVisual& tema){HBRUSH b=CreateSolidBrush(tema.botonFondo);FillRect(hdc,&r,b);DeleteObject(b);SetTextColor(hdc,tema.textoPrincipal);DrawTextW(hdc,t,-1,const_cast<RECT*>(&r),DT_CENTER|DT_VCENTER|DT_SINGLELINE|DT_NOPREFIX);}
    std::shared_ptr<ServicioTextoV2> m_texto; std::shared_ptr<ServicioNavegacionV2> m_nav; bool m_codigo{}; std::vector<std::wstring> m_archivos; size_t m_indiceArchivo{}; size_t m_lineaInicial{}; DocumentoTextoV2 m_documento; RECT m_rcAnterior{},m_rcSiguiente{},m_rcComparar{},m_rcCopiar{},m_rcRenombrar{},m_rcIndice{};
};
}
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
#include <memory>
#include <string>
#include <vector>

namespace SideViewV2 {

class VistaComparadorArchivosV2 : public IPanelContenido {
public:
    explicit VistaComparadorArchivosV2(
        std::shared_ptr<ServicioComparadorArchivosV2> servicio)
        : m_servicio(std::move(servicio)) {}

    void AlCambiarSeleccion(const std::vector<std::wstring>& archivos,
                            const std::wstring&) override {
        m_archivos = archivos;
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
        }
        if (msg == WM_KEYDOWN && wParam == VK_ESCAPE) {
            PostMessageW(hwndPanel, WM_SIDEVIEW_CLOSE_COMPARADOR_ARCHIVOS, 0, 0);
            return true;
        }
        return false;
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

        RECT info{rect.left + margen, rect.top + 58, rect.right - margen, rect.top + 194};
        DibujarInformacionComun(hdc, info, tema, texto);
        m_rcAreaEspecifica = {rect.left + margen, rect.top + 208,
                              rect.right - margen, rect.bottom - margen};
        RenderizarContenido(hdc, m_rcAreaEspecifica, tema, texto);

        SelectObject(hdc, anterior);
        DeleteObject(titulo);
        DeleteObject(texto);
    }

protected:
    virtual const wchar_t* NombreVista() const { return L"Comparar archivos"; }

    virtual void RenderizarContenido(HDC hdc, const RECT& rect,
                                     const TemaVisual& tema, HFONT fuente) {
        SetTextColor(hdc, tema.textoSecundario);
        RECT mensaje = rect;
        DrawTextW(hdc, m_estado.mensaje.c_str(), -1, &mensaje,
                  DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);
    }

    void CargarComparacionComun(const std::vector<std::wstring>& archivos) {
        m_estado = m_servicio ? m_servicio->CrearComparacion(archivos)
                               : EstadoComparadorArchivosV2{};
    }

    const EstadoComparadorArchivosV2& EstadoComun() const { return m_estado; }
    const RECT& AreaEspecifica() const { return m_rcAreaEspecifica; }

    void DibujarInformacionComun(HDC hdc, const RECT& info,
                                 const TemaVisual& tema, HFONT fuente) {
        HBRUSH fondo = CreateSolidBrush(tema.fondoTarjeta);
        FillRect(hdc, &info, fondo);
        DeleteObject(fondo);
        HPEN borde = CreatePen(PS_SOLID, 1, tema.borde);
        HPEN anterior = static_cast<HPEN>(SelectObject(hdc, borde));
        HBRUSH nulo = static_cast<HBRUSH>(GetStockObject(NULL_BRUSH));
        HBRUSH anteriorBrocha = static_cast<HBRUSH>(SelectObject(hdc, nulo));
        RoundRect(hdc, info.left, info.top, info.right, info.bottom, 8, 8);
        SelectObject(hdc, anteriorBrocha);
        SelectObject(hdc, anterior);
        DeleteObject(borde);

        SelectObject(hdc, fuente);
        SetTextColor(hdc, tema.textoPrincipal);
        RECT titulo{info.left + 12, info.top + 8, info.right - 12, info.top + 28};
        DrawTextW(hdc, L"INFORMACIÓN GENERAL", -1, &titulo,
                  DT_LEFT | DT_SINGLELINE | DT_NOPREFIX);
        const int mitad = (info.right - info.left) / 2;
        DibujarDato(hdc, info, info.left + 12, info.left + mitad - 12,
                    m_estado.izquierda, L"Archivo 1", tema);
        DibujarDato(hdc, info, info.left + mitad + 12, info.right - 12,
                    m_estado.derecha, L"Archivo 2", tema);
        SetTextColor(hdc, tema.acento);
        RECT diferencia{info.left + 12, info.bottom - 26, info.right - 12, info.bottom - 8};
        std::wstring peso = L"Diferencia de peso: " +
            (m_servicio ? m_servicio->FormatearDiferencia(
                m_estado.izquierda.bytes, m_estado.derecha.bytes) : L"+0 B");
        DrawTextW(hdc, peso.c_str(), -1, &diferencia,
                  DT_LEFT | DT_SINGLELINE | DT_NOPREFIX);
    }

    void DibujarContenedor(HDC hdc, const RECT& rect, const TemaVisual& tema) {
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
                      const TemaVisual& tema) {
        HBRUSH fondo = CreateSolidBrush(tema.botonFondo);
        FillRect(hdc, &rect, fondo);
        DeleteObject(fondo);
        SetTextColor(hdc, tema.textoPrincipal);
        RECT textoRect = rect;
        DrawTextW(hdc, texto, -1, &textoRect,
                  DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
    }

    static HFONT CrearFuente(int altura, int peso) {
        return CreateFontW(altura, 0, 0, 0, peso, FALSE, FALSE, FALSE,
                           DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                           CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
                           L"Segoe UI Variable Text");
    }

private:
    void DibujarDato(HDC hdc, const RECT& info, int izquierda, int derecha,
                     const DatosArchivoComparacionV2& datos,
                     const wchar_t* etiqueta, const TemaVisual& tema) {
        RECT nombre{izquierda, info.top + 36, derecha, info.top + 58};
        SetTextColor(hdc, tema.textoPrincipal);
        std::wstring texto = std::wstring(etiqueta) + L": " + datos.nombre;
        DrawTextW(hdc, texto.c_str(), -1, &nombre,
                  DT_LEFT | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);
        RECT tamano{izquierda, info.top + 60, derecha, info.top + 82};
        SetTextColor(hdc, tema.textoSecundario);
        std::wstring peso = L"Peso: " + (m_servicio ? m_servicio->FormatearTamano(datos.bytes) : L"0 B");
        DrawTextW(hdc, peso.c_str(), -1, &tamano,
                  DT_LEFT | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);
        RECT hash{izquierda, info.top + 84, derecha, info.top + 106};
        std::wstring sha = L"SHA-256: " + datos.sha256;
        DrawTextW(hdc, sha.c_str(), -1, &hash,
                  DT_LEFT | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);
    }

    std::shared_ptr<ServicioComparadorArchivosV2> m_servicio;
    EstadoComparadorArchivosV2 m_estado;
    std::vector<std::wstring> m_archivos;
    HWND m_panel{nullptr};
    RECT m_rcVolver{};
    RECT m_rcAreaEspecifica{};
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
        LiberarReproductores();
        m_panel = nullptr;
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
        DibujarBoton(hdc, m_rcRetroceder, L"-10 s", tema, true);
        DibujarBoton(hdc, m_rcReproducir,
                     m_estado.reproduciendo ? L"Pausar" : L"Reproducir", tema, true);
        DibujarBoton(hdc, m_rcAvanzar, L"+10 s", tema, true);

        SelectObject(hdc, texto);
        SetTextColor(hdc, tema.textoSecundario);
        RECT tiempo{m_rcAvanzar.right + 12, yBotones,
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
        if (duracionIzquierda > 0.0 && duracionDerecha > 0.0) {
            m_estado.duracionSegundos = std::min(duracionIzquierda, duracionDerecha);
        } else if (duracionIzquierda > 0.0) {
            m_estado.duracionSegundos = duracionIzquierda;
        } else if (duracionDerecha > 0.0) {
            m_estado.duracionSegundos = duracionDerecha;
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
};

} // namespace SideViewV2
/**** ended inlining v2/vistas/vista_comparador_videos.hpp ****/
/**** skipping file: v2/vistas/vista_comparador_archivos.hpp ****/
/**** start inlining v2/vistas/vista_comparador_texto.hpp ****/

/**** skipping file: v2/vistas/vista_comparador_archivos.hpp ****/
/**** skipping file: v2/nucleo/servicio_diff.hpp ****/
#include <windowsx.h>
#include <algorithm>
#include <memory>

namespace SideViewV2 {

class VistaComparadorTextoV2 final : public VistaComparadorArchivosV2 {
public:
    explicit VistaComparadorTextoV2(std::shared_ptr<ServicioComparadorTextoV2> servicio)
        : VistaComparadorArchivosV2(servicio), m_diff(std::move(servicio)) {}

    void AlCambiarSeleccion(const std::vector<std::wstring>& archivos,
                            const std::wstring& carpeta) override {
        VistaComparadorArchivosV2::AlCambiarSeleccion(archivos, carpeta);
        m_estadoDiff = m_diff ? m_diff->CrearDiff(archivos) : EstadoDiffV2{};
        m_scroll = 0;
    }

    bool ProcesarMensajeRaton(UINT msg, WPARAM wParam, LPARAM lParam,
                              HWND hwndPanel) override {
        if (VistaComparadorArchivosV2::ProcesarMensajeRaton(msg, wParam, lParam, hwndPanel)) return true;
        if (msg == WM_MOUSEWHEEL) {
            const int delta = GET_WHEEL_DELTA_WPARAM(wParam) > 0 ? -3 : 3;
            const int filas = static_cast<int>(m_estadoDiff.lineas.size());
            m_scroll = std::max(0, std::min(m_scroll + delta, std::max(0, filas - 8)));
            InvalidateRect(hwndPanel, nullptr, FALSE);
            return true;
        }
        return false;
    }

protected:
    const wchar_t* NombreVista() const override { return L"Comparar texto · Diff estilo GitHub"; }

    void RenderizarContenido(HDC hdc, const RECT& rect, const TemaVisual& tema,
                             HFONT fuente) override {
        DibujarContenedor(hdc, rect, tema);
        RECT cabeceraIzquierda{rect.left + 10, rect.top + 8,
                               rect.left + (rect.right - rect.left) / 2 - 6, rect.top + 32};
        RECT cabeceraDerecha{rect.left + (rect.right - rect.left) / 2 + 6, rect.top + 8,
                             rect.right - 10, rect.top + 32};
        SetTextColor(hdc, tema.textoPrincipal);
        DrawTextW(hdc, L"Archivo 1", -1, &cabeceraIzquierda,
                  DT_LEFT | DT_SINGLELINE | DT_NOPREFIX);
        DrawTextW(hdc, L"Archivo 2", -1, &cabeceraDerecha,
                  DT_LEFT | DT_SINGLELINE | DT_NOPREFIX);

        const int mitad = (rect.right - rect.left) / 2;
        const int altoFila = 22;
        const int inicio = rect.top + 36;
        const int visibles = std::max(0, static_cast<int>(
            (rect.bottom - inicio - 6) / altoFila));
        SelectObject(hdc, fuente);
        for (int visible = 0; visible < visibles; ++visible) {
            const std::size_t indice = static_cast<std::size_t>(m_scroll + visible);
            if (indice >= m_estadoDiff.lineas.size()) break;
            const LineaDiffV2& linea = m_estadoDiff.lineas[indice];
            const int y = inicio + visible * altoFila;
            COLORREF color = tema.fondoTarjeta;
            if (linea.tipo == TipoLineaDiffV2::Agregada) color = RGB(218, 245, 220);
            else if (linea.tipo == TipoLineaDiffV2::Eliminada) color = RGB(250, 220, 220);
            else if (linea.tipo == TipoLineaDiffV2::Modificada) color = RGB(250, 240, 204);
            HBRUSH fondoFila = CreateSolidBrush(tema.modoOscuro ? Mezclar(color, tema.fondo, 35) : color);
            RECT filaIzq{rect.left + 2, y, rect.left + mitad - 2, y + altoFila};
            RECT filaDer{rect.left + mitad + 2, y, rect.right - 2, y + altoFila};
            FillRect(hdc, &filaIzq, fondoFila);
            FillRect(hdc, &filaDer, fondoFila);
            DeleteObject(fondoFila);
            SetTextColor(hdc, tema.textoSecundario);
            wchar_t numeroIzq[24]{}, numeroDer[24]{};
            swprintf_s(numeroIzq, L"%4zu", linea.lineaIzquierda);
            swprintf_s(numeroDer, L"%4zu", linea.lineaDerecha);
            RECT ni{filaIzq.left + 4, y, filaIzq.left + 42, y + altoFila};
            RECT nd{filaDer.left + 4, y, filaDer.left + 42, y + altoFila};
            DrawTextW(hdc, numeroIzq, -1, &ni, DT_RIGHT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
            DrawTextW(hdc, numeroDer, -1, &nd, DT_RIGHT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
            SetTextColor(hdc, linea.tipo == TipoLineaDiffV2::Agregada ? RGB(25, 130, 50) :
                               linea.tipo == TipoLineaDiffV2::Eliminada ? RGB(170, 40, 40) :
                               tema.textoPrincipal);
            RECT ti{filaIzq.left + 50, y, filaIzq.right - 6, y + altoFila};
            RECT td{filaDer.left + 50, y, filaDer.right - 6, y + altoFila};
            DrawTextW(hdc, linea.izquierda.c_str(), -1, &ti,
                      DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);
            DrawTextW(hdc, linea.derecha.c_str(), -1, &td,
                      DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);
        }
        SetTextColor(hdc, tema.textoSecundario);
        RECT estado{rect.left + 10, rect.bottom - 24, rect.right - 10, rect.bottom - 6};
        DrawTextW(hdc, m_estadoDiff.mensaje.c_str(), -1, &estado,
                  DT_LEFT | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);
    }

private:
    static COLORREF Mezclar(COLORREF frente, COLORREF fondo, int porcentaje) {
        const int p = std::clamp(porcentaje, 0, 100);
        return RGB((GetRValue(frente) * p + GetRValue(fondo) * (100 - p)) / 100,
                   (GetGValue(frente) * p + GetGValue(fondo) * (100 - p)) / 100,
                   (GetBValue(frente) * p + GetBValue(fondo) * (100 - p)) / 100);
    }

    std::shared_ptr<ServicioComparadorTextoV2> m_diff;
    EstadoDiffV2 m_estadoDiff;
    int m_scroll{0};
};

} // namespace SideViewV2
/**** ended inlining v2/vistas/vista_comparador_texto.hpp ****/
/**** start inlining v2/vistas/vista_comparador_documentos.hpp ****/

/**** skipping file: v2/vistas/vista_comparador_archivos.hpp ****/
/**** skipping file: v2/nucleo/servicio_diff.hpp ****/

namespace SideViewV2 {

class VistaComparadorDocumentosV2 final : public VistaComparadorArchivosV2 {
public:
    explicit VistaComparadorDocumentosV2(
        std::shared_ptr<ServicioComparadorDocumentosV2> servicio)
        : VistaComparadorArchivosV2(std::move(servicio)) {}

protected:
    const wchar_t* NombreVista() const override { return L"Comparar documentos"; }

    void RenderizarContenido(HDC hdc, const RECT& rect, const TemaVisual& tema,
                             HFONT) override {
        DibujarContenedor(hdc, rect, tema);
        SetTextColor(hdc, tema.textoPrincipal);
        RECT titulo{rect.left + 16, rect.top + 20, rect.right - 16, rect.top + 48};
        DrawTextW(hdc, L"Comparación documental", -1, &titulo,
                  DT_CENTER | DT_SINGLELINE | DT_NOPREFIX);
        SetTextColor(hdc, tema.textoSecundario);
        RECT mensaje{rect.left + 24, rect.top + 64, rect.right - 24, rect.bottom - 24};
        DrawTextW(hdc,
                  L"El comparador documental hereda nombre, peso y SHA-256.\n"
                  L"La extracción visual específica del formato se conectará mediante otro adaptador.",
                  -1, &mensaje, DT_CENTER | DT_WORDBREAK | DT_NOPREFIX);
    }
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
        if (seleccion.size() != 2) return false;
        const TipoVistaContenido primero = ClasificarRuta(seleccion[0].c_str());
        const TipoVistaContenido segundo = ClasificarRuta(seleccion[1].c_str());
        if (tipoResultado) *tipoResultado = primero;
        return primero == segundo &&
               primero != TipoVistaContenido::Tutorial &&
               primero != TipoVistaContenido::Carpetas &&
               primero != TipoVistaContenido::Desconocido;
    }

    static bool DebeCrearComparador(
        const std::vector<std::wstring>& seleccion) {
        TipoVistaContenido tipo{};
        if (!SeleccionaDosDelMismoTipo(seleccion, &tipo)) return false;
        return tipo != TipoVistaContenido::Video ||
               ComparadorVideosExperimentalDisponibleV2();
    }

    static std::shared_ptr<IPanelContenido> CrearVistaParaSeleccion(
        const std::vector<std::wstring>& seleccion,
        const std::wstring& carpeta,
        bool forzarVistaNormal = false) {

        TipoVistaContenido tipo = ClasificarSeleccion(seleccion, carpeta);

        if (!forzarVistaNormal && DebeCrearComparador(seleccion)) {
            TipoVistaContenido tipoComparador{};
            SeleccionaDosDelMismoTipo(seleccion, &tipoComparador);
            if (tipoComparador == TipoVistaContenido::Video) {
                return CrearVistaComparadorVideos(seleccion);
            }
            if (tipoComparador == TipoVistaContenido::Texto ||
                tipoComparador == TipoVistaContenido::Codigo) {
                auto vista = std::make_shared<VistaComparadorTextoV2>(
                    Dependencias().comparadorTexto);
                vista->AlCambiarSeleccion(seleccion, carpeta);
                return vista;
            }
            if (tipoComparador == TipoVistaContenido::Documentos) {
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
                    Dependencias().inspeccionCarpetas, Dependencias().versionMod);
                break;

            case TipoVistaContenido::Texto:
                vista = std::make_shared<VistaTextoCodigo>(Dependencias().texto, Dependencias().navegacion, false);
                break;

            case TipoVistaContenido::Imagen:
                vista = std::make_shared<VistaImagenes>(Dependencias().navegacion, Dependencias().abrirConImagenes, Dependencias().previsualizacionMedia);
                break;

            case TipoVistaContenido::Video:
                vista = std::make_shared<VistaVideos>(Dependencias().navegacion, Dependencias().abrirConVideos, Dependencias().previsualizacionMedia);
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

inline bool EsComparadorArchivosActivoPorSeleccionV2(
    const std::vector<std::wstring>& seleccion) {
    return FabricaVistas::DebeCrearComparador(seleccion);
}

LRESULT CALLBACK ProcedimientoVentanaPanel(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK ProcedimientoSubclasePadre(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam,
                                            UINT_PTR uIdSubclass, DWORD_PTR dwRefData);
LRESULT CALLBACK ProcedimientoSubclaseTop(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam,
                                          DWORD_PTR dwRefData);

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

    if (estado->vistaContenido) {
        estado->vistaContenido->Liberar();
        estado->vistaContenido.reset();
    }

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

    if (estado->panel && IsWindow(estado->panel)) {
        KillTimer(estado->panel, 101);
        ShowWindow(estado->panel, SW_HIDE);
    }

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

    HWND padre = GetParent(ventanaVista);
    if (!padre) padre = tab ? tab : top;

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

    if (estado && (estado->view != ventanaVista || estado->parent != padre)) {
        DestruirEstado(estado, true);
        estado = nullptr;
    }

    if (!estado) {
        estado = new EstadoPanelV2();
        estado->top = top;
        estado->tab = clave;
        estado->view = ventanaVista;
        estado->parent = padre;
        estado->tema = ObtenerTemaActual();

        GetWindowRect(ventanaVista, &estado->viewOriginal);
        MapWindowPoints(HWND_DESKTOP, padre, reinterpret_cast<POINT*>(&estado->viewOriginal), 2);
        GetClientRect(padre, &estado->parentCliente);

        HINSTANCE hInst = GetModuleHandleW(nullptr);
        estado->panel = CreateWindowExW(
            0, CLASE_PANEL_V2, L"",
            WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
            0, 0, 10, 10, padre, nullptr, hInst, estado);

        if (!estado->panel) {
            delete estado;
            vistaShell->Release();
            return nullptr;
        }

        SetWindowSubclass(padre, ProcedimientoSubclasePadre, 1, reinterpret_cast<DWORD_PTR>(estado));
        if (tab && tab != padre) {
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
            estado->comparadorArchivosActivo = EsComparadorArchivosActivoPorSeleccionV2(archivos);
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
            estado->vistaContenido->Liberar();
            estado->vistaContenido = FabricaVistas::CrearVistaParaSeleccion(
                estado->archivosSeleccionados, estado->carpetaContenedora, true);
            estado->comparadorArchivosActivo = false;
            if (estado->vistaContenido) estado->vistaContenido->AlActivar(hwnd);
            DisponerPanel(estado);
            InvalidateRect(hwnd, nullptr, FALSE);
            return 0;
        }

        if (msg == WM_SIDEVIEW_SETTINGS) {
            if (estado->comparadorArchivosActivo &&
                !EsComparadorArchivosActivoPorSeleccionV2(estado->archivosSeleccionados)) {
                estado->vistaContenido->Liberar();
                estado->vistaContenido = FabricaVistas::CrearVistaParaSeleccion(
                    estado->archivosSeleccionados, estado->carpetaContenedora);
                estado->comparadorArchivosActivo = false;
                if (estado->vistaContenido) estado->vistaContenido->AlActivar(hwnd);
                DisponerPanel(estado);
            } else if (!estado->comparadorArchivosActivo &&
                       EsComparadorArchivosActivoPorSeleccionV2(estado->archivosSeleccionados)) {
                estado->vistaContenido->Liberar();
                estado->vistaContenido = FabricaVistas::CrearVistaParaSeleccion(
                    estado->archivosSeleccionados, estado->carpetaContenedora);
                estado->comparadorArchivosActivo = true;
                if (estado->vistaContenido) estado->vistaContenido->AlActivar(hwnd);
                DisponerPanel(estado);
            }
            if (estado->vistaContenido) estado->vistaContenido->ActualizarConfiguracion();
            InvalidateRect(hwnd, nullptr, FALSE);
            return 0;
        }

        if (msg == WM_SIDEVIEW_REQUEST_COMPARADOR_ARCHIVOS &&
            !estado->comparadorArchivosActivo &&
            EsComparadorArchivosActivoPorSeleccionV2(estado->archivosSeleccionados)) {
            estado->vistaContenido->Liberar();
            estado->vistaContenido = FabricaVistas::CrearVistaParaSeleccion(
                estado->archivosSeleccionados, estado->carpetaContenedora);
            estado->comparadorArchivosActivo = true;
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
                            EsComparadorArchivosActivoPorSeleccionV2(seleccion);

                        // Si el comparador sigue recibiendo exactamente dos
                        // videos, sólo renueva sus fuentes; no desmonta la UX.
                        if (mantenerComparador) {
                            estado->vistaContenido->AlCambiarSeleccion(seleccion, carpeta);
                        } else if (estado->comparadorArchivosActivo ||
                                   tipoPrevio != tipoNuevo ||
                                   EsComparadorArchivosActivoPorSeleccionV2(seleccion) !=
                                       estado->comparadorArchivosActivo ||
                                   !estado->vistaContenido) {
                            if (estado->vistaContenido) estado->vistaContenido->Liberar();
                            estado->vistaContenido = FabricaVistas::CrearVistaParaSeleccion(seleccion, carpeta);
                            estado->comparadorArchivosActivo = EsComparadorArchivosActivoPorSeleccionV2(seleccion);
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
            vk = VK_F1 + numero - 1;
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
    g_comparadorVideosExperimentalV2.store(
        Wh_GetIntSetting(L"comparadorVideosExperimental") != 0,
        std::memory_order_release);
    g_comparadorVideosPantallaCompletaV2.store(
        Wh_GetIntSetting(L"comparadorVideosPantallaCompleta") != 0,
        std::memory_order_release);
    g_mostrarVersionModV2.store(
        Wh_GetIntSetting(L"mostrarVersionMod") != 0,
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
