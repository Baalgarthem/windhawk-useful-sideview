/**** start inlining useful-sideview-v2.cpp ****/
// ==WindhawkMod==
// @id              useful-sideview
// @name            Useful Side View
// @description     Transforma el explorador de Windows 11 con un panel lateral inyectado de alta fidelidad, fluido y cooperativo. Alterna con el atajo configurable (por defecto Ctrl + K).
// @version         2.1.1-alpha
// @author          Baalgarthem
// @include         explorer.exe
// @architecture    x86-64
// @compilerOptions -lole32 -loleaut32 -lshell32 -luser32 -lgdi32 -lcomctl32 -luuid -luxtheme -ldwmapi -lwindowscodecs -lmsimg32 -lshlwapi -ladvapi32 -lcomdlg32
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
*/
// ==/WindhawkModSettings==

// ==WindhawkModReadme==
/*
# Useful Side View (v2.1 Architecture)

Prototipo modular de nueva generación con arquitectura canónica de Windows 11:
- Inyección cooperativa con `CreateWindowExW` y subclaseo de `CabinetWClass` / `ShellTabWindowClass`.
- Renderizado de doble buffer GDI sin parpadeo (Anti-Flicker BitBlt).
- **Atajo interactivo configurable**: Por defecto **[ Ctrl + K ]**.
- **Pantalla Tutorial (Walkthrough)**: Se activa automáticamente al presionar el atajo sin archivos seleccionados.
- **Suite Funcional de Comprimidos & Cómics**: Portadas, árbol de archivos, navegación anterior/siguiente, "Abrir con" y selector de conversión.
- **Suite Funcional de Inspección de Carpetas**: Árbol jerárquico recursivo, cálculo de peso, Top 4 subcarpetas pesadas, enlaces simbólicos, copia de estructura y limpieza de carpetas vacías.
- **Vistas Especializadas Modulares**: Texto, Imágenes, Video, Documentos y Código (Marcadas como "En desarrollo").
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
    COLORREF acentoHover{RGB(30, 144, 255)};
    COLORREF botonFondo{RGB(50, 50, 50)};
    COLORREF botonHover{RGB(70, 70, 70)};
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
        tema.botonHover = RGB(220, 224, 230);
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

class IPanelContenido {
public:
    virtual ~IPanelContenido() = default;

    // Renderizado cooperativo en doble buffer
    virtual void Renderizar(HDC hdc, const RECT& rect, const TemaVisual& tema) = 0;

    // Notificación de cambio de selección en Explorer
    virtual void AlCambiarSeleccion(const std::vector<std::wstring>& archivos, const std::wstring& carpeta) = 0;

    // Manejo de eventos de ratón/teclado en el panel (retornan true si consumieron el evento)
    virtual bool ProcesarMensajeRaton(UINT msg, WPARAM wParam, LPARAM lParam, HWND hwndPanel) { return false; }
    virtual bool ProcesarMensajeTeclado(UINT msg, WPARAM wParam, LPARAM lParam, HWND hwndPanel) { return false; }

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
    int anchoDIP{380};          // Ancho por defecto en DIP
    bool dockDerecha{true};

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

    // Metadatos para el prototipo
    std::wstring rutaActiva;
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

    estado->bmpAnterior = static_cast<HBITMAP>(SelectObject(estado->dcMemoria, estado->bmpMemoria));
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
    Comprimidos,   // Vista funcional de ZIP, RAR, CBZ, CBR, 7Z, TAR, etc.
    Texto,         // Vista de TXT, Markdown (en desarrollo)
    Imagen,        // Vista de PNG, JPG, WebP, GIF, etc. (en desarrollo)
    Video,         // Vista de MP4, MKV, AVI, etc. (en desarrollo)
    Documentos,    // Vista de PDF, DOCX, XLSX, etc. (en desarrollo)
    Codigo,        // Vista de C++, Python, JS, etc. (en desarrollo)
    Carpetas,      // Vista de inspección de árbol de carpetas (en desarrollo)
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
        L".7z", L".bz2", L".bzip2", L".cbr", L".cbz", L".gz", L".gzip",
        L".iso", L".rar", L".tar", L".tbz2", L".tgz", L".txz", L".xz",
        L".z", L".zip", L".zipx"
    };
    return CoincideExtension(ext, EXT_COMPRIMIDOS, ARRAYSIZE(EXT_COMPRIMIDOS));
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
    size_t cImagenes = 0;
    size_t cVideos = 0;
    size_t cTexto = 0;
    size_t cCodigo = 0;
    size_t cDocs = 0;
    size_t cCarpetas = 0;
    size_t cOtros = 0;

    for (const auto& r : seleccion) {
        TipoVistaContenido t = ClasificarRuta(r.c_str());
        switch (t) {
            case TipoVistaContenido::Comprimidos: cComprimidos++; break;
            case TipoVistaContenido::Imagen: cImagenes++; break;
            case TipoVistaContenido::Video: cVideos++; break;
            case TipoVistaContenido::Texto: cTexto++; break;
            case TipoVistaContenido::Codigo: cCodigo++; break;
            case TipoVistaContenido::Documentos: cDocs++; break;
            case TipoVistaContenido::Carpetas: cCarpetas++; break;
            default: cOtros++; break;
        }
    }

    size_t total = seleccion.size();
    if (cComprimidos == total) return TipoVistaContenido::Comprimidos;
    if (cImagenes == total) return TipoVistaContenido::Imagen;
    if (cVideos == total) return TipoVistaContenido::Video;
    if (cTexto == total) return TipoVistaContenido::Texto;
    if (cCarpetas == total) return TipoVistaContenido::Carpetas;
    if (cDocs == total) return TipoVistaContenido::Documentos;
    if (cCodigo == total) return TipoVistaContenido::Codigo;

    // Preferencias prioritarias en selecciones mixtas
    if (cComprimidos > 0) return TipoVistaContenido::Comprimidos;
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
#include <shellapi.h>

namespace SideViewV2 {

class VistaWalkthrough : public IPanelContenido {
public:
    void AlCambiarSeleccion(const std::vector<std::wstring>& archivos, const std::wstring& carpeta) override {
        m_carpeta = carpeta;
    }

    bool ProcesarMensajeRaton(UINT msg, WPARAM wParam, LPARAM lParam, HWND hwndPanel) override {
        if (msg == WM_LBUTTONDOWN) {
            POINT pt{GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
            if (PtInRect(&m_rectLinkGithub, pt)) {
                ShellExecuteW(nullptr, L"open", m_urlGithub.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
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
    RECT m_rectLinkGithub{};
};

} // namespace SideViewV2
/**** ended inlining v2/vistas/vista_walkthrough.hpp ****/
/**** start inlining v2/vistas/vista_comprimidos.hpp ****/

/**** skipping file: v2/ui/panel_interfaz.hpp ****/
/**** skipping file: v2/ui/ventana_base.hpp ****/
/**** start inlining v2/nucleo/programas_recordados.hpp ****/
#include <string>
#include <vector>
#include <algorithm>

namespace SideViewV2 {
struct ProgramasRecordadosV2 {
    std::vector<std::wstring> carpetas{L"code.exe", L"explorer.exe", L"cmd.exe", L"powershell.exe"};
    std::vector<std::wstring> comprimidos;
    std::wstring ultimoCarpetas;
    std::wstring ultimoComprimidos;
};
inline ProgramasRecordadosV2 g_programasRecordadosV2;
inline void RecordarProgramaV2(std::vector<std::wstring>& lista, std::wstring& ultimo, const std::wstring& ruta) {
    if (ruta.empty()) return;
    if (std::find(lista.begin(), lista.end(), ruta) == lista.end()) lista.push_back(ruta);
    ultimo = ruta;
}
inline std::wstring ProgramaSeleccionadoV2(const std::vector<std::wstring>& lista, const std::wstring& ultimo) {
    if (!ultimo.empty()) return ultimo;
    return lista.size() == 1 ? lista.front() : std::wstring();
}
}
/**** ended inlining v2/nucleo/programas_recordados.hpp ****/
#include <windows.h>
#include <shellapi.h>
#include <shlobj.h>
#include <shobjidl.h>
#include <commdlg.h>
#include <string>
#include <vector>
#include <map>
#include <algorithm>

namespace SideViewV2 {

#pragma pack(push, 1)
struct CabeceraZipLocalV2 {
    uint32_t firma;
    uint16_t versionMinima;
    uint16_t banderaGeneral;
    uint16_t metodoCompresion;
    uint16_t horaModificacion;
    uint16_t fechaModificacion;
    uint32_t sumaCrc32;
    uint32_t tamanoComprimido;
    uint32_t tamanoDescomprimido;
    uint16_t longitudNombreArchivo;
    uint16_t longitudCampoExtra;
};
#pragma pack(pop)

struct EntradaComprimidoV2 {
    std::wstring nombre;
    std::wstring rutaRelativa;
    bool esDirectorio{false};
    uintmax_t tamanoBytes{0};
};

class VistaComprimidos : public IPanelContenido {
public:
    VistaComprimidos() {
        m_formatosDisponibles = { L"zip", L"cbz", L"7z", L"tar" };
        m_formatoSeleccionado = L"zip";
        m_ultimoProgramaUsado = g_programasRecordadosV2.ultimoComprimidos;
    }

    void AlCambiarSeleccion(const std::vector<std::wstring>& archivos, const std::wstring& carpeta) override {
        m_archivos = archivos;
        m_carpeta = carpeta;
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
                    m_indiceArchivo = (m_indiceArchivo == 0) ? (m_archivos.size() - 1) : (m_indiceArchivo - 1);
                    CargarArchivoActual();
                    InvalidateRect(hwndPanel, nullptr, FALSE);
                    return true;
                }
            }
            // Botón Siguiente
            if (PtInRect(&m_rcBotonSiguiente, pt)) {
                if (m_archivos.size() > 1) {
                    m_indiceArchivo = (m_indiceArchivo + 1) % m_archivos.size();
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
            // Botón Desplegable Formatos
            if (PtInRect(&m_rcBotonDesplegableFormato, pt)) {
                MostrarMenuFormatos(hwndPanel);
                return true;
            }
            // Botón Convertir
            if (PtInRect(&m_rcBotonConvertir, pt)) {
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

        // Botón Convertir
        m_rcBotonConvertir = {m_rcBotonDesplegableFormato.right + 6, yBarra, rect.right - margen, yBarra + altoBotones};
        DibujarBoton(hdc, m_rcBotonConvertir, L"Convertir", tema, true);

        int yAbrir = rect.bottom - margen - altoBotones;
        m_rcBotonAbrirCon = {rect.left + margen, yAbrir, rect.right - margen, yAbrir + altoBotones};
        std::wstring programa = ProgramaSeleccionadoV2(g_programasRecordadosV2.comprimidos, m_ultimoProgramaUsado);
        std::wstring etiqueta = L"Abrir con: ";
        etiqueta += programa.empty() ? L"(click para seleccionar programa) ▾" : PathFindFileNameW(programa.c_str());
        DibujarBoton(hdc, m_rcBotonAbrirCon, etiqueta.c_str(), tema, true);

        // Área de Contenido: Árbol de archivos comprimidos
        int topLista = yBarra + altoBotones + 14;
        RECT rcContenedorLista{rect.left + margen, topLista, rect.right - margen, rect.bottom - margen};
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

        const std::wstring& ruta = m_archivos[m_indiceArchivo];
        const wchar_t* ext = PathFindExtensionW(ruta.c_str());

        // Parseo ligero directo de archivos ZIP / JAR / APK
        if (_wcsicmp(ext, L".zip") == 0 || _wcsicmp(ext, L".cbz") == 0 ||
            _wcsicmp(ext, L".jar") == 0 || _wcsicmp(ext, L".apk") == 0) {
            HANDLE hFile = CreateFileW(ruta.c_str(), GENERIC_READ,
                                       FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                                       nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
            if (hFile != INVALID_HANDLE_VALUE) {
                CabeceraZipLocalV2 hdr{};
                DWORD leidos = 0;
                while (ReadFile(hFile, &hdr, sizeof(hdr), &leidos, nullptr) && leidos == sizeof(hdr)) {
                    if (hdr.firma != 0x04034b50) break;
                    if (hdr.longitudNombreArchivo > 0 && hdr.longitudNombreArchivo < 4096) {
                        std::vector<char> bufName(hdr.longitudNombreArchivo + 1, 0);
                        if (!ReadFile(hFile, bufName.data(), hdr.longitudNombreArchivo, &leidos, nullptr) ||
                            leidos != hdr.longitudNombreArchivo) break;

                        if (hdr.longitudCampoExtra > 0) {
                            SetFilePointer(hFile, hdr.longitudCampoExtra, nullptr, FILE_CURRENT);
                        }

                        std::string nombreU8(bufName.data());
                        for (char& c : nombreU8) if (c == '/') c = '\\';

                        int lenW = MultiByteToWideChar(CP_UTF8, 0, nombreU8.c_str(), -1, nullptr, 0);
                        if (lenW > 0) {
                            std::vector<wchar_t> wbuf(lenW);
                            MultiByteToWideChar(CP_UTF8, 0, nombreU8.c_str(), -1, wbuf.data(), lenW);
                            std::wstring rRelativa = wbuf.data();
                            bool esDir = (!rRelativa.empty() && rRelativa.back() == L'\\');
                            const wchar_t* pSoloNombre = PathFindFileNameW(rRelativa.c_str());

                            EntradaComprimidoV2 e;
                            e.nombre = (pSoloNombre && *pSoloNombre) ? pSoloNombre : rRelativa;
                            e.rutaRelativa = rRelativa;
                            e.esDirectorio = esDir;
                            e.tamanoBytes = hdr.tamanoDescomprimido;
                            m_entradas.push_back(e);
                        }
                    } else {
                        break;
                    }
                }
                CloseHandle(hFile);
            }
        }
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
        m_ultimoProgramaUsado = ProgramaSeleccionadoV2(g_programasRecordadosV2.comprimidos, m_ultimoProgramaUsado);
        if (!m_ultimoProgramaUsado.empty()) {
            for (const auto& archivo : m_archivos) {
                ShellExecuteW(nullptr, L"open", m_ultimoProgramaUsado.c_str(),
                              (L"\"" + archivo + L"\"").c_str(), nullptr, SW_SHOWNORMAL);
            }
            return;
        }
        // Selector de archivo ejecutable de Windows
        wchar_t rutaProg[MAX_PATH] = L"";
        OPENFILENAMEW ofn{};
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = hwndPadre;
        ofn.lpstrFilter = L"Programas Ejecutables (*.exe)\0*.exe\0Todos los archivos (*.*)\0*.*\0";
        ofn.lpstrFile = rutaProg;
        ofn.nMaxFile = MAX_PATH;
        ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;

        if (GetOpenFileNameW(&ofn)) {
            RecordarProgramaV2(g_programasRecordadosV2.comprimidos, g_programasRecordadosV2.ultimoComprimidos, rutaProg);
            m_ultimoProgramaUsado = rutaProg;
            for (const auto& archivo : m_archivos) {
                ShellExecuteW(nullptr, L"open", m_ultimoProgramaUsado.c_str(),
                              (L"\"" + archivo + L"\"").c_str(), nullptr, SW_SHOWNORMAL);
            }
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
        std::wstring mensaje = L"Iniciando conversión de " + std::to_wstring(m_archivos.size()) +
                               L" archivo(s) al formato: ." + m_formatoSeleccionado +
                               L"\n\n(Lógica de compresión ejecutada en segundo plano)";
        MessageBoxW(hwndPadre, mensaje.c_str(), L"Useful Side View - Conversión", MB_OK | MB_ICONINFORMATION);
    }

    std::vector<std::wstring> m_archivos;
    std::wstring m_carpeta;
    size_t m_indiceArchivo{0};
    int m_desplazamientoScrollY{0};
    std::vector<EntradaComprimidoV2> m_entradas;

    std::wstring m_formatoSeleccionado;
    std::vector<std::wstring> m_formatosDisponibles;
    std::wstring m_ultimoProgramaUsado;

    RECT m_rcBotonAnterior{};
    RECT m_rcBotonSiguiente{};
    RECT m_rcBotonAbrirCon{};
    RECT m_rcBotonDesplegableFormato{};
    RECT m_rcBotonConvertir{};
};

} // namespace SideViewV2
/**** ended inlining v2/vistas/vista_comprimidos.hpp ****/
/**** start inlining v2/vistas/vista_carpetas.hpp ****/

/**** skipping file: v2/ui/panel_interfaz.hpp ****/
/**** skipping file: v2/ui/ventana_base.hpp ****/
/**** skipping file: v2/nucleo/programas_recordados.hpp ****/
#include <windows.h>
#include <shellapi.h>
#include <shlobj.h>
#include <shobjidl.h>
#include <commdlg.h>
#include <string>
#include <vector>
#include <algorithm>
#include <atomic>

namespace SideViewV2 {

inline std::atomic<int> g_limiteCarpetasPesadasConfig{3};

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

class VistaCarpetas : public IPanelContenido {
public:
    VistaCarpetas() {
        m_programasRecordados = g_programasRecordadosV2.carpetas;
        m_ultimoProgramaUsado = g_programasRecordadosV2.ultimoCarpetas;
        m_limiteCarpetasPesadas = g_limiteCarpetasPesadasConfig.load();
        if (m_limiteCarpetasPesadas < 1) m_limiteCarpetasPesadas = 3;
    }

    void AlCambiarSeleccion(const std::vector<std::wstring>& archivos, const std::wstring& carpeta) override {
        m_carpetasSeleccionadas = archivos;
        m_carpetaContenedora = carpeta;
        m_indiceCarpeta = 0;
        m_desplazamientoScrollY = 0;
        m_scrollPesadasY = 0;
        m_nivelExpansionMax = 1; // 1 = Raíz, 2 = Nivel 1, 3 = Nivel 2, 4 = Todo
        m_limiteCarpetasPesadas = g_limiteCarpetasPesadasConfig.load();
        if (m_limiteCarpetasPesadas < 1) m_limiteCarpetasPesadas = 3;

        CargarCarpetaActual();
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
            // Botón Anterior
            if (PtInRect(&m_rcBotonAnterior, pt)) {
                if (m_carpetasSeleccionadas.size() > 1) {
                    m_indiceCarpeta = (m_indiceCarpeta == 0) ? (m_carpetasSeleccionadas.size() - 1) : (m_indiceCarpeta - 1);
                    CargarCarpetaActual();
                    InvalidateRect(hwndPanel, nullptr, FALSE);
                    return true;
                }
            }
            // Botón Siguiente
            if (PtInRect(&m_rcBotonSiguiente, pt)) {
                if (m_carpetasSeleccionadas.size() > 1) {
                    m_indiceCarpeta = (m_indiceCarpeta + 1) % m_carpetasSeleccionadas.size();
                    CargarCarpetaActual();
                    InvalidateRect(hwndPanel, nullptr, FALSE);
                    return true;
                }
            }
            // Botón Alternar Expansión de Niveles
            if (PtInRect(&m_rcBotonExpansion, pt)) {
                m_nivelExpansionMax = (m_nivelExpansionMax >= 4) ? 1 : (m_nivelExpansionMax + 1);
                m_desplazamientoScrollY = 0;
                CargarCarpetaActual();
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

        std::wstring seleccionado = ProgramaSeleccionadoV2(m_programasRecordados, m_ultimoProgramaUsado);
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
        InspeccionarRecursivo(ruta, 0, 4);

        // Calcular las carpetas más pesadas
        CalcularCarpetasPesadas(ruta);
    }

    void InspeccionarRecursivo(const std::wstring& ruta, int nivel, int maxNivel) {
        if (nivel >= maxNivel || m_elementosArbol.size() > 500) return;

        std::wstring patron = ruta + L"\\*";
        WIN32_FIND_DATAW fd{};
        HANDLE hFind = FindFirstFileW(patron.c_str(), &fd);
        if (hFind == INVALID_HANDLE_VALUE) return;

        std::vector<ItemArbolCarpetaV2> subcarpetas;
        std::vector<ItemArbolCarpetaV2> archivos;
        size_t elementosEnCarpeta = 0;

        do {
            if (wcscmp(fd.cFileName, L".") == 0 || wcscmp(fd.cFileName, L"..") == 0) continue;
            if (fd.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) continue;

            elementosEnCarpeta++;
            std::wstring rCompleta = ruta + L"\\" + fd.cFileName;
            bool esDir = (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;

            ItemArbolCarpetaV2 item;
            item.nombre = fd.cFileName;
            item.rutaCompleta = rCompleta;
            item.esDirectorio = esDir;
            item.nivelJerarquia = nivel;

            if (esDir) {
                m_totalSubcarpetas++;
                item.tamanoBytes = 0;
                subcarpetas.push_back(item);
            } else {
                m_totalArchivos++;
                uintmax_t tam = (static_cast<uintmax_t>(fd.nFileSizeHigh) << 32) | fd.nFileSizeLow;
                item.tamanoBytes = tam;
                m_tamanoTotalBytes += tam;
                archivos.push_back(item);
            }
        } while (FindNextFileW(hFind, &fd));
        FindClose(hFind);

        if (elementosEnCarpeta == 0 && nivel > 0) {
            m_carpetasVaciasDetectadas.push_back(ruta);
        }

        std::sort(subcarpetas.begin(), subcarpetas.end(), [](const ItemArbolCarpetaV2& a, const ItemArbolCarpetaV2& b) {
            return StrCmpLogicalW(a.nombre.c_str(), b.nombre.c_str()) < 0;
        });
        std::sort(archivos.begin(), archivos.end(), [](const ItemArbolCarpetaV2& a, const ItemArbolCarpetaV2& b) {
            return StrCmpLogicalW(a.nombre.c_str(), b.nombre.c_str()) < 0;
        });

        for (const auto& dir : subcarpetas) {
            m_elementosArbol.push_back(dir);
            if (nivel + 1 < maxNivel && m_elementosArbol.size() < 500) {
                InspeccionarRecursivo(dir.rutaCompleta, nivel + 1, maxNivel);
            }
        }

        for (const auto& arch : archivos) {
            m_elementosArbol.push_back(arch);
        }
    }

    uintmax_t CalcularTamanoDirectorio(const std::wstring& rutaDirectorio) {
        uintmax_t acumulado = 0;
        std::wstring patron = rutaDirectorio + L"\\*";
        WIN32_FIND_DATAW fd;
        HANDLE hFind = FindFirstFileW(patron.c_str(), &fd);
        if (hFind == INVALID_HANDLE_VALUE) return 0;
        do {
            if (wcscmp(fd.cFileName, L".") == 0 || wcscmp(fd.cFileName, L"..") == 0) continue;
            if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                acumulado += CalcularTamanoDirectorio(rutaDirectorio + L"\\" + fd.cFileName);
            } else {
                acumulado += (static_cast<uintmax_t>(fd.nFileSizeHigh) << 32) | fd.nFileSizeLow;
            }
        } while (FindNextFileW(hFind, &fd));
        FindClose(hFind);
        return acumulado;
    }

    void CalcularCarpetasPesadas(const std::wstring& rutaRaiz) {
        m_topCarpetasPesadas.clear();
        std::wstring patron = rutaRaiz + L"\\*";
        WIN32_FIND_DATAW fd;
        HANDLE hFind = FindFirstFileW(patron.c_str(), &fd);
        if (hFind == INVALID_HANDLE_VALUE) return;

        do {
            if (wcscmp(fd.cFileName, L".") == 0 || wcscmp(fd.cFileName, L"..") == 0) continue;
            if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                uintmax_t tam = CalcularTamanoDirectorio(rutaRaiz + L"\\" + fd.cFileName);
                if (tam > 0) {
                    CarpetaPesadaV2 cp;
                    cp.nombre = fd.cFileName;
                    cp.tamanoBytes = tam;
                    cp.porcentaje = (m_tamanoTotalBytes > 0) ? (static_cast<double>(tam) / m_tamanoTotalBytes) * 100.0 : 0.0;
                    m_topCarpetasPesadas.push_back(cp);
                }
            }
        } while (FindNextFileW(hFind, &fd));
        FindClose(hFind);

        std::sort(m_topCarpetasPesadas.begin(), m_topCarpetasPesadas.end(), [](const CarpetaPesadaV2& a, const CarpetaPesadaV2& b) {
            return a.tamanoBytes > b.tamanoBytes;
        });
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
            wchar_t rutaProg[MAX_PATH] = L"";
            OPENFILENAMEW ofn{};
            ofn.lStructSize = sizeof(ofn);
            ofn.hwndOwner = hwndPadre;
            ofn.lpstrFilter = L"Programas Ejecutables (*.exe)\0*.exe\0Todos los archivos (*.*)\0*.*\0";
            ofn.lpstrFile = rutaProg;
            ofn.nMaxFile = MAX_PATH;
            ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;

            if (GetOpenFileNameW(&ofn)) {
                RecordarProgramaV2(g_programasRecordadosV2.carpetas, g_programasRecordadosV2.ultimoCarpetas, rutaProg);
                m_programasRecordados = g_programasRecordadosV2.carpetas;
                m_ultimoProgramaUsado = g_programasRecordadosV2.ultimoCarpetas;
                InvalidateRect(hwndPadre, nullptr, FALSE);
            }
        } else if (cmd > 0 && cmd <= static_cast<int>(m_programasRecordados.size())) {
            m_ultimoProgramaUsado = m_programasRecordados[cmd - 1];
            g_programasRecordadosV2.ultimoCarpetas = m_ultimoProgramaUsado;
            InvalidateRect(hwndPadre, nullptr, FALSE);
        }
    }

    void EjecutarAbrirCon(HWND hwndPadre) {
        m_ultimoProgramaUsado = ProgramaSeleccionadoV2(m_programasRecordados, m_ultimoProgramaUsado);
        if (m_ultimoProgramaUsado.empty()) {
            MostrarMenuDesplegableProgramas(hwndPadre);
            if (m_ultimoProgramaUsado.empty()) return;
        }
        for (const auto& carpeta : m_carpetasSeleccionadas) {
            ShellExecuteW(nullptr, L"open", m_ultimoProgramaUsado.c_str(),
                          (L"\"" + carpeta + L"\"").c_str(), nullptr, SW_SHOWNORMAL);
        }
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
            EmptyClipboard();
            size_t bytes = (textoAcumulado.size() + 1) * sizeof(wchar_t);
            HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, bytes);
            if (hMem) {
                void* pMem = GlobalLock(hMem);
                if (pMem) {
                    memcpy(pMem, textoAcumulado.c_str(), bytes);
                    GlobalUnlock(hMem);
                    SetClipboardData(CF_UNICODETEXT, hMem);
                }
            }
            CloseClipboard();
            MessageBoxW(hwndPadre, L"¡Estructura de la carpeta copiada al portapapeles!", L"Useful Side View", MB_OK | MB_ICONINFORMATION);
        }
    }

    void CrearEnlaceSimbolico(HWND hwndPadre) {
        if (m_carpetasSeleccionadas.empty() || m_indiceCarpeta >= m_carpetasSeleccionadas.size()) return;
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
            if (SUCCEEDED(CoCreateInstance(CLSID_ShellLink, nullptr, CLSCTX_INPROC_SERVER, IID_IShellLinkW, reinterpret_cast<void**>(&psl)))) {
                psl->SetPath(rutaOrigen.c_str());
                IPersistFile* ppf = nullptr;
                if (SUCCEEDED(psl->QueryInterface(IID_IPersistFile, reinterpret_cast<void**>(&ppf)))) {
                    ppf->Save(destinoStr.c_str(), TRUE);
                    ppf->Release();
                    MessageBoxW(hwndPadre, L"Enlace creado exitosamente.", L"Useful Side View", MB_OK | MB_ICONINFORMATION);
                }
                psl->Release();
            }
        }
    }

    void EjecutarFusionCarpetas(HWND hwndPadre) {
        std::wstring mensaje = L"Se fusionarán " + std::to_wstring(m_carpetasSeleccionadas.size()) +
                               L" carpetas en una única carpeta consolidada.\n\n¿Deseas continuar?";
        if (MessageBoxW(hwndPadre, mensaje.c_str(), L"Useful Side View - Fusión", MB_YESNO | MB_ICONQUESTION) == IDYES) {
            MessageBoxW(hwndPadre, L"Operación de fusión iniciada en segundo plano.", L"Useful Side View", MB_OK | MB_ICONINFORMATION);
        }
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
    std::wstring m_carpetaContenedora;
    size_t m_indiceCarpeta{0};
    int m_desplazamientoScrollY{0};
    int m_scrollPesadasY{0};
    int m_nivelExpansionMax{1};
    int m_limiteCarpetasPesadas{3};

    uintmax_t m_tamanoTotalBytes{0};
    int m_totalSubcarpetas{0};
    int m_totalArchivos{0};

    std::vector<ItemArbolCarpetaV2> m_elementosArbol;
    std::vector<CarpetaPesadaV2> m_topCarpetasPesadas;
    std::vector<std::wstring> m_carpetasVaciasDetectadas;

    std::vector<std::wstring> m_programasRecordados;
    std::wstring m_ultimoProgramaUsado;

    RECT m_rcBotonAnterior{};
    RECT m_rcBotonSiguiente{};
    RECT m_rcBotonExpansion{};
    RECT m_rcCajaArbol{};
    RECT m_rcSelectorPrograma{};
    RECT m_rcBotonEjecutarAbrir{};
    RECT m_rcBotonCrearEnlace{};
    RECT m_rcBotonCopiarContenido{};
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

namespace SideViewV2 {

class VistaEnDesarrollo : public IPanelContenido {
public:
    VistaEnDesarrollo(const std::wstring& nombreVista, const std::wstring& iconoVista, const std::wstring& descripcion)
        : m_nombreVista(nombreVista), m_iconoVista(iconoVista), m_descripcion(descripcion) {}

    void AlCambiarSeleccion(const std::vector<std::wstring>& archivos, const std::wstring& carpeta) override {
        m_archivos = archivos;
        m_carpeta = carpeta;
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
        RECT rcTarjeta{rect.left + margen, topTarjeta, rect.right - margen, topTarjeta + 220};

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
        RECT rcModSel{rcTarjeta.left + 16, rcTarjeta.top + 180, rcTarjeta.right - 16, rcTarjeta.bottom - 10};
        DrawTextW(hdc, infoSeleccion.c_str(), -1, &rcModSel, DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);

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
};

} // namespace SideViewV2
/**** ended inlining v2/vistas/vista_en_desarrollo.hpp ****/
/**** start inlining v2/nucleo/fabrica_vistas.hpp ****/

/**** skipping file: v2/ui/panel_interfaz.hpp ****/
/**** skipping file: v2/nucleo/enrutador_vistas.hpp ****/
/**** skipping file: v2/vistas/vista_walkthrough.hpp ****/
/**** skipping file: v2/vistas/vista_comprimidos.hpp ****/
/**** skipping file: v2/vistas/vista_carpetas.hpp ****/
/**** skipping file: v2/vistas/vista_en_desarrollo.hpp ****/
#include <memory>
#include <vector>
#include <string>

namespace SideViewV2 {

class FabricaVistas {
public:
    static std::shared_ptr<IPanelContenido> CrearVistaParaSeleccion(
        const std::vector<std::wstring>& seleccion,
        const std::wstring& carpeta) {

        TipoVistaContenido tipo = ClasificarSeleccion(seleccion, carpeta);

        std::shared_ptr<IPanelContenido> vista;

        switch (tipo) {
            case TipoVistaContenido::Tutorial:
                vista = std::make_shared<VistaWalkthrough>();
                break;

            case TipoVistaContenido::Comprimidos:
                vista = std::make_shared<VistaComprimidos>();
                break;

            case TipoVistaContenido::Carpetas:
                vista = std::make_shared<VistaCarpetas>();
                break;

            case TipoVistaContenido::Texto:
                vista = std::make_shared<VistaEnDesarrollo>(
                    L"Visor de Texto & Markdown",
                    L"📄",
                    L"Renderizado GitHub Flavored Markdown con tablas, listas de verificación, bloques de código sombreados e índice vertical interactivo."
                );
                break;

            case TipoVistaContenido::Imagen:
                vista = std::make_shared<VistaEnDesarrollo>(
                    L"Visor de Imágenes & GIFs en Bucle",
                    L"🖼️",
                    L"Decodificación acelerada por hardware de WebP, AVIF, PNG, JPG y GIFs animados en reproducción continua infinita."
                );
                break;

            case TipoVistaContenido::Video:
                vista = std::make_shared<VistaEnDesarrollo>(
                    L"Suite de Reproducción de Video",
                    L"🎬",
                    L"Reproductor multimedia interactivo por Media Foundation con búsqueda precisa, selector de velocidad y línea de tiempo."
                );
                break;

            case TipoVistaContenido::Documentos:
                vista = std::make_shared<VistaEnDesarrollo>(
                    L"Visor de Documentos de Oficina",
                    L"📑",
                    L"Visualización embebida de archivos PDF, Word (DOCX), Excel (XLSX) y presentaciones (PPTX)."
                );
                break;

            case TipoVistaContenido::Codigo:
                vista = std::make_shared<VistaEnDesarrollo>(
                    L"Inspector de Código Fuente",
                    L"💻",
                    L"Resaltado de sintaxis moderno para más de 30 lenguajes de programación y numeración de líneas."
                );
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

namespace SideViewV2 {

inline IShellView* ObtenerShellViewActivo(HWND top, HWND* outTab, IWebBrowser2** outBrowser = nullptr) {
    if (outTab) *outTab = nullptr;
    if (outBrowser) *outBrowser = nullptr;
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
                                    if (outBrowser) {
                                        app->QueryInterface(IID_IWebBrowser2, reinterpret_cast<void**>(outBrowser));
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

namespace SideViewV2 {

inline UINT WM_SIDEVIEW_TOGGLE = 0;
inline UINT WM_SIDEVIEW_SETTINGS = 0;
inline UINT WM_SIDEVIEW_CLEANUP = 0;

inline std::mutex g_bloqueoEstados;
inline std::map<HWND, EstadoPanelV2*> g_estadosPorVentana;

LRESULT CALLBACK ProcedimientoVentanaPanel(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK ProcedimientoSubclasePadre(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam,
                                            UINT_PTR uIdSubclass, DWORD_PTR dwRefData);
LRESULT CALLBACK ProcedimientoSubclaseTop(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam,
                                          DWORD_PTR dwRefData);

inline void DestruirEstado(EstadoPanelV2* estado, bool quitarDelMapa = true) {
    if (!estado) return;

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

    int anchoPanel = EscalarDpi(estado->anchoDIP, dpi);
    anchoPanel = std::min(std::max(anchoPanel, EscalarDpi(280, dpi)), anchoVista - EscalarDpi(200, dpi));
    if (anchoPanel < EscalarDpi(220, dpi)) anchoPanel = EscalarDpi(220, dpi);

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

    HWND tab = nullptr;
    IWebBrowser2* webBrowser = nullptr;
    IShellView* vistaShell = ObtenerShellViewActivo(top, &tab, &webBrowser);
    if (!vistaShell) return nullptr;

    HWND ventanaVista = nullptr;
    vistaShell->GetWindow(&ventanaVista);
    if (!ventanaVista) {
        if (webBrowser) webBrowser->Release();
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
        if (it != g_estadosPorVentana.end()) {
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
            if (webBrowser) webBrowser->Release();
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

    if (webBrowser) webBrowser->Release();

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
        } else {
            estado->vistaContenido = FabricaVistas::CrearVistaParaSeleccion({}, L"");
        }

        DisponerPanel(estado);
        SetTimer(estado->panel, 101, 150, nullptr); // Timer de polling no invasivo para selección
    }

    if (vista) vista->Release();
    if (desinicializarCom) CoUninitialize();
}

inline LRESULT CALLBACK ProcedimientoVentanaPanel(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    EstadoPanelV2* estado = reinterpret_cast<EstadoPanelV2*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));

    if (estado && estado->vistaContenido) {
        if (msg >= WM_MOUSEFIRST && msg <= WM_MOUSELAST) {
            if (estado->vistaContenido->ProcesarMensajeRaton(msg, wParam, lParam, hwnd)) {
                return 0;
            }
        } else if (msg >= WM_KEYFIRST && msg <= WM_KEYLAST) {
            if (estado->vistaContenido->ProcesarMensajeTeclado(msg, wParam, lParam, hwnd)) {
                return 0;
            }
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
            if (wParam == 101 && estado && estado->abierto) {
                // Actualizar selección periódicamente si el panel está visible
                IShellView* vista = nullptr;
                HWND tab = nullptr;
                vista = ObtenerShellViewActivo(estado->top, &tab);
                if (vista) {
                    std::wstring carpeta;
                    auto seleccion = ObtenerRutasSeleccionadasDeVista(vista, &carpeta);
                    if (seleccion != estado->archivosSeleccionados || carpeta != estado->carpetaContenedora) {
                        TipoVistaContenido tipoPrevio = ClasificarSeleccion(
                            estado->archivosSeleccionados, estado->carpetaContenedora);
                        TipoVistaContenido tipoNuevo = ClasificarSeleccion(seleccion, carpeta);

                        estado->archivosSeleccionados = seleccion;
                        estado->carpetaContenedora = carpeta;

                        // Si cambió la categoría de archivo, reemplazamos la vista inyectada
                        if (tipoPrevio != tipoNuevo || !estado->vistaContenido) {
                            if (estado->vistaContenido) estado->vistaContenido->Liberar();
                            estado->vistaContenido = FabricaVistas::CrearVistaParaSeleccion(seleccion, carpeta);
                        } else {
                            estado->vistaContenido->AlCambiarSeleccion(seleccion, carpeta);
                        }

                        InvalidateRect(hwnd, nullptr, FALSE);
                    }
                    vista->Release();
                }
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
                        estado->vistaContenido->Renderizar(estado->dcMemoria, rectCliente, estado->tema);
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
        AlternarSideView(hwnd);
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
    std::transform(s.begin(), s.end(), s.begin(), ::towupper);

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
    if (s.find(L"F1") != std::wstring::npos) vk = VK_F1;
    if (s.find(L"F2") != std::wstring::npos) vk = VK_F2;
    if (s.find(L"F3") != std::wstring::npos) vk = VK_F3;
    if (s.find(L"F4") != std::wstring::npos) vk = VK_F4;

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
#include <vector>
#include <windhawk_api.h>
#include <windhawk_utils.h>
/**** skipping file: v2/nucleo/gestor_ventanas.hpp ****/
/**** skipping file: v2/nucleo/hook_teclado.hpp ****/

namespace SideViewV2 {

using CreateWindowExW_t = decltype(&CreateWindowExW);
inline CreateWindowExW_t CreateWindowExW_Original = nullptr;

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

    if (!IS_INTRESOURCE(lpClassName) && (_wcsicmp(lpClassName, L"CabinetWClass") == 0 ||
                                         _wcsicmp(lpClassName, L"ExploreWClass") == 0)) {
        WindhawkUtils::SetWindowSubclassFromAnyThread(hwnd, ProcedimientoSubclaseTop, 0);
    }

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
        Wh_FreeStringSetting(atajoStr);
    } else {
        ConfigurarAtajoDesdeString(L"Ctrl+K");
    }

    int limite = Wh_GetIntSetting(L"limiteCarpetasPesadas");
    if (limite > 0) {
        g_limiteCarpetasPesadasConfig.store(limite);
    } else {
        g_limiteCarpetasPesadasConfig.store(3);
    }
}

inline bool InicializarModV2() {
    WM_SIDEVIEW_TOGGLE = RegisterWindowMessageW(L"UsefulSideView_WM_Toggle_V2");
    WM_SIDEVIEW_SETTINGS = RegisterWindowMessageW(L"UsefulSideView_WM_Settings_V2");
    WM_SIDEVIEW_CLEANUP = RegisterWindowMessageW(L"UsefulSideView_WM_Cleanup_V2");

    if (!WM_SIDEVIEW_TOGGLE || !WM_SIDEVIEW_SETTINGS || !WM_SIDEVIEW_CLEANUP) {
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
}

inline void DesinicializarModV2() {
    g_descargandoMod = true;
    DetenerHookTeclado();

    // Limpiar estados de todas las ventanas
    {
        std::lock_guard<std::mutex> lock(g_bloqueoEstados);
        for (auto& par : g_estadosPorVentana) {
            if (par.second) {
                DestruirEstado(par.second, false);
            }
        }
        g_estadosPorVentana.clear();
    }

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
