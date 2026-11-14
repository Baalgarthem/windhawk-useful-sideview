#pragma once

#include <windows.h>
#include <string>
#include <vector>
#include "v2/ui/tema_visual.hpp"

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
