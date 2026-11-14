#pragma once

#include <windows.h>
#include <commctrl.h>
#include <dwmapi.h>
#include <algorithm>
#include <memory>
#include <string>
#include <vector>
#include "v2/ui/tema_visual.hpp"
#include "v2/ui/panel_interfaz.hpp"

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
