#pragma once

#include <windows.h>
#include <string>
#include <vector>
#include "v2/ui/tema_visual.hpp"

namespace SideViewV2 {

inline constexpr UINT WM_SIDEVIEW_CLOSE_COMPARADOR_ARCHIVOS = WM_APP + 0x4A3;
inline constexpr UINT WM_SIDEVIEW_VIDEO_EVENT_V2 = WM_APP + 0x4A4;
inline constexpr UINT WM_SIDEVIEW_COMPARADOR_TICK = WM_APP + 0x4A5;
inline constexpr UINT WM_SIDEVIEW_COMPARADOR_PIP_CLICK = WM_APP + 0x4A6;
inline constexpr UINT WM_SIDEVIEW_REQUEST_COMPARADOR_ARCHIVOS = WM_APP + 0x4A7;
inline constexpr UINT WM_SIDEVIEW_TOGGLE_PANEL_FULLSCREEN = WM_APP + 0x4A8;

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
