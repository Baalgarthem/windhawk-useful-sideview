#pragma once

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
